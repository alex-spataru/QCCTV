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

#ifndef _QCCTV_REMOTE_CAMERA_H
#define _QCCTV_REMOTE_CAMERA_H

#include <QTcpSocket>
#include <QUdpSocket>

class QCCTV_Watchdog;
class QCCTV_ImageSaver;
struct QCCTV_InfoPacket;
struct QCCTV_ImagePacket;
struct QCCTV_CommandPacket;

class QCCTV_RemoteCamera : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void newCameraGroup();
    void newImage (const int id);
    void connected (const int id);
    void fpsChanged (const int id);
    void disconnected (const int id);
    void newCameraName (const int id);
    void newCameraStatus (const int id);
    void zoomLevelChanged (const int id);
    void resolutionChanged (const int id);
    void lightStatusChanged (const int id);
    void zoomSupportChanged (const int id);
    void autoRegulateResolutionChanged (const int id);

public:
    QCCTV_RemoteCamera (QObject* parent = NULL);
    ~QCCTV_RemoteCamera();

    int fps();
    int zoom();
    int status();
    QImage image();
    QString name();
    QString group();
    int resolution();
    bool supportsZoom();
    QString statusString();
    bool flashlightEnabled();
    bool autoRegulateResolution();

    int id() const;
    bool isConnected() const;
    QHostAddress address() const;
    bool saveIncomingMedia() const;
    QString incomingMediaPath() const;

public Q_SLOTS:
    void start();
    void requestFocus();
    void changeID (const int id);
    void changeFPS (const int fps);
    void changeZoom (const int zoom);
    void setSaveIncomingMedia (const bool save);
    void readInfoPacket (const QByteArray& data);
    void changeResolution (const int resolution);
    void setAddress (const QHostAddress& address);
    void changeAutoRegulate (const bool regulate);
    void changeFlashlightStatus (const int status);
    void setIncomingMediaPath (const QString& path);

private Q_SLOTS:
    void clearBuffer();
    void endConnection();
    void sendCommandPacket();
    void resetFocusRequest();
    void onImageDataReceived();
    void updateFPS (const int fps);
    void updateZoom (const int zoom);
    void updateStatus (const int status);
    void updateName (const QString& name);
    void updateGroup (const QString& group);
    void updateConnected (const bool status);
    void updateZoomSupport (const bool support);
    void updateResolution (const int resolution);
    void updateAutoRegulate (const bool regulate);
    void updateFlashlightEnabled (const bool enabled);

private:
    void readImagePacket();
    void acknowledgeReception();
    QCCTV_InfoPacket* infoPacket();
    QCCTV_ImagePacket* imagePacket();
    QCCTV_CommandPacket* commandPacket();

private:
    int m_id;
    bool m_connected;
    QByteArray m_data;
    QHostAddress m_address;
    QString m_incomingMediaPath;
    bool m_saveIncomingMedia;

    QTcpSocket* m_socket;
    QUdpSocket* m_commandSocket;

    QCCTV_ImageSaver* m_saver;
    QCCTV_Watchdog* m_watchdog;

    QCCTV_InfoPacket* m_infoPacket;
    QCCTV_ImagePacket* m_imagePacket;
    QCCTV_CommandPacket* m_commandPacket;
};

#endif
