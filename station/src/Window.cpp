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

/**
 * Configures the toolbar, the status bar and the QCCTV station
 */
Window::Window()
{
    configureStation();
    addToolbarActions();
    updateStatusLabel();
    configureStatusbar();
    setWindowTitle (qApp->applicationName());

    setMinimumSize (640, 420);
}

/**
 * Removes all the registered cameras, which will allow the station to
 * re-establish the connections every camera in the LAN
 */
void Window::rescan()
{
    m_station.removeAllCameras();
}

void Window::showHelp()
{
    QDesktopServices::openUrl (QUrl ("mailto:alex_spataru@outlook.com"));
}

/**
 * Shows the about dialog
 */
void Window::showAbout()
{

}

/**
 * Shows the settings dialog
 */
void Window::showSettings()
{

}

/**
 * Opens the file manager and directs it to the QCCTV recordings directory
 */
void Window::showRecordings()
{

}

/**
 * Toggles between full-screen and normal window, called when the 'fullscreen'
 * icon on the toolbar is clicked
 */
void Window::toggleFullScreen()
{
    if (isFullScreen())
        showNormal();
    else
        showFullScreen();

#ifdef Q_OS_MAC
    ac_full->nativeToolBarItem();
#else
    ac_full->setChecked (isFullScreen());
#endif
}

/**
 * Connects the signals/slots of the QCCTV station and the Window
 */
void Window::configureStation()
{
    connect (&m_station, SIGNAL (cameraCountChanged()),
             this,         SLOT (generateCameraGrid()));
    connect (&m_station, SIGNAL (cameraCountChanged()),
             this,         SLOT (updateStatusLabel()));
}

/**
 * Generates the toolbar actions and binds them to the
 * respective functions of the \c Window
 */
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
    ac_exit = m_toolbar.addItem (ic_exit,       str_exit);
    ac_pref = m_toolbar.addItem (ic_settings,   str_settings);
    ac_rcrd = m_toolbar.addItem (ic_recordings, str_recordings);
    ac_full = m_toolbar.addItem (ic_fullscreen, str_fullscreen);
    ac_help = m_toolbar.addItem (ic_help,       str_help);
    ac_info = m_toolbar.addItem (ic_about,      str_about);
    ac_rscn = m_toolbar.addItem (ic_rescan,     str_rescan);

    /* Connect signals/slots */
    connect (ac_exit, SIGNAL (activated()), this, SLOT (close()));
    connect (ac_rscn, SIGNAL (activated()), this, SLOT (rescan()));
    connect (ac_help, SIGNAL (activated()), this, SLOT (showHelp()));
    connect (ac_info, SIGNAL (activated()), this, SLOT (showAbout()));
    connect (ac_pref, SIGNAL (activated()), this, SLOT (showSettings()));
    connect (ac_rcrd, SIGNAL (activated()), this, SLOT (showRecordings()));
    connect (ac_full, SIGNAL (activated()), this, SLOT (toggleFullScreen()));

    /* Make fullscreen button checkable */
    ac_full->setSelectable (true);

    /* Attach the toolbar to the window */
    window()->winId();
    m_toolbar.attachToWindow (window()->windowHandle());
#endif

    /* Create normal QToolbar */
#ifndef Q_OS_MAC
    ac_exit = m_toolbar.addAction (ic_exit,       str_exit);
    ac_pref = m_toolbar.addAction (ic_settings,   str_settings);
    ac_rcrd = m_toolbar.addAction (ic_recordings, str_recordings);
    ac_full = m_toolbar.addAction (ic_fullscreen, str_fullscreen);
    ac_help = m_toolbar.addAction (ic_help,       str_help);
    ac_info = m_toolbar.addAction (ic_about,      str_about);
    ac_rscn = m_toolbar.addAction (ic_rescan,     str_rescan);

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

/**
 * Updates the text of the label that indicates the number of connected
 * QCCTV Cameras
 */
void Window::updateStatusLabel()
{
    m_cameraCount.setText (tr ("Connected Cameras: %1")
                           .arg (QString::number (m_station.cameraCount())));
}

/**
 * Registers the statusbar with the window and adds the camera count label
 * to the statusbar
 */
void Window::configureStatusbar()
{
    setStatusBar (&m_statusBar);
    m_statusBar.addWidget (&m_cameraCount);
}

/**
 * Generates the layout that arranges the camera display widgets in a
 * dynamically generated grid
 */
void Window::generateCameraGrid()
{
    /* Delete camera widgets */
    for (int i = 0; i < m_cameras.count(); ++i)
        m_cameras.at (i)->deleteLater();

    /* Generate the camera widgets */
    m_cameras.clear();
    for (int i = 0; i < m_station.cameraCount(); ++i) {
        Camera* camera = new Camera (this);
        camera->setCameraID (i);
        camera->setStation (&m_station);
        m_cameras.append (camera);
    }

    /* Calculate ideal size of the grid */
    QPair<int, int> table = calculateGridSize (m_cameras.count());
    int rows = table.first;
    int cols = table.second;

    /* Delete the central widget */
    if (m_widget)
        m_widget->deleteLater();

    /* Create new central widget and layout */
    m_widget = new QWidget (this);
    QGridLayout* layout = new QGridLayout (m_widget);
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

        layout->addWidget (camera->view(), row, col);
    }

    /* Replace the layout of the central widget */
    setCentralWidget (m_widget);
    centralWidget()->setLayout (layout);
}

/**
 * Calculates the ideal number of rows and columns to use to generate a
 * widget table/grid with the given number of \a items
 */
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

    return qMakePair (rows, qMax (columns, 1));
}
