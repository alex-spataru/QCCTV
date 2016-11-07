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

#include "Window.h"

#include <QUrl>
#include <QGridLayout>
#include <QApplication>
#include <QDesktopServices>

Window::Window()
{
    configureStation();
    addToolbarActions();
    updateStatusLabel();
    configureStatusbar();
    setWindowTitle (qApp->applicationName());

    setMinimumSize (640, 420);
}

Window::~Window()
{

}

void Window::rescan()
{
    m_station.removeAllCameras();
}

void Window::showHelp()
{
    QDesktopServices::openUrl (QUrl ("mailto:alex_spataru@outlook.com"));
}

void Window::showAbout()
{

}

void Window::showSettings()
{

}

void Window::showRecordings()
{

}

void Window::toggleFullScreen()
{
    if (isFullScreen())
        showNormal();
    else
        showFullScreen();
}

void Window::configureStation()
{
    connect (&m_station, SIGNAL (connected (int)),
             this,         SLOT (addCamera (int)));
    connect (&m_station, SIGNAL (disconnected (int)),
             this,         SLOT (removeCamera (int)));
}

void Window::addToolbarActions()
{
    /* Set action strings */
    QString str_exit = tr ("Exit");
    QString str_help = tr ("Help");
    QString str_about = tr ("About");
    QString str_rescan = tr ("Rescan");
    QString str_settings = tr ("Settings");
    QString str_recordings = tr ("Recordings");
    QString str_fullscreen = tr ("Fullscreen");

    /* Set action icons */
    QIcon ic_exit = QIcon (":/icons/toolbar/exit.svg");
    QIcon ic_help = QIcon (":/icons/toolbar/help.svg");
    QIcon ic_about = QIcon (":/icons/toolbar/about.svg");
    QIcon ic_rescan = QIcon (":/icons/toolbar/rescan.svg");
    QIcon ic_settings = QIcon (":/icons/toolbar/settings.svg");
    QIcon ic_fullscreen = QIcon (":/icons/toolbar/display.svg");
    QIcon ic_recordings = QIcon (":/icons/toolbar/recording.svg");

    /* Create OS X toolbar */
#ifdef Q_OS_MAC
    QMacToolBarItem* i_exit = m_toolbar.addItem (ic_exit,       str_exit);
    QMacToolBarItem* i_pref = m_toolbar.addItem (ic_settings,   str_settings);
    QMacToolBarItem* i_rcrd = m_toolbar.addItem (ic_recordings, str_recordings);
    QMacToolBarItem* i_full = m_toolbar.addItem (ic_fullscreen, str_fullscreen);
    QMacToolBarItem* i_help = m_toolbar.addItem (ic_help,       str_help);
    QMacToolBarItem* i_info = m_toolbar.addItem (ic_about,      str_about);
    QMacToolBarItem* i_rscn = m_toolbar.addItem (ic_rescan,     str_rescan);

    /* Connect signals/slots */
    connect (i_exit, SIGNAL (activated()), this, SLOT (close()));
    connect (i_rscn, SIGNAL (activated()), this, SLOT (rescan()));
    connect (i_help, SIGNAL (activated()), this, SLOT (showHelp()));
    connect (i_info, SIGNAL (activated()), this, SLOT (showAbout()));
    connect (i_pref, SIGNAL (activated()), this, SLOT (showSettings()));
    connect (i_rcrd, SIGNAL (activated()), this, SLOT (showRecordings()));
    connect (i_full, SIGNAL (activated()), this, SLOT (toggleFullScreen()));

    /* Make fullscreen button checkable */
    i_full->setSelectable (true);

    /* Attach the toolbar to the window */
    window()->winId();
    m_toolbar.attachToWindow (window()->windowHandle());
#endif

    /* Create normal QToolbar */
#ifndef Q_OS_MAC
    QAction* ac_exit = m_toolbar.addAction (ic_exit,       str_exit);
    QAction* ac_pref = m_toolbar.addAction (ic_settings,   str_settings);
    QAction* ac_rcrd = m_toolbar.addAction (ic_recordings, str_recordings);
    QAction* ac_full = m_toolbar.addAction (ic_fullscreen, str_fullscreen);
    QAction* ac_help = m_toolbar.addAction (ic_help,       str_help);
    QAction* ac_info = m_toolbar.addAction (ic_about,      str_about);
    QAction* ac_rscn = m_toolbar.addAction (ic_rescan,     str_rescan);

    /* Connect signals/slots */
    connect (ac_exit, SIGNAL (triggered()), this, SLOT (close()));
    connect (ac_rscn, SIGNAL (triggered()), this, SLOT (rescan()));
    connect (ac_help, SIGNAL (triggered()), this, SLOT (showHelp()));
    connect (ac_info, SIGNAL (triggered()), this, SLOT (showAbout()));
    connect (ac_pref, SIGNAL (triggered()), this, SLOT (showSettings()));
    connect (ac_rcrd, SIGNAL (triggered()), this, SLOT (showRecordings()));
    connect (ac_full, SIGNAL (triggered()), this, SLOT (toggleFullScreen()));

    /* Make the fullscreen button checkable */
    ac_full->setCheckable (true);

    /* Set toolbar options */
    m_toolbar.setMovable (false);
    m_toolbar.setIconSize (QSize (24, 24));
    m_toolbar.setToolButtonStyle (Qt::ToolButtonTextUnderIcon);

    /* Register the toolbar with the window */
    addToolBar (&m_toolbar);
#endif
}

void Window::updateStatusLabel()
{
    m_cameraCount.setText (tr ("Connected Cameras: %1")
                           .arg (QString::number (m_cameras.count())));
}

void Window::configureStatusbar()
{
    setStatusBar (&m_statusBar);
    m_statusBar.addWidget (&m_cameraCount);
}

void Window::generateCameraGrid()
{
    /* Calculate ideal size of the grid */
    QPair<int, int> table = calculateGridSize (m_cameras.count());
    int rows = table.first;
    int cols = table.second;

    /* Delete central widget */
    if (centralWidget()) {
        QWidget* widget = centralWidget();
        delete widget;
    }

    /* Create new central widget and layout */
    QWidget* widget = new QWidget (this);
    QGridLayout* layout = new QGridLayout (widget);
    layout->setContentsMargins (0, 0, 0, 0);

    /* Add each camera to the grid */
    int row = 0;
    int col = -1;
    foreach (Camera* camera, m_cameras) {
        if (col < (cols - 1))
            ++col;

        else if (row < (rows - 1)) {
            ++row;
            col = 0;
        }

        layout->addWidget (camera, row, col);
    }

    /* Change the central widget */
    widget->setLayout (layout);
    setCentralWidget (widget);
}

void Window::addCamera (const int camera)
{
    Camera* cam = new Camera (this);
    cam->setCameraID (camera);
    cam->setStation (&m_station);
    m_cameras.append (cam);

    updateStatusLabel();
    generateCameraGrid();
}

void Window::removeCamera (const int camera)
{
    if (m_cameras.count() > camera)
        m_cameras.removeAt (camera);

    updateStatusLabel();
    generateCameraGrid();
}

QPair<int, int> Window::calculateGridSize (const int items)
{
    int rows = 0;
    int columns = 0;
    bool toggled = false;

    while (rows * columns < items) {
        if (toggled)
            ++columns;
        else
            ++rows;

        toggled = !toggled;
    }

    return qMakePair (rows, columns);
}
