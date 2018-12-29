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
import Qt.labs.settings 1.0
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0

import "."

ApplicationWindow {
    id: app
    property int spacing: 8
    
    //
    // Geometry options
    //
    width: 720
    height: 480
    color: "#000"
    visible: true
    x: isMobile ? 0 : 100
    y: isMobile ? 0 : 100
    title: AppDspName + " " + AppVersion

    //
    // Settings
    //
    Settings {
        property alias x: app.x
        property alias y: app.y
        property alias width: app.width
        property alias height: app.height
    }

    //
    // Application style
    //
    Material.accent: "#8fc859"
    Material.theme: Material.Dark

    //
    // Show the window on launch
    //
    Component.onCompleted: {
        if (isMobile)
            showMaximized()
        else
            showNormal()
    }

    //
    // Update the video output image automatically
    //
    Connections {
        target: QCCTVCamera

        onImageChanged: {
            image.source = ""
            image.source = "image://qcctv/"
        }
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
            onSettingsButtonClicked: settings.open()

            background: Image {
                id: image
                cache: false
                anchors.fill: parent
                fillMode: Image.PreserveAspectCrop
            }
        }

        Stations {
            id: stations
        }
    }

    //
    // Settings dialog
    //
    SettingsDialog {
        id: settings
    }
}
