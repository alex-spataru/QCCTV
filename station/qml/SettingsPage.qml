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
import Qt.labs.settings 1.0
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtQuick.Controls.Universal 2.0

import Qt.labs.settings 1.0

Page {
    //
    // Settings
    //
    Settings {
        property alias quality: quality.value
        property alias saveRecordings: saveIncomingMedia.checked
        property alias recordingsPath: textField.placeholderText
    }

    //
    // QCCTV signals/slots
    //
    Connections {
        target: QCCTVStation
        onImageQualityChanged: quality.value = QCCTVStation.imageQuality()
        onRecordingsPathChanged: {
            if (textField.placeholderText !== QCCTVStation.recordingsPath())
                textField.placeholderText = QCCTVStation.recordingsPath()
        }
    }

    //
    // Background item
    //
    background: Rectangle {
        color: "#000"

        ColumnLayout {
            spacing: app.spacing
            anchors.fill: parent
            anchors.margins: 2 * app.spacing
            anchors.rightMargin: app.spacing

            //
            // Save incoming media checkbox & path
            //
            RowLayout {
                spacing: app.spacing * 2
                Layout.fillWidth: true

                Image {
                    fillMode: Image.Pad
                    sourceSize: Qt.size (72, 72)
                    source: "qrc:/images/download.svg"
                    verticalAlignment: Image.AlignVCenter
                    horizontalAlignment: Image.AlignHCenter
                }

                ColumnLayout {
                    spacing: app.spacing
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    CheckBox {
                        id: saveIncomingMedia
                        Layout.fillWidth: true
                        text: qsTr ("Save incoming media")
                        checked: QCCTVStation.saveIncomingMedia()
                        onCheckedChanged: QCCTVStation.setSaveIncomingMedia (checked)
                    }

                    RowLayout {
                        spacing: app.spacing
                        Layout.fillWidth: true

                        TextField {
                            id: textField
                            enabled: false
                            Layout.fillWidth: true
                            opacity: saveIncomingMedia.checked ? 1 : 0.5
                            placeholderText: QCCTVStation.recordingsPath()
                            onPlaceholderTextChanged: QCCTVStation.setRecordingsPath (placeholderText)
                        }

                        Button {
                            enabled: saveIncomingMedia.checked
                            opacity: saveIncomingMedia.checked ? 1 : 0.5

                            contentItem: Image {
                                fillMode: Image.Pad
                                sourceSize: Qt.size (24, 24)
                                source: "qrc:/images/open.svg"
                                verticalAlignment: Image.AlignVCenter
                                horizontalAlignment: Image.AlignHCenter
                            }

                            onClicked: QCCTVStation.chooseRecordingsPath()
                        }
                    }
                }
            }

            //
            // Image quality selector
            //
            RowLayout {
                spacing: app.spacing * 2
                Layout.fillWidth: true

                Image {
                    fillMode: Image.Pad
                    sourceSize: Qt.size (72, 72)
                    source: "qrc:/images/gradient.svg"
                    verticalAlignment: Image.AlignVCenter
                    horizontalAlignment: Image.AlignHCenter
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    CheckBox {
                        id: qltyCheckbox
                        Layout.fillWidth: true
                        checked: quality.value > 0
                        text: qsTr ("Limit image quality to") + ":"

                        onCheckStateChanged: {
                            if (!checked)
                                quality.value = 0
                        }
                    }

                    RowLayout {
                        spacing: app.spacing
                        Layout.fillWidth: true

                        Slider {
                            id: quality

                            to: 100
                            from: 0
                            value: 0
                            stepSize: 1
                            Layout.fillWidth: true
                            opacity: enabled ? 1 : 0.5
                            snapMode: Slider.SnapAlways
                            enabled: qltyCheckbox.checked
                            onValueChanged: QCCTVStation.setImageQuality (value)
                        }

                        Label {
                            id: indicator
                            text: {
                                if (!qltyCheckbox.checked)
                                    return qsTr ("Auto")
                                else
                                    Math.round (quality.visualPosition * 100) + " %"
                            }
                        }
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            Button {
                Layout.fillWidth: true
                Layout.maximumWidth: 440
                text: qsTr ("Open Recordings")
                onClicked: QCCTVStation.openRecordingsPath()
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Button {
                text: qsTr ("About")
                Layout.fillWidth: true
                Layout.maximumWidth: 440
                onClicked: aboutDialog.open()
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }

    //
    // About popup
    //
    Popup {
        id: aboutDialog

        dim: true
        modal: true
        contentWidth: column.width
        contentHeight: column.height

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        Material.theme: Material.Light
        Universal.theme: Universal.Light

        ColumnLayout {
            id: column
            spacing: app.spacing

            Image {
                smooth: true
                sourceSize: Qt.size (96, 96)
                source: "qrc:/images/qcctv-station.png"
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Label {
                font.bold: true
                text: AppDspName
                font.pixelSize: 24
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Label {
                font.pixelSize: 20
                text: qsTr ("Version %1").arg (AppVersion)
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            Button {
                Layout.fillWidth: true
                text: qsTr ("Open Website")
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Button {
                Layout.fillWidth: true
                text: qsTr ("Report Bug")
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: Qt.openUrlExternally ("https://github.com/alex-spataru/qcctv/issues")
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: 260
            }
        }
    }
}
