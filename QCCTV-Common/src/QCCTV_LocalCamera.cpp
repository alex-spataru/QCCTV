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
#include "QCCTV_FrameGrabber.h"

#include <QFont>
#include <QBuffer>
#include <QPainter>
#include <QCameraInfo>
#include <QCameraExposure>

/**
 * Initializes the class by binding the sockets and connecting the
 * signals/slots of the different sockets used by the local camera.
 *
 * Additionaly, the class will try to gain access to the frontal camera
 * during initalization...
 */
QCCTV_LocalCamera::QCCTV_LocalCamera()
{
    /* Listen for incoming connections */
    m_server.listen (QHostAddress::Any, QCCTV_STREAM_PORT);

    /* Connect sockets signals/slots */
    connect (&m_server, SIGNAL (newConnection()),
             this,        SLOT (acceptConnection()));

    /* Setup the frame grabber */
    connect (&m_frameGrabber, SIGNAL (newFrame (QPixmap)),
             this,              SLOT (changeImage (QPixmap)));

    /* Setup the watchdog */
    m_watchdog.setExpirationTime (QCCTV_COMMAND_PKT_TIMEOUT);
    connect (&m_watchdog, SIGNAL (expired()), this, SLOT (disconnectStation()));

    /* Set default values */
    setFPS (24);
    setGroup ("default");
    setName ("Unknown Camera");
    setCameraStatus (QCCTV_CAMSTATUS_DEFAULT);
    setFlashlightStatus (QCCTV_FLASHLIGHT_OFF);

    /* Set default image */
    m_image = QPixmap (320, 240);
    m_image.fill (QColor ("#00f").rgb());
    QPainter painter (&m_image);

    /* Set default image text */
    painter.setPen (Qt::white);
    painter.setFont (QFont ("Arial"));
    painter.drawText (QRectF (0, 0, 320, 240),
                      Qt::AlignCenter, "NO CAMERA IMAGE");

    /* Start the event loops */
    QTimer::singleShot (1000, Qt::CoarseTimer, this, SLOT (update()));
    QTimer::singleShot (1000, Qt::CoarseTimer, this, SLOT (broadcastInformation()));
}

/**
 * Closes all the sockets and the frontal camera device
 */
QCCTV_LocalCamera::~QCCTV_LocalCamera()
{
    /* Close broadcast socket */
    m_broadcastSocket.close();

    /* Close TCP connections */
    foreach (QTcpSocket* socket, m_sockets)
        if (socket)
            socket->close();
}

/**
 * Returns the current FPS of the camera
 */
int QCCTV_LocalCamera::fps() const
{
    return m_fps;
}

/**
 * Returns the current status of the flashlight (on or off)
 */
QCCTV_LightStatus QCCTV_LocalCamera::lightStatus() const
{
    return (QCCTV_LightStatus) m_flashlightStatus;
}

/**
 * Returns \c true if we should send a grayscale image to the QCCTV Station
 */
bool QCCTV_LocalCamera::isGrayscale() const
{
    return m_frameGrabber.isGrayscale();
}

/**
 * Returns the shrink ratio used to resize the image to send it to the
 * local network
 */
qreal QCCTV_LocalCamera::shrinkRatio() const
{
    return m_frameGrabber.shrinkRatio();
}

/**
 * Returns \c true if the flash light is on
 */
bool QCCTV_LocalCamera::flashlightOn() const
{
    return lightStatus() == QCCTV_FLASHLIGHT_ON;
}

/**
 * Returns \c true if the flash light is off
 */
bool QCCTV_LocalCamera::flashlightOff() const
{
    return lightStatus() == QCCTV_FLASHLIGHT_OFF;
}

/**
 * Returns the user-assigned name of the camera
 */
QString QCCTV_LocalCamera::cameraName() const
{
    return m_name;
}

/**
 * Returns the camera group associated with this camera
 */
QString QCCTV_LocalCamera::cameraGroup() const
{
    return m_group;
}

/**
 * Returns the current image recorded by the camera
 */
QPixmap QCCTV_LocalCamera::currentImage() const
{
    return m_image;
}

/**
 * Returns the current status of QCCTV in a string
 */
QString QCCTV_LocalCamera::statusString() const
{
    return QCCTV_STATUS_STRING (cameraStatus());
}

/**
 * Returns \c true if the camera's flashlight is ready for use
 */
bool QCCTV_LocalCamera::flashlightAvailable() const
{
    if (m_camera)
        return m_camera->exposure()->isFlashReady();

    return false;
}

/**
 * Returns a list with all the connected QCCTV stations to this camera
 */
QStringList QCCTV_LocalCamera::connectedHosts() const
{
    QStringList list;

    foreach (QTcpSocket* socket, m_sockets)
        list.append (socket->peerAddress().toString());

    return list;
}

/**
 * Returns the current status of the camera itself
 */
int QCCTV_LocalCamera::cameraStatus() const
{
    return m_cameraStatus;
}

/**
 * Forces the camera to re-focus the image
 */
void QCCTV_LocalCamera::focusCamera()
{
    if (m_camera) {
        m_camera->searchAndLock (QCamera::LockFocus);
        emit focusStatusChanged();
    }
}

/**
 * Attempts to turn on the camera flashlight/torch
 */
void QCCTV_LocalCamera::turnOnFlashlight()
{
    setFlashlightStatus (QCCTV_FLASHLIGHT_ON);
}

/**
 * Attempts to turn off the camera flashlight/torch
 */
void QCCTV_LocalCamera::turnOffFlashlight()
{
    setFlashlightStatus (QCCTV_FLASHLIGHT_OFF);
}

/**
 * Changes the FPS of the camera
 */
void QCCTV_LocalCamera::setFPS (const int fps)
{
    if (m_fps != QCCTV_GET_VALID_FPS (fps)) {
        m_fps = QCCTV_GET_VALID_FPS (fps);
        emit fpsChanged();
    }
}

/**
 * Changes the camera used to capture images to send to the QCCTV network
 */
void QCCTV_LocalCamera::setCamera (QCamera* camera)
{
    if (camera) {
        m_camera = camera;
        m_frameGrabber.setSource (m_camera);
    }
}

/**
 * Changes the name assigned to this camera
 */
void QCCTV_LocalCamera::setName (const QString& name)
{
    if (m_name != name) {
        m_name = name;
        emit cameraNameChanged();
    }
}

/**
 * Enables or disables sending a grayscale image to the QCCTV Stations
 * in the local network
 */
void QCCTV_LocalCamera::setGrayscale (const bool gray)
{
    m_frameGrabber.setGrayscale (gray);
}

/**
 * Change the group assigned to this camera
 */
void QCCTV_LocalCamera::setGroup (const QString& group)
{
    if (m_group != group) {
        m_group = group;
        emit cameraGroupChanged();
    }
}

/**
 * Changes the shrink factor used to resize the image before sending it to
 * the QCCTV Stations in the local network
 */
void QCCTV_LocalCamera::setShrinkRatio (const qreal ratio)
{
    m_frameGrabber.setShrinkRatio (ratio);
}

/**
 * Obtains a new image from the camera, updates the camera status and sends
 * the newly obtained data to the stations
 */
void QCCTV_LocalCamera::update()
{
    /* Enable frame grabber */
    m_frameGrabber.setEnabled (true);

    /* Update operation status and stream data */
    updateStatus();
    sendCameraData();

    /* Schedule another function call */
    QTimer::singleShot (QCCTV_CSTREAM_PKT_TIMING (m_fps),
                        Qt::PreciseTimer,
                        this, SLOT (update()));
}

/**
 * Updates the status code of the camera
 */
void QCCTV_LocalCamera::updateStatus()
{
    /* No camera present */
    if (!m_camera)
        addStatusFlag (QCCTV_CAMSTATUS_VIDEO_FAILURE);

    /* Check if camera is available */
    else if (m_camera->status() != QCamera::ActiveStatus)
        addStatusFlag (QCCTV_CAMSTATUS_VIDEO_FAILURE);

    /* Video is OK, ensure that VIDEO_FAILURE is removed */
    else
        removeStatusFlag (QCCTV_CAMSTATUS_VIDEO_FAILURE);

    /* Check if flash light is available */
    if (!flashlightAvailable())
        addStatusFlag (QCCTV_CAMSTATUS_LIGHT_FAILURE);
    else
        removeStatusFlag (QCCTV_CAMSTATUS_LIGHT_FAILURE);

    /* Check if we are connected to a CCTV station */
    if (!connectedHosts().isEmpty())
        addStatusFlag (QCCTV_CAMSTATUS_CONNECTED);
    else
        removeStatusFlag (QCCTV_CAMSTATUS_CONNECTED);
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
void QCCTV_LocalCamera::sendCameraData()
{
    QByteArray data;

    /* Add camera name string */
    data.append (cameraName().length());
    data.append (cameraName());

    /* Add camera group */
    data.append (cameraGroup().length());
    data.append (cameraGroup());

    /* Add FPS, light, operation status and color mode */
    data.append (fps());
    data.append (lightStatus());
    data.append (cameraStatus());

    /* Get image buffer */
    QByteArray img;
    QBuffer buffer (&img);
    buffer.open (QIODevice::WriteOnly);
    currentImage().save (&buffer, QCCTV_IMAGE_FORMAT);

    /* Add image buffer to packet */
    if (img.size() > 0 && !currentImage().isNull()) {
        data.append (qCompress (img, 9));
        buffer.close();
    }

    /* Add image error flag (if required) */
    else
        data.append (QCCTV_NO_IMAGE_FLAG);

    /* Send generated data */
    foreach (QTcpSocket* socket, m_sockets)
        if (socket)
            socket->write (data);
}

/**
 * Interprets a connection request packet and decides whenever to accept it
 * or not
 */
void QCCTV_LocalCamera::acceptConnection()
{
    while (m_server.hasPendingConnections()) {
        QTcpSocket* socket = m_server.nextPendingConnection();
        m_sockets.append (socket);

        connect (socket, SIGNAL (readyRead()),
                 this,    SLOT  (readCommandPacket()));
    }
}

/**
 * Changes the camera status to notify the application that we are not connected
 * to any QCCTV Station
 */
void QCCTV_LocalCamera::disconnectStation()
{
    removeStatusFlag (QCCTV_CAMSTATUS_CONNECTED);
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
void QCCTV_LocalCamera::readCommandPacket()
{
    /* Get socket and data */
    QTcpSocket* socket = qobject_cast<QTcpSocket*> (sender());
    QByteArray data = socket->readAll();

    /* Packet length is invalid */
    if (data.size() != 3)
        return;

    /* Change the FPS */
    setFPS ((int) data.at (0));

    /* Change the light status */
    setFlashlightStatus ((QCCTV_LightStatus) data.at (1));

    /* Focus the camera */
    if (data.at (2) == QCCTV_FORCE_FOCUS)
        focusCamera();

    /* Feed watchdog timer */
    m_watchdog.reset();
}

/**
 * Creates and sends a new packet that announces the existence of this
 * camera to the local network
 */
void QCCTV_LocalCamera::broadcastInformation()
{
    QString str = "QCCTV_DISCOVERY_SERVICE";
    m_broadcastSocket.writeDatagram (str.toUtf8(),
                                     QCCTV_DISCOVERY_ADDR,
                                     QCCTV_DISCOVERY_PORT);

    QTimer::singleShot (QCCTV_DISCVRY_PKT_TIMING, Qt::PreciseTimer,
                        this, SLOT (broadcastInformation()));
}

/**
 * Replaces the current image and notifies the application
 */
void QCCTV_LocalCamera::changeImage (const QPixmap& image)
{
    m_image = image;
    m_frameGrabber.setEnabled (false);
}

/**
 * Registers the given \a status flag to the operation status flags
 */
void QCCTV_LocalCamera::addStatusFlag (const QCCTV_CameraStatus status)
{
    if (! (m_cameraStatus & status)) {
        m_cameraStatus |= status;
        emit cameraStatusChanged();
    }
}

/**
 * Overrides the camera status flags with the given \a status
 */
void QCCTV_LocalCamera::setCameraStatus (const QCCTV_CameraStatus status)
{
    m_cameraStatus = status;
    emit cameraStatusChanged();
}

/**
 * Removes the given \a status flag from the operation status of the camera
 */
void QCCTV_LocalCamera::removeStatusFlag (const QCCTV_CameraStatus status)
{
    if (m_cameraStatus & status) {
        m_cameraStatus ^= status;
        emit cameraStatusChanged();
    }
}

/**
 * Changes the light status of the camera
 */
void QCCTV_LocalCamera::setFlashlightStatus (const QCCTV_LightStatus status)
{
    if (m_flashlightStatus != status) {
        m_flashlightStatus = status;

        if (!m_camera)
            return;

        if (flashlightAvailable()) {
            if (flashlightOn()) {
                m_camera->exposure()->setFlashMode (QCameraExposure::FlashVideoLight);
                focusCamera();
            }

            else
                m_camera->exposure()->setFlashMode (QCameraExposure::FlashOff);

            emit lightStatusChanged();
        }
    }
}
