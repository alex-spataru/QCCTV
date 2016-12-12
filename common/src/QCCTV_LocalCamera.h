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

#ifndef _QCCTV_LOCAL_CAMERA_H
#define _QCCTV_LOCAL_CAMERA_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>

#include "QCCTV.h"
#include "QCCTV_CRC32.h"
#include "QCCTV_Watchdog.h"
#include "QCCTV_ImageCapture.h"

class QCamera;
class QCameraImageCapture;

class QCCTV_LocalCamera : public QObject
{
    Q_OBJECT

signals:
    void fpsChanged();
    void imageChanged();
    void groupNameChanged();
    void hostCountChanged();
    void resolutionChanged();
    void cameraNameChanged();
    void lightStatusChanged();
    void focusStatusChanged();
    void cameraStatusChanged();
    void autoRegulateResolutionChanged();

public:
    QCCTV_LocalCamera();
    ~QCCTV_LocalCamera();

    Q_INVOKABLE int fps() const;
    Q_INVOKABLE int resolution() const;
    Q_INVOKABLE int minimumFPS() const;
    Q_INVOKABLE int maximumFPS() const;
    Q_INVOKABLE int cameraStatus() const;
    Q_INVOKABLE QString cameraName() const;
    Q_INVOKABLE QString cameraGroup() const;
    Q_INVOKABLE QImage currentImage() const;
    Q_INVOKABLE QString statusString() const;
    Q_INVOKABLE bool readyForCapture() const;
    Q_INVOKABLE bool flashlightEnabled() const;
    Q_INVOKABLE bool flashlightAvailable() const;
    Q_INVOKABLE QStringList connectedHosts() const;
    Q_INVOKABLE bool autoRegulateResolution() const;
    Q_INVOKABLE QStringList availableResolutions() const;

public slots:
    void takePhoto();
    void focusCamera();
    void setFPS (const int fps);
    void setCamera (QCamera* camera);
    void setName (const QString& name);
    void setGroup (const QString& group);
    void setResolution (const int resolution);
    void setFlashlightEnabled (const bool enabled);
    void setAutoRegulateResolution (const bool regulate);

private slots:
    void update();
    void broadcastInfo();
    void onDisconnected();
    void acceptConnection();
    void readCommandPacket();
    void onWatchdogTimeout();
    void changeImage (const QImage& image);

private:
    void updateStatus();
    void generateData();
    void sendCameraData();
    void addStatusFlag (const QCCTV_CameraStatus status);
    void setCameraStatus (const QCCTV_CameraStatus status);
    void removeStatusFlag (const QCCTV_CameraStatus status);
    void setFlashlightStatus (const QCCTV_LightStatus status);

private:
    int m_fps;
    int m_frame;
    int m_cameraStatus;
    int m_flashlightStatus;

    bool m_autoRegulateResolution;

    QImage m_image;
    QString m_name;
    QString m_group;
    QByteArray m_data;
    QByteArray m_imageData;

    QCamera* m_camera;
    QCameraImageCapture* m_capture;

    QCCTV_CRC32 m_crc32;
    QCCTV_Resolution m_resolution;
    QCCTV_ImageCapture m_imageCapture;

    QTcpServer m_server;
    QUdpSocket m_cmdSocket;
    QUdpSocket m_broadcastSocket;
    QList<QTcpSocket*> m_sockets;
    QList<QCCTV_Watchdog*> m_watchdogs;
};

#endif
