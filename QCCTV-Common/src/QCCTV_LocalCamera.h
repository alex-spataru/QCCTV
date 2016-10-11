#ifndef _QCCTV_LOCAL_CAMERA_H
#define _QCCTV_LOCAL_CAMERA_H

#include <QPixmap>
#include <QObject>
#include <QCamera>
#include <QUdpSocket>
#include <QVideoWidget>
#include <QQuickImageProvider>

#include "QCCTV.h"
#include "QCCTV_Watchdog.h"

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
    Q_INVOKABLE int cameraStatus() const;
    Q_INVOKABLE bool flashlightOn() const;
    Q_INVOKABLE bool flashlightOff() const;
    Q_INVOKABLE QString cameraName() const;
    Q_INVOKABLE QString cameraGroup() const;
    Q_INVOKABLE QPixmap currentImage() const;
    Q_INVOKABLE QString statusString() const;
    Q_INVOKABLE bool flashlightAvailable() const;
    Q_INVOKABLE QStringList connectedHosts() const;
    Q_INVOKABLE QCCTV_LightStatus lightStatus() const;

public slots:
    void focusCamera();
    void setFPS (const int fps);
    void setName (const QString& name);
    void setGroup (const QString& group);

    inline void turnOnFlashlight()
    {
        setFlashlightStatus (QCCTV_FLASHLIGHT_ON);
    }

    inline void turnOffFlashlight()
    {
        setFlashlightStatus (QCCTV_FLASHLIGHT_OFF);
    }

private slots:
    void update();
    void updateImage();
    void updateStatus();
    void sendCameraData();
    void disconnectStation();
    void readRequestPacket();
    void readCommandPacket();
    void broadcastInformation();
    void addStatusFlag (const QCCTV_CameraStatus status);
    void setCameraStatus (const QCCTV_CameraStatus status);
    void removeStatusFlag (const QCCTV_CameraStatus status);
    void setFlashlightStatus (const QCCTV_LightStatus status);

private:
    int m_fps;
    int m_cameraStatus;
    int m_flashlightStatus;

    QCCTV_Watchdog m_watchdog;
    QList<QHostAddress> m_hosts;

    QString m_name;
    QString m_group;

    QPixmap m_image;
    QCamera* m_camera;
    QVideoWidget m_videoWidget;

    QUdpSocket m_senderSocket;
    QUdpSocket m_commandSocket;
    QUdpSocket m_requestSocket;
    QUdpSocket m_broadcastSocket;
};

class QCCTV_LocalImageProvider : public QQuickImageProvider
{

public:
    QCCTV_LocalImageProvider (QCCTV_LocalCamera* camera);
    QPixmap requestPixmap (const QString& id, QSize* size,
                           const QSize& requestedSize);

private:
    QCCTV_LocalCamera* m_camera;
};

#endif
