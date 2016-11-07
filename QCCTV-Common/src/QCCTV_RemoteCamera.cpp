/*
 * Copyright (c) 2016 Alex Spataru
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE
 */

#include "QCCTV.h"
#include "QCCTV_RemoteCamera.h"

#include <QApplication>

QCCTV_RemoteCamera::QCCTV_RemoteCamera()
{
    /* Initialize default variables */
    m_id = 0;
    m_focus = false;
    m_connected = false;
    m_name = "Unknown Camera";
    m_fps = QCCTV_DEFAULT_FPS;
    m_lightStatus = QCCTV_FLASHLIGHT_OFF;
    m_cameraStatus = QCCTV_CAMSTATUS_DEFAULT;

    /* Configure the socket */
    m_watchdog.setExpirationTime (1000);
    connect (&m_socket,   SIGNAL (readyRead()), this, SLOT (onDataReceived()));
    connect (&m_watchdog, SIGNAL (expired()),   this, SLOT (onDisconnected()));

    /* Set default image */
    m_image = QCCTV_GET_STATUS_IMAGE (QSize (640, 480), "NO CAMERA IMAGE");

    /* Begin sending command packets */
    sendCommandPacket();
}

/**
 * Close the TCP sockets during the destruction of the class
 */
QCCTV_RemoteCamera::~QCCTV_RemoteCamera()
{
    m_socket.close();
}

/**
 * Returns the camera ID set by the station
 */
int QCCTV_RemoteCamera::id() const
{
    return m_id;
}

/**
 * Returns the FPS set by the station or by the camera itself
 */
int QCCTV_RemoteCamera::fps() const
{
    return m_fps;
}

/**
 * Returns the operation status reported by the camera
 */
int QCCTV_RemoteCamera::cameraStatus() const
{
    return m_cameraStatus;
}

/**
 * Returns \c true if the station has an active connection
 * with a QCCTV camera
 */
bool QCCTV_RemoteCamera::isConnected() const
{
    return m_connected;
}

/**
 * Returns the name of the camera
 */
QString QCCTV_RemoteCamera::cameraName() const
{
    return m_name;
}

/**
 * Returns the latest image captured by the camera
 */
QImage QCCTV_RemoteCamera::currentImage() const
{
    return m_image;
}

/**
 * Returns the camera status flags as a string
 */
QString QCCTV_RemoteCamera::statusString() const
{
    return QCCTV_STATUS_STRING (m_cameraStatus);
}

/**
 * Returns the network address of the camera
 */
QHostAddress QCCTV_RemoteCamera::address() const
{
    return m_address;
}

/**
 * Returns the light status used by the camera
 */
QCCTV_LightStatus QCCTV_RemoteCamera::lightStatus() const
{
    return m_lightStatus;
}

/**
 * Instructs the class to generate a packet that requests the camera to perform
 * a forced focus.
 *
 * The focus byte will be reset after sending some packets to the camera
 */
void QCCTV_RemoteCamera::requestFocus()
{
    m_focus = true;
    QTimer::singleShot (500, Qt::PreciseTimer,
                        this, SLOT (resetFocusRequest()));
}

/**
 * Instructs the camera to turn on its flashlight (on the next command packet)
 */
void QCCTV_RemoteCamera::turnOnFlashlight()
{
    if (m_lightStatus != QCCTV_FLASHLIGHT_ON) {
        m_lightStatus = QCCTV_FLASHLIGHT_ON;
        emit newLightStatus (id());
    }
}

/**
 * Instructs the camera to turn off its flashlight (on the next command packet)
 */
void QCCTV_RemoteCamera::turnOffFlashlight()
{
    if (m_lightStatus != QCCTV_FLASHLIGHT_OFF) {
        m_lightStatus = QCCTV_FLASHLIGHT_OFF;
        emit newLightStatus (id());
    }
}

/**
 * Changes the ID of the camera
 */
void QCCTV_RemoteCamera::setID (const int id)
{
    m_id = id;
}

/**
 * Updates the FPS value reported (or assigned) by the camera
 */
void QCCTV_RemoteCamera::setFPS (const int fps)
{
    m_fps = QCCTV_GET_VALID_FPS (fps);
    m_watchdog.setExpirationTime (qMax (1000, qMin (m_fps * 50, 5000)));

    emit fpsChanged (id());
}

/**
 * Changes the flashlight status of the camera and emits the appropiate signals
 */
void QCCTV_RemoteCamera::setFlashlightStatus (const int status)
{
    if (m_lightStatus != status) {
        m_lightStatus = (QCCTV_LightStatus) status;
        emit newLightStatus (id());
    }
}

/**
 * Updates the connection status of the camera and emits the appropiate signals
 */
void QCCTV_RemoteCamera::setConnected (const bool status)
{
    m_connected = status;

    if (m_connected)
        emit connected (id());
    else
        emit disconnected (id());
}

/**
 * Changes the camera's remote address
 */
void QCCTV_RemoteCamera::setAddress (const QHostAddress& address)
{
    if (m_address == address)
        return;

    m_address = address;
    m_socket.disconnectFromHost();
    m_socket.connectToHost (address, QCCTV_STREAM_PORT);
    m_socket.setSocketOption (QTcpSocket::LowDelayOption, 1);
    m_socket.setSocketOption (QTcpSocket::KeepAliveOption, 1);
    m_socket.setSocketOption (QTcpSocket::ReceiveBufferSizeSocketOption, INT_MAX);
}

/**
 * Obtains the information received by the TCP socket and calls the functions
 * neccessary to interpret the received data
 */
void QCCTV_RemoteCamera::onDataReceived()
{
    m_data.append (m_socket.readAll());

    if (!m_data.isEmpty())
        readCameraPacket();
}

/**
 * Called when the TCP connection between the camera and the station is aborted
 */
void QCCTV_RemoteCamera::onDisconnected()
{
    setConnected (false);
    setAddress (address());
    m_watchdog.setExpirationTime (1000);
    setFlashlightStatus (QCCTV_FLASHLIGHT_OFF);
}

/**
 * Disables the focus flag. This function is called after 3 command packets
 * instructing the camera to re-focus itself have been sent.
 */
void QCCTV_RemoteCamera::resetFocusRequest()
{
    m_focus = false;
}

/**
 * Sends a command packet to the camera, which instructs it to:
 *
 * - Change its FPS
 * - Change its light status
 * - Focus the camera device (if required)
 *
 * These packets are sent every 500 milliseconds
 */
void QCCTV_RemoteCamera::sendCommandPacket()
{
    /* Generate the data */
    QByteArray data;
    data.append (fps());
    data.append (lightStatus());
    data.append (m_focus ? QCCTV_FORCE_FOCUS : 0x00);

    /* Send the generated data */
    if (m_socket.isWritable() && isConnected())
        m_socket.write (data);

    /* Call this function again */
    QTimer::singleShot (500, Qt::PreciseTimer,
                        this, SLOT (sendCommandPacket()));
}

/**
 * Updates the \a name reported by the camera
 */
void QCCTV_RemoteCamera::setName (const QString& name)
{
    if (m_name != name) {
        m_name = name;

        if (m_name.isEmpty())
            m_name = "Unknown Camera";

        emit newCameraName (id());
    }
}

/**
 * Changes the operation status of the camera and emits the appropiate signals
 */
void QCCTV_RemoteCamera::setCameraStatus (const int status)
{
    if (m_cameraStatus != status) {
        m_cameraStatus = status;
        emit newCameraStatus (id());
    }
}

/**
 * Called when we receive a datagram from the camera, this function interprets
 * the status bytes and the image data contained in the received packet.
 *
 * The stream packets (the ones that we receive from the camera) have the
 * following structure:
 *
 * - CRC32 (4 bytes)
 * - Length of camera name (1 byte)
 * - Camera name string
 * - Camera FPS (1 byte)
 * - Light status (1 byte)
 * - Operation status (1 byte)
 * - Image length (3 bytes)
 * - Compressed image data
 */
void QCCTV_RemoteCamera::readCameraPacket()
{
    /* Data is invalid */
    if (m_data.isEmpty())
        return;

    /* Data is too small to read checksum */
    if (m_data.size() < 4) {
        m_watchdog.reset();
        return;
    }

    /* Get the checksum */
    quint8 a = m_data.at (0);
    quint8 b = m_data.at (1);
    quint8 c = m_data.at (2);
    quint8 d = m_data.at (3);
    quint32 checksum = (a << 24) | (b << 16) | (c << 8) | (d & 0xff);

    /* Create byte array without checksum header */
    QByteArray stream = m_data;
    stream.remove (0, 4);

    /* Stream is too small */
    if (stream.isEmpty()) {
        m_watchdog.reset();
        return;
    }

    /* Compare checksums */
    quint32 crc = m_crc32.compute (stream);
    if (checksum != crc) {
        m_watchdog.reset();
        return;
    }

    /* Uncompress the stream data */
    stream = qUncompress (stream);

    /* This is the first packet, emit connected() signal */
    if (!isConnected())
        setConnected (true);

    /* Get camera name */
    QString name;
    int name_len = stream.at (0);
    for (int i = 0; i < name_len; ++i) {
        int pos = 1 + i;
        if (pos < stream.size())
            name.append (stream.at (1 + i));

        else {
            m_watchdog.reset();
            return;
        }
    }

    /* Stream is to small to get camera status */
    if (stream.size() < name_len + 4) {
        m_watchdog.reset();
        return;
    }

    /* Get camera FPS and status */
    int fps = stream.at (name_len + 1);
    int light = stream.at (name_len + 2);
    int status = stream.at (name_len + 3);

    /* Update values */
    setFPS (fps);
    setName (name);
    setCameraStatus (status);
    setFlashlightStatus (light);

    /* Stream is too small to get image length */
    if (stream.size() < name_len + 7) {
        m_watchdog.reset();
        return;
    }

    /* Get image length */
    quint8 img_a = stream.at (name_len + 4);
    quint8 img_b = stream.at (name_len + 5);
    quint8 img_c = stream.at (name_len + 6);
    quint32 img_len = (img_a << 16) | (img_b << 8) | (img_c & 0xff);

    /* Get image bytes */
    QByteArray raw_image;
    for (quint16 i = 0; i < img_len; ++i) {
        int pos = name_len + 7 + i;
        if (pos < stream.size())
            raw_image.append (stream.at (pos));

        else {
            m_watchdog.reset();
            return;
        }
    }

    /* Decode image */
    QImage img = QCCTV_DECODE_IMAGE (raw_image);
    if (!img.isNull()) {
        m_image = img;
        m_data.clear();
        m_watchdog.reset();

        emit newImage (id());
    }

    /* Data is too large (> 200 KB) */
    if (m_data.size() >= 200 * 1024 * 1024)
        m_data.clear();
}
