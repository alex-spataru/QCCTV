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
import QtQuick.Controls 2.0

Item {
    id: camera

    signal back
    property int id: 0
    property bool controlsEnabled: false
    property bool returnButtonEnabled: QCCTVStation.cameraCount() > 1

    //
    // React to QCCTV events
    //
    Connections {
        target: QCCTVStation
        onCameraCountChanged: {
            camera.returnButtonEnabled = QCCTVStation.cameraCount() > 1
        }

        onCameraStatusChanged: {
            if (camera === id)
                status.text = QCCTVStation.statusString (id)
        }

        onNewCameraImage: {
            if (camera === id) {
                image.source = ""
                image.source = "image://qcctv/" + id
            }
        }

        onCameraNameChanged: {
            if (camera === id) {
                name.text = QCCTVStation.cameraName (id)
                altName.text = QCCTVStation.cameraName (id)
            }
        }

        onFpsChanged: {
            if (camera === id)
                fps.text = QCCTVStation.fps (id) + " FPS"
        }
    }

    //
    // Latest image
    //
    Image {
        id: image
        anchors.fill: parent

        //
        // Status label
        //
        Label {
            id: status
            color: "#888"
            font.family: app.fontFamily
            text: QCCTVStation.statusString (id)

            anchors {
                right: parent.right
                bottom: parent.bottom
                margins: app.spacing
            }

            opacity: camera.controlsEnabled ? 1 : 0
            Behavior on opacity { NumberAnimation {} }
        }

        //
        // Alternative name label
        //
        Label {
            id: altName
            color: "#fff"
            font.family: app.fontFamily
            text: QCCTVStation.cameraName (id)

            anchors {
                top: parent.top
                left: parent.left
                margins: app.spacing
            }

            opacity: camera.controlsEnabled ? 0 : 1
            Behavior on opacity { NumberAnimation {} }
        }

        //
        // Allow user to enlarge camera image to fill app screen
        //
        MouseArea {
            anchors.fill: parent
            onClicked: camera.controlsEnabled = !camera.controlsEnabled
        }
    }

    //
    // Back button
    //
    Rectangle {
        id: back

        //
        // Geometry specifications
        //
        radius: 2
        border.width: 1
        width: camera.returnButtonEnabled ? 24 + app.spacing : 0
        height: camera.returnButtonEnabled > 0 ? 24 + app.spacing : 0

        //
        // Colors
        //
        color: "#444"
        border.color: "#999"

        //
        // Control visibility
        //
        opacity: camera.controlsEnabled ? 0.85 : 0

        //
        // Animations
        //
        Behavior on width { NumberAnimation {} }
        Behavior on height { NumberAnimation {} }
        Behavior on opacity { NumberAnimation {} }
        Behavior on anchors.margins { NumberAnimation {} }

        //
        // Layout options
        //
        anchors {
            top: image.top
            left: image.left
            margins: camera.returnButtonEnabled ? app.spacing : 0
        }

        //
        // Icon
        //
        Image {
            anchors.centerIn: parent
            sourceSize: Qt.size (18, 18)
            source: "qrc:/images/back.png"
        }

        //
        // Mouse area
        //
        MouseArea {
            anchors.fill: parent
            onClicked: camera.back()
            enabled: camera.returnButtonEnabled
        }
    }

    //
    // Top status bar
    //
    Rectangle {
        id: menu

        //
        // Geometry specifications
        //
        radius: 2
        border.width: 1
        height: 24 + app.spacing

        //
        // Colors
        //
        color: "#444"
        border.color: "#999"

        //
        // Control visibility
        //
        opacity: camera.controlsEnabled ? 0.85 : 0
        Behavior on opacity { NumberAnimation {} }

        //
        // Layout options
        //
        anchors {
            top: image.top
            left: back.right
            right: image.right
            margins: app.spacing
        }

        //
        // Camera name
        //
        Label {
            id: name
            color: "#fff"
            font.family: app.fontFamily
            text: QCCTVStation.cameraName (id)

            anchors {
                left: parent.left
                margins: app.spacing
                verticalCenter: parent.verticalCenter
            }
        }

        //
        // FPS indicator
        //
        Label {
            id: fps
            color: "#fff"
            font.family: app.fontFamily
            text: QCCTVStation.fps (id) + " FPS"

            anchors {
                right: parent.right
                margins: app.spacing
                verticalCenter: parent.verticalCenter
            }
        }
    }
}
