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

Camera::Camera (QWidget* parent) : QGraphicsView (parent)
{
    m_id = -1;
    m_station = NULL;

    /* Configure the graphics view */
    setScene (&m_scene);
    setInteractive (false);
    setFrameShape (QFrame::NoFrame);
    setDragMode (QGraphicsView::NoDrag);
    setCacheMode (QGraphicsView::CacheBackground);
    setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
    setViewportUpdateMode (QGraphicsView::SmartViewportUpdate);
}

Camera::~Camera()
{

}

int Camera::id() const
{
    return m_id;
}

QCCTV_Station* Camera::station() const
{
    return m_station;
}

void Camera::update (const int camera)
{
    /* Camera ID is different */
    if (camera != id())
        return;

    /* QCCTV Station is not assigned */
    if (!station())
        return;

    /* Get camera object */
    QCCTV_RemoteCamera* cam = station()->getCamera (id());

    /* Update camera information */
    if (cam) {
        /* Get the scaled camera pixmap */
        QPixmap image = QPixmap::fromImage (cam->currentImage());
        image = image.scaled (size(),
                              Qt::KeepAspectRatioByExpanding,
                              Qt::SmoothTransformation);

        /* Reset the scene */
        m_scene.clear();
        m_scene.setSceneRect (frameGeometry());

        /* Configure the font */
        QFont font = QFontDatabase::systemFont (QFontDatabase::FixedFont);
        font.setPixelSize (12);

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

void Camera::setCameraID (const int camera)
{
    m_id = camera;
}

void Camera::setStation (QCCTV_Station* station)
{
    if (m_station) {
        disconnect (m_station, SIGNAL (connected           (int)),
                    this,        SLOT (update              (int)));
        disconnect (m_station, SIGNAL (fpsChanged          (int)),
                    this,        SLOT (update              (int)));
        disconnect (m_station, SIGNAL (disconnected        (int)),
                    this,        SLOT (update              (int)));
        disconnect (m_station, SIGNAL (newCameraImage      (int)),
                    this,        SLOT (update              (int)));
        disconnect (m_station, SIGNAL (cameraNameChanged   (int)),
                    this,        SLOT (update              (int)));
        disconnect (m_station, SIGNAL (lightStatusChanged  (int)),
                    this,        SLOT (update              (int)));
        disconnect (m_station, SIGNAL (cameraStatusChanged (int)),
                    this,        SLOT (update              (int)));
    }

    if (station) {
        m_station = station;
        connect (m_station, SIGNAL (connected           (int)),
                 this,        SLOT (update              (int)));
        connect (m_station, SIGNAL (fpsChanged          (int)),
                 this,        SLOT (update              (int)));
        connect (m_station, SIGNAL (disconnected        (int)),
                 this,        SLOT (update              (int)));
        connect (m_station, SIGNAL (newCameraImage      (int)),
                 this,        SLOT (update              (int)));
        connect (m_station, SIGNAL (cameraNameChanged   (int)),
                 this,        SLOT (update              (int)));
        connect (m_station, SIGNAL (lightStatusChanged  (int)),
                 this,        SLOT (update              (int)));
        connect (m_station, SIGNAL (cameraStatusChanged (int)),
                 this,        SLOT (update              (int)));
    }
}
