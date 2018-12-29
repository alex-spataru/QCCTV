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

#include "ImageProvider.h"

#include <QCCTV.h>
#include <QCCTV_Station.h>

QCCTV_StationImage::QCCTV_StationImage (QCCTV_Station* parent) :
    QQuickImageProvider (QQuickImageProvider::Image,
                         QQuickImageProvider::ForceAsynchronousImageLoading)
{
    m_station = parent;
    m_cameraError = QCCTV_CreateStatusImage (QSize (640, 480), "CAMERA ERROR");
}

QImage QCCTV_StationImage::requestImage (const QString& id, QSize* size,
                                         const QSize& requestedSize)
{
    QImage result;

    if (m_station && !id.isEmpty()) {
        QImage image = m_station->currentImage (id.toInt());
        if (!image.isNull())
            result = image;
    }

    if (result.isNull())
        result = m_cameraError;

    if (requestedSize.isValid())
        result = result.scaled (requestedSize);

    if (size)
        *size = result.size();

    return result;
}
