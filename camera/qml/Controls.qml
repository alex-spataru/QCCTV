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
import Qt.labs.settings 1.0

import QtQuick.Controls.Material 2.0
import QtQuick.Controls.Universal 2.0

Page {
    id: controls

    //
    // Global variables
    //
    property int fps: 0
    property string name: ""
    property string group: ""
    property int resolution: 0
    property bool autoRegulate: true

    //
    // Settings
    //
    Settings {
        property alias fps: controls.fps
        property alias name: controls.name
        property alias group: controls.group
        property alias resolution: controls.resolution
        property alias autoRegulate: controls.autoRegulate
    }

    //
    // Instructs QCCTV to focus the camera and updates the UI
    //
    function focusCamera() {
        QCCTVCamera.focusCamera()
        row.showTooltip (qsTr ("Focusing camera") + "...")
    }

    //
    // Change QCCTV variables automatically
    //
    onFpsChanged: QCCTVCamera.fps = fps
    onNameChanged: QCCTVCamera.name = name
    onGroupChanged: QCCTVCamera.group = group
    onResolutionChanged: QCCTVCamera.resolution = resolution
    onAutoRegulateChanged: QCCTVCamera.autoRegulateResolution = autoRegulate

    //
    // Update variables automatically
    //
    Connections {
        target: QCCTVCamera
        onFpsChanged: controls.fps = QCCTVCamera.fps
        onNameChanged: controls.name = QCCTVCamera.name
        onGroupChanged: controls.group = QCCTVCamera.group
        onResolutionChanged: controls.resolution = QCCTVCamera.resolution
        onZoomLevelChanged: zoomSlider.value = QCCTVCamera.zoomLevel
        onAutoRegulateResolutionChanged: controls.autoRegulate = QCCTVCamera.autoRegulateResolution
    }

    //
    // Zoom camera when user double taps the image
    //
    MouseArea {
        anchors.fill: parent
        onDoubleClicked: controls.focusCamera()
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
            color: "#fff"
            text: QCCTVCamera.name
            background: Rectangle {
                color: "#000"
                opacity: 0.65
            }
        }

        Label {
            color: "#ccc"
            text: QCCTVCamera.statusString
            background: Rectangle {
                color: "#000"
                opacity: 0.65
            }
        }

        Label {
            color: "#ccc"
            text: QCCTVCamera.fps + " " + qsTr ("FPS")
            background: Rectangle {
                color: "#000"
                opacity: 0.65
            }
        }
    }

    //
    // Zoom control
    //
    Slider {
        id: zoomSlider

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
        enabled: zoomBt.enabled && zoomBt.checked

        onVisualPositionChanged: {
            if (orientation === Qt.Vertical)
                QCCTVCamera.zoomLevel = (1 - visualPosition) * 100
            else
                QCCTVCamera.zoomLevel = visualPosition * 100
        }

        Behavior on opacity { NumberAnimation {} }
    }

    //
    // Settings dialog
    //
    Popup {
        id: settings

        modal: true
        x: (app.width - width) / 2

        Material.theme: Material.Light
        Universal.theme: Universal.Dark

        contentWidth: column.width
        contentHeight: column.height

        ColumnLayout {
            id: column
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
                text: QCCTVCamera.name
                Layout.fillWidth: true
                Layout.minimumWidth: 280
                onTextChanged: {
                    if (text.length > 0)
                        QCCTVCamera.name = text
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
                text: QCCTVCamera.group
                Layout.minimumWidth: 280
                onTextChanged: {
                    if (text.length > 0)
                        QCCTVCamera.group = text
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
                value: QCCTVCamera.fps
                Layout.fillWidth: true
                to: QCCTVCamera.maximumFps
                from: QCCTVCamera.minimumFps
                onValueChanged: QCCTVCamera.fps = value
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
                model: QCCTVCamera.resolutions
                currentIndex: QCCTVCamera.resolution
                onCurrentIndexChanged: QCCTVCamera.resolution = currentIndex
            }

            //
            // Auto-regulate switch
            //
            Switch {
                id: autoRegulateCheck
                checked: QCCTVCamera.autoRegulateResolution
                text: qsTr ("Auto-regulate video resolution")
                onCheckedChanged: QCCTVCamera.autoRegulateResolution = checked
            }

            //
            // Spacer
            //
            Item {
                Layout.fillWidth: true
                Layout.minimumWidth: app.spacing
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
    ColumnLayout {
        spacing: app.spacing

        anchors {
            left: parent.left
            right: parent.right
            margins: app.spacing
            bottom: parent.bottom
        }

        //
        // Camera buttons
        //
        RowLayout {
            id: row
            spacing: app.spacing
            Layout.fillWidth: true

            property size buttonSize: Qt.size (36, 36)

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
                    sourceSize: row.buttonSize
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
                    sourceSize: row.buttonSize
                    source: "qrc:/images/camera.svg"
                    verticalAlignment: Image.AlignVCenter
                    horizontalAlignment: Image.AlignHCenter
                }

                onClicked: {
                    if (QCCTVCamera.readyForCapture) {
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
                    sourceSize: row.buttonSize
                    verticalAlignment: Image.AlignVCenter
                    horizontalAlignment: Image.AlignHCenter
                    source: QCCTVCamera.flashlightEnabled ? "qrc:/images/flash-on.svg" :
                                                            "qrc:/images/flash-off.svg"
                }

                onClicked: {
                    QCCTVCamera.flashlightEnabled = !QCCTVCamera.flashlightEnabled

                    if (QCCTVCamera.flashlightAvailable)
                        row.showTooltip (QCCTVCamera.flashlightEnabled ? qsTr ("Flashlight On") :
                                                                         qsTr ("Flashlight Off"))

                    else {
                        QCCTVCamera.flashlightEnabled = false
                        row.showTooltip (qsTr ("Flashlight Error"))
                    }
                }
            }

            //
            // Zoom Button
            //
            Button {
                id: zoomBt
                checkable: true
                visible: enabled
                enabled: QCCTVCamera.supportsZoom

                contentItem: Image {
                    fillMode: Image.Pad
                    sourceSize: row.buttonSize
                    source: "qrc:/images/zoom.svg"
                    verticalAlignment: Image.AlignVCenter
                    horizontalAlignment: Image.AlignHCenter
                }
            }

            //
            // Focus button (only visible when zoom button is not)
            //
            Button {
                visible: enabled
                enabled: !zoomBt.enabled
                onClicked: controls.focusCamera()

                contentItem: Image {
                    fillMode: Image.Pad
                    sourceSize: row.buttonSize
                    source: "qrc:/images/focus.svg"
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
}
