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
#include "QCCTV_FrameGrabber.h"

class QCamera;
class QCameraImageCapture;

class QCCTV_LocalCamera : public QObject
{
    Q_OBJECT

signals:
    void fpsChanged();
    void imageChanged();
    void cameraNameChanged();
    void lightStatusChanged();
    void focusStatusChanged();
    void cameraStatusChanged();

public:
    QCCTV_LocalCamera();
    ~QCCTV_LocalCamera();

    Q_INVOKABLE int fps() const;
    Q_INVOKABLE int cameraStatus() const;
    Q_INVOKABLE bool flashlightOn() const;
    Q_INVOKABLE bool flashlightOff() const;
    Q_INVOKABLE QString cameraName() const;
    Q_INVOKABLE QImage currentImage() const;
    Q_INVOKABLE QString statusString() const;
    Q_INVOKABLE bool readyForCapture() const;
    Q_INVOKABLE bool flashlightAvailable() const;
    Q_INVOKABLE QStringList connectedHosts() const;
    Q_INVOKABLE QCCTV_LightStatus lightStatus() const;

    Q_INVOKABLE bool isGrayscale() const;
    Q_INVOKABLE qreal shrinkRatio() const;

public slots:
    void takePhoto();
    void focusCamera();
    void turnOnFlashlight();
    void turnOffFlashlight();
    void setFPS (const int fps);
    void setCamera (QCamera* camera);
    void setName (const QString& name);
    void setGrayscale (const bool gray);
    void setShrinkRatio (const qreal ratio);

private slots:
    void update();
    void generateData();
    void updateStatus();
    void broadcastInfo();
    void sendCameraData();
    void onDisconnected();
    void onWatchdogTimeout();
    void acceptConnection();
    void readCommandPacket();
    void changeImage (const QImage& image);
    void addStatusFlag (const QCCTV_CameraStatus status);
    void setCameraStatus (const QCCTV_CameraStatus status);
    void removeStatusFlag (const QCCTV_CameraStatus status);
    void setFlashlightStatus (const QCCTV_LightStatus status);

private:
    int m_fps;
    int m_cameraStatus;
    int m_flashlightStatus;

    QImage m_image;
    QString m_name;
    QByteArray m_data;
    QByteArray m_imageData;

    QCCTV_CRC32 m_crc32;
    QCamera* m_camera;
    QCameraImageCapture* m_capture;
    QCCTV_FrameGrabber m_frameGrabber;

    QUdpSocket m_cmdSocket;

    QTcpServer m_server;
    QUdpSocket m_broadcastSocket;
    QList<QTcpSocket*> m_sockets;
    QList<QCCTV_Watchdog*> m_watchdogs;
};


#endif
