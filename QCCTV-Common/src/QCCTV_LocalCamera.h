#ifndef _QCCTV_LOCAL_CAMERA_H
#define _QCCTV_LOCAL_CAMERA_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>

#include "QCCTV.h"
#include "QCCTV_Watchdog.h"
#include "QCCTV_FrameGrabber.h"

class QCamera;
class QCameraImageCapture;

class QCCTV_LocalCamera : public QObject
{
    Q_OBJECT

signals:
    void fpsChanged();
    void imageChanged();
    void cameraNameChanged();
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
    Q_INVOKABLE QImage currentImage() const;
    Q_INVOKABLE QString statusString() const;
    Q_INVOKABLE bool readyForCapture() const;
    Q_INVOKABLE bool flashlightAvailable() const;
    Q_INVOKABLE QStringList connectedHosts() const;
    Q_INVOKABLE QCCTV_LightStatus lightStatus() const;

    Q_INVOKABLE bool isGrayscale() const;
    Q_INVOKABLE qreal shrinkRatio() const;

public slots:
    void takePhoto();
    void focusCamera();
    void turnOnFlashlight();
    void turnOffFlashlight();
    void setFPS (const int fps);
    void setCamera (QCamera* camera);
    void setName (const QString& name);
    void setGrayscale (const bool gray);
    void setShrinkRatio (const qreal ratio);

private slots:
    void update();
    void updateStatus();
    void broadcastInfo();
    void sendCameraData();
    void onDisconnected();
    void acceptConnection();
    void readCommandPacket();
    void generateDataStream();
    void changeImage (const QImage& image);
    void addStatusFlag (const QCCTV_CameraStatus status);
    void setCameraStatus (const QCCTV_CameraStatus status);
    void removeStatusFlag (const QCCTV_CameraStatus status);
    void setFlashlightStatus (const QCCTV_LightStatus status);

private:
    int m_fps;
    int m_cameraStatus;
    int m_flashlightStatus;

    QImage m_image;
    QString m_name;
    QByteArray m_dataStream;

    QCamera* m_camera;
    QCameraImageCapture* m_capture;
    QCCTV_FrameGrabber m_frameGrabber;

    QTcpServer m_server;
    QUdpSocket m_broadcastSocket;
    QList<QTcpSocket*> m_sockets;
};


#endif
