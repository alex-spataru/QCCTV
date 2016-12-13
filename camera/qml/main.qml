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
import QtMultimedia 5.2
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.0
import Qt.labs.settings 1.0

import QtQuick.Controls.Material 2.0
import QtQuick.Controls.Universal 2.0

ApplicationWindow {
    id: app
    
    width: 720
    height: 480
    color: "#000"
    visible: true
    x: isMobile ? 0 : 100
    y: isMobile ? 0 : 100
    title: AppDspName + " " + AppVersion

    //
    // Global variables
    //
    property var fps: 0
    property var spacing: 8
    property var resolution: 0
    property bool flashOn: false
    property string cameraName: ""
    property string cameraGroup: ""
    property string cameraStatus: ""
    property bool autoRegulate: true

    //
    // Settings
    //
    Settings {
        property alias x: app.x
        property alias y: app.y
        property alias fps: app.fps
        property alias width: app.width
        property alias height: app.height
        property alias cameraName: app.cameraName
        property alias resolution: app.resolution
        property alias cameraGroup: app.cameraGroup
        property alias autoRegulate: app.autoRegulate
    }

    //
    // Update the UI when the camera name changes
    //
    onCameraNameChanged: {
        nameLabel.text = cameraName
        nameInput.text = cameraName
    }

    //
    // Update the UI when the camera group changes
    //
    onCameraGroupChanged: groupInput.text = cameraGroup

    //
    // Update the UI when the FPS changes
    //
    onFpsChanged: {
        fpsSpin.value = fps
        fpsLabel.text = fps + " " + qsTr ("FPS")
    }

    //
    // Update the UI when the auto regulate variable changes
    //
    onAutoRegulateChanged: autoRegulateCheck.checked = autoRegulate

    //
    // Update the UI when the status changes
    //
    onCameraStatusChanged: statusLabel.text = cameraStatus

    //
    // Change the flashlight status when the user clicks on the flashlight
    //
    onFlashOnChanged: QCCTVCamera.setFlashlightEnabled (flashOn)

    //
    // Update the UI when the resolution chages
    //
    onResolutionChanged: resolutions.currentIndex = resolution

    //
    // Load the initial camera information during launch
    //
    Component.onCompleted: {
        if (isMobile)
            showMaximized()

        Material.theme = Material.Dark
        Material.accent = Material.Teal

        fps = QCCTVCamera.fps()
        cameraName = QCCTVCamera.cameraName()
        resolution = QCCTVCamera.resolution()
        cameraGroup = QCCTVCamera.cameraGroup()
        cameraStatus = QCCTVCamera.statusString()
    }

    //
    // QCCTV-to-UI signal processing
    //
    Connections {
        target: QCCTVCamera

        onImageChanged: {
            image.source = ""
            image.source = "image://qcctv/"
        }

        onFpsChanged: fps = QCCTVCamera.fps()
        onResolutionChanged: resolution = QCCTVCamera.resolution()
        onCameraNameChanged: cameraName = QCCTVCamera.cameraName()
        onLightStatusChanged: flashOn = QCCTVCamera.flashlightStatus()
        onCameraStatusChanged: cameraStatus = QCCTVCamera.statusString()
        onAutoRegulateResolutionChanged: autoRegulate = QCCTVCamera.autoRegulateResolution()
    }

    //
    // Video output image
    //
    Image {
        id: image
        cache: false
        anchors.fill: parent
        fillMode: Image.PreserveAspectCrop
    }

    //
    // Hidden video output
    //
    VideoOutput {
        width: 0
        height: 0

        source: Camera {
            objectName: "camera"
            captureMode: Camera.CaptureStillImage
        }
    }

    //
    // Camera information labels
    //
    ColumnLayout {
        spacing: app.spacing / 2

        anchors {
            top: parent.top
            left: parent.left
            margins: app.spacing
        }

        Label {
            id: nameLabel
            color: "#fff"
        }

        Label {
            id: statusLabel
            color: "#ccc"
        }

        Label {
            id: fpsLabel
            color: "#ccc"
        }
    }

    //
    // Settings dialog
    //
    Popup {
        id: settings

        modal: true
        x: (app.width - width) / 2
        y: (app.height - height) / 2

        Material.theme: Material.Light
        Universal.theme: Universal.Light

        ColumnLayout {
            spacing: app.spacing

            //
            // Camera name label
            //
            Label {
                text: qsTr ("Camera Name") + ":"
            }

            //
            // Camera name text input
            //
            TextField {
                id: nameInput
                Layout.fillWidth: true
                Layout.minimumWidth: 280
                onTextChanged: {
                    if (text.length > 0)
                        QCCTVCamera.setName (text)
                }
            }

            //
            // Camera group label
            //
            Label {
                text: qsTr ("Camera Group") + ":"
            }

            //
            // Camera group text input
            //
            TextField {
                id: groupInput
                Layout.fillWidth: true
                Layout.minimumWidth: 280
                onTextChanged: {
                    if (text.length > 0)
                        QCCTVCamera.setGroup (text)
                }
            }

            //
            // FPS label
            //
            Label {
                text: qsTr ("Camera FPS") + ":"
            }

            //
            // FPS spinbox
            //
            SpinBox {
                id: fpsSpin
                Layout.fillWidth: true
                to: QCCTVCamera.maximumFPS()
                from: QCCTVCamera.minimumFPS()
                onValueChanged: QCCTVCamera.setFPS (value)
            }

            //
            // Resolution label
            //
            Label {
                text: qsTr ("Target Resolution") + ":"
            }

            //
            // Resolution selector
            //
            ComboBox {
                id: resolutions
                Layout.fillWidth: true
                model: QCCTVCamera.availableResolutions()
                onCurrentIndexChanged: QCCTVCamera.setResolution (currentIndex)
            }

            //
            // Auto-regulate switch
            //
            Switch {
                id: autoRegulateCheck
                checked: QCCTVCamera.autoRegulateResolution()
                text: qsTr ("Auto-regulate video resolution")
                onCheckedChanged: QCCTVCamera.setAutoRegulateResolution (checked)
            }

            //
            // Spacer
            //
            Item {
                Layout.minimumHeight: app.spacing * 2
            }

            //
            // Close button
            //
            Button {
                text: qsTr ("Close")
                Layout.fillWidth: true
                onClicked: settings.close()
            }
        }
    }

    //
    // Camera control buttons
    //
    RowLayout {
        id: row
        spacing: app.spacing

        anchors {
            left: parent.left
            right: parent.right
            margins: app.spacing
            bottom: parent.bottom
        }

        function showTooltip (text) {
            tooltip.text = text
            tooltip.visible = true
        }

        //
        // Status tooltip
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
        // Settings Button
        //
        Button {
            contentItem: Image {
                fillMode: Image.Pad
                sourceSize: Qt.size (48, 48)
                source: "qrc:/images/settings.svg"
                verticalAlignment: Image.AlignVCenter
                horizontalAlignment: Image.AlignHCenter
            }

            onClicked: settings.open()
        }

        //
        // Camera Capture Button
        //
        Button {
            contentItem: Image {
                fillMode: Image.Pad
                sourceSize: Qt.size (48, 48)
                source: "qrc:/images/camera.svg"
                verticalAlignment: Image.AlignVCenter
                horizontalAlignment: Image.AlignHCenter
            }

            onClicked: {
                if (QCCTVCamera.readyForCapture()) {
                    QCCTVCamera.takePhoto()
                    row.showTooltip (qsTr ("Image Captured"))
                }

                else
                    row.showTooltip (qsTr ("Camera not ready!"))
            }
        }

        //
        // Flashlight Button
        //
        Button {
            contentItem: Image {
                fillMode: Image.Pad
                sourceSize: Qt.size (48, 48)
                verticalAlignment: Image.AlignVCenter
                horizontalAlignment: Image.AlignHCenter
                source: flashOn ? "qrc:/images/flash-on.svg" :
                                  "qrc:/images/flash-off.svg"
            }

            onClicked: {
                flashOn = !flashOn

                if (QCCTVCamera.flashlightAvailable())
                    row.showTooltip (flashOn ? qsTr ("Flashlight On") :
                                               qsTr ("Flashlight Off"))

                else
                    row.showTooltip (qsTr ("Flashlight Error"))
            }
        }

        //
        // Focus Button
        //
        Button {

            contentItem: Image {
                fillMode: Image.Pad
                sourceSize: Qt.size (48, 48)
                source: "qrc:/images/focus.svg"
                verticalAlignment: Image.AlignVCenter
                horizontalAlignment: Image.AlignHCenter
            }

            onClicked: {
                QCCTVCamera.focusCamera()
                row.showTooltip (qsTr ("Focusing Camera") + "...")
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
