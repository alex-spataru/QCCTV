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

#include <QDir>
#include <QSysInfo>
#include <QtConcurrent/QtConcurrent>

#include "QCCTV.h"
#include "QCCTV_Watchdog.h"
#include "QCCTV_ImageSaver.h"
#include "QCCTV_RemoteCamera.h"
#include "QCCTV_Communications.h"

static const QString hostName()
{
    QString host = QSysInfo::machineHostName();
    if (host.isEmpty())
        host = QSysInfo::prettyProductName();
    else
        host += " (" + QSysInfo::prettyProductName() + ")";

    return host;
}

QCCTV_RemoteCamera::QCCTV_RemoteCamera (QObject* parent) : QObject (parent)
{
    m_id = 0;
    m_connected = false;
    m_saveIncomingMedia = false;
    m_saver = new QCCTV_ImageSaver (this);
    m_infoPacket = new QCCTV_InfoPacket;
    m_imagePacket = new QCCTV_ImagePacket;
    m_commandPacket = new QCCTV_CommandPacket;

    setIncomingMediaPath ("");
    QCCTV_InitInfo (infoPacket());
    QCCTV_InitImage (imagePacket());
    QCCTV_InitCommand (commandPacket(), infoPacket());

    commandPacket()->host = hostName();
}

/**
 * Close the TCP sockets during the destruction of the class
 */
QCCTV_RemoteCamera::~QCCTV_RemoteCamera()
{
    if (m_socket) {
        m_socket->close();
        delete m_socket;
    }

    if (m_commandSocket) {
        m_commandSocket->close();
        delete m_commandSocket;
    }

    if (m_watchdog)
        delete m_watchdog;

    delete m_saver;
    delete m_infoPacket;
    delete m_imagePacket;
    delete m_commandPacket;
}

/**
 * Returns the FPS set by the station or by the camera itself
 */
int QCCTV_RemoteCamera::fps()
{
    return infoPacket()->fps;
}

/**
 * Returns the current zoom level of the camera
 */
int QCCTV_RemoteCamera::zoom()
{
    return infoPacket()->zoom;
}

/**
 * Returns the operation status reported by the camera
 */
int QCCTV_RemoteCamera::status()
{
    return infoPacket()->cameraStatus;
}

/**
 * Returns the latest image captured by the camera
 */
QImage QCCTV_RemoteCamera::image()
{
    return imagePacket()->image;
}

/**
 * Returns the name of the camera
 */
QString QCCTV_RemoteCamera::name()
{
    return infoPacket()->cameraName;
}

/**
 * Returns the group assigned to the camera
 */
QString QCCTV_RemoteCamera::group()
{
    return infoPacket()->cameraGroup;
}

/**
 * Returns the current resolution used by the camera
 */
int QCCTV_RemoteCamera::resolution()
{
    return infoPacket()->resolution;
}

/**
 * Returns \c true if the remote camera supports zooming
 */
bool QCCTV_RemoteCamera::supportsZoom()
{
    return infoPacket()->supportsZoom;
}

/**
 * Returns the camera status flags as a string
 */
QString QCCTV_RemoteCamera::statusString()
{
    return QCCTV_GetStatusString (status());
}

/**
 * Returns \c true if the flashlight of the camera is turned on
 */
bool QCCTV_RemoteCamera::flashlightEnabled()
{
    return infoPacket()->flashlightEnabled;
}

/**
 * Returns \c true if the camera is allowed to autoregulate its resolution
 */
bool QCCTV_RemoteCamera::autoRegulateResolution()
{
    return infoPacket()->autoRegulateResolution;
}

/**
 * Returns the camera ID set by the station
 */
int QCCTV_RemoteCamera::id() const
{
    return m_id;
}

/**
 * Returns \c true if the station has an active connection
 * with a QCCTV camera
 */
bool QCCTV_RemoteCamera::isConnected() const
{
    return m_connected;
}

/**
 * Returns the network address of the camera
 */
QHostAddress QCCTV_RemoteCamera::address() const
{
    return m_address;
}

/**
 * Returns \c true if the class shall save to the disk the received images
 */
bool QCCTV_RemoteCamera::saveIncomingMedia() const
{
    return m_saveIncomingMedia;
}

/**
 * Returns the folder path in which the incoming media is saved
 */
QString QCCTV_RemoteCamera::incomingMediaPath() const
{
    return m_incomingMediaPath;
}

/**
 * Initializes the watchdog timers after the thread has been created
 */
void QCCTV_RemoteCamera::start()
{
    /* Initialize the watchdog */
    m_watchdog = new QCCTV_Watchdog (this);
    m_watchdog->setExpirationTime (QCCTV_MIN_WATCHDOG_TIME);
    connect (m_watchdog, SIGNAL (expired()),
             this,         SLOT (clearBuffer()));

    /* Initialize sockets */
    m_socket = new QTcpSocket (this);
    m_commandSocket = new QUdpSocket (this);

    /* Configure signals/slots */
    connect (m_socket,     SIGNAL (readyRead()),
             this,           SLOT (onImageDataReceived()));
    connect (m_socket,     SIGNAL (disconnected()),
             this,           SLOT (endConnection()));

    /* Connect to camera */
    m_socket->connectToHost (m_address, QCCTV_STREAM_PORT);
    m_socket->setSocketOption (QTcpSocket::LowDelayOption, 1);
    m_socket->setSocketOption (QTcpSocket::KeepAliveOption, 1);
}

/**
 * Instructs the class to generate a packet that requests the camera to perform
 * a forced focus.
 *
 * The focus byte will be reset after sending some packets to the camera
 */
void QCCTV_RemoteCamera::requestFocus()
{
    m_commandPacket->focusRequest = true;
    QTimer::singleShot (500, this, SLOT (resetFocusRequest()));
}

/**
 * Changes the ID of the camera
 */
void QCCTV_RemoteCamera::changeID (const int id)
{
    m_id = id;
}

/**
 * Updates the FPS value reported (or assigned) by the camera
 */
void QCCTV_RemoteCamera::changeFPS (const int fps)
{
    int validFps = QCCTV_ValidFps (fps);
    commandPacket()->newFps = validFps;

    if (m_watchdog)
        m_watchdog->setExpirationTime (QCCTV_GetWatchdogTime (validFps));
}

/**
 * Updates the zoom level used by the remote camera
 */
void QCCTV_RemoteCamera::changeZoom (const int zoom)
{
    commandPacket()->newZoom = qMin (qMax (zoom, 0), 100);
}

/**
 * Allows or disallows saving the incoming images to the disk
 */
void QCCTV_RemoteCamera::setSaveIncomingMedia (const bool save)
{
    m_saveIncomingMedia = save;
}

/**
 * Reads and interprets an information packet coming from the camera
 */
void QCCTV_RemoteCamera::readInfoPacket (const QByteArray& data)
{
    QCCTV_InfoPacket packet;
    if (QCCTV_ReadInfoPacket (&packet, data)) {
        updateFPS (packet.fps);
        updateZoom (packet.zoom);
        updateName (packet.cameraName);
        updateGroup (packet.cameraGroup);
        updateStatus (packet.cameraStatus);
        updateResolution (packet.resolution);
        updateZoomSupport (packet.supportsZoom);
        updateAutoRegulate (packet.autoRegulateResolution);
        updateFlashlightEnabled (packet.flashlightEnabled);
        acknowledgeReception();
    }
}

/**
 * Changes the resoltion of the camera
 */
void QCCTV_RemoteCamera::changeResolution (const int resolution)
{
    commandPacket()->newResolution = resolution;
}

/**
 * Changes the camera's remote address, this can be done only
 * once during the instance's runtime
 */
void QCCTV_RemoteCamera::setAddress (const QHostAddress& address)
{
    if (!m_address.isNull() || address.isNull())
        return;

    m_address = address;
}

/**
 * Allows or disallows the camera from autoregulating its resolution
 */
void QCCTV_RemoteCamera::changeAutoRegulate (const bool regulate)
{
    commandPacket()->newAutoRegulateResolution = regulate;
}

/**
 * Changes the flashlight status of the camera and emits the appropiate signals
 */
void QCCTV_RemoteCamera::changeFlashlightStatus (const int status)
{
    commandPacket()->newFlashlightEnabled = status;
}

/**
 * Changes the path in which we should save incoming images
 */
void QCCTV_RemoteCamera::setIncomingMediaPath (const QString& path)
{
    if (!path.isEmpty())
        m_incomingMediaPath = path;
    else
        m_incomingMediaPath = QDir::homePath() + "/QCCTV/QCCTV_Media/";
}

/**
 * Called when we stop receiving constant packets from the camera, this
 * function deletes the temporary data buffer to avoid storing too much
 * information that we cannot use directly.
 *
 * Also, this function is called when the class successfully reads a stream
 * packet from the remote camera.
 */
void QCCTV_RemoteCamera::clearBuffer()
{
    m_data.clear();
}

/**
 * Called when the TCP connection with the camera is closed. This function
 * clears the temporary buffers and notifies the station that the
 * connection with the camera has ended, which is sort like commiting suicide
 * for this class...
 */
void QCCTV_RemoteCamera::endConnection()
{
    if (m_socket)
        m_socket->abort();

    clearBuffer();
    updateConnected (false);
    emit disconnected (id());
}

/**
 * Disables the focus flag. This function is called after 3 command packets
 * instructing the camera to re-focus itself have been sent.
 */
void QCCTV_RemoteCamera::resetFocusRequest()
{
    commandPacket()->focusRequest = false;
}

/**
 * Obtains the information received by the TCP socket and calls the functions
 * neccessary to interpret the received data
 */
void QCCTV_RemoteCamera::onImageDataReceived()
{
    if (!m_socket) {
        clearBuffer();
        return;
    }

    else {
        m_data.append (m_socket->readAll());

        if (!m_data.isEmpty())
            readImagePacket();

        if (m_data.size() >= QCCTV_MAX_BUFFER_SIZE)
            clearBuffer();
    }
}


/**
 * Sends a command packet to the camera, which instructs it to:
 *
 * - Change its FPS
 * - Change its light status
 * - Focus the camera device (if required)
 */
void QCCTV_RemoteCamera::sendCommandPacket()
{
    QByteArray data = QCCTV_CreateCommandPacket (commandPacket());

    if (m_commandSocket)
        m_commandSocket->writeDatagram (data, address(), QCCTV_COMMAND_PORT);
}

/**
 * Updates the \a fps reported by the camera
 */
void QCCTV_RemoteCamera::updateFPS (const int fps)
{
    if (infoPacket()->fps != fps) {
        infoPacket()->fps = fps;
        commandPacket()->oldFps = fps;
        commandPacket()->newFps = fps;
        emit fpsChanged (id());
    }
}

/**
 * Updates the zoom level of the camera and emits the appropiate signals
 */
void QCCTV_RemoteCamera::updateZoom (const int zoom)
{
    if (infoPacket()->zoom != zoom) {
        infoPacket()->zoom = zoom;
        commandPacket()->oldZoom = zoom;
        commandPacket()->newZoom = zoom;
        emit zoomLevelChanged (id());
    }
}

/**
 * Updates the operation status of the camera and emits the appropiate signals
 */
void QCCTV_RemoteCamera::updateStatus (const int status)
{
    if (infoPacket()->cameraStatus != status) {
        infoPacket()->cameraStatus = status;
        emit newCameraStatus (id());
    }
}

/**
 * Updates the \a name reported by the camera
 */
void QCCTV_RemoteCamera::updateName (const QString& name)
{
    if (infoPacket()->cameraName != name) {
        infoPacket()->cameraName = name;
        if (infoPacket()->cameraName.isEmpty())
            infoPacket()->cameraName = "Unknown Camera";

        emit newCameraName (id());
    }
}

/**
 * Updates the \a group reported by the camera
 */
void QCCTV_RemoteCamera::updateGroup (const QString& group)
{
    if (infoPacket()->cameraGroup != group) {
        infoPacket()->cameraGroup = group;
        if (infoPacket()->cameraGroup.isEmpty())
            infoPacket()->cameraGroup = "Default";

        emit newCameraGroup();
    }
}

/**
 * Updates the connection status of the camera and emits the appropiate signals
 */
void QCCTV_RemoteCamera::updateConnected (const bool status)
{
    m_connected = status;

    if (m_connected) {
        emit connected (id());
        emit fpsChanged (id());
        emit resolutionChanged (id());
        emit autoRegulateResolutionChanged (id());
    }
}

/**
 * Changes the zoom support status of the camera and emits the appropiate signals
 */
void QCCTV_RemoteCamera::updateZoomSupport (const bool support)
{
    if (infoPacket()->supportsZoom != support) {
        infoPacket()->supportsZoom = support;
        emit zoomSupportChanged (id());
    }
}

/**
 * Updates the image resolution status of the camera
 */
void QCCTV_RemoteCamera::updateResolution (const int resolution)
{
    if (infoPacket()->resolution != resolution) {
        infoPacket()->resolution = resolution;
        commandPacket()->oldResolution = resolution;
        commandPacket()->newResolution = resolution;
        emit resolutionChanged (id());
    }
}

/**
 * Updates the auto-regulate status flag of the camera
 */
void QCCTV_RemoteCamera::updateAutoRegulate (const bool regulate)
{
    if (infoPacket()->autoRegulateResolution != regulate) {
        infoPacket()->autoRegulateResolution = regulate;
        commandPacket()->oldAutoRegulateResolution = regulate;
        commandPacket()->newAutoRegulateResolution = regulate;
        emit autoRegulateResolutionChanged (id());
    }
}

/**
 * Updates the flashlight status flag of the camera
 */
void QCCTV_RemoteCamera::updateFlashlightEnabled (const bool enabled)
{
    if (infoPacket()->flashlightEnabled != enabled) {
        infoPacket()->flashlightEnabled = enabled;
        commandPacket()->oldFlashlightEnabled = enabled;
        commandPacket()->newFlashlightEnabled = enabled;
        emit lightStatusChanged (id());
    }
}

/**
 * Called when we receive a datagram from the camera, this function
 * obtains the latest image from the camera
 */
void QCCTV_RemoteCamera::readImagePacket()
{
    QCCTV_ImagePacket packet;
    if (QCCTV_ReadImagePacket (&packet, m_data)) {
        /* Clear buffer and send another command packet */
        clearBuffer();
        acknowledgeReception();

        /* Re-assign image */
        imagePacket()->image = packet.image;
        emit newImage (id());

        /* Reset the watchdog */
        if (m_watchdog)
            m_watchdog->reset();

        /* Save image to disk */
        if (saveIncomingMedia()) {
            QtConcurrent::run (m_saver, &QCCTV_ImageSaver::saveImage,
                               incomingMediaPath(),
                               name(),
                               address().toString(),
                               image());
        }
    }
}

/**
 * Resets the watchdog and sends a command packet to the camera, which allows
 * it to know if we are doing OK.
 *
 * If the camera does not receive a command packet after some time, it will
 * try to reduce its image size automatically
 */
void QCCTV_RemoteCamera::acknowledgeReception()
{
    sendCommandPacket();

    if (!isConnected())
        updateConnected (true);
}

/**
 * Returns the pointer to the stream packet structure
 */
QCCTV_InfoPacket* QCCTV_RemoteCamera::infoPacket()
{
    if (m_infoPacket)
        return m_infoPacket;

    m_infoPacket = new QCCTV_InfoPacket;
    return m_infoPacket;
}

/**
 * Returns the pointer to the image packet structure
 */
QCCTV_ImagePacket* QCCTV_RemoteCamera::imagePacket()
{
    if (m_imagePacket)
        return m_imagePacket;

    m_imagePacket = new QCCTV_ImagePacket;
    return m_imagePacket;
}

/**
 * Returns the pointer to the stream command structure
 */
QCCTV_CommandPacket* QCCTV_RemoteCamera::commandPacket()
{
    if (m_commandPacket)
        return m_commandPacket;

    m_commandPacket = new QCCTV_CommandPacket;
    return m_commandPacket;
}

