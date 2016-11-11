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

#include "QCCTV_Station.h"
#include "QCCTV_Discovery.h"

QCCTV_Station::QCCTV_Station()
{
    /* Attempt to connect to a camera as we find it */
    QCCTV_Discovery* discovery = QCCTV_Discovery::getInstance();
    connect (discovery, SIGNAL (newCamera (QHostAddress)),
             this,        SLOT (connectToCamera (QHostAddress)));

    /* Remove a camera when we disconnect from it */
    connect (this, SIGNAL (connected (int)),
             this, SIGNAL (cameraCountChanged()));
    connect (this, SIGNAL (disconnected (int)),
             this,   SLOT (removeCamera (int)));

    /* Set camera error image */
    m_cameraError = QCCTV_GET_STATUS_IMAGE (QSize (640, 480), "CAMERA ERROR");
}

/**
 * Removes all the registered cameras during the deconstruction of the
 * \c QCCTV_Station class
 */
QCCTV_Station::~QCCTV_Station()
{
    for (int i = 0; i < cameraCount(); ++i)
        removeCamera (i);
}

/**
 * Returns the minimum FPS value allowed by QCCTV, this function can be used
 * to set control/widget limits of QML or classic interfaces
 */
int QCCTV_Station::minimumFPS() const
{
    return QCCTV_MIN_FPS;
}

/**
 * Returns the maximum FPS value allowed by QCCTV, this function can be used
 * to set control/widget limits of QML or classic interfaces
 */
int QCCTV_Station::maximumFPS() const
{
    return QCCTV_MAX_FPS;
}

/**
 * Returns the number of cameras that the station is connected to
 */
int QCCTV_Station::cameraCount() const
{
    return m_cameraList.count();
}

/**
 * Returns an ordered list with the available image resolutions, this function
 * can be used to populate a combobox or a QML model
 */
QStringList QCCTV_Station::availableResolutions() const
{
    return QCCTV_AVAILABLE_RESOLUTIONS();
}

/**
 * Returns the FPS reported by the given \a camera
 * \note If an invalid camera ID is given to this function,
 *       then this function shall return \c -1
 */
int QCCTV_Station::fps (const int camera)
{
    if (getCamera (camera))
        return getCamera (camera)->fps();

    return -1;
}

/**
 * Returns the current resolution used by the camera
 */
int QCCTV_Station::resolution (const int camera)
{
    if (getCamera (camera))
        return (int) getCamera (camera)->resolution();

    return (int) QCCTV_QCIF;
}

/**
 * Returns the camera status byte for the given \a camera
 * \note If an invalid camera ID is given to this function,
 *       then this function shall return \c -1
 */
int QCCTV_Station::cameraStatus (const int camera)
{
    if (getCamera (camera))
        return getCamera (camera)->cameraStatus();

    return -1;
}

/**
 * Returns the name of the given \a camera
 * \note If an invalid camera ID is given to this function,
 *       then this function shall return an empty string
 */
QString QCCTV_Station::cameraName (const int camera)
{
    if (getCamera (camera))
        return getCamera (camera)->cameraName();

    return "";
}

/**
 * Returns the latest image captured by the given \a camera
 * \note If an invalid camera ID is given to this function,
 *       then this function shall return a generic error image
 */
QImage QCCTV_Station::currentImage (const int camera)
{
    if (getCamera (camera))
        return getCamera (camera)->currentImage();

    return m_cameraError;
}

/**
 * Returns the network address of the given \a camera
 * \note If an invalid camera ID is given to this function,
 *       then this function shall return an empty address
 */
QHostAddress QCCTV_Station::address (const int camera)
{
    if (getCamera (camera))
        return getCamera (camera)->address();

    return QHostAddress ("");
}

/**
 * Returns the status string of the given \a camera
 * \note If an invalid camera ID is given to this function,
 *       then this function shall return an empty string
 */
QString QCCTV_Station::statusString (const int camera)
{
    if (getCamera (camera))
        return getCamera (camera)->statusString();

    return "";
}

/**
 * Returns the flash light status of the given \a camera
 * \note If an invalid camera ID is given to this function,
 *       then this function shall return \c QCCTV_FLASHLIGHT_OFF
 */
bool QCCTV_Station::flashlightEnabled (const int camera)
{
    if (flashlightAvailable (camera))
        return (getCamera (camera)->lightStatus() == QCCTV_FLASHLIGHT_ON);

    return false;
}

/**
 * Returns \c true if the flashlight of the given \a camera works
 * \note If an invalid camera ID is given to this function,
 *       then this function shall return \c flase
 */
bool QCCTV_Station::flashlightAvailable (const int camera)
{
    if (getCamera (camera))
        return ! (getCamera (camera)->cameraStatus() & QCCTV_CAMSTATUS_LIGHT_FAILURE);

    return false;
}

/**
 * Returns \c true if the camera is allowed to auto-regulate the image
 * resolution to improve communication times
 *
 * \note If an invalid camera ID is given to this function,
 *       then this function shall return \c false
 */
bool QCCTV_Station::autoRegulateResolution (const int camera)
{
    if (getCamera (camera))
        return getCamera (camera)->autoRegulateResolution();

    return false;
}

/**
 * Returns a pointer to the controller of the given \a camera
 * \note If an invalid camera ID is given to this function,
 *       then this function shall return a \c NULL pointer
 */
QCCTV_RemoteCamera* QCCTV_Station::getCamera (const int camera)
{
    if (camera < cameraCount())
        return m_cameraList.at (camera);

    return NULL;
}

/**
 * De-registers all the cameras from the station, this can be useful in the
 * case that the user wants to force the station to reconnect to all cameras
 * again.
 */
void QCCTV_Station::removeAllCameras()
{
    for (int i = 0; i < cameraCount(); ++i)
        removeCamera (i);
}

/**
 * Changes the \a fps value of the given \a camera
 * \note If the \a camera parameter is invalid, then this function
 *       shall have no effect
 */
void QCCTV_Station::changeFPS (const int camera, const int fps)
{
    if (getCamera (camera))
        getCamera (camera)->changeFPS (fps);
}

/**
 * Changes the flashlight \a status for all cameras connected to the station
 */
void QCCTV_Station::setFlashlightEnabledAll (const bool enabled)
{
    for (int i = 0; i < cameraCount(); ++i)
        setFlashlightEnabled (i, enabled);
}

/**
 * Changes the \a resoltion for the given \a camera
 * \note If the \a camera parameter is invalid, then this function
 *       shall have no effect
 */
void QCCTV_Station::changeResolution (const int camera, const int resolution)
{
    if (getCamera (camera))
        getCamera (camera)->changeResolution (resolution);
}

/**
 * Changes the flashlight \a status value of the given \a camera
 * \note If the \a camera parameter is invalid, then this function
 *       shall have no effect
 */
void QCCTV_Station::setFlashlightEnabled (const int camera, const bool enabled)
{
    if (getCamera (camera))
        getCamera (camera)->setFlashlightStatus ((int) enabled);
}

/**
 * Allows or disallows the \a camera to autoregulate its image resolution to
 * improve communication tiems
 * \note If the \a camera parameter is invalid, then this function shall
 *       have no effect
 */
void QCCTV_Station::setAutoRegulateResolution (const int camera,
                                               const bool regulate)
{
    if (getCamera (camera))
        getCamera (camera)->changeAutoRegulate (regulate);
}

/**
 * Removes the given \a camera from the registered cameras list
 * \note Cameras that where registered after the removed camera shall
 *       change their respective ID's after calling this function
 */
void QCCTV_Station::removeCamera (const int camera)
{
    if (getCamera (camera)) {
        getCamera (camera)->deleteLater();

        m_cameraIPs.removeAt (camera);
        m_cameraList.removeAt (camera);

        foreach (QCCTV_RemoteCamera* remoteCam, m_cameraList) {
            if (remoteCam->id() > camera)
                remoteCam->changeID (remoteCam->id() - 1);
        }

        emit cameraCountChanged();
    }
}

/**
 * Tries to establish a connection with a QCCTV camera running
 * in a host with the given \a ip address
 *
 * If the remote camera does not respond after some seconds,
 * then the new camera controller shall be automatically
 * deleted from the camera list
 */
void QCCTV_Station::connectToCamera (const QHostAddress& ip)
{
    if (!ip.isNull() && !m_cameraIPs.contains (ip)) {
        QCCTV_RemoteCamera* camera = new QCCTV_RemoteCamera();
        m_cameraIPs.append (ip);
        m_cameraList.append (camera);
        m_cameraList.last()->changeID (m_cameraList.count() - 1);

        connect (camera, SIGNAL (connected (int)),
                 this,   SIGNAL (connected (int)));
        connect (camera, SIGNAL (disconnected (int)),
                 this,   SIGNAL (disconnected (int)));
        connect (camera, SIGNAL (fpsChanged (int)),
                 this,   SIGNAL (fpsChanged (int)));
        connect (camera, SIGNAL (newCameraName (int)),
                 this,   SIGNAL (cameraNameChanged (int)));
        connect (camera, SIGNAL (newCameraStatus (int)),
                 this,   SIGNAL (cameraStatusChanged (int)));
        connect (camera, SIGNAL (newImage (int)),
                 this,   SIGNAL (newCameraImage (int)));
        connect (camera, SIGNAL (resolutionChanged (int)),
                 this,   SIGNAL (resolutionChanged (int)));
        connect (camera, SIGNAL (autoRegulateResolutionChanged (int)),
                 this,   SIGNAL (autoRegulateResolutionChanged (int)));

        camera->setAddress (ip);
    }
}
