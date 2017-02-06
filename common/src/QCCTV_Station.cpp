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

#include <QDir>
#include <QThread>
#include <QFileDialog>
#include <QDesktopServices>

QCCTV_Station::QCCTV_Station()
{
    /* Attempt to connect to a camera as we find it */
    QCCTV_Discovery* discovery = QCCTV_Discovery::getInstance();
    connect (discovery, SIGNAL (newCamera       (QHostAddress)),
             this,        SLOT (connectToCamera (QHostAddress)));
    connect (discovery, SIGNAL (newInfoPacket   (QHostAddress, QByteArray)),
             this,        SLOT (readInfoPacket  (QHostAddress, QByteArray)));

    /* Remove a camera when we disconnect from it */
    connect (this, SIGNAL (connected (int)),
             this, SIGNAL (cameraCountChanged()));
    connect (this, SIGNAL (disconnected (int)),
             this,   SLOT (removeCamera (int)));
    connect (this, SIGNAL (cameraCountChanged()),
             this,   SLOT (updateGroups()));

    /* Set camera error image */
    setRecordingsPath ("");
    setSaveIncomingMedia (true);
    m_cameraError = QCCTV_CreateStatusImage (QSize (640, 480), "CAMERA ERROR");
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
 * Returns the number of groups that have been found by the station
 */
int QCCTV_Station::groupCount() const
{
    return m_groups.count();
}

/**
 * Returns the number of cameras that the station is connected to
 */
int QCCTV_Station::cameraCount() const
{
    return m_cameras.count();
}

/**
 * Returns the number of cameras associated with the given \a group ID
 */
int QCCTV_Station::cameraCount (const int group) const
{
    return getGroupCameras (group).count();
}

/**
 * Returns a list with all available groups
 */
QStringList QCCTV_Station::groups() const
{
    return m_groups;
}

/**
 * Returns the path in which the QCCTV recordings are saved
 */
QString QCCTV_Station::recordingsPath() const
{
    return m_recordingsPath;
}

/**
 * Returns \c true if the station should save received camera frames
 * to the hard disk
 */
bool QCCTV_Station::saveIncomingMedia() const
{
    return m_saveIncomingMedia;
}

/**
 * Returns an ordered list with the available image resolutions, this function
 * can be used to populate a combobox or a QML model
 */
QStringList QCCTV_Station::availableResolutions() const
{
    return QCCTV_Resolutions();
}

/**
 * Returns a list with the ID of each camera assigned to the given \a group ID.
 *
 * The \a group ID can be obtained by using the \c indexOf(QString) function
 * of the list returned by \a groups().
 */
QList<int> QCCTV_Station::getGroupCameraIDs (const int group) const
{
    QList<int> list;

    foreach (QCCTV_RemoteCamera* camera, getGroupCameras (group))
        list.append (camera->id());

    return list;
}

/**
 * Returns a list with the pointers to the cameras that are assigned to the given
 * \a group ID
 *
 * The \a group ID can be obtained by using the \c indexOf(QString) function
 * of the list returned by \a groups().
 */
QList<QCCTV_RemoteCamera*> QCCTV_Station::getGroupCameras (const int group) const
{
    QList<QCCTV_RemoteCamera*> list;

    if (group < groupCount()) {
        QString name = groups().at (group);
        foreach (QCCTV_RemoteCamera* camera, m_cameras) {
            if (camera) {
                if (camera->group().toLower() == name.toLower())
                    list.append (camera);
            }
        }
    }

    return list;
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
 * Returns the current zoom level used by the given \a camera
 */
int QCCTV_Station::zoom (const int camera)
{
    if (getCamera (camera))
        return getCamera (camera)->zoom();

    return 0;
}

/**
 * Returns the current resolution used by the camera
 */
int QCCTV_Station::resolution (const int camera)
{
    if (getCamera (camera))
        return getCamera (camera)->resolution();

    return (int) QCCTV_QCIF;
}

/**
 * Returns \c true if the given \a camera supports zooming
 */
bool QCCTV_Station::supportsZoom (const int camera)
{
    if (getCamera (camera))
        return getCamera (camera)->supportsZoom();

    return false;
}

/**
 * Returns the camera status byte for the given \a camera
 * \note If an invalid camera ID is given to this function,
 *       then this function shall return \c -1
 */
int QCCTV_Station::cameraStatus (const int camera)
{
    if (getCamera (camera))
        return getCamera (camera)->status();

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
        return getCamera (camera)->name();

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
        return getCamera (camera)->image();

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
 * Returns the IP address of the given \a camera
 * \note If an invalid camera ID is given to this function,
 *       then this function shall return an empty string
 */
QString QCCTV_Station::addressString (const int camera)
{
    if (getCamera (camera))
        return getCamera (camera)->address().toString();

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
        return (getCamera (camera)->flashlightEnabled());

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
        return ! (getCamera (camera)->status() & QCCTV_CAMSTATUS_LIGHT_FAILURE);

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
 * Returns a list with the IP's of the connected cameras
 */
QList<QHostAddress> QCCTV_Station::cameraIPs()
{
    QList<QHostAddress> list;

    for (int i = 0; i < cameraCount(); ++i) {
        if (getCamera (i))
            list.append (getCamera (i)->address());
    }

    return list;
}

/**
 * Returns the name of the group at the given \a index
 * This function is made available to ease QML-based implementations
 */
QString QCCTV_Station::getGroupName (const int group)
{
    if (group < groupCount())
        return groups().at (group);

    return "";
}

/**
 * Returns a pointer to the controller of the given \a camera
 * \note If an invalid camera ID is given to this function,
 *       then this function shall return a \c NULL pointer
 */
QCCTV_RemoteCamera* QCCTV_Station::getCamera (const int camera)
{
    if (camera >= 0 && camera < cameraCount())
        return m_cameras.at (camera);

    return NULL;
}

/**
 * Re-generates the camera groups list
 */
void QCCTV_Station::updateGroups()
{
    m_groups.clear();

    foreach (QCCTV_RemoteCamera* camera, m_cameras)
        if (!m_groups.contains (camera->group().toLower()))
            m_groups.append (camera->group().toLower());

    emit groupCountChanged();
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
 * Opens the folder in which we save QCCTV recordings
 */
void QCCTV_Station::openRecordingsPath()
{
    QDesktopServices::openUrl (QUrl::fromLocalFile (recordingsPath()));
}

/**
 * Prompts the user to select where to save the QCCTV recordings
 */
void QCCTV_Station::chooseRecordingsPath()
{
    QString dir = QFileDialog::getExistingDirectory (NULL,
                                                     tr ("Select recordings folder"),
                                                     QDir::homePath(),
                                                     QFileDialog::ShowDirsOnly |
                                                     QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty())
        setRecordingsPath (dir);
}

/**
 * Instructs the remote \a camera to perform a focus
 * \note If the \a camera parameter is invalid, then this function
 *       shall have no effect
 */
void QCCTV_Station::focusCamera (const int camera)
{
    if (getCamera (camera))
        getCamera (camera)->requestFocus();
}

/**
 * Allows or disallows the QCCTV Station to save incoming media
 */
void QCCTV_Station::setSaveIncomingMedia (const bool save)
{
    m_saveIncomingMedia = save;

    foreach (QCCTV_RemoteCamera* camera, m_cameras)
        camera->setSaveIncomingMedia (saveIncomingMedia());

    emit saveIncomingMediaChanged();
}

/**
 * Changes the directory in which the QCCTV recordings are saved.
 *
 * \note If the \a path is empty, then the default directory shall
 *       be used to store QCCTV recordings
 */
void QCCTV_Station::setRecordingsPath (const QString& path)
{
    if (!path.isEmpty() && m_recordingsPath == path)
        return;

    if (!path.isEmpty())
        m_recordingsPath = QDir (path).absolutePath();
    else
        m_recordingsPath = QDir (QCCTV_RECORDINGS_PATH).absolutePath();

    if (!m_recordingsPath.endsWith ("/QCCTV_Media/") &&
        !m_recordingsPath.endsWith ("/QCCTV_Media")) {
        m_recordingsPath += "/QCCTV_Media/";
        m_recordingsPath = QDir (m_recordingsPath).absolutePath();
    }

    foreach (QCCTV_RemoteCamera* camera, m_cameras)
        if (camera)
            camera->setIncomingMediaPath (recordingsPath());

    emit recordingsPathChanged();
}

/**
 * Changes the zoom status of the given \a camera
 */
void QCCTV_Station::setZoom (const int camera, const int zoom)
{
    if (getCamera (camera))
        getCamera (camera)->changeZoom (zoom);
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
        getCamera (camera)->changeFlashlightStatus ((int) enabled);
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
        /* Stop the camera */
        m_cameras.at (camera)->deleteLater();
        m_cameras.removeAt (camera);

        /* Stop the thread */
        m_threads.at (camera)->deleteLater();
        m_threads.removeAt (camera);

        /* Change the ID of cameras following the removed camera */
        foreach (QCCTV_RemoteCamera* cam, m_cameras) {
            if (cam) {
                if (cam->id() > camera)
                    cam->changeID (cam->id() - 1);
            }
        }

        /* Notify UI */
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
    if (!ip.isNull() && !cameraIPs().contains (ip)) {
        QThread* thread = new QThread;
        QCCTV_RemoteCamera* camera = new QCCTV_RemoteCamera;

        /* Register thread and camera pointers */
        m_threads.append (thread);
        m_cameras.append (camera);

        /* Configure camera */
        camera->setAddress (ip);
        camera->changeID (cameraCount() - 1);
        camera->setIncomingMediaPath (recordingsPath());
        camera->setSaveIncomingMedia (saveIncomingMedia());

        /* Start timers when thread is started */
        connect (thread, SIGNAL (started()),
                 camera,   SLOT (start()));

        /* Move remote camera to different thread */
        thread->start (QThread::HighPriority);
        camera->moveToThread (thread);

        /* Connect equivalent signals between station and camera */
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
        connect (camera, SIGNAL (zoomLevelChanged (int)),
                 this,   SIGNAL (zoomLevelChanged (int)));
        connect (camera, SIGNAL (zoomSupportChanged (int)),
                 this,   SIGNAL (zoomSupportChanged (int)));
        connect (camera, SIGNAL (resolutionChanged (int)),
                 this,   SIGNAL (resolutionChanged (int)));
        connect (camera, SIGNAL (lightStatusChanged (int)),
                 this,   SIGNAL (lightStatusChanged (int)));
        connect (camera, SIGNAL (autoRegulateResolutionChanged (int)),
                 this,   SIGNAL (autoRegulateResolutionChanged (int)));
        connect (camera, SIGNAL (newCameraGroup()),
                 this,     SLOT (updateGroups()));
    }
}

/**
 * Figures out from which remote camera did the \a data come from and instructs
 * the remote camera manager assigned to that \a address to read the \a data
 */
void QCCTV_Station::readInfoPacket (const QHostAddress& address,
                                    const QByteArray& data)
{
    if (cameraIPs().contains (address)) {
        int camera = cameraIPs().indexOf (address);
        if (getCamera (camera))
            getCamera (camera)->readInfoPacket (data);
    }
}
