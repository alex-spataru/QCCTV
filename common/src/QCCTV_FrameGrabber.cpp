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

QCCTV_FrameGrabber::QCCTV_FrameGrabber (QObject* parent) : QAbstractVideoSurface (parent)
{
    m_ratio = 1.0;
    m_enabled = false;
    m_grayscale = false;
}

bool QCCTV_FrameGrabber::isEnabled() const
{
    return m_enabled;
}

bool QCCTV_FrameGrabber::isGrayscale() const
{
    return m_grayscale;
}

qreal QCCTV_FrameGrabber::shrinkRatio() const
{
    return m_ratio;
}

bool QCCTV_FrameGrabber::present (const QVideoFrame& frame)
{
    /* Grabber is disabled, abort */
    if (!isEnabled())
        return false;

    /* Create variables */
    QImage image;
    QVideoFrame clone (frame);
    clone.map (QAbstractVideoBuffer::ReadOnly);

    /* Get the image from the frame */
    image = QImage (clone.bits(),
                    clone.width(),
                    clone.height(),
                    clone.bytesPerLine(),
                    QVideoFrame::imageFormatFromPixelFormat (clone.pixelFormat()));

    /* Image is invalid, abort */
    if (image.isNull())
        return false;

    /* Resize the image */
    if (shrinkRatio() > 1) {
        image = image.scaled (image.width() / shrinkRatio(),
                              image.height() / shrinkRatio(),
                              Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    /* Do not let the image be larger than 720p */
    if (image.width() > 720)
        image = image.scaledToWidth (720, Qt::SmoothTransformation);

    /* Make the image grayscale */
    if (isGrayscale())
        image = makeGrayscale (image);

    /* Fix mirrored image issue on Windows */
#if defined Q_OS_WIN
    image = image.mirrored (false, true);
#endif

    /* Send image to parent object */
    clone.unmap();
    emit newFrame (image);
    return true;
}

QList<QVideoFrame::PixelFormat> QCCTV_FrameGrabber::supportedPixelFormats
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

QImage QCCTV_FrameGrabber::makeGrayscale (QImage& image)
{
    for (int i = 0; i < image.height(); i++) {
        int depth = 4;
        uchar* scan = image.scanLine (i);

        for (int j = 0; j < image.width(); j++) {
            QRgb* rgbpixel = reinterpret_cast<QRgb*> (scan + j * depth);
            int gray = qGray (*rgbpixel);
            *rgbpixel = QColor (gray, gray, gray).rgba();
        }
    }

    return image;
}
