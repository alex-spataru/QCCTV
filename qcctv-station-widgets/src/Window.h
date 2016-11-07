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

#ifndef _WINDOW_H
#define _WINDOW_H

#include <QList>
#include <QStatusBar>
#include <QMainWindow>
#include <QGridLayout>
#include <QGridLayout>
#include <QApplication>

#ifdef Q_OS_MAC
    #include <QMacToolBar>
    #define QCCTV_Toolbar QMacToolBar
#else
    #include <QToolBar>
    #define QCCTV_Toolbar QToolBar
#endif

#include "Camera.h"
#include "QCCTV_Station.h"

class Window : public QMainWindow
{
    Q_OBJECT

public:
    Window();
    ~Window();

private slots:
    void rescan();
    void showHelp();
    void showAbout();
    void showSettings();
    void checkForUpdates();
    void configureStation();
    void addToolbarActions();
    void configureStatusbar();
    void generateCameraGrid();
    void addCamera (const int camera);
    void removeCamera (const int camera);

private:
    QList<Camera*> m_cameras;

    QStatusBar m_statusBar;
    QCCTV_Station m_station;
    QCCTV_Toolbar m_toolbar;
};

#endif
