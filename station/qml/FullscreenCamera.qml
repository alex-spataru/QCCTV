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
import QtQuick.Controls.Material 2.0
import QtQuick.Controls.Universal 2.0

Item {
    id: cam

    //
    // Properties
    //
    property int fps: 0
    property int camNumber: 0
    property int resolution: 0
    property bool flashOn: false
    property string cameraName: ""
    property string cameraStatus: ""

    //
    // Update items automatically
    //
    onFpsChanged: fpsSpinbox.value = fps
    onResolutionChanged: resolutions.currentIndex = resolution

    //
    // Obtains latest camera data from QCCTV
    //
    function reloadData() {
        fps = QCCTVStation.fps (camNumber)
        resolution = QCCTVStation.resolution (camNumber)
        cameraName = QCCTVStation.cameraName (camNumber)
        flashOn = QCCTVStation.flashlightEnabled (camNumber)
        cameraStatus = QCCTVStation.statusString (camNumber)

        image.source = "image://qcctv/reload/" + camNumber
        image.source = "image://qcctv/" + camNumber
        image.sourceChanged (image.source)
    }

    //
    // Hides the camera widget
    //
    function hideCamera() {
        opacity = 0
        enabled = 0
        grid.enabled = 1
    }

    //
    // Shows the camera widge
    //
    function showCamera (camera) {
        grid.enabled = 0

        opacity = 1
        enabled = 1
        camNumber = camera

        reloadData()
    }

    //
    // Load initial camera data during creation
    //
    Component.onCompleted: reloadData()

    //
    // React to QCCTV events
    //
    Connections {
        enabled: true
        target: QCCTVStation

        onNewCameraImage: {
            if (camera === camNumber && enabled) {
                image.source = "image://qcctv/reload/" + camNumber
                image.source = "image://qcctv/" + camNumber
                image.sourceChanged (image.source)
            }
        }

        onFpsChanged: {
            if (camera === camNumber && enabled)
                fps = QCCTVStation.fps (camNumber)
        }

        onLightStatusChanged: {
            if (camera === camNumber && enabled)
                flashOn = QCCTVStation.flashlightEnabled (camNumber)
        }

        onResolutionChanged: {
            if (camera === camNumber && enabled)
                resolution = QCCTVStation.resolution (camNumber)
        }

        onDisconnected: {
            if (camera === cam.camNumber)
                hideCamera()

            if (QCCTVStation.cameraCount() === 1)
                showCamera (0)
        }

        onCameraNameChanged: {
            if (camera === camNumber)
                cameraName = QCCTVStation.cameraName (camNumber)
        }

        onCameraStatusChanged: {
            if (camera === camNumber)
                cameraStatus = QCCTVStation.statusString (camNumber)
        }

        onCameraCountChanged: {
            backButton.visible = QCCTVStation.cameraCount() > 1

            if (QCCTVStation.cameraCount() === 1)
                showCamera (0)

            if (QCCTVStation.cameraCount() < 1 && fpsDialog.visible)
                fpsDialog.close()
        }
    }

    Behavior on opacity { NumberAnimation {}}

    //
    // Camera image
    //
    Image {
        id: image
        cache: false
        asynchronous: true
        anchors.fill: parent
        fillMode: Image.PreserveAspectCrop
    }

    //
    // Title labels
    //
    ColumnLayout {
        spacing: app.spacing / 2

        anchors {
            top: parent.top
            left: parent.left
            margins: app.spacing
        }

        Label {
            color: "#fff"
            text: cameraName
        }

        Label {
            color: "#ccc"
            text: cameraStatus
        }
    }

    //
    // FPS setter dialog
    //
    Popup {
        id: fpsDialog

        dim: true
        modal: true

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        Material.theme: Material.Light

        ColumnLayout {
            spacing: app.spacing
            anchors.centerIn: parent
            anchors.margins: app.spacing

            Label {
                text: qsTr ("Set target FPS") + ":"
            }

            SpinBox {
                id: fpsSpinbox
                Layout.fillWidth: true
                Layout.minimumWidth: 180
                to: QCCTVStation.maximumFPS()
                from: QCCTVStation.minimumFPS()
                onValueChanged: QCCTVStation.changeFPS (camNumber, value)
            }

            Label {
                text: qsTr ("Video Resolution") + ":"
            }

            ComboBox {
                id: resolutions
                Layout.minimumWidth: 220
                model: QCCTVStation.availableResolutions()

                property bool firstIndexChange: true

                onCurrentIndexChanged: {
                    if (!firstIndexChange)
                        QCCTVStation.changeResolution (camNumber, currentIndex)
                    else
                        firstIndexChange = false
                }
            }

            Button {
                text: qsTr ("Close")
                Layout.fillWidth: true
                onClicked: fpsDialog.close()
            }
        }
    }

    //
    // Camera Controls
    //
    RowLayout {
        spacing: app.spacing

        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            margins: app.spacing
        }

        //
        // Tooltip
        //
        ToolTip {
            id: tooltip
            timeout: 2000
        }

        //
        // Spacer
        //
        Item {
            Layout.fillWidth: true
        }

        //
        // Back Button
        //
        Button {
            id: backButton
            visible: false
            enabled: visible

            contentItem: Image {
                fillMode: Image.Pad
                sourceSize: Qt.size (36, 36)
                source: "qrc:/images/back.svg"
                verticalAlignment: Image.AlignVCenter
                horizontalAlignment: Image.AlignHCenter
            }

            onClicked: hideCamera()
        }

        //
        // FPS button
        //
        Button {
            contentItem: Image {
                fillMode: Image.Pad
                sourceSize: Qt.size (36, 36)
                source: "qrc:/images/settings.svg"
                verticalAlignment: Image.AlignVCenter
                horizontalAlignment: Image.AlignHCenter
            }

            onClicked: fpsDialog.open()
        }

        //
        // Light button
        //
        Button {
            id: lightButton

            contentItem: Image {
                fillMode: Image.Pad
                sourceSize: Qt.size (36, 36)
                verticalAlignment: Image.AlignVCenter
                horizontalAlignment: Image.AlignHCenter
                source: flashOn ? "qrc:/images/flash-on.svg" :
                                  "qrc:/images/flash-off.svg"
            }

            onClicked: {
                flashOn = !flashOn

                if (flashOn && QCCTVStation.flashlightAvailable (camNumber))
                    QCCTVStation.setFlashlightEnabled (camNumber, flashOn)

                else if (flashOn) {
                    flashOn = false
                    tooltip.text = qsTr ("Flashlight Error")
                    tooltip.visible = true
                }
            }
        }

        //
        // Sound button
        //
        Button {
            id: soundButton
            property bool soundOn: true

            contentItem: Image {
                fillMode: Image.Pad
                sourceSize: Qt.size (36, 36)
                verticalAlignment: Image.AlignVCenter
                horizontalAlignment: Image.AlignHCenter
                source: parent.soundOn ? "qrc:/images/volume-on.svg" :
                                         "qrc:/images/volume-off.svg"
            }

            onClicked: {
                soundOn = !soundOn
            }
        }

        //
        // Spacer
        //
        Item {
            Layout.fillWidth: true
        }
    }
}
