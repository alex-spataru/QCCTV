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

Page {
    id: controls

    //
    // Instructs QCCTV to focus the camera and updates the UI
    //
    function focusCamera() {
        QCCTVCamera.focusCamera()
        row.showTooltip (qsTr ("Focusing camera") + "...")
    }

    //
    // Zoom camera with user gesture
    //
    PinchArea {
        property double oldZoom: 0

        function zoomDelta (zoom, percent) {
            return zoom + (Math.log (percent) / Math.log (2))
        }

        anchors.fill: parent
        pinch.minimumScale: 0
        pinch.maximumScale: 100
        onPinchStarted: oldZoom = QCCTVCamera.zoomLevel
        onPinchUpdated: QCCTVCamera.zoomLevel = zoomDelta (oldZoom, pinch.scale)
        onPinchFinished: QCCTVCamera.zoomLevel = zoomDelta (oldZoom, pinch.scale)
    }

    //
    // Emitted when the user clicks the settings button
    //
    signal settingsButtonClicked

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

                onClicked: settingsButtonClicked()
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
            // Focus button
            //
            Button {
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
