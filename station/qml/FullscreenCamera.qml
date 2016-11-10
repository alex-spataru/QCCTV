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

import "."

Camera {
    id: cam
    opacity: 0
    enabled: false
    controlsEnabled: true

    function hideCamera() {
        cam.opacity = 0
        cam.enabled = 0
        grid.enabled = 1
    }

    function showCamera (camera) {
        grid.enabled = 0

        cam.opacity = 1
        cam.enabled = 1
        cam.camNumber = camera

        cam.reloadData()
    }

    Connections {
        target: QCCTVStation
        onDisconnected: {
            if (camera === cam.camNumber)
                cam.hideCamera()

            if (QCCTVStation.cameraCount() === 1)
                cam.showCamera (0)
        }

        onCameraCountChanged: {
            cam.returnButtonEnabled = QCCTVStation.cameraCount() > 1

            if (QCCTVStation.cameraCount() === 1)
                cam.showCamera (0)
        }
    }

    Behavior on opacity { NumberAnimation {}}
}
