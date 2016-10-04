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

#include <QImage>
#include <QObject>

#include "QCCTV.h"

class QCCTV_LocalCamera : public QObject
{
    Q_OBJECT
    Q_ENUM (QCCTV_LightStatus)
    Q_ENUM (QCCTV_CameraStatus)

signals:
    void connected();
    void disconnected();
    void newImageRecorded();
    void cameraNameChanged();
    void lightStatusChanged();
    void focusStatusChanged();
    void cameraStatusChanged();

public:
    QCCTV_LocalCamera();
    ~QCCTV_LocalCamera();

    Q_INVOKABLE int fps() const;
    Q_INVOKABLE QString cameraName() const;
    Q_INVOKABLE QString cameraGroup() const;
    Q_INVOKABLE QImage currentImage() const;
    Q_INVOKABLE QCCTV_LightStatus lightStatus() const;
    Q_INVOKABLE QCCTV_CameraStatus cameraStatus() const;

public slots:
    void focusCamera();
    void setFPS (const int fps);
    void setName (const QString& name);
    void setGroup (const QString& group);
    void setLightStatus (const QCCTV_LightStatus status);
    void setCameraStatus (const QCCTV_CameraStatus status);

private slots:
    void sendCameraData();
    void broadcastInformation();

private:
    QByteArray createCameraPacket();
    QByteArray createBroadcastPacket();

    int m_fps;
    QString m_cameraName;
    QString m_cameraGroup;
    QCCTV_LightStatus m_lightStatus;
    QCCTV_CameraStatus m_cameraStatus;
};

#endif
