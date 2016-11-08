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

import QtQuick 2.0
import QtMultimedia 5.4
import QtQuick.Layouts 1.0
import QtQuick.Controls 1.0
import Qt.labs.settings 1.0

import "."
import "qrc:/common/"

ApplicationWindow {
    id: app
    width: 840
    height: 520
    color: "#000"
    visible: true
    x: isMobile ? 0 : 100
    y: isMobile ? 0 : 100
    title: AppDspName + " " + AppVersion

    //
    // Global variables
    //
    property int spacing: 8

    //
    // Resize grid cells when user resizes window
    //
    onWidthChanged: grid.redraw()
    onHeightChanged: grid.redraw()

    //
    // Show window correctly on mobile devices
    //
    Component.onCompleted: {
        if (isMobile)
            showMaximized()
    }

    //
    // Settings
    //
    Settings {
        property alias x: app.x
        property alias y: app.y
        property alias width: app.width
        property alias height: app.height
    }

    //
    // QCCTV signals/slots
    //
    Connections {
        target: QCCTVStation
        onCameraCountChanged: {
            grid.model = 0
            grid.model = QCCTVStation.cameraCount()
            loadingScreen.opacity = QCCTVStation.cameraCount() > 0 ? 0 : 1
            grid.redraw()
        }
    }

    //
    // Camera grid
    //
    GridView {
        id: grid
        focus: false
        anchors.fill: parent

        //
        // Re-sizes the cells to fit the application window size
        //
        function redraw() {
            var h = 1
            var w = 1
            var bool = false

            while (h * w < model) {
                if (bool)
                    w += 1
                else
                    h += 1

                bool = !bool
            }

            /* Set initial size */
            cellWidth = app.width / Math.max (w, 1)
            cellHeight = app.height / Math.max (h, 1)

            /* Ensure that cells are not too small */
            cellWidth = Math.max (cellWidth, Math.max (app.width / 12, 140))
            cellHeight = Math.max (cellHeight, Math.max (app.height / 9, 100))
        }

        //
        // Fade while hidding or showing
        //
        opacity: enabled ? 1 : 0
        Behavior on opacity { NumberAnimation {} }

        //
        // Camera view object
        //
        delegate: Camera {
            id: cam
            camNumber: index
            enabled: grid.enabled
            width: grid.cellWidth
            height: grid.cellHeight
            controlsEnabled: grid.model === 1
            onClicked: fullscreenCamera.showCamera (camNumber)
        }
    }

    //
    // Fullscreen camera (shown when user clicks on a camera)
    //
    FullscreenCamera {
        id: fullscreenCamera
    }

    //
    // QCCTV loading screen (shown when there are no cameras)
    //
    LoadingScreen {
        id: loadingScreen
        anchors.fill: parent
        opacity: QCCTVStation.cameraCount() > 0 ? 0 : 1

        Behavior on opacity { NumberAnimation{} }
    }
}
