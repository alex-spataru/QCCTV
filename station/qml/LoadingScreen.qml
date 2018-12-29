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
    id: screen

    Rectangle {
        anchors.fill: parent
        color: app.backgroundColor
    }

    Image {
        anchors.centerIn: parent
        source: app.getIcon ("qcctv.png")
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Column {
        spacing: app.spacing

        anchors {
            left: parent.left
            bottom: parent.bottom
            margins: 4 * app.spacing
        }

        Label {
            property string base: qsTr ("Looking for cameras")

            text: base
            id: looking
            font.bold: true
            font.pixelSize: 24
            anchors.left: parent.left
            anchors.right: parent.right

            Timer {
                repeat: true
                interval: 500
                Component.onCompleted: start()
                onTriggered: {
                    switch (looking.text) {
                    case looking.base:
                        looking.text = looking.base + "."
                        break
                    case looking.base + ".":
                        looking.text = looking.base + ".."
                        break
                    case looking.base + "..":
                        looking.text = looking.base + "..."
                        break
                    default:
                        looking.text = looking.base
                    }
                }
            }
        }

        Label {
            color: app.disabledForegroundColor
            text: qsTr ("Try launching QCCTV Camera in some of your LAN devices")
        }
    }
}
