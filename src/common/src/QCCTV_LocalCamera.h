#ifndef _QCCTV_LOCAL_CAMERA_H
#define _QCCTV_LOCAL_CAMERA_H

#include <QTimer>
#include <QImage>
#include <QObject>
#include <QUdpSocket>

#include "QCCTV.h"

class QCCTV_LocalCamera : public QObject
{
    Q_OBJECT

signals:
    void fpsChanged();
    void newImageRecorded();
    void cameraNameChanged();
    void cameraGroupChanged();
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
    Q_INVOKABLE QStringList connectedHosts() const;
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
    void readRequestPacket();
    void readCommandPacket();
    void broadcastInformation();

private:
    int m_fps;
    QCCTV_LightStatus m_lightStatus;
    QCCTV_CameraStatus m_cameraStatus;

    QList<QHostAddress> m_allowedHosts;

    QString m_cameraName;
    QString m_cameraGroup;

    QTimer m_timer;
    QImage m_currentImage;
    QUdpSocket m_senderSocket;
    QUdpSocket m_commandSocket;
    QUdpSocket m_requestSocket;
    QUdpSocket m_broadcastSocket;
};

#endif
