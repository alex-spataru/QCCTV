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

#include "yuv2rgb/yuv2rgb.h"
#include "QCCTV_ImageCapture.h"

#include <QScreen>
#include <QCamera>
#include <QCameraInfo>
#include <QGuiApplication>

/**
 * Initializes the default variables
 */
QCCTV_ImageCapture::QCCTV_ImageCapture (QObject* parent) : QObject (parent)
{
    m_rotation = 0;
    m_enabled = false;
    connect (&m_probe, SIGNAL (videoFrameProbed (QVideoFrame)),
             this,       SLOT (processVideoFrame (QVideoFrame)));
}

/**
 * Changes the source from which we shall obtain (and process) the images
 */
void QCCTV_ImageCapture::setSource (QCamera* source)
{
    m_camera = source;
    m_probe.setSource (source);
    m_info = QCameraInfo (*source);
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
void QCCTV_ImageCapture::publishImage()
{
    /* Image is invalid */
    if (m_image.isNull() || !m_camera)
        return;

    /* Get current display rotation */
    const QScreen* screen = QGuiApplication::primaryScreen();
    const int angle = screen->angleBetween (screen->nativeOrientation(), screen->orientation());

    /* Calculate rotation */
    m_rotation = (360 - m_info.orientation() + angle) % 360;

    /* Rotate image */
    m_image = m_image.transformed (QTransform().rotate (m_rotation));
    emit newFrame (m_image);
}

/**
 * Generates a \c QImage from the given \a frame
 */
void QCCTV_ImageCapture::processVideoFrame (const QVideoFrame frame)
{
    /* Frame is invalid or grabber is disabled */
    if (!frame.isValid() || !m_enabled)
        return;

    /* Convert the videoframe to an image */
    QVideoFrame clone (frame);
    if (clone.map (QAbstractVideoBuffer::ReadOnly)) {
        const QImage::Format format = QVideoFrame::imageFormatFromPixelFormat (clone.pixelFormat());

        /* This is simple, image is supported natively by Qt */
        if (format != QImage::Format_Invalid)
            m_image = QImage (clone.bits(),
                              clone.width(),
                              clone.height(),
                              format);

        /* This is an NV12/NV21 image */
        else if (clone.pixelFormat() == QVideoFrame::Format_NV12 ||
                 clone.pixelFormat() == QVideoFrame::Format_NV21) {
            bool success = false;

#ifdef Q_OS_ANDROID
            uchar* rgb = new uchar [clone.mappedBytes() * 3];
#else
            uchar rgb [clone.mappedBytes() * 3];
#endif

            /* Perform NV12 to ARGB32 conversion */
            if (clone.pixelFormat() == QVideoFrame::Format_NV12)
                success = nv12_to_rgb (rgb, clone.bits(), clone.width(), clone.height());

            /* Perform NV21 to ARGB32 conversion */
            else if (clone.pixelFormat() == QVideoFrame::Format_NV21)
                success = nv21_to_rgb (rgb, clone.bits(), clone.width(), clone.height());

            /* Conversion finished, generate image */
            if (success) {
                m_image = QImage ((uchar*) rgb,
                                  clone.width(),
                                  clone.height(),
                                  QImage::Format_RGB888);
            }
        }

        /* Last ditch attempt to save the day */
        else
            m_image = QImage::fromData (clone.bits(), clone.mappedBytes());

        /* Unmap the frame data */
        clone.unmap();
        publishImage();
    }
}
