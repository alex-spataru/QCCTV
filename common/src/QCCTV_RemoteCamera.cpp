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

#include <QDir>
#include <QPen>
#include <QFont>
#include <QFile>
#include <QPainter>
#include <QDateTime>
#include <QApplication>
#include <QFontDatabase>

#if defined Q_OS_MAC
    #define MONOSPACE_FONT "Menlo"
#elif defined Q_OS_WIN
    #define MONOSPACE_FONT "Consolas"
#else
    #define MONOSPACE_FONT "Monospace"
#endif

QCCTV_RemoteCamera::QCCTV_RemoteCamera (QObject* parent) :
    QObject (parent),
    m_id (0),
    m_quality (-1),
    m_focus (false),
    m_watchdog (NULL),
    m_group ("Unknown"),
    m_connected (false),
    m_oldAutoRegulate (true),
    m_newAutoRegulate (true),
    m_name ("Unknown Camera"),
    m_saveIncomingMedia (true),
    m_oldFPS (QCCTV_DEFAULT_FPS),
    m_newFPS (QCCTV_DEFAULT_FPS),
    m_oldResolution (QCCTV_DEFAULT_RES),
    m_newResolution (QCCTV_DEFAULT_RES),
    m_cameraStatus (QCCTV_CAMSTATUS_DEFAULT),
    m_oldFlashlightStatus (QCCTV_FLASHLIGHT_OFF),
    m_newFlashlightStatus (QCCTV_FLASHLIGHT_OFF),
    m_image (QCCTV_GET_STATUS_IMAGE (QSize (640, 480), "NO CAMERA IMAGE"))
{
    setRecordingsPath ("");
    connect (&m_socket,   SIGNAL (readyRead()),
             this,          SLOT (onDataReceived()));
    connect (&m_socket,   SIGNAL (disconnected()),
             this,          SLOT (endConnection()));
}

/**
 * Close the TCP sockets during the destruction of the class
 */
QCCTV_RemoteCamera::~QCCTV_RemoteCamera()
{
    m_socket.close();
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
    return m_oldFPS;
}

/**
 * Returns the operation status reported by the camera
 */
int QCCTV_RemoteCamera::status() const
{
    return m_cameraStatus;
}

/**
 * Returns the latest image captured by the camera
 */
QImage QCCTV_RemoteCamera::image() const
{
    return m_image;
}

/**
 * Returns the name of the camera
 */
QString QCCTV_RemoteCamera::name() const
{
    return m_name;
}

/**
 * Returns the group assigned to the camera
 */
QString QCCTV_RemoteCamera::group() const
{
    return m_group;
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
 * Returns \c true if the camera is allowed to autoregulate its resolution
 */
bool QCCTV_RemoteCamera::autoRegulateResolution() const
{
    return m_oldAutoRegulate;
}

/**
 * Returns the current resolution used by the camera
 */
QCCTV_Resolution QCCTV_RemoteCamera::resolution() const
{
    return (QCCTV_Resolution) m_oldResolution;
}

/**
 * Returns the light status used by the camera
 */
QCCTV_LightStatus QCCTV_RemoteCamera::lightStatus() const
{
    return (QCCTV_LightStatus) m_oldFlashlightStatus;
}

/**
 * Initializes the watchdog timers after the thread has been created
 */
void QCCTV_RemoteCamera::start()
{
    m_watchdog = new QCCTV_Watchdog (this);
    m_watchdog->setExpirationTime (QCCTV_MIN_WATCHDOG_TIME);

    connect (m_watchdog, SIGNAL (expired()), this, SLOT (clearBuffer()));
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

    QTimer timer;
    timer.singleShot (500, this, SLOT (resetFocusRequest()));
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
    m_newFPS = QCCTV_GET_VALID_FPS (fps);

    if (m_watchdog)
        m_watchdog->setExpirationTime (QCCTV_WATCHDOG_TIME (m_newFPS));
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
 * Changes the path in which we should save incoming images
 */
void QCCTV_RemoteCamera::setRecordingsPath (const QString& path)
{
    if (!path.isEmpty())
        m_recordingsPath = path;
    else
        m_recordingsPath = QDir::homePath() + "/QCCTV/Recordings/";
}

/**
 * Changes the resoltion of the camera
 */
void QCCTV_RemoteCamera::changeResolution (const int resolution)
{
    m_newResolution = resolution;
}

/**
 * Changes the camera's remote address
 */
void QCCTV_RemoteCamera::setAddress (const QHostAddress& address)
{
    if (m_address == address)
        return;

    m_address = address;
    m_socket.disconnectFromHost();
    m_socket.connectToHost (address, QCCTV_STREAM_PORT);
    m_socket.setSocketOption (QTcpSocket::LowDelayOption, 1);
    m_socket.setSocketOption (QTcpSocket::KeepAliveOption, 1);
    m_socket.setSocketOption (QTcpSocket::ReceiveBufferSizeSocketOption, INT_MAX);
}

/**
 * Allows or disallows the camera from autoregulating its resolution
 */
void QCCTV_RemoteCamera::changeAutoRegulate (const bool regulate)
{
    m_newAutoRegulate = regulate;
}

/**
 * Changes the flashlight status of the camera and emits the appropiate signals
 */
void QCCTV_RemoteCamera::changeFlashlightStatus (const int status)
{
    m_newFlashlightStatus = status;
}

/**
 * Called when we stop receiving constant packets from the camera, this
 * function deletes the temporary data buffer to avoid storing too much
 * information that we cannot use directly
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
    m_data.clear();
    m_socket.abort();

    updateConnected (false);
    emit disconnected (id());
}


/**
 * Obtains the information received by the TCP socket and calls the functions
 * neccessary to interpret the received data
 */
void QCCTV_RemoteCamera::onDataReceived()
{
    m_data.append (m_socket.readAll());

    if (!m_data.isEmpty())
        readCameraPacket();

    if (m_data.size() >= QCCTV_MAX_BUFFER_SIZE)
        m_data.clear();
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
 * Sends a command packet to the camera, which instructs it to:
 *
 * - Change its FPS
 * - Change its light status
 * - Focus the camera device (if required)
 */
void QCCTV_RemoteCamera::sendCommandPacket()
{
    /* Generate the data */
    QByteArray data;
    data.append (m_oldFPS);
    data.append (m_newFPS);
    data.append (m_oldResolution);
    data.append (m_newResolution);
    data.append (m_oldFlashlightStatus);
    data.append (m_newFlashlightStatus);
    data.append (m_focus ? QCCTV_FORCE_FOCUS : 0x00);
    data.append (m_oldAutoRegulate ? QCCTV_AUTOREGULATE_RES : 0x00);
    data.append (m_newAutoRegulate ? QCCTV_AUTOREGULATE_RES : 0x00);

    /* Send the generated data */
    m_commandSocket.writeDatagram (data, address(), QCCTV_COMMAND_PORT);
}

/**
 * Updates the \a fps reported by the camera
 */
void QCCTV_RemoteCamera::updateFPS (const int fps)
{
    if (m_oldFPS != fps) {
        m_oldFPS = fps;
        m_newFPS = fps;
        emit fpsChanged (id());
    }
}

/**
 * Updates the operation status of the camera and emits the appropiate signals
 */
void QCCTV_RemoteCamera::updateStatus (const int status)
{
    if (m_cameraStatus != status) {
        m_cameraStatus = status;
        emit newCameraStatus (id());
    }
}

/**
 * Updates the \a name reported by the camera
 */
void QCCTV_RemoteCamera::updateName (const QString& name)
{
    if (m_name != name) {
        m_name = name;

        if (m_name.isEmpty())
            m_name = "Unknown Camera";

        emit newCameraName (id());
    }
}

/**
 * Updates the \a group reported by the camera
 */
void QCCTV_RemoteCamera::updateGroup (const QString& group)
{
    if (m_group != group) {
        m_group = group;

        if (m_group.isEmpty())
            m_group = "Default";

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
    if (m_oldResolution != resolution) {
        m_newResolution = resolution;
        m_oldResolution = resolution;
        emit resolutionChanged (id());
    }
}

/**
 * Updates the auto-regulate status flag of the camera
 */
void QCCTV_RemoteCamera::updateAutoRegulate (const bool regulate)
{
    if (m_oldAutoRegulate != regulate) {
        m_newAutoRegulate = regulate;
        m_oldAutoRegulate = regulate;
        emit autoRegulateResolutionChanged (id());
    }
}

/**
 * Updates the flashlight status flag of the camera
 */
void QCCTV_RemoteCamera::updateFlashlightStatus (const int status)
{
    if (m_oldFlashlightStatus != status) {
        m_newFlashlightStatus = status;
        m_oldFlashlightStatus = status;
        emit lightStatusChanged (id());
    }
}

/**
 * Called when we receive a datagram from the camera, this function interprets
 * the status bytes and the image data contained in the received packet.
 *
 * The stream packets (the ones that we receive from the camera) have the
 * following structure:
 *
 * - CRC32 (4 bytes)
 * - Length of camera name (1 byte)
 * - Camera name string
 * - Camera FPS (1 byte)
 * - Light status (1 byte)
 * - Operation status (1 byte)
 * - Image length (3 bytes)
 * - Compressed image data
 */
void QCCTV_RemoteCamera::readCameraPacket()
{
    /* Data is invalid */
    if (m_data.isEmpty())
        return;

    /* Data is too small to read checksum */
    if (m_data.size() < 4)
        return;

    /* Get the checksum */
    quint8 a = m_data.at (0);
    quint8 b = m_data.at (1);
    quint8 c = m_data.at (2);
    quint8 d = m_data.at (3);
    quint32 checksum = (a << 24) | (b << 16) | (c << 8) | (d & 0xff);

    /* Create byte array without checksum header */
    QByteArray stream = m_data;
    stream.remove (0, 4);

    /* Stream is too small */
    if (stream.isEmpty())
        return;

    /* Compare checksums */
    quint32 crc = m_crc32.compute (stream);
    if (checksum != crc)
        return;

    /* Uncompress the stream data */
    int offset = 0;
    stream = qUncompress (stream);

    /* Get camera name */
    QString name;
    int name_len = stream.at (0);
    for (int i = 0; i < name_len; ++i) {
        int pos = 1 + i;
        if (stream.size() > pos)
            name.append (stream.at (pos));

        else
            return;
    }

    /* Get camera group */
    QString group;
    int group_len = stream.at (name_len + 1);
    for (int i = 0; i < group_len; ++i) {
        int pos = name_len + 2 + i;
        if (stream.size() > pos)
            group.append (stream.at (pos));

        else
            return;
    }

    /* Set offset value */
    offset = name_len + group_len + 1;

    /* Get camera information  */
    quint8 fps = stream.at (offset + 1);
    quint8 light = stream.at (offset + 2);
    quint8 status = stream.at (offset + 3);
    quint8 resolution = stream.at (offset + 4);
    quint8 autoregulate = stream.at (offset + 5);

    /* Update values */
    updateFPS (fps);
    updateName (name);
    updateGroup (group);
    updateStatus (status);
    updateResolution (resolution);
    updateFlashlightStatus (light);

    /* Update auto-regulate option */
    if (autoregulate == QCCTV_AUTOREGULATE_RES)
        updateAutoRegulate (true);
    else
        updateAutoRegulate (false);

    /* Get image length */
    quint8 img_a = stream.at (offset + 6);
    quint8 img_b = stream.at (offset + 7);
    quint8 img_c = stream.at (offset + 8);
    quint32 img_len = (img_a << 16) | (img_b << 8) | (img_c & 0xff);

    /* Get image bytes */
    QByteArray raw_image;
    for (quint32 i = 0; i < img_len; ++i) {
        int pos = offset + 9 + i;
        if (stream.size() > pos)
            raw_image.append (stream [pos]);
        else
            return;
    }

    /* Decode the image and save it */
    QImage img = QCCTV_DECODE_IMAGE (raw_image);
    if (!img.isNull()) {
        m_image = img;
        saveImage (img);
        emit newImage (id());
    }

    /* Reset data buffer */
    m_data.clear();
    acknowledgeReception();
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
 * Writes the current date and time on the top-left corner of the given image
 */
void QCCTV_RemoteCamera::saveImage (QImage& image)
{
    /* Construct strings */
    QDateTime current = QDateTime::currentDateTime();
    QString fmt = current.toString ("dd/MMM/yyyy hh:mm:ss:zzz");
    QString str = name() + "\n" + fmt;

    /* Get font */
    QFont font;
    font.setFamily (MONOSPACE_FONT);
    font.setPixelSize (qMax (image.height() / 24, 9));
    QFontMetrics fm (font);

    /* Get text location */
    QRect rect (fm.height() / 2, fm.height() / 2,
                image.width(), image.height());

    /* Paint text over image */
    QPainter painter (&image);
    painter.setFont (font);
    painter.setPen (QPen (Qt::green));
    painter.drawText (rect, Qt::AlignTop | Qt::AlignLeft, str);

    /* We are not allowed to save the image, we are done for now */
    if (!m_saveIncomingMedia)
        return;

    /* Get recordings directory */
    QString path = QString ("%1/%2/%3/%4/%5/%5 %6/%7 Hours/Minute %8/")
                   .arg (m_recordingsPath)
                   .arg (name())
                   .arg (address().toString())
                   .arg (current.toString ("yyyy"))
                   .arg (current.toString ("MMM"))
                   .arg (current.toString ("dd"))
                   .arg (current.toString ("hh"))
                   .arg (current.toString ("mm"));

    /* Create directory if it does not exist */
    QDir dir = QDir (path);
    if (!dir.exists())
        dir.mkpath (".");

    /* Get image name (based on seconds & msecs) */
    QString name = QString ("%1 sec %2 ms.%3")
                   .arg (current.toString ("ss"))
                   .arg (current.toString ("zzz"))
                   .arg (QCCTV_IMAGE_FORMAT);

    /* Save image */
    image.save (dir.absoluteFilePath (name), QCCTV_IMAGE_FORMAT, m_quality);
}
