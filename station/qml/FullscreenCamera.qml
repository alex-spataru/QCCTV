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
    property bool autoRegulate: true
    property bool zoomSupport: false
    property bool controlsEnabled: true
    property size buttonSize: Qt.size (36, 36)

    //
    // Update FPS automatically
    //
    onFpsChanged: {
        fpsSpinbox.value = fps
        fpsText.text = fps + " FPS"
    }

    //
    // Update resolution indicator automatically
    //
    onResolutionChanged: resolutions.currentIndex = resolution

    //
    // Update auto-regulate checkbox automatically
    //
    onAutoRegulateChanged: {
        autoRegulateCheck.checked = autoRegulate
        QCCTVStation.setAutoRegulateResolution (camNumber, autoRegulate)
    }

    //
    // Obtains latest camera data from QCCTV
    //
    function reloadData() {
        fps = QCCTVStation.fps (camNumber)
        resolution = QCCTVStation.resolution (camNumber)
        cameraName = QCCTVStation.cameraName (camNumber)
        zoomSupport = QCCTVStation.supportsZoom (camNumber)
        flashOn = QCCTVStation.flashlightEnabled (camNumber)
        cameraStatus = QCCTVStation.statusString (camNumber)
        autoRegulate = QCCTVStation.autoRegulateResolution (camNumber)

        fpsText.text = fps + " FPS"
    }

    //
    // Hides the camera widget
    //
    function hideCamera() {
        opacity = 0
        enabled = 0
    }

    //
    // Shows the camera widge
    //
    function showCamera (camera) {
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
        target: QCCTVStation

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

        onZoomSupportChanged: {
            if (camera === camNumber && enabled)
                zoomSupport = QCCTVStation.supportsZoom (camNumber)
        }

        onZoomLevelChanged: {
            if (camera === camNumber && enabled)
                zoomControl.value = QCCTVStation.zoom (camNumber)
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

        onAutoRegulateResolutionChanged: {
            if (camera === camNumber)
                autoRegulate = QCCTVStation.autoRegulateResolution (camNumber)
        }

        onCameraCountChanged: fpsDialog.close()
    }

    Behavior on opacity { NumberAnimation {}}

    //
    // Camera image
    //
    CameraVideo {
        cameraId: camNumber
        anchors.fill: parent
        enabled: cam.enabled
        fillMode: fillButton.checked ? Image.PreserveAspectFit :
                                       Image.PreserveAspectCrop
    }

    //
    // Toggler mouse area
    //
    MouseArea {
        anchors.fill: parent
        onClicked: controlsEnabled = !controlsEnabled
    }

    //
    // Zoom control
    //
    Slider {
        id: zoomControl

        anchors {
            right: parent.right
            margins: app.spacing
            verticalCenter: parent.verticalCenter
        }

        to: 100
        from: 0
        orientation: Qt.Vertical
        height: app.height * 0.6
        opacity: enabled ? 1 : 0
        enabled: zoomSupport && zoomButton.checked && controlsEnabled

        onVisualPositionChanged: {
            if (orientation === Qt.Vertical)
                QCCTVStation.setZoom (camNumber, (1 - visualPosition) * 100)
            else
                QCCTVStation.setZoom (camNumber, 1 - visualPosition * 100)
        }

        Behavior on opacity { NumberAnimation {} }
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
            id: fpsText
            color: app.disabledForegroundColor
            background: Rectangle {
                opacity: 0.65
                color: app.backgroundColor
            }
        }
    }

    //
    // Settings dialog
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
                Layout.fillWidth: true
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

            Switch {
                id: autoRegulateCheck
                text: qsTr ("Auto-regulate video resolution")
                onCheckedChanged: autoRegulate = checked
            }

            Item {
                Layout.minimumHeight: app.spacing * 2
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
        opacity: controlsEnabled ? 1 : 0

        Behavior on opacity { NumberAnimation{} }

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

            contentItem: Image {
                fillMode: Image.Pad
                sourceSize: cam.buttonSize
                source: app.getIcon ("back.svg")
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
                sourceSize: cam.buttonSize
                source: app.getIcon ("settings.svg")
                verticalAlignment: Image.AlignVCenter
                horizontalAlignment: Image.AlignHCenter
            }

            onClicked: fpsDialog.open()
        }

        //
        // Light button
        //
        Button {
            contentItem: Image {
                fillMode: Image.Pad
                sourceSize: cam.buttonSize
                verticalAlignment: Image.AlignVCenter
                horizontalAlignment: Image.AlignHCenter
                source: flashOn ? app.getIcon ("flash-on.svg") :
                                  app.getIcon ("flash-off.svg")
            }

            onClicked: {
                flashOn = !flashOn

                if (QCCTVStation.flashlightAvailable (camNumber))
                    QCCTVStation.setFlashlightEnabled (camNumber, flashOn)

                else {
                    flashOn = false
                    tooltip.text = qsTr ("Flashlight Error")
                    tooltip.visible = true
                }
            }
        }

        //
        // Focus button
        //
        Button {
            contentItem: Image {
                fillMode: Image.Pad
                sourceSize: cam.buttonSize
                source: app.getIcon ("focus.svg")
                verticalAlignment: Image.AlignVCenter
                horizontalAlignment: Image.AlignHCenter
            }

            onClicked: {
                QCCTVStation.focusCamera (camNumber)
                tooltip.text = qsTr ("Focusing Camera") + "..."
                tooltip.visible = true
            }
        }

        //
        // Fill/fit button
        //
        Button {
            id: fillButton
            checkable: true
            contentItem: Image {
                fillMode: Image.Pad
                sourceSize: cam.buttonSize
                verticalAlignment: Image.AlignVCenter
                horizontalAlignment: Image.AlignHCenter
                source: fillButton.checked ? app.getIcon ("fill.svg") :
                                             app.getIcon ("fit.svg")
            }
        }

        //
        // Zoom button
        //
        Button {
            id: zoomButton
            checkable: true
            enabled: zoomSupport
            visible: zoomSupport

            contentItem: Image {
                fillMode: Image.Pad
                sourceSize: cam.buttonSize
                source: app.getIcon ("zoom.svg")
                verticalAlignment: Image.AlignVCenter
                horizontalAlignment: Image.AlignHCenter
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
