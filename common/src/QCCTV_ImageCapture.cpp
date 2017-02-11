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

#include "yuv2rgb.h"
#include "QCCTV_ImageCapture.h"

#include "QCCTV.h"

#include <QScreen>
#include <QCamera>
#include <QVideoProbe>
#include <QCameraInfo>
#include <QGuiApplication>

QCCTV_ImageCapture::QCCTV_ImageCapture (QObject* parent) :
    QAbstractVideoSurface (parent)
{
    m_enabled = false;
    m_probe = Q_NULLPTR;
    m_camera = Q_NULLPTR;

    if (!parent) {
        m_thread.start();
        moveToThread (&m_thread);
    }
}

QCCTV_ImageCapture::~QCCTV_ImageCapture()
{
    if (m_probe)
        delete m_probe;
}

/**
 * Returns the supported pixel formats of the frame grabber, we just return
 * all available pixel formats to avoid issues...
 */
QList<QVideoFrame::PixelFormat> QCCTV_ImageCapture::supportedPixelFormats
(QAbstractVideoBuffer::HandleType handleType) const
{
    Q_UNUSED (handleType);

    return QList<QVideoFrame::PixelFormat>()
           << QVideoFrame::Format_ARGB32
           << QVideoFrame::Format_ARGB32_Premultiplied
           << QVideoFrame::Format_RGB32
           << QVideoFrame::Format_RGB24
           << QVideoFrame::Format_RGB565
           << QVideoFrame::Format_RGB555
           << QVideoFrame::Format_ARGB8565_Premultiplied
           << QVideoFrame::Format_BGRA32
           << QVideoFrame::Format_BGRA32_Premultiplied
           << QVideoFrame::Format_BGR32
           << QVideoFrame::Format_BGR24
           << QVideoFrame::Format_BGR565
           << QVideoFrame::Format_BGR555
           << QVideoFrame::Format_BGRA5658_Premultiplied
           << QVideoFrame::Format_AYUV444
           << QVideoFrame::Format_AYUV444_Premultiplied
           << QVideoFrame::Format_YUV444
           << QVideoFrame::Format_YUV420P
           << QVideoFrame::Format_YV12
           << QVideoFrame::Format_UYVY
           << QVideoFrame::Format_YUYV
           << QVideoFrame::Format_NV12
           << QVideoFrame::Format_NV21
           << QVideoFrame::Format_IMC1
           << QVideoFrame::Format_IMC2
           << QVideoFrame::Format_IMC3
           << QVideoFrame::Format_IMC4
           << QVideoFrame::Format_Y8
           << QVideoFrame::Format_Y16
           << QVideoFrame::Format_Jpeg
           << QVideoFrame::Format_CameraRaw
           << QVideoFrame::Format_AdobeDng;
}

/**
 * Returns the current processed camera frame
 */
QImage QCCTV_ImageCapture::image() const
{
    return m_image;
}

/**
 * Returns \c true if the capturer is allowed to process image frames from
 * the media source (camera)
 */
bool QCCTV_ImageCapture::isEnabled() const
{
    return m_enabled;
}

/**
 * Changes the source from which we shall obtain (and process) the images
 */
void QCCTV_ImageCapture::setSource (QCamera* source)
{
    m_camera = source;
    m_info = QCameraInfo (*source);

#ifdef QCCTV_USE_FALLBACK_INTERFACE
    if (m_probe)
        delete m_probe;

    m_probe = new QVideoProbe;
    m_probe->setSource (source);
    connect (m_probe, SIGNAL (videoFrameProbed (QVideoFrame)),
             this,      SLOT (present          (QVideoFrame)));
#else
    m_camera->setViewfinder (this);
#endif

    if (m_camera->state() != QCamera::ActiveState)
        m_camera->start();
}

/**
 * Changes the \a enabled state of the grabber
 */
void QCCTV_ImageCapture::setEnabled (const bool enabled)
{
    m_enabled = enabled;
}

/**
 * Checks if the image is valid and rotates it to fix issues with mobile/touch screens
 */
bool QCCTV_ImageCapture::publishImage()
{
    /* Image is invalid or camera not loaded */
    if (m_image.isNull() || !m_camera)
        m_image = QCCTV_CreateStatusImage (QSize (640, 480), "NO CAMERA IMAGE");

    /* Image is valid, rotate image to compensate camera orientation */
    else {
        /* Get current display rotation */
        const QScreen* screen = QGuiApplication::primaryScreen();
        const int angle = screen->angleBetween (screen->nativeOrientation(),
                                                screen->orientation());

        /* Rotate image */
        const int rotation = (360 - m_info.orientation() + angle) % 360;
        m_image = m_image.transformed (QTransform().rotate (rotation));

        /* Fix upside-down image on Windows */
#if defined Q_OS_WIN
        m_image = m_image.mirrored (false, true);
#endif
    }

    /* Notify QCCTV */
    emit newFrame();
    return !m_image.isNull();
}

/**
 * Generates a \c QImage from the given \a frame
 */
bool QCCTV_ImageCapture::present (const QVideoFrame& frame)
{
    /* Frame is invalid or grabber is disabled */
    if (!frame.isValid() || !isEnabled())
        return false;

    /* Clone the frame (so that we can use it) */
    QVideoFrame clone (frame);
    if (!clone.map (QAbstractVideoBuffer::ReadOnly))
        return false;

    /* Get the image format from the pixel format of the frame */
    const QImage::Format format = QVideoFrame::imageFormatFromPixelFormat (clone.pixelFormat());

    /* This is simple, the format is supported natively by Qt */
    if (format != QImage::Format_Invalid)
        m_image = QImage (clone.bits(),
                          clone.width(),
                          clone.height(),
                          clone.bytesPerLine(),
                          format);

    /* This is an NV12/NV21 image (Qt does not support YUV images yet) */
    else if (clone.pixelFormat() == QVideoFrame::Format_NV12 ||
             clone.pixelFormat() == QVideoFrame::Format_NV21) {
        /* Create initial image */
        bool success = false;
        QImage image (clone.width(), clone.height(), QImage::Format_RGB888);

        /* Perform NV12 to RGB conversion */
        if (clone.pixelFormat() == QVideoFrame::Format_NV12)
            success = nv12_to_rgb (image.bits(),
                                   clone.bits(),
                                   clone.width(),
                                   clone.height());

        /* Perform NV21 to RGB conversion */
        else if (clone.pixelFormat() == QVideoFrame::Format_NV21)
            success = nv21_to_rgb (image.bits(),
                                   clone.bits(),
                                   clone.width(),
                                   clone.height());

        /* Re-assign the image */
        if (success)
            m_image = image;
    }

    /* Image format is not handled by Qt or QCCTV, generate grayscale image */
    else if (clone.bits()) {
        m_image = QImage (clone.bits(),
                          clone.width(),
                          clone.height(),
                          clone.bytesPerLine(),
                          QImage::Format_Grayscale8);
    }

    /* Unmap the frame data and process the obtained image */
    clone.unmap();
    return publishImage();
}
