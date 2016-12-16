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

#include <QThread>
#include <QSysInfo>
#include <QCameraInfo>
#include <QCameraExposure>
#include <QCameraImageCapture>

#include "QCCTV.h"
#include "QCCTV_Watchdog.h"
#include "QCCTV_LocalCamera.h"
#include "QCCTV_ImageCapture.h"
#include "QCCTV_Communications.h"

#define ERROR_IMG QCCTV_CreateStatusImage (QSize (640, 480), "NO CAMERA IMAGE")

QCCTV_LocalCamera::QCCTV_LocalCamera (QObject* parent) : QObject (parent)
{
    /* Initialize pointers */
    m_camera = Q_NULLPTR;
    m_capture = Q_NULLPTR;
    m_thread = new QThread (this);
    m_imageCapture = new QCCTV_ImageCapture;
    m_streamPacket = new QCCTV_StreamPacket;
    m_commandPacket = new QCCTV_CommandPacket;

    /* Configure sockets */
    connect (&m_server,    SIGNAL (newConnection()),
             this,           SLOT (acceptConnection()));
    connect (&m_cmdSocket, SIGNAL (readyRead()),
             this,           SLOT (readCommandPacket()));

    /* Configure listener sockets */
    m_server.listen (QHostAddress::Any, QCCTV_STREAM_PORT);
    m_cmdSocket.bind (QCCTV_COMMAND_PORT, QUdpSocket::ShareAddress);

    /* Setup the frame grabber */
    m_thread->start (QThread::TimeCriticalPriority);
    m_imageCapture->moveToThread (m_thread);
    connect (m_imageCapture, SIGNAL (newFrame()),
             this,             SLOT (changeImage()));

    /* Set default camera information */
    QCCTV_InitStream (streamPacket());
    streamPacket()->cameraName = deviceName();

    /* Start the event loops */
    QTimer::singleShot (1000, Qt::CoarseTimer, this, SLOT (update()));
    QTimer::singleShot (1000, Qt::CoarseTimer, this, SLOT (broadcastInfo()));
}

/**
 * Closes all the sockets and the frontal camera device
 */
QCCTV_LocalCamera::~QCCTV_LocalCamera()
{
    /* Stop image processing thread */
    m_thread->exit();

    /* Close all TCP connections */
    foreach (QTcpSocket* socket, m_sockets) {
        socket->close();
        socket->deleteLater();
    }

    /* Delete all watchdogs */
    foreach (QCCTV_Watchdog* watchdog, m_watchdogs)
        watchdog->deleteLater();

    /* Close TCP server and clear socket lists */
    m_server.close();
    m_sockets.clear();
    m_watchdogs.clear();
    m_broadcastSocket.close();

    /* Delete camera capture object */
    if (m_capture)
        delete m_capture;

    /* Delete children */
    delete m_thread;
    delete m_imageCapture;
    delete m_streamPacket;
    delete m_commandPacket;
}

/**
 * Returns the current FPS of the camera
 */
int QCCTV_LocalCamera::fps()
{
    return streamPacket()->fps;
}

/**
 * Returns the user-assigned name of the camera
 */
QString QCCTV_LocalCamera::name()
{
    return streamPacket()->cameraName;
}

/**
 * Returns the user-assigned group of the camera
 */
QString QCCTV_LocalCamera::group()
{
    return streamPacket()->cameraGroup;
}

/**
 * Returns the current resolution as an \c int, which can be used by QML
 * interfaces directly
 */
int QCCTV_LocalCamera::resolution()
{
    return streamPacket()->resolution;
}

/**
 * Returns the current status of the camera itself
 */
int QCCTV_LocalCamera::cameraStatus()
{
    return streamPacket()->cameraStatus;
}

/**
 * Returns the image that is currently being sent to the QCCTV stations
 */
QImage QCCTV_LocalCamera::currentImage()
{
    return streamPacket()->image;
}

/**
 * Returns the current status of QCCTV in a string
 */
QString QCCTV_LocalCamera::statusString()
{
    return QCCTV_GetStatusString (cameraStatus());
}

/**
 * Returns the current status of the flash light, which can be
 * set either by the camera itself or a remote QCCTV Station
 */
int QCCTV_LocalCamera::flashlightEnabled()
{
    return streamPacket()->flashlightEnabled;
}

/**
 * Returns \c true if the camera is allows to auto-regulate its image
 * resolution to improve communication times
 */
bool QCCTV_LocalCamera::autoRegulateResolution()
{
    return streamPacket()->autoRegulateResolution;
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
 * Returns a list with all the image resolutions supported by QCCTV.
 * This function can be used to populate a \c QComboBox or a QML model
 */
QStringList QCCTV_LocalCamera::availableResolutions() const
{
    return QCCTV_Resolutions();
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
    if (streamPacket()->fps != QCCTV_ValidFps (fps)) {
        streamPacket()->fps = QCCTV_ValidFps (fps);
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
        m_imageCapture->setSource (m_camera);

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
    /* Names are the same, abort */
    if (streamPacket()->cameraName == name)
        return;

    /* Check if user just gave us empty spaces as input */
    QString no_spaces = name;
    no_spaces = no_spaces.replace (" ", "");

    /* Re-assign the camera name */
    if (no_spaces.isEmpty())
        streamPacket()->cameraName = deviceName();
    else
        streamPacket()->cameraName = name;

    /* Notify UI */
    emit nameChanged();
}

/**
 * Changes the \a group assigned to the camera
 */
void QCCTV_LocalCamera::setGroup (const QString& group)
{
    /* Names are the same, abort */
    if (streamPacket()->cameraGroup == group)
        return;

    /* Check if user just gave us empty spaces as input */
    QString no_spaces = group;
    no_spaces = no_spaces.replace (" ", "");

    /* Re-assign group name */
    if (no_spaces.isEmpty())
        streamPacket()->cameraGroup = "Default";
    else
        streamPacket()->cameraGroup = group;

    /* Notify UI */
    emit groupChanged();
}

/**
 * Changes the resolution of the image that the camera sends to the station
 */
void QCCTV_LocalCamera::setResolution (const int resolution)
{
    if (streamPacket()->resolution != resolution) {
        streamPacket()->resolution = resolution;
        emit resolutionChanged();
    }
}

/**
 * Turns on or off the flashlight based on the value of the \a enabled
 * parameter
 */
void QCCTV_LocalCamera::setFlashlightEnabled (const bool enabled)
{
    streamPacket()->flashlightEnabled = enabled;

    if (flashlightAvailable() && flashlightEnabled())
        m_camera->exposure()->setFlashMode (QCameraExposure::FlashVideoLight);

    else
        m_camera->exposure()->setFlashMode (QCameraExposure::FlashOff);

    emit lightStatusChanged();
}

/**
 * Allows or disallows the camera from auto-regulating the image resolution
 * to improve the communication times.
 */
void QCCTV_LocalCamera::setAutoRegulateResolution (const bool regulate)
{
    if (streamPacket()->autoRegulateResolution != regulate) {
        streamPacket()->autoRegulateResolution = regulate;
        emit autoRegulateResolutionChanged();
    }
}

/**
 * Obtains a new image from the camera and updates the camera status
 */
void QCCTV_LocalCamera::update()
{
    /* Enable the frame grabber */
    m_imageCapture->setEnabled (true);
    emit imageChanged();

    /* Generate a new camera status */
    updateStatus();

    /* Generate and send camera data */
    QByteArray data = QCCTV_CreateStreamPacket (*streamPacket());
    foreach (QTcpSocket* socket, m_sockets) {
        if (socket->isWritable())
            socket->write (data);
    }

    /* Call this function again in several milliseconds */
    QTimer::singleShot (1000 / fps(), Qt::PreciseTimer, this, SLOT (update()));
}

/**
 * Replaces the current image and notifies the application
 */
void QCCTV_LocalCamera::changeImage()
{
    m_imageCapture->setEnabled (false);
    streamPacket()->image = m_imageCapture->image();
    if (streamPacket()->image.isNull())
        streamPacket()->image = ERROR_IMG;
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

    /* Read the command packet */
    bool success = QCCTV_ReadCommandPacket (commandPacket(), data);
    if (!success)
        return;

    /* Change FPS */
    if (commandPacket()->fpsChanged)
        if (commandPacket()->oldFps == fps())
            setFPS (commandPacket()->newFps);

    /* Set resolution */
    if (commandPacket()->resolutionChanged)
        if (commandPacket()->oldResolution == resolution())
            setResolution (commandPacket()->newResolution);

    /* Set flashlight status */
    if (commandPacket()->flashlightEnabledChanged)
        if (commandPacket()->oldFlashlightEnabled == flashlightEnabled())
            setFlashlightEnabled (commandPacket()->newFlashlightEnabled);

    /* Change the auto-regulate resolution option */
    if (commandPacket()->autoRegulateResolutionChanged)
        if (commandPacket()->oldAutoRegulateResolution == autoRegulateResolution())
            setAutoRegulateResolution (commandPacket()->newAutoRegulateResolution);

    /* Focus the camera */
    if (commandPacket()->focusRequest)
        focusCamera();

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
 * Updates the status code of the camera
 */
void QCCTV_LocalCamera::updateStatus()
{
    /* Check if camera exists */
    if (!m_camera)
        addStatusFlag (QCCTV_CAMSTATUS_VIDEO_FAILURE);

    /* Check if camera is active */
    else if (m_camera->status() != QCamera::ActiveStatus)
        addStatusFlag (QCCTV_CAMSTATUS_VIDEO_FAILURE);
    else
        removeStatusFlag (QCCTV_CAMSTATUS_VIDEO_FAILURE);

    /* Check if flash light is available */
    if (!flashlightAvailable())
        addStatusFlag (QCCTV_CAMSTATUS_LIGHT_FAILURE);
    else
        removeStatusFlag (QCCTV_CAMSTATUS_LIGHT_FAILURE);
}

/**
 * Returns the name and operating system of the camera device
 */
QString QCCTV_LocalCamera::deviceName()
{
    QString device = QSysInfo::machineHostName();
    if (device.isEmpty())
        device = QSysInfo::prettyProductName();
    else
        device += " (" + QSysInfo::prettyProductName() + ")";

    return device;
}

/**
 * Returns the pointer to the stream packet structure
 */
QCCTV_StreamPacket* QCCTV_LocalCamera::streamPacket()
{
    if (m_streamPacket)
        return m_streamPacket;

    m_streamPacket = new QCCTV_StreamPacket;
    return m_streamPacket;
}

/**
 * Returns the pointer to the command packet structure
 */
QCCTV_CommandPacket* QCCTV_LocalCamera::commandPacket()
{
    if (m_commandPacket)
        return m_commandPacket;

    m_commandPacket = new QCCTV_CommandPacket;
    return m_commandPacket;
}

/**
 * Registers the given \a status flag to the operation status flags
 */
void QCCTV_LocalCamera::addStatusFlag (const int status)
{
    if (! (streamPacket()->cameraStatus & status)) {
        streamPacket()->cameraStatus |= status;
        emit cameraStatusChanged();
    }
}

/**
 * Overrides the camera status flags with the given \a status
 */
void QCCTV_LocalCamera::setCameraStatus (const int status)
{
    streamPacket()->cameraStatus = status;
    emit cameraStatusChanged();
}

/**
 * Removes the given \a status flag from the operation status of the camera
 */
void QCCTV_LocalCamera::removeStatusFlag (const int status)
{
    if (streamPacket()->cameraStatus & status) {
        streamPacket()->cameraStatus ^= status;
        emit cameraStatusChanged();
    }
}
