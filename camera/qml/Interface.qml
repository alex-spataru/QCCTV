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

Item {
    id: ui

    //
    // Global variables
    //
    property int spacing: 8
    property int fps: 0
    property string name: ""
    property string group: ""
    property int resolution: 0
    property bool autoRegulate: true

    //
    // Settings
    //
    Settings {
        property alias fps: ui.fps
        property alias name: ui.name
        property alias group: ui.group
        property alias resolution: ui.resolution
        property alias autoRegulate: ui.autoRegulate
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
        onFpsChanged: ui.fps = QCCTVCamera.fps
        onNameChanged: ui.name = QCCTVCamera.name
        onGroupChanged: ui.group = QCCTVCamera.group
        onResolutionChanged: ui.resolution = QCCTVCamera.resolution
        onAutoRegulateResolutionChanged: ui.autoRegulate = QCCTVCamera.autoRegulateResolution
    }

    //
    // Camera information labels
    //
    ColumnLayout {
        spacing: ui.spacing / 2

        anchors {
            top: parent.top
            left: parent.left
            margins: ui.spacing
        }

        Label {
            color: "#fff"
            text: QCCTVCamera.name
        }

        Label {
            color: "#ccc"
            text: QCCTVCamera.statusString
        }

        Label {
            color: "#ccc"
            text: QCCTVCamera.fps + " " + qsTr ("FPS")
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
        Universal.theme: Universal.Dark

        contentWidth: column.width
        contentHeight: column.height

        ColumnLayout {
            id: column
            spacing: ui.spacing

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
                Layout.minimumHeight: ui.spacing * 2
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
        spacing: ui.spacing

        anchors {
            left: parent.left
            right: parent.right
            margins: ui.spacing
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
                sourceSize: Qt.size (48, 48)
                verticalAlignment: Image.AlignVCenter
                horizontalAlignment: Image.AlignHCenter
                source: QCCTVCamera.flashlightEnabled ? "qrc:/images/flash-on.svg" :
                                                        "qrc:/images/flash-off.svg"
            }

            onClicked: {
                flashOn = !flashOn

                if (QCCTVCamera.flashlightAvailable)
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
