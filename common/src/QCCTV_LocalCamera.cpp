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

#include <QBuffer>
#include <QSysInfo>
#include <QCameraInfo>
#include <QCameraExposure>
#include <QCameraImageCapture>

/**
 * Initializes the class by:
 *     - Configuring the TCP server
 *     - Starting the broadcast service
 *     - Generating the default image
 *     - Configuring the TCP server
 *     - Configuring the frame grabber
 */
QCCTV_LocalCamera::QCCTV_LocalCamera()
{
    /* Initialize pointers */
    m_camera = NULL;
    m_capture = NULL;

    /* Configure sockets */
    connect (&m_server,    SIGNAL (newConnection()),
             this,           SLOT (acceptConnection()));
    connect (&m_cmdSocket, SIGNAL (readyRead()),
             this,           SLOT (readCommandPacket()));

    /* Configure listener sockets */
    m_server.listen (QHostAddress::Any, QCCTV_STREAM_PORT);
    m_cmdSocket.bind (QCCTV_COMMAND_PORT, QUdpSocket::ShareAddress);

    /* Setup the frame grabber */
    connect (&m_frameGrabber, SIGNAL (newFrame (QImage)),
             this,              SLOT (changeImage (QImage)));

    /* Set default values */
    setFPS (QCCTV_DEFAULT_FPS);
    setCameraStatus (QCCTV_CAMSTATUS_DEFAULT);
    setFlashlightStatus (QCCTV_FLASHLIGHT_OFF);

    /* Get host name (or OS name & version) */
    QString name = QSysInfo::machineHostName();
    if (name.isEmpty())
        name = QSysInfo::prettyProductName();
    else
        name = name + " (" + QSysInfo::prettyProductName() + ")";

    /* Set default camera name */
    setName (name);

    /* Set default image */
    m_image = QCCTV_GET_STATUS_IMAGE (QSize (640, 480), "NO CAMERA IMAGE");

    /* Start the event loops */
    QTimer::singleShot (1000, Qt::CoarseTimer, this, SLOT (update()));
    QTimer::singleShot (1000, Qt::CoarseTimer, this, SLOT (broadcastInfo()));
}

/**
 * Closes all the sockets and the frontal camera device
 */
QCCTV_LocalCamera::~QCCTV_LocalCamera()
{
    foreach (QTcpSocket* socket, m_sockets) {
        socket->close();
        socket->deleteLater();
    }

    m_server.close();
    m_sockets.clear();
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
 * Returns the image that is currently being sent to the CCTV stations
 */
QImage QCCTV_LocalCamera::currentImage() const
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
 * Returns \c true if the camera is ready for saving photos
 */
bool QCCTV_LocalCamera::readyForCapture() const
{
    if (m_camera && m_capture)
        return m_capture->isReadyForCapture();

    return false;
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
 * Attempts to take a photo using the current camera
 */
void QCCTV_LocalCamera::takePhoto()
{
    if (readyForCapture())
        m_capture->capture();
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

        foreach (QCCTV_Watchdog* watchdog, m_watchdogs)
            watchdog->setExpirationTime (QCCTV_WATCHDOG_TIME (m_fps));

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
        m_camera->setViewfinder (&m_frameGrabber);
        m_camera->setCaptureMode (QCamera::CaptureStillImage);
        m_camera->start();

        if (m_capture)
            delete m_capture;

        m_capture = new QCameraImageCapture (m_camera);
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
 * Changes the shrink factor used to resize the image before sending it to
 * the QCCTV Stations in the local network
 */
void QCCTV_LocalCamera::setShrinkRatio (const qreal ratio)
{
    m_frameGrabber.setShrinkRatio (ratio);
}

/**
 * Obtains a new image from the camera and updates the camera status
 */
void QCCTV_LocalCamera::update()
{
    /* Generate a new camera status */
    updateStatus();

    /* Enable the frame grabber */
    m_frameGrabber.setEnabled (true);

    /* Construct a new stream of data to send */
    generateData();
    sendCameraData();

    /* Call this function again in several milliseconds */
    QTimer::singleShot (1000 / fps(), Qt::PreciseTimer, this, SLOT (update()));
}

/**
 * Generates a byte array with the following information:
 *
 * - CRC32 bytes
 * - The camera name
 * - The FPS of the camera
 * - The light status of the camera
 * - The camera status
 * - The latest camera image
 *
 * This byte array will be sent to all connected QCCTV Stations in the LAN
 */
void QCCTV_LocalCamera::generateData()
{
    /* Only generate the data stream if the previous one has been sent */
    if (m_data.isEmpty()) {
        /* Add camera name */
        m_data.append (cameraName().length());
        m_data.append (cameraName());

        /* Add FPS, light status and camera status */
        m_data.append (fps());
        m_data.append (lightStatus());
        m_data.append (cameraStatus());

        /* Add raw image bytes */
        if (!m_imageData.isEmpty()) {
            m_data.append ((m_imageData.length() & 0xff0000) >> 16);
            m_data.append ((m_imageData.length() & 0xff00) >> 8);
            m_data.append ((m_imageData.length() & 0xff));
            m_data.append (m_imageData);
        }

        /* Compress packet */
        m_data = qCompress (m_data, 9);

        /* Add the cheksum at the start of the data */
        quint32 crc = m_crc32.compute (m_data);
        m_data.prepend ((crc & 0xff));
        m_data.prepend ((crc & 0xff00) >> 8);
        m_data.prepend ((crc & 0xff0000) >> 16);
        m_data.prepend ((crc & 0xff000000) >> 24);
    }
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
}

/**
 * Sends the generated data packet to all connected QCCTV Stations.
 * The data is sent 'chunk by chunk' to avoid errors and ensure that the
 * generated data is interpreted correctly by the station(s).
 */
void QCCTV_LocalCamera::sendCameraData()
{
    if (m_data.isEmpty())
        return;

    foreach (QTcpSocket* socket, m_sockets) {
        if (socket->isWritable())
            socket->write (m_data);
    }

    m_data.clear();
}

/**
 * Creates and sends a new packet that announces the existence of this
 * camera to the local network
 */
void QCCTV_LocalCamera::broadcastInfo()
{
    QString str = "QCCTV_DISCOVERY_SERVICE";

    m_broadcastSocket.writeDatagram (str.toUtf8(),
                                     QHostAddress::Broadcast,
                                     QCCTV_DISCOVERY_PORT);

    QTimer::singleShot (QCCTV_DISCVRY_PKT_TIMING, Qt::PreciseTimer,
                        this, SLOT (broadcastInfo()));
}

/**
 * Closes and un-registers a station when the TCP connection is aborted
 */
void QCCTV_LocalCamera::onDisconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*> (sender());
    int index = m_sockets.indexOf (socket);

    /* Abort connection */
    socket->abort();
    socket->deleteLater();

    /* Delete objects */
    m_sockets.at (index)->deleteLater();
    m_watchdogs.at (index)->deleteLater();

    /* Unregister watchdog and socket */
    m_sockets.removeAt (index);
    m_watchdogs.removeAt (index);

    /* Set image quality to 1 if no other stations are connected */
    if (m_sockets.count() < 1)
        setShrinkRatio (1);
}

/**
 * Gradually lowers the image quality when the station fails to reply on time
 */
void QCCTV_LocalCamera::onWatchdogTimeout()
{
    /* Lower the image resolution */
    setShrinkRatio (shrinkRatio() + 0.5);

    /* Reset shrink ratio to 1 */
    if (shrinkRatio() >= 4.5)
        setShrinkRatio (1);
}

/**
 * Called when a CCTV station wants to receive images from this camera
 * This function shall configure the TCP socket used for streaming the
 * images and receiving commands from the station
 */
void QCCTV_LocalCamera::acceptConnection()
{
    while (m_server.hasPendingConnections()) {
        QCCTV_Watchdog* watchdog = new QCCTV_Watchdog (this);
        watchdog->setExpirationTime (QCCTV_WATCHDOG_TIME (fps()));

        m_watchdogs.append (watchdog);
        m_sockets.append (m_server.nextPendingConnection());

        m_sockets.last()->setSocketOption (QTcpSocket::LowDelayOption, 1);
        m_sockets.last()->setSocketOption (QTcpSocket::KeepAliveOption, 1);
        m_sockets.last()->setSocketOption (QTcpSocket::SendBufferSizeSocketOption, INT_MAX);

        connect (m_watchdogs.last(), SIGNAL (expired()),
                 this,                 SLOT (onWatchdogTimeout()));
        connect (m_sockets.last(),   SIGNAL (disconnected()),
                 this,                 SLOT (onDisconnected()));
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
    /* Get data and address */
    QByteArray data;
    QHostAddress address;
    data.resize (m_cmdSocket.pendingDatagramSize());
    m_cmdSocket.readDatagram (data.data(), data.length(), &address);

    /* Datagram is too small */
    if (data.size() != 3)
        return;

    /* Set FPS and flashlight status */
    setFPS ((quint8) data.at (0));
    setFlashlightStatus ((QCCTV_LightStatus) data.at (1));

    /* Focus the camera */
    if (data.at (2) == QCCTV_FORCE_FOCUS)
        focusCamera();

    /* Feed the watchdog for this connection */
    foreach (QTcpSocket* socket, m_sockets) {
        if (socket->peerAddress() == address)
            m_watchdogs.at (m_sockets.indexOf (socket))->reset();
    }
}

/**
 * Replaces the current image and notifies the application
 */
void QCCTV_LocalCamera::changeImage (const QImage& image)
{
    m_frameGrabber.setEnabled (false);

    if (!image.isNull()) {
        m_imageData = QCCTV_ENCODE_IMAGE (image);
        m_image = QCCTV_DECODE_IMAGE (m_imageData);
        emit imageChanged();
    }
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

        if (!flashlightAvailable())
            return;

        if (flashlightOn()) {
            m_camera->exposure()->setFlashMode (QCameraExposure::FlashVideoLight);
            focusCamera();
        }

        else
            m_camera->exposure()->setFlashMode (QCameraExposure::FlashOff);

        emit lightStatusChanged();
    }
}
