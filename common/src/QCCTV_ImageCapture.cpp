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

/**
 * Initializes the default variables
 */
QCCTV_ImageCapture::QCCTV_ImageCapture (QObject* parent) : QObject (parent)
{
    m_enabled = false;
    connect (&m_probe, SIGNAL (videoFrameProbed (QVideoFrame)),
             this,       SLOT (processVideoFrame (QVideoFrame)));
}

/**
 * Changes the \a enabled state of the grabber
 */
void QCCTV_ImageCapture::setEnabled (const bool enabled)
{
    m_enabled = enabled;
}

/**
 * Changes the source from which we shall obtain (and process) the images
 */
void QCCTV_ImageCapture::setSource (QMediaObject* source)
{
    m_probe.setSource (source);
}

/**
 * Generates a \c QImage from the given \a frame
 */
#include <qdebug.h>
void QCCTV_ImageCapture::processVideoFrame (const QVideoFrame frame)
{
    /* Frame is invalid or grabber is disabled */
    if (!frame.isValid() && !m_enabled)
        return;

    /* Convert the videoframe to an image */
    QVideoFrame clone (frame);
    if (clone.map (QAbstractVideoBuffer::ReadOnly)) {
        QImage::Format format = QVideoFrame::imageFormatFromPixelFormat (clone.pixelFormat());

        /* This is simple, image is supported natively by Qt */
        if (format != QImage::Format_Invalid)
            emit newFrame (QImage (clone.bits(),
                                   clone.width(),
                                   clone.height(),
                                   format));

        /* This is an NV12/NV21 image */
        else if (clone.pixelFormat() == QVideoFrame::Format_NV12 ||
                 clone.pixelFormat() == QVideoFrame::Format_NV21) {
            /* Perform conversion */
            uchar* rgb = new uchar [clone.mappedBytes() * 4];
            bool converted = nv12_to_rgb (rgb,
                                          clone.bits(),
                                          clone.width(),
                                          clone.height());

            /* Conversion OK, generate image */
            if (converted) {
                QImage image ((const uchar*) rgb,
                              clone.width(),
                              clone.height(),
                              QImage::Format_RGB888);

                if (!image.isNull())
                    emit newFrame (image);
            }

            /* Free RGB map */
            if (rgb != NULL)
                delete[] rgb;
        }

        /* God bless you */
        else {
            QImage image = QImage::fromData (clone.bits(), clone.mappedBytes());
            if (!image.isNull())
                emit newFrame (image);
        }

        /* Unmap the frame data */
        clone.unmap();
    }

    qDebug() << "A";
}
