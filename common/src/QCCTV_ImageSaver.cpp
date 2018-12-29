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
#include <QImage>
#include <QPainter>
#include <QDateTime>
#include <QFontDatabase>

#define IMAGE_FORMAT "jpg"

#if defined Q_OS_MAC
    #define MONOSPACE_FONT "Menlo"
#elif defined Q_OS_WIN
    #define MONOSPACE_FONT "Consolas"
#else
    #define MONOSPACE_FONT "Monospace"
#endif

/**
 * Initializes the class
 */
QCCTV_ImageSaver::QCCTV_ImageSaver (QObject* parent) : QObject (parent)
{
    QDateTime current = QDateTime::currentDateTime();
    m_hour = current.time().hour();
    m_minute = current.time().minute();
}

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
 */
void QCCTV_ImageSaver::saveImage (const QString& path,
                                  const QString& name,
                                  const QString& address,
                                  const QImage& image)
{
    /* Check if arguments are valid */
    if (path.isEmpty() || name.isEmpty() || address.isEmpty() || image.isNull())
        return;

    /* Copy image (so that we can modify it) */
    QImage copy = image;

    /* Construct strings */
    QDateTime current = QDateTime::currentDateTime();
    QString fmt = current.toString ("dd/MMM/yyyy hh:mm:ss:zzz");

    /* Get font */
    QFont font;
    font.setFamily (MONOSPACE_FONT);
    font.setPixelSize (qMax (copy.height() / 24, 9));
    QFontMetrics metrics (font);

    /* Set painter font */
    QPainter painter (&copy);
    painter.setFont (font);
    painter.setPen (Qt::white);

    /* Get text and background location */
    int w = metrics.width (fmt);
    int h = metrics.height();
    int s = h * .1;

    /* Paint text over image */
    QBrush brush (QColor (0, 0, 0, 100));
    painter.fillRect (QRect (0, 0, w + s, h + s), brush);
    painter.drawText (QRect (s, s, w, h), Qt::AlignTop | Qt::AlignLeft, fmt);

    /* Get recordings directory */
    int hour = current.time().hour();
    int minute = current.time().minute();
    QString f_path = getPath (path, name, address, hour, minute);

    /* Create directory if it does not exist */
    QDir dir = QDir (f_path);
    if (!dir.exists())
        dir.mkpath (".");

    /* Get image name (based on seconds & msecs) */
    QString f_name = QString ("%1_%2.%3")
                     .arg (current.toString ("ss"))
                     .arg (current.toString ("zzz"))
                     .arg (IMAGE_FORMAT);

    /* Save image */
    copy.save (dir.absoluteFilePath (f_name), IMAGE_FORMAT, 100);

    /* If the minute was changed, generate video from all saved images */
    if (minute != m_minute) {
        createMinuteVideo (getPath (path, name, address, m_hour, m_minute));
        m_minute = minute;
    }

    /* If hour was changed, join all one-minute videos into a one-hour video */
    if (hour != m_hour) {
        createHourVideo (getPath (path, name, address, m_hour, m_minute));
        m_hour = hour;
    }
}

/**
 * Generates a MP4 video from all the JPEG images in the given \a path
 */
void QCCTV_ImageSaver::createMinuteVideo (const QString& path)
{
    /* Directory does not exist */
    QDir dir (path);
    if (!dir.exists())
        return;

    /* Get list of JPEG files */
    QStringList validImages;
    QStringList nameFilter = QStringList() << "*." IMAGE_FORMAT;
    QStringList images = dir.entryList (nameFilter,
                                        QDir::Files,
                                        QDir::Name);

    /* Get times between frames */
    QList<QPair<int, int>> times;
    foreach (QString image, images) {
        /* Strip extension from image name */
        QString name = image;
        name = name.replace ("." IMAGE_FORMAT, "");

        /* Get seconds and milliseconds from image name */
        QStringList information = name.split ("_");
        if (information.count() == 2) {
            validImages.append (image);
            times.append (qMakePair (information.at (0).toInt(),
                                     information.at (1).toInt()));
        }
    }

    /* Encode video */
}

/**
 * Joins all the one-minute videos from the given \a path into a one-hour video
 */
void QCCTV_ImageSaver::createHourVideo (const QString& path)
{

}

/**
 * Returns the file path for the given options
 */
QString QCCTV_ImageSaver::getPath (const QString& path,
                                   const QString& name,
                                   const QString& address,
                                   const int hour,
                                   const int minute)
{
    QDateTime current = QDateTime::currentDateTime();
    return QString ("%1/%2/%3/%4/%5/%5 %6/%7 Hours/Minute %8/")
           .arg (path)
           .arg (name)
           .arg (address)
           .arg (current.toString ("yyyy"))
           .arg (current.toString ("MMM"))
           .arg (current.toString ("dd"))
           .arg (hour)
           .arg (minute);
}
