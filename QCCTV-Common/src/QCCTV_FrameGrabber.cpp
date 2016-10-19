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
#include "QCCTV_FrameGrabber.h"

QCCTV_FrameGrabber::QCCTV_FrameGrabber (QObject* parent) : QVideoProbe (parent)
{
    m_ratio = 2.0;
    m_enabled = false;
    m_grayscale = false;

    connect (this, SIGNAL (videoFrameProbed (QVideoFrame)),
             this,   SLOT (processImage (QVideoFrame)));
}

bool QCCTV_FrameGrabber::isEnabled() const
{
    return m_enabled;
}

qreal QCCTV_FrameGrabber::shrinkRatio() const
{
    return m_ratio;
}

bool QCCTV_FrameGrabber::isGrayscale() const
{
    return m_grayscale;
}

void QCCTV_FrameGrabber::setEnabled (const bool enabled)
{
    m_enabled = enabled;
}

void QCCTV_FrameGrabber::setShrinkRatio (const qreal ratio)
{
    if (ratio != 0) {
        if (ratio < 0)
            m_ratio = ratio * -1;
        else
            m_ratio = ratio;
    }
}

void QCCTV_FrameGrabber::setGrayscale (const bool grayscale)
{
    m_grayscale = grayscale;
}

void QCCTV_FrameGrabber::processImage (const QVideoFrame& frame)
{
    if (isEnabled()) {
        QImage image;
        QVideoFrame clone (frame);
        clone.map (QAbstractVideoBuffer::ReadOnly);
        QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat (clone.pixelFormat());

        /* Get the image from the frame */
        if (imageFormat != QImage::Format_Invalid) {
            image = QImage (clone.bits(),
                            clone.width(),
                            clone.height(),
                            clone.bytesPerLine(),
                            imageFormat);
        }

        /* Image is in NV12 or NV21 format */
        else if (clone.pixelFormat() == QVideoFrame::Format_NV12 ||
                 clone.pixelFormat() == QVideoFrame::Format_NV21) {
            uchar* rgb = (uchar*) calloc (1, clone.mappedBytes() * 3);

            nv21_to_rgba (rgb, 1,
                          clone.bits(),
                          clone.width(),
                          clone.height());

            image = QImage (rgb,
                            clone.width(),
                            clone.height(),
                            QImage::Format_ARGB32_Premultiplied);

            free (rgb);
            rgb = NULL;
        }

        /* Last ditch attempt to save the world */
        else
            image = QImage::fromData (clone.bits(), clone.mappedBytes());

        /* Resize the image */
        image = image.scaled (image.width() / shrinkRatio(),
                              image.height() / shrinkRatio(),
                              Qt::KeepAspectRatio);

        /* Notify application */
        clone.unmap();
        emit newFrame (image);
    }
}
