#ifndef _QCCTV_LOCAL_CAMERA_H
#define _QCCTV_LOCAL_CAMERA_H

#include <QPixmap>
#include <QObject>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QTcpServer>

#include "QCCTV.h"
#include "QCCTV_Watchdog.h"
#include "QCCTV_FrameGrabber.h"

class QCamera;

class QCCTV_LocalCamera : public QObject
{
    Q_OBJECT

signals:
    void fpsChanged();
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
    Q_INVOKABLE QImage currentImage() const;
    Q_INVOKABLE QString statusString() const;
    Q_INVOKABLE bool flashlightAvailable() const;
    Q_INVOKABLE QStringList connectedHosts() const;
    Q_INVOKABLE QCCTV_LightStatus lightStatus() const;

    Q_INVOKABLE bool isGrayscale() const;
    Q_INVOKABLE qreal shrinkRatio() const;

public slots:
    void focusCamera();
    void turnOnFlashlight();
    void turnOffFlashlight();
    void setFPS (const int fps);
    void setCamera (QCamera* camera);
    void setName (const QString& name);
    void setGrayscale (const bool gray);
    void setGroup (const QString& group);
    void setShrinkRatio (const qreal ratio);

private slots:
    void update();
    void updateStatus();
    void sendCameraData();
    void acceptConnection();
    void disconnectStation();
    void readCommandPacket();
    void broadcastInformation();
    void changeImage (const QImage& image);
    void addStatusFlag (const QCCTV_CameraStatus status);
    void setCameraStatus (const QCCTV_CameraStatus status);
    void removeStatusFlag (const QCCTV_CameraStatus status);
    void setFlashlightStatus (const QCCTV_LightStatus status);

private:
    int m_fps;
    int m_cameraStatus;
    int m_flashlightStatus;

    QCCTV_Watchdog m_watchdog;

    QString m_name;
    QString m_group;

    QImage m_image;
    QCamera* m_camera;
    QCCTV_FrameGrabber m_frameGrabber;

    QTcpServer m_server;
    QUdpSocket m_broadcastSocket;
    QList<QTcpSocket*> m_sockets;
};


#endif
