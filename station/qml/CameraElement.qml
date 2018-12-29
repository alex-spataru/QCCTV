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
    id: element
    property int camNumber: 0

    enabled: visible
    visible: opacity > 0
    width: enabled ? controls.width : 0
    height: enabled ? controls.height : 0

    Behavior on width { NumberAnimation {} }
    Behavior on height { NumberAnimation {} }
    Behavior on opacity { NumberAnimation {} }

    //
    // Update UI when QCCTV emits some signals
    //
    Connections {
        target: QCCTVStation
        onCameraNameChanged: {
            if (camera === camNumber)
                name.text = QCCTVStation.cameraName (camNumber)
        }
    }

    //
    // Controls
    //
    RowLayout {
        id: controls
        spacing: app.spacing
        Layout.fillWidth: true

        //
        // Video image
        //
        Item {
            width: 220
            height: 120

            CameraVideo {
                cameraId: camNumber
                anchors.fill: parent
                enabled: element.enabled
            }
        }

        //
        // Camera name and camera IP
        //
        ColumnLayout {
            spacing: app.spacing
            Layout.fillWidth: true
            Layout.fillHeight: true

            Label {
                id: name
                font.bold: true
                font.pixelSize: 18
                text: QCCTVStation.cameraName (camNumber)
            }

            Label {
                font.pixelSize: 14
                color: app.disabledForegroundColor
                text: QCCTVStation.addressString (camNumber)
            }
        }

        //
        // Spacer
        //
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }

    //
    // Mouse area
    //
    MouseArea {
        anchors.fill: parent
        enabled: parent.enabled
        onClicked: app.showCamera (camNumber)
    }
}
