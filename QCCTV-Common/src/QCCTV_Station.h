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

#ifndef _QCCTV_STATION_H
#define _QCCTV_STATION_H

#include "QCCTV.h"
#include "QCCTV_RemoteCamera.h"

#include <QUdpSocket>

class QCCTV_Station : public QObject
{
    Q_OBJECT

signals:
    void cameraCountChanged();
    void connected (const int camera);
    void fpsChanged (const int camera);
    void disconnected (const int camera);
    void newCameraImage (const int camera);
    void cameraNameChanged (const int camera);
    void lightStatusChanged (const int camera);
    void cameraStatusChanged (const int camera);

public:
    QCCTV_Station();
    ~QCCTV_Station();

    Q_INVOKABLE int cameraCount() const;
    Q_INVOKABLE int fps (const int camera);
    Q_INVOKABLE int cameraStatus (const int camera);
    Q_INVOKABLE QString cameraName (const int camera);
    Q_INVOKABLE QImage currentImage (const int camera);
    Q_INVOKABLE QHostAddress address (const int camera);
    Q_INVOKABLE QString statusString (const int camera);
    Q_INVOKABLE QCCTV_LightStatus lightStatus (const int camera);

    Q_INVOKABLE QCCTV_RemoteCamera* getCamera (const int camera);

public slots:
    void setFPS (const int camera, const int fps);
    void setLightStatusAll (const QCCTV_LightStatus status);
    void setLightStatus (const int camera, const QCCTV_LightStatus status);

private slots:
    void removeCamera (const int camera);
    void connectToCamera (const QHostAddress& ip);

private:
    QList<QHostAddress> m_cameraIPs;
    QList<QCCTV_RemoteCamera*> m_cameraList;
};

#endif
