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

#include <QImage>
#include <QObject>

#include "QCCTV_RemoteCamera.h"

class QThread;
class QCCTV_Station : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void groupCountChanged();
    void cameraCountChanged();
    void recordingsPathChanged();
    void saveIncomingMediaChanged();
    void connected (const int camera);
    void fpsChanged (const int camera);
    void disconnected (const int camera);
    void newCameraImage (const int camera);
    void zoomLevelChanged (const int camera);
    void cameraNameChanged (const int camera);
    void resolutionChanged (const int camera);
    void lightStatusChanged (const int camera);
    void zoomSupportChanged (const int camera);
    void cameraStatusChanged (const int camera);
    void autoRegulateResolutionChanged (const int camera);

public:
    QCCTV_Station();
    ~QCCTV_Station();

    Q_INVOKABLE int minimumFPS() const;
    Q_INVOKABLE int maximumFPS() const;

    Q_INVOKABLE int groupCount() const;
    Q_INVOKABLE int cameraCount() const;
    Q_INVOKABLE int cameraCount (const int group) const;

    Q_INVOKABLE QStringList groups() const;
    Q_INVOKABLE QString recordingsPath() const;
    Q_INVOKABLE bool saveIncomingMedia() const;
    Q_INVOKABLE QStringList availableResolutions() const;

    Q_INVOKABLE QList<int> getGroupCameraIDs (const int group) const;
    Q_INVOKABLE QList<QCCTV_RemoteCamera*> getGroupCameras (const int group) const;

    Q_INVOKABLE int fps (const int camera);
    Q_INVOKABLE int zoom (const int camera);
    Q_INVOKABLE int resolution (const int camera);
    Q_INVOKABLE int cameraStatus (const int camera);
    Q_INVOKABLE bool supportsZoom (const int camera);
    Q_INVOKABLE QString cameraName (const int camera);
    Q_INVOKABLE QImage currentImage (const int camera);
    Q_INVOKABLE QHostAddress address (const int camera);
    Q_INVOKABLE QString statusString (const int camera);
    Q_INVOKABLE QString addressString (const int camera);
    Q_INVOKABLE bool flashlightEnabled (const int camera);
    Q_INVOKABLE bool flashlightAvailable (const int camera);
    Q_INVOKABLE bool autoRegulateResolution (const int camera);

    Q_INVOKABLE QList<QHostAddress> cameraIPs();
    Q_INVOKABLE QString getGroupName (const int group);
    Q_INVOKABLE QCCTV_RemoteCamera* getCamera (const int camera);

public Q_SLOTS:
    void updateGroups();
    void removeAllCameras();
    void openRecordingsPath();
    void chooseRecordingsPath();
    void focusCamera (const int camera);
    void setSaveIncomingMedia (const bool save);
    void setRecordingsPath (const QString& path);
    void setZoom (const int camera, const int zoom);
    void changeFPS (const int camera, const int fps);
    void setFlashlightEnabledAll (const bool enabled);
    void changeResolution (const int camera, const int resolution);
    void setFlashlightEnabled (const int camera, const bool enabled);
    void setAutoRegulateResolution (const int camera, const bool regulate);

private Q_SLOTS:
    void removeCamera (const int camera);
    void connectToCamera (const QHostAddress& ip);
    void readInfoPacket (const QHostAddress& address, const QByteArray& data);

private:
    QImage m_cameraError;
    QStringList m_groups;
    QString m_recordingsPath;
    bool m_saveIncomingMedia;
    QList<QThread*> m_threads;
    QList<QCCTV_RemoteCamera*> m_cameras;
};

#endif
