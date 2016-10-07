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
#include "QCCTV_LocalCamera.h"

/**
 * Initializes the class by binding the sockets and connecting the
 * signals/slots of the different sockets used by the local camera.
 *
 * Additionaly, the class will try to gain access to the frontal camera
 * during initalization...
 */
QCCTV_LocalCamera::QCCTV_LocalCamera() {
    /* Set default values */
    m_fps = 24;
    m_cameraName = "Cam0";
    m_cameraGroup = "Default";

    /* Start the event loops */
    QTimer::singleShot (1000, Qt::CoarseTimer, this, SLOT (update()));
    QTimer::singleShot (1000, Qt::CoarseTimer, this, SLOT (broadcastInformation()));

    /* Bind sockets */
    m_commandSocket.bind (QHostAddress::Any,
                          QCCTV_COMMAND_PORT,
                          QUdpSocket::ShareAddress |
                          QUdpSocket::ReuseAddressHint);

    /* Connect sockets signals/slots */
    connect (&m_commandSocket, SIGNAL (readyRead()),
             this,               SLOT (readCommandPacket()));
}

/**
 * Closes all the sockets and the frontal camera device
 */
QCCTV_LocalCamera::~QCCTV_LocalCamera() {
    m_senderSocket.close();
    m_commandSocket.close();
    m_requestSocket.close();
    m_broadcastSocket.close();
}

/**
 * Returns the current FPS of the camera
 */
int QCCTV_LocalCamera::fps() const {
    return m_fps;
}

/**
 * Returns the user-assigned name of the camera
 */
QString QCCTV_LocalCamera::cameraName() const {
    return m_cameraName;
}

/**
 * Returns the camera group associated with this camera
 */
QString QCCTV_LocalCamera::cameraGroup() const {
    return m_cameraGroup;
}

/**
 * Returns the current image recorded by the camera
 */
QImage QCCTV_LocalCamera::currentImage() const {
    return m_currentImage;
}

/**
 * Returns a list with all the connected QCCTV stations to this camera
 */
QStringList QCCTV_LocalCamera::connectedHosts() const {
    QStringList list;

    foreach (QHostAddress address, m_allowedHosts)
        list.append (address.toString());

    return list;
}

/**
 * Returns the current assigned status for the camera light
 */
QCCTV_LightStatus QCCTV_LocalCamera::lightStatus() const {
    return m_lightStatus;
}

/**
 * Returns the current status of the camera itself
 */
QCCTV_CameraStatus QCCTV_LocalCamera::cameraStatus() const {
    return m_cameraStatus;
}

/**
 * Forces the camera to re-focus the image
 */
void QCCTV_LocalCamera::focusCamera() {
    emit focusStatusChanged();
}

/**
 * Changes the FPS of the camera
 */
void QCCTV_LocalCamera::setFPS (const int fps) {
    m_fps = fps;
    emit fpsChanged();
}

/**
 * Changes the name assigned to this camera
 */
void QCCTV_LocalCamera::setName (const QString& name) {
    m_cameraName = name;
    emit cameraNameChanged();
}

/**
 * Change the group assigned to this camera
 */
void QCCTV_LocalCamera::setGroup (const QString& group) {
    m_cameraGroup = group;
    emit cameraGroupChanged();
}

/**
 * Changes the light status of the camera
 */
void QCCTV_LocalCamera::setLightStatus (const QCCTV_LightStatus status) {
    m_lightStatus = status;
    emit lightStatusChanged();
}

/**
 * Changes the camera status
 */
void QCCTV_LocalCamera::setCameraStatus (const QCCTV_CameraStatus status) {
    m_cameraStatus = status;
    emit cameraStatusChanged();
}

/**
 * Obtains a new image from the camera, updates the camera status and sends
 * the newly obtained data to the stations
 */
void QCCTV_LocalCamera::update() {
    updateImage();
    updateStatus();
    sendCameraData();

    QTimer::singleShot (1000 / fps(), Qt::PreciseTimer, this, SLOT (update()));
}

/**
 * Obtains a new image from the camera
 */
void QCCTV_LocalCamera::updateImage() {

}

/**
 * Updates the status code of the camera
 */
void QCCTV_LocalCamera::updateStatus() {

}

/**
 * Generates a network packet with the following information
 *
 * - The camera name
 * - The camera group
 * - The FPS of the camera
 * - The light status of the camera
 * - The camera status
 * - The latest camera image
 */
void QCCTV_LocalCamera::sendCameraData() {
    QByteArray data;

    data.append (cameraName().length());
    data.append (cameraName());
    data.append (cameraGroup().length());
    data.append (cameraGroup());
    data.append (fps());
    data.append (lightStatus());
    data.append (cameraStatus());

    foreach (QHostAddress address, m_allowedHosts)
        m_senderSocket.writeDatagram (data, address, QCCTV_STREAM_PORT);
}

/**
 * Interprets a connection request packet and decides whenever to accept it
 * or not
 */
void QCCTV_LocalCamera::readRequestPacket() {

}

/**
 * Interprets a command packet issued by the QCCTV station in the LAN.
 *
 * This packet contains the following data/instructions:
 *
 * - A new FPS to use
 * - The new light status
 * - A force focus request
 */
void QCCTV_LocalCamera::readCommandPacket() {
    while (m_commandSocket.hasPendingDatagrams()) {
        int error;
        QByteArray data;
        QHostAddress ip;

        /* Resize byte array to incomind data */
        data.resize (m_commandSocket.pendingDatagramSize());

        /* Read the datagram */
        error = m_commandSocket.readDatagram (data.data(),
                                              data.size(),
                                              &ip, NULL);

        /* Remote IP is not on allowed hosts list, ignore packet */
        if (!m_allowedHosts.contains (ip))
            return;

        /* Packet length is invalid */
        if (data.size() != 3 || error <= 0)
            return;

        /* Change the FPS */
        setFPS ((int) data.at (0));

        /* Change the light status */
        setLightStatus ((QCCTV_LightStatus) data.at (1));

        /* Focus the camera */
        if (data.at (2) == QCCTV_FORCE_FOCUS)
            focusCamera();
    }
}

/**
 * Creates and sends a new packet that announces the existence of this
 * camera to the rest of the local network
 */
void QCCTV_LocalCamera::broadcastInformation() {
    QString str = "QCCTV_DISCOVERY_SERVICE";
    m_broadcastSocket.writeDatagram (str.toUtf8(),
                                     QHostAddress::Any,
                                     QCCTV_DISCOVERY_PORT);

    QTimer::singleShot (1000, Qt::PreciseTimer,
                        this, SLOT (broadcastInformation()));
}

