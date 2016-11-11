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

#include <QPen>
#include <QFont>
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

QCCTV_RemoteCamera::QCCTV_RemoteCamera()
{
    /* Initialize default variables */
    m_id = 0;
    m_focus = false;
    m_connected = false;
    m_oldAutoRegulate = true;
    m_newAutoRegulate = true;
    m_name = "Unknown Camera";
    m_newFPS = QCCTV_DEFAULT_FPS;
    m_oldFPS = QCCTV_DEFAULT_FPS;
    m_newResolution = QCCTV_DEFAULT_RES;
    m_oldResolution = QCCTV_DEFAULT_RES;
    m_lightStatus = QCCTV_FLASHLIGHT_OFF;
    m_cameraStatus = QCCTV_CAMSTATUS_DEFAULT;

    /* Configure the socket */
    connect (&m_watchdog, SIGNAL (expired()),
             this,          SLOT (clearBuffer()));
    connect (&m_socket,   SIGNAL (readyRead()),
             this,          SLOT (onDataReceived()));
    connect (&m_socket,   SIGNAL (disconnected()),
             this,          SLOT (endConnection()));

    /* Start saving video to hard disk */
    saveVideoRecording();

    /* Set default image & configure watchdog */
    m_watchdog.setExpirationTime (QCCTV_MIN_WATCHDOG_TIME);
    m_image = QCCTV_GET_STATUS_IMAGE (QSize (640, 480), "NO CAMERA IMAGE");
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
int QCCTV_RemoteCamera::cameraStatus() const
{
    return m_cameraStatus;
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
    return m_lightStatus;
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
    QTimer::singleShot (500, Qt::PreciseTimer,
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
    m_watchdog.setExpirationTime (QCCTV_WATCHDOG_TIME (m_newFPS));
}

/**
 * Changes the flashlight status of the camera and emits the appropiate signals
 */
void QCCTV_RemoteCamera::setFlashlightStatus (const int status)
{
    if (m_lightStatus != status) {
        m_lightStatus = (QCCTV_LightStatus) status;
        emit newLightStatus (id());
    }
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
    data.append (m_lightStatus);
    data.append (m_focus ? QCCTV_FORCE_FOCUS : 0x00);
    data.append (m_oldAutoRegulate ? QCCTV_AUTOREGULATE_RES : 0x00);
    data.append (m_newAutoRegulate ? QCCTV_AUTOREGULATE_RES : 0x00);

    /* Send the generated data */
    m_commandSocket.writeDatagram (data, address(), QCCTV_COMMAND_PORT);
}

/**
 * Saves a video recording of the camera every 1 second
 * \todo everything
 */
void QCCTV_RemoteCamera::saveVideoRecording()
{
    if (!m_images.isEmpty())
        m_images.clear();

    QTimer::singleShot (1000, Qt::PreciseTimer,
                        this, SLOT (saveVideoRecording()));
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
    stream = qUncompress (stream);

    /* Get camera name */
    QString name;
    int name_len = stream.at (0);
    for (int i = 0; i < name_len; ++i) {
        int pos = 1 + i;
        if (stream.size() > pos)
            name.append (stream.at (1 + i));

        else
            return;
    }

    /* Get camera information  */
    quint8 fps = stream.at (name_len + 1);
    quint8 light = stream.at (name_len + 2);
    quint8 status = stream.at (name_len + 3);
    quint8 resolution = stream.at (name_len + 4);
    quint8 autoregulate = stream.at (name_len + 5);

    /* Update values */
    updateFPS (fps);
    updateName (name);
    updateStatus (status);
    setFlashlightStatus (light);
    updateResolution (resolution);

    /* Update auto-regulate option */
    if (autoregulate == QCCTV_AUTOREGULATE_RES)
        updateAutoRegulate (true);
    else
        updateAutoRegulate (false);

    /* Get image length */
    quint8 img_a = stream.at (name_len + 6);
    quint8 img_b = stream.at (name_len + 7);
    quint8 img_c = stream.at (name_len + 8);
    quint32 img_len = (img_a << 16) | (img_b << 8) | (img_c & 0xff);

    /* Get image bytes */
    QByteArray raw_image;
    for (quint32 i = 0; i < img_len; ++i) {
        int pos = name_len + 9 + i;
        if (stream.size() > pos)
            raw_image.append (stream [pos]);
        else
            return;
    }

    /* Decode image */
    QImage img = QCCTV_DECODE_IMAGE (raw_image);
    if (!img.isNull()) {
        m_image = img;
        m_images.append (addCurrentDateTime (img));
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
    m_watchdog.reset();
    sendCommandPacket();

    if (!isConnected())
        updateConnected (true);
}

/**
 * Writes the current date and time on the top-left corner of the given image
 */
QImage QCCTV_RemoteCamera::addCurrentDateTime (QImage& image)
{
    /* Construct strings */
    QDateTime current = QDateTime::currentDateTimeUtc();
    QString utc = current.toString ("dd/MMM/yyyy hh:mm:ss:zzz UTC");
    QString str = cameraName() + "\n" + utc;

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
    painter.setPen (QPen (Qt::white));
    painter.drawText (rect, Qt::AlignTop | Qt::AlignLeft, str);

    /* Return final image */
    return image;
}
