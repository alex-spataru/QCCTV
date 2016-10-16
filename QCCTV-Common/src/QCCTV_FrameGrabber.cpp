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

void QCCTV_FrameGrabber::grayscale (QImage* image)
{
    if (image) {
        for (int i = 0; i < image->height(); i++) {
            uchar* scan = image->scanLine (i);
            int depth = 4;
            for (int j = 0; j < image->width(); j++) {
                QRgb* rgbpixel = reinterpret_cast<QRgb*> (scan + j * depth);
                int gray = qGray (*rgbpixel);
                *rgbpixel = QColor (gray, gray, gray).rgba();
            }
        }
    }
}

void QCCTV_FrameGrabber::processImage (const QVideoFrame& frame)
{
    if (isEnabled()) {
        QImage image;
        QVideoFrame clone (frame);
        clone.map (QAbstractVideoBuffer::ReadOnly);
        QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat (clone.pixelFormat());

        /* We know the frame format, construct the image directly */
        if (imageFormat != QImage::Format_Invalid) {
            image = QImage (clone.bits(),
                            clone.width(),
                            clone.height(),
                            imageFormat);
        }

        /* Construct the image using the raw bytes */
        else
            image = QImage::fromData (clone.bits(), clone.mappedBytes());

        /* Image is invalid, abort */
        if (image.isNull())
            return;

        /* Resize the image */
        image = image.scaled (image.width() / shrinkRatio(),
                              image.height() / shrinkRatio(),
                              Qt::KeepAspectRatio);

        /* Make the image black and white */
        if (isGrayscale())
            grayscale (&image);

        /* Notify application */
        clone.unmap();
        emit newFrame (QPixmap::fromImage (image));
    }
}
