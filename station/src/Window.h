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
#include <QPair>
#include <QStatusBar>
#include <QMainWindow>
#include <QApplication>

#ifdef Q_OS_MAC
    #include <QMacToolBar>
#else
    #include <QToolBar>
#endif

#include "Camera.h"
#include "QCCTV_Station.h"

class Window : public QMainWindow
{
    Q_OBJECT

public:
    Window();

private slots:
    void rescan();
    void showHelp();
    void showAbout();
    void showSettings();
    void showRecordings();
    void toggleFullScreen();
    void configureStation();
    void addToolbarActions();
    void updateStatusLabel();
    void configureStatusbar();
    void generateCameraGrid();

private:
    QPair<int, int> calculateGridSize (const int items);

private:
    QWidget* m_widget;
    QLabel m_cameraCount;
    QList<Camera*> m_cameras;

    QStatusBar m_statusBar;
    QCCTV_Station m_station;

#ifdef Q_OS_MAC
    QMacToolBar m_toolbar;
    QMacToolBarItem* ac_exit;
    QMacToolBarItem* ac_pref;
    QMacToolBarItem* ac_rcrd;
    QMacToolBarItem* ac_full;
    QMacToolBarItem* ac_help;
    QMacToolBarItem* ac_info;
    QMacToolBarItem* ac_rscn;
#else
    QAction* ac_exit;
    QAction* ac_pref;
    QAction* ac_rcrd;
    QAction* ac_full;
    QAction* ac_help;
    QAction* ac_info;
    QAction* ac_rscn;
    QToolBar m_toolbar;
#endif
};

#endif
