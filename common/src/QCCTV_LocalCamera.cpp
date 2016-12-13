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
#include "QCCTV_ImageCapture.h"

#include <QBuffer>
#include <QSysInfo>
#include <QCameraInfo>
#include <QCameraExposure>
#include <QCameraImageCapture>
#include <QAbstractVideoSurface>

#define ERROR_IMG QCCTV_GET_STATUS_IMAGE (QSize (640, 480), "NO CAMERA IMAGE")

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
    connect (&m_imageCapture, SIGNAL (newFrame (QImage)),
             this,              SLOT (changeImage (QImage)));

    /* Set default values (and emit signals) */
    setName ("");
    setGroup ("");
    changeImage (ERROR_IMG);
    setFPS (QCCTV_DEFAULT_FPS);
    setAutoRegulateResolution (true);
    setResolution (QCCTV_DEFAULT_RES);
    setCameraStatus (QCCTV_CAMSTATUS_DEFAULT);
    setFlashlightStatus (QCCTV_FLASHLIGHT_OFF);

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

    foreach (QCCTV_Watchdog* watchdog, m_watchdogs)
        watchdog->deleteLater();

    m_server.close();
    m_sockets.clear();
    m_watchdogs.clear();
    m_broadcastSocket.close();

    if (m_capture)
        delete m_capture;
}

/**
 * Returns the current FPS of the camera
 */
int QCCTV_LocalCamera::fps() const
{
    return m_fps;
}

/**
 * Returns the current resolution as an \c int, which can be used by QML
 * interfaces directly
 */
int QCCTV_LocalCamera::resolution() const
{
    return (int) m_resolution;
}

/**
 * Returns the minimum FPS value allowed by QCCTV, this function can be used
 * to set control/widget limits of QML or classic interfaces
 */
int QCCTV_LocalCamera::minimumFPS() const
{
    return QCCTV_MIN_FPS;
}

/**
 * Returns the maximum FPS value allowed by QCCTV, this function can be used
 * to set control/widget limits of QML or classic interfaces
 */
int QCCTV_LocalCamera::maximumFPS() const
{
    return QCCTV_MAX_FPS;
}

/**
 * Returns an ordered list with the available image resolutions, this function
 * can be used to populate a combobox or a QML model
 */
QStringList QCCTV_LocalCamera::availableResolutions() const
{
    return QCCTV_AVAILABLE_RESOLUTIONS();
}

/**
 * Returns the user-assigned name of the camera
 */
QString QCCTV_LocalCamera::cameraName() const
{
    return m_name;
}

/**
 * Returns the user-assigned group of the camera
 */
QString QCCTV_LocalCamera::cameraGroup() const
{
    return m_group;
}

/**
 * Returns the image that is currently being sent to the QCCTV stations
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
 * Returns \c true if the camera is allows to auto-regulate its image
 * resolution to improve communication times
 */
bool QCCTV_LocalCamera::autoRegulateResolution() const
{
    return m_autoRegulateResolution;
}

/**
 * Returns the current status of the camera itself
 */
int QCCTV_LocalCamera::cameraStatus() const
{
    return m_cameraStatus;
}

/**
 * Returns the current status of the flash light, which can be
 * set either by the camera itself or a remote QCCTV Station
 */
int QCCTV_LocalCamera::flashlightStatus() const
{
    return m_flashlightStatus;
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
        m_camera->setCaptureMode (QCamera::CaptureStillImage);
        m_imageCapture.setSource (m_camera);

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
    QString no_spaces = name;
    no_spaces = no_spaces.replace (" ", "");

    if (no_spaces.isEmpty()) {
        QString host = QSysInfo::machineHostName();
        if (host.isEmpty())
            host = QSysInfo::prettyProductName();
        else
            host = host + " (" + QSysInfo::prettyProductName() + ")";

        m_name = host;
        emit cameraNameChanged();
    }

    else if (m_name != name) {
        m_name = name;
        emit cameraNameChanged();
    }
}

/**
 * Changes the \a group assigned to the camera
 */
void QCCTV_LocalCamera::setGroup (const QString& group)
{
    if (group.isEmpty())
        m_group = "Default";
    else
        m_group = group;
}

/**
 * Changes the resolution of the image that the camera sends to the station
 */
void QCCTV_LocalCamera::setResolution (const int resolution)
{
    if ((int) m_resolution != resolution) {
        m_resolution = (QCCTV_Resolution) resolution;
        emit resolutionChanged();
    }
}

/**
 * Turns on or off the flashlight based on the value of the \a enabled
 * parameter
 */
void QCCTV_LocalCamera::setFlashlightEnabled (const bool enabled)
{
    setFlashlightStatus (enabled ? QCCTV_FLASHLIGHT_ON : QCCTV_FLASHLIGHT_OFF);
}

/**
 * Allows or disallows the camera from auto-regulating the image resolution
 * to improve the communication times.
 */
void QCCTV_LocalCamera::setAutoRegulateResolution (const bool regulate)
{
    if (m_autoRegulateResolution != regulate) {
        m_autoRegulateResolution = regulate;
        emit autoRegulateResolutionChanged();
    }
}

/**
 * Obtains a new image from the camera and updates the camera status
 */
void QCCTV_LocalCamera::update()
{
    /* Enable the frame grabber */
    m_imageCapture.setEnabled (true);

    /* Generate a new camera status */
    updateStatus();
    generateData();
    sendCameraData();

    /* Call this function again in several milliseconds */
    QTimer::singleShot (1000 / fps(), Qt::PreciseTimer, this, SLOT (update()));
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

    QTimer::singleShot (1000, this, SLOT (broadcastInfo()));
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

    /* Notify application */
    emit hostCountChanged();
}

/**
 * Called when a QCCTV station wants to receive images from this camera
 * This function shall configure the TCP socket used for streaming data
 */
void QCCTV_LocalCamera::acceptConnection()
{
    while (m_server.hasPendingConnections()) {
        QCCTV_Watchdog* watchdog = new QCCTV_Watchdog (this);
        watchdog->setExpirationTime (500);

        m_watchdogs.append (watchdog);
        m_sockets.append (m_server.nextPendingConnection());

        m_sockets.last()->setSocketOption (QTcpSocket::LowDelayOption, 1);
        m_sockets.last()->setSocketOption (QTcpSocket::KeepAliveOption, 1);
        m_sockets.last()->setSocketOption (QTcpSocket::SendBufferSizeSocketOption, INT_MAX);

        connect (m_watchdogs.last(), SIGNAL (expired()),
                 this,                 SLOT (onWatchdogTimeout()));
        connect (m_sockets.last(),   SIGNAL (disconnected()),
                 this,                 SLOT (onDisconnected()));

        emit hostCountChanged();
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
    if (data.size() < 8)
        return;

    /* Obtain data */
    quint8 old_fps = data.at (0);
    quint8 new_fps = data.at (1);
    quint8 old_res = data.at (2);
    quint8 new_res = data.at (3);
    quint8 old_lit = data.at (4);
    quint8 new_lit = data.at (5);
    bool s_focus = (data.at (6) == QCCTV_FORCE_FOCUS);
    bool old_reg = (data.at (7) == QCCTV_AUTOREGULATE_RES);
    bool new_reg = (data.at (8) == QCCTV_AUTOREGULATE_RES);

    /* Change FPS */
    if (old_fps != new_fps && old_fps == fps())
        setFPS (new_fps);

    /* Set resolution */
    if (old_res != new_res && old_res == resolution())
        setResolution (new_res);

    /* Set flashlight status */
    if (old_lit != new_lit && old_lit == flashlightStatus())
        setFlashlightStatus ((QCCTV_LightStatus) new_lit);

    /* Focus the camera */
    if (s_focus)
        focusCamera();

    /* Change the auto-regulate resolution option */
    if (old_reg != new_reg && old_reg == autoRegulateResolution())
        setAutoRegulateResolution (new_reg);

    /* Feed the watchdog for this connection */
    foreach (QTcpSocket* socket, m_sockets) {
        if (socket->peerAddress() == address)
            m_watchdogs.at (m_sockets.indexOf (socket))->reset();
    }
}

/**
 * Gradually lowers the image quality when the station fails to reply on time
 */
void QCCTV_LocalCamera::onWatchdogTimeout()
{
    if (resolution() == QCCTV_QCIF || !autoRegulateResolution())
        return;

    if (connectedHosts().isEmpty())
        return;

    setResolution ((QCCTV_Resolution) qMax ((int) QCCTV_CIF, resolution() - 1));
}

/**
 * Replaces the current image and notifies the application
 */
void QCCTV_LocalCamera::changeImage (const QImage& image)
{
    m_imageCapture.setEnabled (false);

    /* Change image */
    m_image = image;
    if (m_image.isNull())
        m_image = ERROR_IMG;

    /* Clear image data */
    m_imageData.clear();
    m_imageData = QCCTV_ENCODE_IMAGE (m_image, (QCCTV_Resolution) resolution());

    /* Notify UI */
    emit imageChanged();
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
        m_data.append (cameraName().toLatin1());

        /* Add camera group */
        m_data.append (cameraGroup().length());
        m_data.append (cameraGroup().toLatin1());

        /* Add FPS, light status and camera status */
        m_data.append ((quint8) fps());
        m_data.append ((quint8) flashlightStatus());
        m_data.append ((quint8) cameraStatus());
        m_data.append ((quint8) resolution());
        m_data.append ((quint8) autoRegulateResolution());

        /* Add raw image bytes */
        if (!m_imageData.isEmpty()) {
            m_data.append ((m_imageData.length() & 0xff0000) >> 16);
            m_data.append ((m_imageData.length() & 0xff00) >> 8);
            m_data.append ((m_imageData.length() & 0xff));
            m_data.append ((m_imageData));
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
    m_flashlightStatus = status;

    if (flashlightAvailable()) {
        switch (flashlightStatus()) {
        case QCCTV_FLASHLIGHT_ON:
            m_camera->exposure()->setFlashMode (QCameraExposure::FlashVideoLight);
            break;
        case QCCTV_FLASHLIGHT_OFF:
            m_camera->exposure()->setFlashMode (QCameraExposure::FlashOff);
            break;
        default:
            setFlashlightStatus (QCCTV_FLASHLIGHT_OFF);
            break;
        }
    }

    else if (flashlightStatus() != QCCTV_FLASHLIGHT_OFF)
        m_flashlightStatus = QCCTV_FLASHLIGHT_OFF;

    emit lightStatusChanged();
}
