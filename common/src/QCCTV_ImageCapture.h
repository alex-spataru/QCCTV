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

#ifndef _QCCTV_IMAGE_CAPTURE_H
#define _QCCTV_IMAGE_CAPTURE_H

#include <QThread>
#include <QObject>
#include <QCameraInfo>
#include <QAbstractVideoSurface>

class QCamera;
class QVideoProbe;

class QCCTV_ImageCapture : public QAbstractVideoSurface
{
    Q_OBJECT

Q_SIGNALS:
    void newFrame();

public:
    QCCTV_ImageCapture (QObject* parent = NULL);
    ~QCCTV_ImageCapture();

    QList<QVideoFrame::PixelFormat> supportedPixelFormats
    (QAbstractVideoBuffer::HandleType handleType) const;

    QImage image() const;
    bool isEnabled() const;

public Q_SLOTS:
    void setSource (QCamera* source);
    void setEnabled (const bool enabled);

private Q_SLOTS:
    bool publishImage();
    bool present (const QVideoFrame& frame);

private:
    bool m_enabled;
    QImage m_image;
    QThread m_thread;
    QCamera* m_camera;
    QCameraInfo m_info;
    QVideoProbe* m_probe;
};

#endif
