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
import QtQuick.Layouts 1.0
import Qt.labs.settings 1.0
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtQuick.Controls.Universal 2.0

import "."

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

        Material.theme = Material.Dark
        Universal.theme = Universal.Dark

        Material.accent = Material.Teal
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
            grid.redraw()
            loadingScreen.opacity = QCCTVStation.cameraCount() > 0 ? 0 : 1
        }
    }

    //
    // Camera grid
    //
    GridView {
        id: grid
        anchors.fill: parent

        //
        // Re-sizes the cells to fit the application window size
        //
        function redraw() {
            /* Reset the model */
            model = 0
            model = QCCTVStation.cameraCount()

            /* There is only one camera, show full screen controller */
            if (model === 1) {
                enabled = false
                fullscreenCamera.showCamera (0)
            } else if (fullscreenCamera.enabled) {
                fullscreenCamera.hideCamera()
            }

            /* Initialize variables */
            var cols = 1
            var rows = 1
            var toggler = false

            /* Calculate ideal rows and columns size */
            while (cols * rows < model) {
                if (toggler)
                    rows += 1
                else
                    cols += 1

                toggler = !toggler
            }

            /* Force the UI to generate a 2x2 grid when having 2 cams */
            if (model === 2) {
                rows = 2
                cols = 2
            }

            /* Get available size */
            var w = app.width
            var h = app.height

            /* Set initial size */
            cellWidth = w / Math.max (rows, 1)
            cellHeight = h / Math.max (cols, 1)

            /* Ensure that cells are not too small */
            cellWidth = Math.max (cellWidth, Math.max (w / 12, 140))
            cellHeight = Math.max (cellHeight, Math.max (h / 9, 100))
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
            onClicked: fullscreenCamera.showCamera (camNumber)
        }
    }

    //
    // Fullscreen camera (shown when user clicks on a camera)
    //
    FullscreenCamera {
        id: fullscreenCamera
        anchors.fill: parent
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
