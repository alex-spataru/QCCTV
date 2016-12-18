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

#include "QCCTV.h"
#include "QCCTV_ImageSaver.h"

#include <QDir>
#include <QPen>
#include <QFont>
#include <QPainter>
#include <QDateTime>
#include <QFontDatabase>

#if defined Q_OS_MAC
    #define MONOSPACE_FONT "Menlo"
#elif defined Q_OS_WIN
    #define MONOSPACE_FONT "Consolas"
#else
    #define MONOSPACE_FONT "Monospace"
#endif

/**
 * Adds some informational text in the upper-right corner of the given
 * image and saves it in the given \a path
 *
 * \param path the path to the folder in which to save the image
 * \param name the camera name, used for creating a dedicated folder for the
 *        camera stream in question
 * \param address the host address of the camera, its used to create an
 *        additional directory under the name folder to avoid saving
 *        conflicting streams from two or more cameras with the same name
 * \param image the image to save
 * \param quality the JPEG quality level to apply to the saved image [0-100]
 */
void QCCTV_ImageSaver::saveImage (const QString& path,
                                  const QString& name,
                                  const QString& address,
                                  const QImage& image,
                                  const int quality)
{
    /* Check if arguments are valid */
    if (path.isEmpty() || name.isEmpty() || address.isEmpty() || image.isNull())
        return;

    /* Copy image (so that we can modify it) */
    QImage copy = image;

    /* Construct strings */
    QDateTime current = QDateTime::currentDateTime();
    QString fmt = current.toString ("dd/MMM/yyyy hh:mm:ss:zzz");
    QString str = name + "\n" + fmt;

    /* Get font */
    QFont font;
    font.setFamily (MONOSPACE_FONT);
    font.setPixelSize (qMax (copy.height() / 24, 9));
    QFontMetrics fm (font);

    /* Get text location */
    QRect rect (fm.height() / 2, fm.height() / 2,
                copy.width(), copy.height());

    /* Paint text over image */
    QPainter painter (&copy);
    painter.setFont (font);
    painter.setPen (QPen (Qt::green));
    painter.drawText (rect, Qt::AlignTop | Qt::AlignLeft, str);

    /* Get recordings directory */
    QString f_path = QString ("%1/%2/%3/%4/%5/%5 %6/%7 Hours/Minute %8/")
                     .arg (path)
                     .arg (name)
                     .arg (address)
                     .arg (current.toString ("yyyy"))
                     .arg (current.toString ("MMM"))
                     .arg (current.toString ("dd"))
                     .arg (current.toString ("hh"))
                     .arg (current.toString ("mm"));

    /* Create directory if it does not exist */
    QDir dir = QDir (f_path);
    if (!dir.exists())
        dir.mkpath (".");

    /* Get image name (based on seconds & msecs) */
    QString f_name = QString ("%1 sec %2 ms.%3")
                     .arg (current.toString ("ss"))
                     .arg (current.toString ("zzz"))
                     .arg ("jpg");

    /* Save image */
    copy.save (dir.absoluteFilePath (f_name), "jpg", quality);
}
