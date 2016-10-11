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

#include "QCCTV_CameraFrame.h"

QCCTV_CameraFrameGrabber::QCCTV_CameraFrameGrabber (QObject* parent) :
    QAbstractVideoSurface (parent)
{

    m_enabled = false;
    m_scaleRatio = 1.5;
    m_orientation = 180;
    m_grayscale = false;
}

QList<QVideoFrame::PixelFormat> QCCTV_CameraFrameGrabber::supportedPixelFormats
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

bool QCCTV_CameraFrameGrabber::isEnabled() const
{
    return m_enabled;
}

qreal QCCTV_CameraFrameGrabber::scaleRatio() const
{
    return m_scaleRatio;
}

bool QCCTV_CameraFrameGrabber::isGrayscale() const
{
    return m_grayscale;
}

qreal QCCTV_CameraFrameGrabber::orientation() const
{
    return m_orientation;
}

bool QCCTV_CameraFrameGrabber::present (const QVideoFrame& frame)
{
    if (frame.isValid()) {
        if (isEnabled()) {
            QImage image;
            QVideoFrame clone (frame);
            clone.map (QAbstractVideoBuffer::ReadOnly);
            QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat (
                                             clone.pixelFormat());

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

            /* Resize the image */
            image = image.scaled (image.width() / scaleRatio(),
                                  image.height() / scaleRatio(),
                                  Qt::KeepAspectRatio);

            /* Make the image black and white */
            if (isGrayscale())
                grayscale (&image);

            /* Rotate image */
            QTransform transform;
            transform.rotate (180 - orientation(), Qt::ZAxis);
            image = image.transformed (transform);

            /* Notify application */
            clone.unmap();
            emit newFrame (QPixmap::fromImage (image));
        }

        return true;
    }

    return false;
}

void QCCTV_CameraFrameGrabber::setEnabled (const bool enabled)
{
    m_enabled = enabled;
}

void QCCTV_CameraFrameGrabber::setScaleRatio (const qreal scale)
{
    if (scale >= 1)
        m_scaleRatio = scale;
}

void QCCTV_CameraFrameGrabber::setGrayscale (const bool grayscale)
{
    m_grayscale = grayscale;
}

void QCCTV_CameraFrameGrabber::setOrientation (const qreal orientation)
{
    m_orientation = orientation;
}

void QCCTV_CameraFrameGrabber::grayscale (QImage* image)
{
    if (image) {
        for (int ii = 0; ii < image->height(); ii++) {
            uchar* scan = image->scanLine (ii);
            int depth = 4;
            for (int jj = 0; jj < image->width(); jj++) {

                QRgb* rgbpixel = reinterpret_cast<QRgb*> (scan + jj * depth);
                int gray = qGray (*rgbpixel);
                *rgbpixel = QColor (gray, gray, gray).rgba();
            }
        }
    }
}
