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
    m_fps = 24;
    m_focus = false;
    m_connected = false;
    m_group = "default";
    m_name = "Unknown Camera";
    m_lightStatus = QCCTV_FLASHLIGHT_OFF;
    m_cameraStatus = QCCTV_CAMSTATUS_DEFAULT;

    /* Configure the watchdog */
    m_watchdog.setExpirationTime (2000);
    connect (&m_watchdog, SIGNAL (expired()), this, SLOT (onCameraTimeout()));

    /* Configure the listener socket */
    connect (&m_server, SIGNAL (newConnection()),
             this,        SLOT (acceptConnection()));

    /* Start loops */
    sendCommandPacket();
    sendRequestPacket();
}

/**
 * Close the sockets
 */
QCCTV_RemoteCamera::~QCCTV_RemoteCamera()
{
    m_sender.close();
    m_server.close();

    foreach (QTcpSocket* socket, m_sockets) {
        socket->close();
        delete socket;
    }

    m_sockets.clear();
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
 * Returns the group asociated with this camera
 */
QString QCCTV_RemoteCamera::group() const
{
    return m_group;
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

        m_server.close();
        m_server.listen (m_address, QCCTV_STREAM_PORT);
    }
}

/**
 * Obtains the information received by the TCP socket and calls the functions
 * neccessary to interpret the received data
 */
void QCCTV_RemoteCamera::onDataReceived()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*> (sender());

    if (socket)
        readCameraPacket (socket->readAll());
}

/**
 * Called when the TCP connection between the camera and the station is aborted
 */
void QCCTV_RemoteCamera::onDisconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*> (sender());

    if (socket) {
        socket->close();
        m_sockets.removeAt (m_sockets.indexOf (socket));

        emit disconnected (id());
    }
}

/**
 * Called when we have not received a stream packet for some time (this forces
 * us to believe that the camera is no longer active).
 *
 * This function resets the light status and changes the camera's
 * operation status to 'disconnected'.
 */
void QCCTV_RemoteCamera::onCameraTimeout()
{
    if (m_cameraStatus & QCCTV_CAMSTATUS_CONNECTED) {
        changeCameraStatus (QCCTV_CAMSTATUS_DEFAULT);
        changeFlashlightStatus (QCCTV_FLASHLIGHT_OFF);
    }
}

/**
 * Accepts any TCP connection request from a QCCTV camera
 */
void QCCTV_RemoteCamera::acceptConnection()
{
    while (m_server.hasPendingConnections()) {
        QTcpSocket* socket = m_server.nextPendingConnection();

        connect (socket, SIGNAL (readyRead()),
                 this,     SLOT (onDataReceived()));
        connect (socket, SIGNAL (disconnected()),
                 this,     SLOT (onDisconnected()));

        m_sockets.append (socket);
    }
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
 * Sends a packet with the group name to the camera.
 * This packet is used to establish a connection with the camera (which decides
 * if we are worthy or not by looking at our group name)
 */
void QCCTV_RemoteCamera::sendRequestPacket()
{
    /* Generate the data */
    QByteArray data;
    data.append (group().isEmpty() ? "default" : group());

    /* Send the data */
    m_sender.writeDatagram (data, address(), QCCTV_REQUEST_PORT);

    /* Call this function again in 1 second */
    QTimer::singleShot (1000, Qt::PreciseTimer,
                        this, SLOT (sendRequestPacket()));
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
    if (!address().isNull() && cameraStatus() & QCCTV_CAMSTATUS_CONNECTED)
        m_sender.writeDatagram (data, address(), QCCTV_COMMAND_PORT);

    /* Schedule the next packet generation process */
    QTimer::singleShot (500, Qt::PreciseTimer, this, SLOT (sendCommandPacket()));
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
 * Updates the group reported by the camera
 */
void QCCTV_RemoteCamera::setGroup (const QString& group)
{
    if (m_group != group) {
        m_group = group;

        if (m_group.isEmpty())
            m_group = "default";

        emit newCameraGroup (id());
    }
}

/**
 * Changes the operation status of the camera and emits the appropiate signals
 */
void QCCTV_RemoteCamera::changeCameraStatus (const int status)
{
    if (m_cameraStatus != status) {
        m_cameraStatus = status;

        if (m_cameraStatus & QCCTV_CAMSTATUS_CONNECTED) {
            m_connected = true;
            emit connected (id());
        } else {
            m_connected = false;
            emit disconnected (id());
        }

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
 * - Length of group name (1 byte)
 * - Group name string
 * - Camera FPS (1 byte)
 * - Light status (1 byte)
 * - Operation status (1 byte)
 * - Compressed image data
 */
void QCCTV_RemoteCamera::readCameraPacket (const QByteArray& data)
{
    /* Data is invalid */
    if (data.isEmpty())
        return;

    /* Get camera name */
    QString name;
    int name_len = data.at (0);
    for (int i = 0; i < name_len; ++i)
        name.append (data.at (1 + i));

    /* Get camera group */
    QString group;
    int group_len = data.at (name_len + 1);
    for (int i = 0; i < group_len; ++i)
        group.append (data.at (name_len + 2 + i));

    /* Get camera FPS and status */
    int offset = name_len + group_len + 1;
    int fps = data.at (offset + 1);
    int light = data.at (offset + 2);
    int status = data.at (offset + 3);

    /* Get raw image */
    QByteArray buffer;
    for (int i = offset + 4; i < data.size(); ++i)
        buffer.append (data.at (i));

    /* Convert raw image to QImage */
    buffer = qUncompress (buffer);
    if (buffer.size() > 1) {
        m_image = QImage::fromData (buffer, QCCTV_IMAGE_FORMAT);
        emit newImage (id());
    }

    /* Update values */
    setFPS (fps);
    setName (name);
    setGroup (group);
    changeCameraStatus (status);
    changeFlashlightStatus (light);

    /* This is the first packet, emit connected() signal */
    if (!m_connected) {
        m_connected = true;

        emit newImage (id());
        emit connected (id());
        emit newCameraName (id());
        emit newCameraGroup (id());
        emit newLightStatus (id());
        emit newCameraStatus (id());
    }

    /* Feed the watchdog */
    m_watchdog.reset();
}
