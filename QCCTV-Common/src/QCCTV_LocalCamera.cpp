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
#include "QCCTV_CameraFrame.h"

#include <QThread>
#include <QBuffer>
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
    /* Try to open the camera */
    m_camera = new QCamera (QCameraInfo::defaultCamera());

    /* Create the viewfinder */
    connect (&m_grabber, SIGNAL (newFrame (QPixmap)),
             this,         SLOT (changeImage (QPixmap)));

    /* We have access to camera, configure it */
    if (m_camera->isAvailable()) {
        m_camera->setViewfinder (&m_grabber);
        m_camera->setCaptureMode (QCamera::CaptureViewfinder);
        m_camera->load();
        m_camera->start();

        /* Cannot start the camera */
        if (m_camera->status() != QCamera::ActiveStatus)
            addStatusFlag (QCCTV_CAMSTATUS_VIDEO_FAILURE);
    }

    /* Cannot open the camera */
    else
        addStatusFlag (QCCTV_CAMSTATUS_VIDEO_FAILURE);

    /* Bind sockets */
    m_commandSocket.bind (QHostAddress::Any,
                          QCCTV_COMMAND_PORT,
                          QUdpSocket::ShareAddress |
                          QUdpSocket::ReuseAddressHint);
    m_requestSocket.bind (QHostAddress::Any,
                          QCCTV_REQUEST_PORT,
                          QUdpSocket::ShareAddress |
                          QUdpSocket::ReuseAddressHint);

    /* Connect sockets signals/slots */
    connect (&m_commandSocket, SIGNAL (readyRead()),
             this,               SLOT (readCommandPacket()));
    connect (&m_requestSocket, SIGNAL (readyRead()),
             this,               SLOT (readRequestPacket()));

    /* Setup the watchdog */
    m_watchdog.setExpirationTime (QCCTV_COMMAND_PKT_TIMEOUT);
    connect (&m_watchdog, SIGNAL (expired()), this, SLOT (disconnectStation()));

    /* Set default values */
    setFPS (24);
    setGroup ("default");
    setName ("Unknown Camera");
    setCameraStatus (QCCTV_CAMSTATUS_DEFAULT);
    setFlashlightStatus (QCCTV_FLASHLIGHT_OFF);

    /* Start the event loops */
    QTimer::singleShot (1000, Qt::CoarseTimer, this, SLOT (update()));
    QTimer::singleShot (1000, Qt::CoarseTimer, this, SLOT (broadcastInformation()));
}

/**
 * Closes all the sockets and the frontal camera device
 */
QCCTV_LocalCamera::~QCCTV_LocalCamera()
{
    /* Close all sockets */
    m_senderSocket.close();
    m_commandSocket.close();
    m_requestSocket.close();
    m_broadcastSocket.close();
}

/**
 * Returns the current FPS of the camera
 */
int QCCTV_LocalCamera::fps() const
{
    return m_fps;
}

/**
 * Returns the number of times that the image is scaled to 50% of the image
 * captured by the device's camera
 */
int QCCTV_LocalCamera::scaleRatio() const
{
    return m_grabber.scaleRatio();
}

/**
 * Returns \c true if the obtained image shall be converted to a black and white
 * image (which slightly decreases image size & CPU usage)
 */
bool QCCTV_LocalCamera::grayscale() const
{
    return m_grabber.isGrayscale();
}

/**
 * Returns the current status of the flashlight (on or off)
 */
QCCTV_LightStatus QCCTV_LocalCamera::lightStatus() const
{
    return (QCCTV_LightStatus) m_flashlightStatus;
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
    return m_camera->exposure()->isFlashReady();
}

/**
 * Returns a list with all the connected QCCTV stations to this camera
 */
QStringList QCCTV_LocalCamera::connectedHosts() const
{
    QStringList list;

    foreach (QHostAddress address, m_hosts)
        list.append (address.toString());

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
 * Changes the size of the image obtained from the camera
 */
void QCCTV_LocalCamera::setScaleRatio (const int ratio)
{
    m_grabber.setScaleRatio (ratio);
}

/**
 * If \a grayscale is set to \c true, then the image obtained from the camera
 * shall be stripped from its colors, which decreases the CPU usage and
 * the size of the generated image.
 *
 * If \a grayscale is set to \c false, then the image will be processed with
 * all the colors reported from the frame generated by the camera
 */
void QCCTV_LocalCamera::setGrayscale (const bool grayscale)
{
    m_grabber.setGrayscale (grayscale);
}

/**
 * Obtains a new image from the camera, updates the camera status and sends
 * the newly obtained data to the stations
 */
void QCCTV_LocalCamera::update()
{
    /* Allow grabber to generate camera images */
    m_grabber.setEnabled (true);

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
    if (!m_hosts.isEmpty())
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
    data.append (grayscale() ? QCCTV_CMD_GRAYSCALE : 0x00);

    /* Get image buffer */
    QByteArray img;
    QBuffer buffer (&img);
    buffer.open (QIODevice::WriteOnly);
    currentImage().save (&buffer, QCCTV_IMAGE_FORMAT);

    /* Add image buffer to packet */
    data.append (qCompress (img, QCCTV_COMPRESSION_FACTOR));
    buffer.close();

    /* Send generated data */
    foreach (QHostAddress address, m_hosts)
        m_senderSocket.writeDatagram (data, address, QCCTV_STREAM_PORT);
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
 * Interprets a connection request packet and decides whenever to accept it
 * or not
 */
void QCCTV_LocalCamera::readRequestPacket()
{
    while (m_requestSocket.hasPendingDatagrams()) {
        QByteArray data;
        QHostAddress address;
        data.resize (m_requestSocket.pendingDatagramSize());
        int bytes = m_requestSocket.readDatagram (data.data(),
                                                  data.size(),
                                                  &address, NULL);

        if (bytes > 0) {
            m_hosts.append (address);
            m_hosts = m_hosts.toSet().toList();
        }
    }
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
    while (m_commandSocket.hasPendingDatagrams()) {
        QByteArray data;
        QHostAddress ip;

        /* Resize byte array to incomind data */
        data.resize (m_commandSocket.pendingDatagramSize());

        /* Read the datagram */
        int bytes = m_commandSocket.readDatagram (data.data(),
                                                  data.size(),
                                                  &ip, NULL);

        /* Remote IP is not on allowed hosts list, ignore packet */
        if (!m_hosts.contains (ip))
            return;

        /* Packet length is invalid */
        if (data.size() != 3 || bytes <= 0)
            return;

        /* Change the FPS */
        setFPS ((int) data.at (0));

        /* Change the light status */
        setFlashlightStatus ((QCCTV_LightStatus) data.at (1));

        /* Focus the camera */
        if (data.at (2) == QCCTV_FORCE_FOCUS)
            focusCamera();

        /* Change the grayscale effect */
        setGrayscale (data.at (3) == QCCTV_CMD_GRAYSCALE);

        /* Feed watchdog timer */
        m_watchdog.reset();
    }
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
    m_grabber.setEnabled (false);

    emit newImageRecorded();
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
            if (flashlightOn())
                m_camera->exposure()->setFlashMode (QCameraExposure::FlashOn);
            else
                m_camera->exposure()->setFlashMode (QCameraExposure::FlashOff);

            emit lightStatusChanged();
        }
    }
}

//==============================================================================
// QCCTV_LocalImageProvider Class
//==============================================================================

/**
 * Initializes the image provider with the given \a camera
 */
QCCTV_LocalImageProvider::QCCTV_LocalImageProvider (QCCTV_LocalCamera* camera) :
    QQuickImageProvider (QQuickImageProvider::Pixmap)
{
    m_camera = camera;
}

/**
 * Returns the latest image captured by the camera
 */
QPixmap QCCTV_LocalImageProvider::requestPixmap (const QString& id, QSize* size,
                                                 const QSize& requestedSize)
{
    Q_UNUSED (id);
    Q_UNUSED (size);
    Q_UNUSED (requestedSize);

    if (m_camera)
        return m_camera->currentImage();

    return QPixmap();
}
