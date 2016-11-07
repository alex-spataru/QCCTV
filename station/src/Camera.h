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

#ifndef _CAMERA_H
#define _CAMERA_H

#include <QLabel>
#include <QGraphicsView>
#include <QGraphicsScene>

class QCCTV_Station;

class Camera : public QGraphicsView
{
    Q_OBJECT

public:
    Camera (QWidget* parent = Q_NULLPTR);
    ~Camera();

    int id() const;
    QCCTV_Station* station() const;

public slots:
    void update (const int camera);
    void setCameraID (const int camera);
    void setStation (QCCTV_Station* station);

private:
    int m_id;
    QLabel m_status;
    QGraphicsScene m_scene;
    QCCTV_Station* m_station;
};

#endif
