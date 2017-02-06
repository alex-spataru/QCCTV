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

Page {
    //
    // Settings
    //
    Settings {
        property alias fullscreen: fullscreen.checked
        property alias saveRecordings: saveIncomingMedia.checked
        property alias recordingsPath: textField.placeholderText
    }

    //
    // QCCTV signals/slots
    //
    Connections {
        target: QCCTVStation
        onRecordingsPathChanged: {
            if (textField.placeholderText !== QCCTVStation.recordingsPath())
                textField.placeholderText = QCCTVStation.recordingsPath()
        }
    }

    //
    // Background item
    //
    background: Rectangle {
        color: app.backgroundColor

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
                    source: app.getIcon ("download.svg")
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
                                source: app.getIcon ("open.svg")
                                verticalAlignment: Image.AlignVCenter
                                horizontalAlignment: Image.AlignHCenter
                            }

                            onClicked: QCCTVStation.chooseRecordingsPath()
                        }
                    }
                }
            }

            //
            // Fullscreen checkbox
            //
            RowLayout {
                spacing: app.spacing * 2
                Layout.fillWidth: true

                Image {
                    fillMode: Image.Pad
                    sourceSize: Qt.size (72, 72)
                    source: app.getIcon ("fullscreen.svg")
                    verticalAlignment: Image.AlignVCenter
                    horizontalAlignment: Image.AlignHCenter
                }

                CheckBox {
                    function update() {
                        if (checked) {
                            app.showNormal()
                            app.showFullScreen()
                        }
                        else
                            app.showNormal()
                    }

                    id: fullscreen
                    checked: false
                    Layout.fillWidth: true
                    onCheckedChanged: update()
                    text: qsTr ("Show Fullscreen")
                    Component.onCompleted: update()
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
        contentWidth: column.width * 1.2
        contentHeight: column.height * 1.2

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        Material.theme: Material.Light

        ColumnLayout {
            id: column
            spacing: app.spacing
            anchors.centerIn: parent

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

            RowLayout {
                spacing: app.spacing
                Layout.fillWidth: true
                Layout.fillHeight: true

                Button {
                    Layout.fillWidth: true
                    text: qsTr ("Open Website")
                }

                Button {
                    Layout.fillWidth: true
                    text: qsTr ("Report Bug")
                    onClicked: Qt.openUrlExternally ("https://github.com/alex-spataru/qcctv/issues")
                }
            }
        }
    }
}
