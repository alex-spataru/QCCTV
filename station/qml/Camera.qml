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
import QtQuick.Controls 2.0

Item {
    id: cam

    signal clicked
    property int camNumber: 0
    property string fps: ""
    property string cameraName: ""
    property string cameraStatus: ""

    Component.onCompleted: {
        fps = QCCTVStation.fps (camNumber) + " FPS"
        cameraName = QCCTVStation.cameraName (camNumber)
        cameraStatus = QCCTVStation.statusString (camNumber)
    }

    Connections {
        enabled: true
        target: QCCTVStation

        onFpsChanged: {
            if (camera === camNumber)
                fps = QCCTVStation.fps (camNumber) + " FPS"
        }

        onCameraNameChanged: {
            if (camera === camNumber)
                cameraName = QCCTVStation.cameraName (camNumber)
        }

        onCameraStatusChanged: {
            if (camera === camNumber)
                cameraStatus = QCCTVStation.statusString (camNumber)
        }
    }

    CameraVideo {
        cameraId: camNumber
        anchors.fill: parent
    }

    ColumnLayout {
        spacing: app.spacing / 2

        anchors {
            top: parent.top
            left: parent.left
            margins: app.spacing
        }

        Label {
            text: cameraName
            background: Rectangle {
                opacity: 0.65
                color: app.backgroundColor
            }
        }

        Label {
            text: cameraStatus
            color: app.disabledForegroundColor
            background: Rectangle {
                opacity: 0.65
                color: app.backgroundColor
            }
        }

        Label {
            text: fps
            color: app.disabledForegroundColor
            background: Rectangle {
                opacity: 0.65
                color: app.backgroundColor
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: cam.enabled ? cam.clicked() : undefined
    }
}
