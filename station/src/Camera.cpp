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

#include "Camera.h"
#include "QCCTV_Station.h"

#include <QFont>
#include <QDateTime>
#include <QFontMetrics>
#include <QFontDatabase>
#include <QGraphicsTextItem>
#include <QGraphicsPixmapItem>

Camera::Camera (QObject* parent) : QObject (parent)
{
    m_id = -1;
    m_station = NULL;
    m_view = new QGraphicsView;

    view()->setScene (&m_scene);
    view()->setFrameShape (QFrame::NoFrame);
    view()->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
    view()->setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
}

/**
 * Deletes the graphics view object
 */
Camera::~Camera()
{
    if (view())
        view()->deleteLater();
}

/**
 * Returns the camera that the widget obtains information from
 */
int Camera::id() const
{
    return m_id;
}

/**
 * Returns a pointer to the graphics view object
 */
QGraphicsView* Camera::view()
{
    return m_view;
}

/**
 * Returns a pointer to the \c QCCTV_Station that the widget uses to
 * obtain camera data from
 */
QCCTV_Station* Camera::station() const
{
    return m_station;
}

/**
 * Regenerates the information and image displayed by the widget
 */
void Camera::update (const int camera)
{
    /* The signal is not for us */
    if (camera != id())
        return;

    /* Update camera information */
    QCCTV_RemoteCamera* cam = station()->getCamera (id());
    if (cam) {
        /* Get the scaled camera pixmap */
        QPixmap image = QPixmap::fromImage (cam->currentImage());
        image = image.scaled (view()->frameGeometry().size(),
                              Qt::KeepAspectRatioByExpanding,
                              Qt::SmoothTransformation);

        /* Reset the scene */
        m_scene.clear();
        m_scene.setSceneRect (view()->frameGeometry());

        /* Configure the font */
        QFont font = QFontDatabase::systemFont (QFontDatabase::FixedFont);
        font.setPixelSize (qMin (12, view()->frameGeometry().height() / 12));

        /* Generate texts */
        QString fmt = "dd/MMM/yyyy hh:mm:ss";
        QString time_str = QDateTime::currentDateTime().toString (fmt);
        QString title_str = QString ("%1 @ %2 FPS")
                            .arg (cam->cameraName())
                            .arg (QString::number (cam->fps()));

        /* Add items to the scene */
        m_scene.addPixmap (image);
        QGraphicsTextItem* time = m_scene.addText (time_str, font);
        QGraphicsTextItem* title = m_scene.addText (title_str, font);

        /* Set the text positions */
        int spacing = 4;
        QFontMetrics fm (font);
        title->setPos (spacing, spacing);
        time->setPos (spacing, fm.height() + spacing);

        /* Set text colors */
        time->setDefaultTextColor (Qt::green);
        title->setDefaultTextColor (Qt::green);
    }
}

/**
 * Changes the camera ID from which the widget shall obtain new data
 */
void Camera::setCameraID (const int camera)
{
    m_id = camera;
}

/**
 * Changes the \c QCCTV_Station from which the widget obtains new camera
 * information
 */
void Camera::setStation (QCCTV_Station* station)
{
    if (station) {
        m_station = station;
        connect (m_station, SIGNAL (newCameraImage (int)),
                 this,        SLOT (update         (int)));
    }
}
