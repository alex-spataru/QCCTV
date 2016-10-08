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

Rectangle {
    id: button

    signal clicked
    property bool toggled: false
    property alias source: image.source

    width: 48
    height: 48
    border.width: 1
    radius: width / 2

    opacity: 0.85
    border.color: "#999"
    color: mouse.containsMouse ? "#666" : "#444"

    Behavior on color { ColorAnimation{} }

    Image {
        id: image
        anchors.centerIn: parent
        sourceSize: Qt.size (button.width * 0.5,
                             button.height * 0.5)
    }


    MouseArea {
        id: mouse
        hoverEnabled: true
        anchors.fill: parent
        onClicked: button.clicked()
    }
}
