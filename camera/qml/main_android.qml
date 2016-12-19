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
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtQuick.Controls.Universal 2.0

import "."

ApplicationWindow {
    id: app
    property int spacing: 8

    //
    // Styling options
    //
    color: "#000"
    visible: true

    //
    // Show the window on launch
    //
    Component.onCompleted: {
        showMaximized()

        Material.accent = "#8fc859"
        Universal.accent = "#8fc859"
        Material.theme = Material.Dark
        Universal.theme = Universal.Dark
    }

    //
    // Tab selector
    //
    header: TabBar {
        id: tabBar

        TabButton {
            text: qsTr ("Camera")
            onClicked: stack.currentIndex = 0
        }

        TabButton {
            text: qsTr ("Stations")
            onClicked: stack.currentIndex = 1
        }
    }

    //
    // Pages
    //
    SwipeView {
        id: stack
        currentIndex: 0
        anchors.fill: parent
        onCurrentIndexChanged: tabBar.currentIndex = currentIndex

        Controls {
            id: controls
            background: VideoOutput {
                anchors.fill: parent
                autoOrientation: true
                fillMode: VideoOutput.PreserveAspectCrop

                source: Camera {
                    objectName: "camera"
                }
            }
        }

        Stations {
            id: stations
        }
    }
}
