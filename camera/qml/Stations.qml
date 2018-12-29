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

Rectangle {
    color: "#000"

    ListView {
        id: list
        spacing: app.spacing
        model: QCCTVCamera.connectedHosts

        anchors {
            fill: parent
            margins: app.spacing
        }

        ScrollBar.vertical: ScrollBar { }

        delegate: RowLayout {
            spacing: app.spacing

            Image {
                fillMode: Image.Pad
                sourceSize: Qt.size (48, 48)
                source: "qrc:/images/computer.svg"
                verticalAlignment: Image.AlignVCenter
                horizontalAlignment: Image.AlignHCenter
            }

            ColumnLayout {
                spacing: app.spacing
                Layout.fillWidth: true

                Label {
                    color: "#fff"
                    font.bold: true
                    font.pixelSize: 14
                    text: QCCTVCamera.hostNames [index]
                }

                Label {
                    color: "#ccc"
                    font.pixelSize: 12
                    text: QCCTVCamera.connectedHosts [index]
                }
            }
        }
    }

    ColumnLayout {
        id: noConnectedHosts
        spacing: app.spacing
        opacity: list.model < 1 ? 1 : 0
        Behavior on opacity { NumberAnimation{} }

        anchors {
            fill: parent
            margins: app.spacing
        }

        Item {
            Layout.fillHeight: true
        }

        Image {
            fillMode: Image.Pad
            sourceSize: Qt.size (128, 128)
            source: "qrc:/images/computer.svg"
            verticalAlignment: Image.AlignVCenter
            horizontalAlignment: Image.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Label {
            color: "#fff"
            font.bold: true
            font.pixelSize: 18
            text: qsTr ("No QCCTV Stations connected")
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Label {
            color: "#ccc"
            font.pixelSize: 12
            text: qsTr ("Check your network configuration")
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
