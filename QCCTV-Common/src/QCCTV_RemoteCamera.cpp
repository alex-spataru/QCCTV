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
    m_requestPackets = 0;
    m_name = "Unknown Camera";
    m_lightStatus = QCCTV_FLASHLIGHT_OFF;
    m_cameraStatus = QCCTV_CAMSTATUS_DEFAULT;

    /* Connect sockets signals/slots */
    connect (&m_receiver, SIGNAL (readyRead()), this, SLOT (readData()));

    /* Start loops */
    sendData();
    sendRequest();
}

/**
 * Close the sockets when the class is destroyed
 */
QCCTV_RemoteCamera::~QCCTV_RemoteCamera()
{
    m_sender.close();
    m_receiver.close();
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
 * The focus byte will be reset after sending 4 packets to the camera
 */
void QCCTV_RemoteCamera::requestFocus()
{
    m_focus = true;
    QTimer::singleShot (2000, Qt::PreciseTimer,
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
 * Sends 10 connection requests to the camera
 */
void QCCTV_RemoteCamera::attemptConnection (const QHostAddress& address)
{
    /* Set camera address */
    m_address = address;

    /* Send 10 request packets */
    m_requestPackets = 10;

    /* Bind the receiver socket */
    m_receiver.close();
    m_receiver.bind (m_address,
                     QCCTV_STREAM_PORT,
                     QUdpSocket::ShareAddress |
                     QUdpSocket::ReuseAddressHint);
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
void QCCTV_RemoteCamera::sendData()
{
    /* Generate the data */
    QByteArray data;
    data.append (fps());
    data.append (lightStatus());
    data.append (m_focus ? QCCTV_FORCE_FOCUS : 0x00);

    /* Send the data (if possible) */
    if (!address().isNull() && cameraStatus() & QCCTV_CAMSTATUS_CONNECTED)
        m_sender.writeDatagram (data, address(), QCCTV_COMMAND_PORT);

    /* Schedule the next packet generation process */
    QTimer::singleShot (500, Qt::PreciseTimer, this, SLOT (sendData()));
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
 * - Raw image data
 */
void QCCTV_RemoteCamera::readData()
{
    while (m_receiver.hasPendingDatagrams()) {
        /* Read incoming data */
        QByteArray data;
        data.resize (m_receiver.pendingDatagramSize());
        int bytes = m_receiver.readDatagram (data.data(),
                                             data.size(),
                                             NULL, NULL);

        /* Packet is invalid */
        if (bytes <= 0)
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
        int offset = name_len + group_len + 2;
        int fps = data.at (offset + 1);
        int light = data.at (offset + 2);
        int status = data.at (offset + 3);

        /* Get raw image */
        QByteArray imageData;
        for (int i = offset + 4; i < data.size(); ++i)
            imageData.append (data.at (i));

        /* Convert raw image to QImage */
        m_image = QImage::fromData (imageData, QCCTV_IMAGE_FORMAT);
        emit newImage (id());

        /* Update values */
        setFPS (fps);
        setName (name);
        setGroup (group);
        changeCameraStatus (status);
        changeFlashlightStatus (light);

        /* This is the first packet, emit connected() signal */
        if (!m_connected) {
            m_connected = true;
            emit connected (id());
        }
    }
}

/**
 * Sends a request packet if required. Request packets only contain the group
 * assigned/used by the QCCTV Station.
 *
 * This function is called periodically, since we need to send several packets
 * to ensure (or increase the chances) of the camera to receive our request.
 */
void QCCTV_RemoteCamera::sendRequest()
{
    /* Send a request packet if required */
    if (m_requestPackets > 0) {
        m_requestPackets -= 1;
        m_sender.writeDatagram (group().toUtf8(), m_address,
                                QCCTV_REQUEST_PORT);
    }

    /* Call this function again in 500 msecs */
    QTimer::singleShot (500, Qt::PreciseTimer, this, SLOT (sendRequest()));
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
    /* We just have lost connection with the camera */
    if (m_cameraStatus & QCCTV_CAMSTATUS_CONNECTED) {
        changeCameraStatus (QCCTV_CAMSTATUS_DEFAULT);
        changeFlashlightStatus (QCCTV_FLASHLIGHT_OFF);
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

        if (m_cameraStatus & QCCTV_CAMSTATUS_CONNECTED)
            emit connected (id());
        else
            emit disconnected (id());

        emit newCameraStatus (id());
    }
}
