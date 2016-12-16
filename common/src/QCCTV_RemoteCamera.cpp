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

#include "QCCTV.h"
#include "QCCTV_Watchdog.h"
#include "QCCTV_ImageSaver.h"
#include "QCCTV_RemoteCamera.h"
#include "QCCTV_Communications.h"

QCCTV_RemoteCamera::QCCTV_RemoteCamera (QObject* parent) : QObject (parent)
{
    m_id = 0;
    m_quality = -1;
    m_connected = false;
    m_saveIncomingMedia = false;
    m_saver = new QCCTV_ImageSaver (this);
    m_streamPacket = new QCCTV_StreamPacket;
    m_commandPacket = new QCCTV_CommandPacket;

    setIncomingMediaPath ("");
    QCCTV_InitStream (streamPacket());
    QCCTV_InitCommand (commandPacket(), streamPacket());
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
    delete m_streamPacket;
    delete m_commandPacket;
}

/**
 * Returns the FPS set by the station or by the camera itself
 */
int QCCTV_RemoteCamera::fps()
{
    return streamPacket()->fps;
}

/**
 * Returns the operation status reported by the camera
 */
int QCCTV_RemoteCamera::status()
{
    return streamPacket()->cameraStatus;
}

/**
 * Returns the latest image captured by the camera
 */
QImage QCCTV_RemoteCamera::image()
{
    return streamPacket()->image;
}

/**
 * Returns the name of the camera
 */
QString QCCTV_RemoteCamera::name()
{
    return streamPacket()->cameraName;
}

/**
 * Returns the group assigned to the camera
 */
QString QCCTV_RemoteCamera::group()
{
    return streamPacket()->cameraGroup;
}

/**
 * Returns the current resolution used by the camera
 */
int QCCTV_RemoteCamera::resolution()
{
    return streamPacket()->resolution;
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
    return streamPacket()->flashlightEnabled;
}

/**
 * Returns \c true if the camera is allowed to autoregulate its resolution
 */
bool QCCTV_RemoteCamera::autoRegulateResolution()
{
    return streamPacket()->autoRegulateResolution;
}

/**
 * Returns the camera ID set by the station
 */
int QCCTV_RemoteCamera::id() const
{
    return m_id;
}

/**
 * Returns the quality level that is applied to saved images from the
 * camera stream. The quality range is from 0 to 100, or -1 if we should
 * let Qt decide by itself the image quality
 */
int QCCTV_RemoteCamera::imageQuality() const
{
    return m_quality;
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
    connect (m_socket, SIGNAL (readyRead()),
             this,       SLOT (onDataReceived()));
    connect (m_socket, SIGNAL (disconnected()),
             this,       SLOT (endConnection()));

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
 * Changes the image quality (from 0 to 100) used when saving incoming media
 */
void QCCTV_RemoteCamera::setImageQuality (const int quality)
{
    if (quality == 0)
        m_quality = -1;
    else
        m_quality = quality;
}

/**
 * Allows or disallows saving the incoming images to the disk
 */
void QCCTV_RemoteCamera::setSaveIncomingMedia (const bool save)
{
    m_saveIncomingMedia = save;
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
 * This function is also called when the class successfully reads a stream
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
 * Obtains the information received by the TCP socket and calls the functions
 * neccessary to interpret the received data
 */
void QCCTV_RemoteCamera::onDataReceived()
{
    if (!m_socket) {
        clearBuffer();
        return;
    }

    else {
        m_data.append (m_socket->readAll());

        if (!m_data.isEmpty())
            readCameraPacket();

        if (m_data.size() >= QCCTV_MAX_BUFFER_SIZE)
            clearBuffer();
    }
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
 * Sends a command packet to the camera, which instructs it to:
 *
 * - Change its FPS
 * - Change its light status
 * - Focus the camera device (if required)
 */
void QCCTV_RemoteCamera::sendCommandPacket()
{
    QByteArray data = QCCTV_CreateCommandPacket (*commandPacket());

    if (m_commandSocket)
        m_commandSocket->writeDatagram (data, address(), QCCTV_COMMAND_PORT);
}

/**
 * Updates the \a fps reported by the camera
 */
void QCCTV_RemoteCamera::updateFPS (const int fps)
{
    if (streamPacket()->fps != fps) {
        streamPacket()->fps = fps;
        commandPacket()->oldFps = fps;
        commandPacket()->newFps = fps;
        emit fpsChanged (id());
    }
}

/**
 * Updates the operation status of the camera and emits the appropiate signals
 */
void QCCTV_RemoteCamera::updateStatus (const int status)
{
    if (streamPacket()->cameraStatus != status) {
        streamPacket()->cameraStatus = status;
        emit newCameraStatus (id());
    }
}

/**
 * Updates the \a name reported by the camera
 */
void QCCTV_RemoteCamera::updateName (const QString& name)
{
    if (streamPacket()->cameraName != name) {
        streamPacket()->cameraName = name;
        if (streamPacket()->cameraName.isEmpty())
            streamPacket()->cameraName = "Unknown Camera";

        emit newCameraName (id());
    }
}

/**
 * Updates the \a group reported by the camera
 */
void QCCTV_RemoteCamera::updateGroup (const QString& group)
{
    if (streamPacket()->cameraGroup != group) {
        streamPacket()->cameraGroup = group;
        if (streamPacket()->cameraGroup.isEmpty())
            streamPacket()->cameraGroup = "Default";

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
 * Updates the image resolution status of the camera
 */
void QCCTV_RemoteCamera::updateResolution (const int resolution)
{
    if (streamPacket()->resolution != resolution) {
        streamPacket()->resolution = resolution;
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
    if (streamPacket()->autoRegulateResolution != regulate) {
        streamPacket()->autoRegulateResolution = regulate;
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
    if (streamPacket()->flashlightEnabled != enabled) {
        streamPacket()->flashlightEnabled = enabled;
        commandPacket()->oldFlashlightEnabled = enabled;
        commandPacket()->newFlashlightEnabled = enabled;
        emit lightStatusChanged (id());
    }
}

/**
 * Called when we receive a datagram from the camera, this function interprets
 * the status bytes and the image data contained in the received packet.
 */
void QCCTV_RemoteCamera::readCameraPacket()
{
    QCCTV_StreamPacket packet;
    if (QCCTV_ReadStreamPacket (&packet, m_data)) {
        /* Update stream information */
        updateFPS (packet.fps);
        updateName (packet.cameraName);
        updateGroup (packet.cameraGroup);
        updateStatus (packet.cameraStatus);
        updateResolution (packet.resolution);
        updateAutoRegulate (packet.autoRegulateResolution);
        updateFlashlightEnabled (packet.flashlightEnabled);

        /* Update image */
        streamPacket()->image = packet.image;
        emit newImage (id());

        /* Save the image (if allowed) */
        if (saveIncomingMedia())
            m_saver->saveImage (incomingMediaPath(),
                                name(),
                                address().toString(),
                                image(),
                                imageQuality());

        /* Clear buffer and send command packet */
        clearBuffer();
        acknowledgeReception();
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
    if (m_watchdog)
        m_watchdog->reset();

    sendCommandPacket();

    if (!isConnected())
        updateConnected (true);
}

/**
 * Returns the pointer to the stream packet structure
 */
QCCTV_StreamPacket* QCCTV_RemoteCamera::streamPacket()
{
    if (m_streamPacket)
        return m_streamPacket;

    m_streamPacket = new QCCTV_StreamPacket;
    return m_streamPacket;
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

