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
import "qrc:/common/"

Item {
    id: cam

    //
    // Signals
    //
    signal back
    signal clicked

    //
    // Properties
    //
    property int camNumber: 0
    property bool controlsEnabled: !returnButtonEnabled
    property bool returnButtonEnabled: QCCTVStation.cameraCount() > 1

    //
    // Loads the same image ID by changing URL (to trick the image element
    // and force it to perform a redraw)
    //
    function reloadImage() {
        image.source = "image://qcctv/reload/" + camNumber
        image.source = "image://qcctv/" + camNumber
        image.sourceChanged (image.source)
    }

    //
    // Reloads the information displayed by the camera object
    //
    function reloadData() {
        reloadImage()
        fps.text = QCCTVStation.fps (camNumber) + " FPS"
        name.text = QCCTVStation.cameraName (camNumber)
        altName.text = QCCTVStation.cameraName (camNumber)
        status.text = QCCTVStation.statusString (camNumber)
    }

    //
    // Load data during initialization
    //
    Component.onCompleted: reloadData()

    //
    // React to QCCTV events
    //
    Connections {
        enabled: true
        target: QCCTVStation

        onCameraStatusChanged: {
            if (camera === camNumber)
                status.text = QCCTVStation.statusString (camNumber)
        }

        onNewCameraImage: {
            if (camera === camNumber)
                reloadImage()
        }

        onCameraNameChanged: {
            if (camera === camNumber) {
                name.text = QCCTVStation.cameraName (camNumber)
                altName.text = QCCTVStation.cameraName (camNumber)
            }
        }

        onFpsChanged: {
            if (camera === camNumber)
                fps.text = QCCTVStation.fps (camNumber) + " FPS"
        }
    }

    //
    // Camera image
    //
    Image {
        id: image
        cache: false
        smooth: false
        anchors.fill: cam
        fillMode: Image.PreserveAspectCrop

        //
        // Status label
        //
        Label {
            id: status
            color: "#888"
            text: QCCTVStation.statusString (camNumber)

            anchors {
                right: parent.right
                bottom: parent.bottom
                margins: app.spacing
            }

            opacity: cam.controlsEnabled ? 1 : 0
            Behavior on opacity { NumberAnimation {} }
        }

        //
        // Alternative name label
        //
        Label {
            id: altName
            text: QCCTVStation.cameraName (camNumber)
            font.pixelSize: Math.min (14, image.height / 12)

            anchors {
                top: parent.top
                left: parent.left
                margins: app.spacing
            }

            opacity: cam.controlsEnabled ? 0 : 1
            Behavior on opacity { NumberAnimation {} }
        }

        //
        // Allow user to enlarge camera image to fill app screen
        //
        MouseArea {
            anchors.fill: parent
            onClicked: cam.enabled ? cam.clicked() : undefined
        }
    }

    //
    // Back button
    //
    Panel {
        id: back
        opacity: cam.controlsEnabled ? 1 : 0
        width: cam.returnButtonEnabled ? menu.height : 0
        height: cam.returnButtonEnabled > 0 ? menu.height : 0

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
            margins: cam.returnButtonEnabled ? app.spacing : 0
        }

        //
        // Icon
        //
        Image {
            anchors.centerIn: parent
            sourceSize: Qt.size (18, 18)
            visible: cam.returnButtonEnabled
            source: "qrc:/common/images/back.png"
        }

        //
        // Mouse area
        //
        MouseArea {
            anchors.fill: parent
            onClicked: cam.back()
            enabled: cam.returnButtonEnabled && cam.enabled
        }
    }

    //
    // Top status bar
    //
    Panel {
        id: menu
        height: 24 + app.spacing
        opacity: cam.controlsEnabled ? 1 : 0

        anchors {
            top: image.top
            left: back.right
            right: image.right
            margins: app.spacing
        }

        Label {
            id: name
            text: QCCTVStation.cameraName (camNumber)

            anchors {
                left: parent.left
                margins: app.spacing
                verticalCenter: parent.verticalCenter
            }
        }

        Label {
            id: fps
            text: QCCTVStation.fps (camNumber) + " FPS"

            anchors {
                right: parent.right
                margins: app.spacing
                verticalCenter: parent.verticalCenter
            }
        }
    }
}
