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

#ifndef _QCCTV_CAMERA_H
#define _QCCTV_CAMERA_H

#include <QTimer>
#include <QImage>
#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>

#include "QCCTV.h"

class QCCTV_RemoteCamera : public QObject
{
    Q_OBJECT

signals:
    void newImage (const int id);
    void connected (const int id);
    void disconnected (const int id);
    void newCameraName (const int id);
    void newCameraGroup (const int id);
    void newLightStatus (const int id);
    void newCameraStatus (const int id);

public:
    QCCTV_RemoteCamera();
    ~QCCTV_RemoteCamera();

    Q_INVOKABLE int id() const;
    Q_INVOKABLE int fps() const;
    Q_INVOKABLE QString group() const;
    Q_INVOKABLE int cameraStatus() const;
    Q_INVOKABLE QString cameraName() const;
    Q_INVOKABLE QImage currentImage() const;
    Q_INVOKABLE QString statusString() const;
    Q_INVOKABLE QHostAddress address() const;
    Q_INVOKABLE QCCTV_LightStatus lightStatus() const;

public slots:
    void requestFocus();
    void turnOnFlashlight();
    void turnOffFlashlight();
    void setID (const int id);
    void setFPS (const int fps);
    void changeFlashlightStatus (const int status);
    void attemptConnection (const QHostAddress& address);

private slots:
    void sendData();
    void readData();
    void sendRequest();
    void onCameraTimeout();
    void resetFocusRequest();
    void setName (const QString& name);
    void setGroup (const QString& group);
    void changeCameraStatus (const int status);

private:
    int m_id;
    int m_fps;
    bool m_focus;
    QString m_name;
    QImage m_image;
    QString m_group;
    bool m_connected;
    int m_cameraStatus;
    int m_requestPackets;
    QHostAddress m_address;
    QCCTV_LightStatus m_lightStatus;

    QUdpSocket m_sender;
    QUdpSocket m_receiver;
};

#endif
