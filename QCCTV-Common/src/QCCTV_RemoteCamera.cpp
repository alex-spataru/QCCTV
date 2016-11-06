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

#include <QPixmap>
#include <QPainter>

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
    m_watchdog.setExpirationTime (2000);
    m_socket.setSocketOption (QTcpSocket::LowDelayOption, 1);
    connect (&m_socket,   SIGNAL (readyRead()), this, SLOT (onDataReceived()));
    connect (&m_watchdog, SIGNAL (expired()),   this, SLOT (onDisconnected()));

    /* Set default image */
    QPixmap pixmap = QPixmap (320, 240);
    pixmap.fill (QColor ("#000").rgb());
    QPainter painter (&pixmap);

    /* Set default image text */
    painter.setPen (Qt::white);
    painter.setFont (QFont ("Arial"));
    painter.drawText (QRectF (0, 0, 320, 240),
                      Qt::AlignCenter, "NO CAMERA IMAGE");

    /* Get generated image buffer */
    m_image = pixmap.toImage();

    /* Start loops */
    sendCommandPacket();
}

/**
 * Close the sockets
 */
QCCTV_RemoteCamera::~QCCTV_RemoteCamera()
{
    m_socket.close();
    m_socket.deleteLater();
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
    emit fpsChanged (id());
}

/**
 * Changes the flashlight status of the camera and emits the appropiate signals
 */
void QCCTV_RemoteCamera::changeFlashlightStatus (const int status)
{
    if (m_lightStatus != status) {
        m_lightStatus = (QCCTV_LightStatus) status;
        emit newLightStatus (id());
    }
}

/**
 * Changes the camera's remote address
 */
void QCCTV_RemoteCamera::setAddress (const QHostAddress& address)
{
    if (m_address != address) {
        m_address = address;
        m_socket.abort();
        m_socket.connectToHost (address, QCCTV_STREAM_PORT);
    }
}

/**
 * Obtains the information received by the TCP socket and calls the functions
 * neccessary to interpret the received data
 */
void QCCTV_RemoteCamera::onDataReceived()
{
    readCameraPacket (m_socket.read (m_socket.bytesAvailable()));
}

/**
 * Called when the TCP connection between the camera and the station is aborted
 */
void QCCTV_RemoteCamera::onDisconnected()
{
    m_socket.abort();
    m_socket.connectToHost (address(), QCCTV_STREAM_PORT);
    changeFlashlightStatus (QCCTV_FLASHLIGHT_OFF);

    emit disconnected (id());
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
    if (m_socket.isWritable())
        m_socket.write (data);
}

/**
 * Updates the name reported by the camera
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
void QCCTV_RemoteCamera::changeCameraStatus (const int status)
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
 * - Image data
 */
void QCCTV_RemoteCamera::readCameraPacket (const QByteArray& data)
{
    /* Data is invalid */
    if (data.isEmpty())
        return;

    /* Get the checksum */
    quint8 a = data.at (0);
    quint8 b = data.at (1);
    quint8 c = data.at (2);
    quint8 d = data.at (3);
    quint32 checksum = (a << 24) | (b << 16) | (c << 8) | (d & 0xff);

    /* Create byte array without checksum header */
    QByteArray original = data;
    original.remove (0, 4);

    /* Compare checksums */
    quint32 crc = m_crc32.compute (original);
    if (checksum != crc) {
        m_watchdog.reset();
        return;
    }

    /* Get camera name */
    QString name;
    int name_len = original.at (0);
    for (int i = 0; i < name_len; ++i) {
        int pos = 1 + i;
        if (pos > original.size())
            name.append (original.at (1 + i));
    }

    /* Get camera FPS and status */
    int fps = original.at (name_len + 1);
    int light = original.at (name_len + 2);
    int status = original.at (name_len + 3);

    /* Update values */
    setFPS (fps);
    setName (name);
    changeCameraStatus (status);
    changeFlashlightStatus (light);

    /* This is the first packet, emit connected() signal */
    if (!m_connected) {
        m_connected = true;
        emit connected (id());
        emit newCameraName (id());
        emit newLightStatus (id());
        emit newCameraStatus (id());
    }

    /* Get image length */
    quint8 img_a = original.at (name_len + 4);
    quint8 img_b = original.at (name_len + 5);
    quint8 img_c = original.at (name_len + 6);
    quint32 img_len = (img_a << 16) | (img_b << 8) | (img_c & 0xff);

    /* Get image bytes */
    QByteArray raw_image;
    for (quint16 i = 0; i < img_len; ++i) {
        int pos = name_len + 7 + i;
        if (pos < original.size())
            raw_image.append (original.at (pos));
    }

    /* Decode image */
    QImage img = QCCTV_DECODE_IMAGE (raw_image);
    if (!img.isNull()) {
        m_image = img;
        emit newImage (id());
    }

    /* Feed the watchdog */
    m_watchdog.reset();
}
