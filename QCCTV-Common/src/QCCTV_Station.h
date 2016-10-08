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

class QCCTV_Station : public QObject
{
    Q_OBJECT

signals:
    void cameraAdded (const int camera);
    void cameraRemoved (const int camera);
    void lightStatusChanged (const int camera, const QCCTV_LightStatus status);
    void cameraStatusChanged (const int camera, const QCCTV_CameraStatus status);

public:
    QCCTV_Station();
    ~QCCTV_Station();

    Q_INVOKABLE QString group() const;
    Q_INVOKABLE int cameraCount() const;
    Q_INVOKABLE QCCTV_RemoteCamera* getCamera (const int camera);

public slots:
    void setCameraGroup (const QString& group);
    void setLightStatusAll (const QCCTV_LightStatus status);
    void setLightStatus (const int camera, const QCCTV_LightStatus status);

private slots:
    void connectToCamera (const QHostAddress& ip);

private:
    QString m_group;
    QList<QCCTV_RemoteCamera> m_cameraList;
};

#endif
