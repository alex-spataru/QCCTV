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
    m_socket.setSocketOption (QTcpSocket::LowDelayOption, true);
    connect (&m_socket,   SIGNAL (readyRead()), this, SLOT (onDataReceived()));
    connect (&m_watchdog, SIGNAL (expired()),   this, SLOT (onDisconnected()));

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

    /* Packet is incomplete, just feed the watchdog */
    if (!data.endsWith (QString (QCCTV_EOD).toUtf8())) {
        m_watchdog.reset();
        return;
    }

    /* Get camera name */
    QString name;
    int name_len = data.at (0);
    for (int i = 0; i < name_len; ++i)
        name.append (data.at (1 + i));

    /* Get camera FPS and status */
    int fps = data.at (name_len + 1);
    int light = data.at (name_len + 2);
    int status = data.at (name_len + 3);

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

    /* Get image bytes */
    QByteArray buf;
    for (int i = 0; i < data.size() - QString (QCCTV_EOD).size(); ++i)
        buf.append (data.at (name_len + 4 + i));

    /* Obtain image from data */
    QImage img = QImage::fromData (buf);
    if (!img.isNull()) {
        m_image = img;
        emit newImage (id());
    }

    /* Feed the watchdog */
    m_watchdog.reset();
}
