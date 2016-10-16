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
#include "QCCTV_Station.h"
#include "QCCTV_Discovery.h"

QCCTV_Station::QCCTV_Station()
{
    /* Attempt to connect to a camera as we find it */
    QCCTV_Discovery* discovery = QCCTV_Discovery::getInstance();
    connect (discovery, SIGNAL (newCamera (QHostAddress)),
             this,        SLOT (connectToCamera (QHostAddress)));

    /* Bind connected() and disconnected() signals to cameraCountChanged() */
    connect (this, SIGNAL (connected (int)),
             this, SIGNAL (cameraCountChanged()));
    connect (this, SIGNAL (disconnected (int)),
             this, SIGNAL (cameraCountChanged()));
    connect (this, SIGNAL (disconnected (int)),
             this,   SLOT (removeCamera (int)));
}

QCCTV_Station::~QCCTV_Station() {}

QString QCCTV_Station::group() const
{
    return m_group;
}

int QCCTV_Station::cameraCount() const
{
    return m_cameraList.count();
}

int QCCTV_Station::fps (const int camera)
{
    if (getCamera (camera))
        return getCamera (camera)->fps();

    return 0;
}

QString QCCTV_Station::group (const int camera)
{
    if (getCamera (camera))
        return getCamera (camera)->group();

    return "";
}

int QCCTV_Station::cameraStatus (const int camera)
{
    if (getCamera (camera))
        return getCamera (camera)->cameraStatus();

    return 0;
}

QString QCCTV_Station::cameraName (const int camera)
{
    if (getCamera (camera))
        return getCamera (camera)->cameraName();

    return "";
}

QPixmap QCCTV_Station::currentImage (const int camera)
{
    if (getCamera (camera))
        return getCamera (camera)->currentImage();

    return QPixmap();
}

QHostAddress QCCTV_Station::address (const int camera)
{
    if (getCamera (camera))
        return getCamera (camera)->address();

    return QHostAddress ("");
}

QString QCCTV_Station::statusString (const int camera)
{
    if (getCamera (camera))
        return getCamera (camera)->statusString();

    return "";
}

QCCTV_LightStatus QCCTV_Station::lightStatus (const int camera)
{
    if (getCamera (camera))
        return getCamera (camera)->lightStatus();

    return QCCTV_FLASHLIGHT_OFF;
}

QCCTV_RemoteCamera* QCCTV_Station::getCamera (const int camera)
{
    if (camera < cameraCount())
        return m_cameraList.at (camera);

    return NULL;
}

void QCCTV_Station::setCameraGroup (const QString& group)
{
    if (m_group != group) {
        m_group = group;
        emit groupChanged();
    }
}

void QCCTV_Station::setFPS (const int camera, const int fps)
{
    if (camera < cameraCount())
        m_cameraList.at (camera)->setFPS (fps);
}

void QCCTV_Station::setLightStatusAll (const QCCTV_LightStatus status)
{
    for (int i = 0; i < cameraCount(); ++i)
        setLightStatus (i, status);
}

void QCCTV_Station::setLightStatus (const int camera,
                                    const QCCTV_LightStatus status)
{
    if (camera < cameraCount())
        m_cameraList.at (camera)->changeFlashlightStatus ((int) status);
}

void QCCTV_Station::removeCamera (const int camera)
{
    if (camera < cameraCount()) {
        m_cameraIPs.removeAt (camera);
        m_cameraList.removeAt (camera);
        emit cameraCountChanged();
    }
}

void QCCTV_Station::connectToCamera (const QHostAddress& ip)
{
    if (!ip.isNull() && !m_cameraIPs.contains (ip)) {
        m_cameraIPs.append (ip);
        m_cameraList.append (new QCCTV_RemoteCamera());

        connect (m_cameraList.last(), SIGNAL (connected (int)),
                 this,                SIGNAL (connected (int)));
        connect (m_cameraList.last(), SIGNAL (disconnected (int)),
                 this,                SIGNAL (disconnected (int)));
        connect (m_cameraList.last(), SIGNAL (fpsChanged (int)),
                 this,                SIGNAL (fpsChanged (int)));
        connect (m_cameraList.last(), SIGNAL (newCameraGroup (int)),
                 this,                SIGNAL (cameraGroupChanged (int)));
        connect (m_cameraList.last(), SIGNAL (newCameraName (int)),
                 this,                SIGNAL (cameraNameChanged (int)));
        connect (m_cameraList.last(), SIGNAL (newCameraStatus (int)),
                 this,                SIGNAL (cameraStatusChanged (int)));
        connect (m_cameraList.last(), SIGNAL (newImage (int)),
                 this,                SIGNAL (newCameraImage (int)));

        m_cameraList.last()->attemptConnection (ip);
    }
}
