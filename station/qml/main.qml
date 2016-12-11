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

import "."

ApplicationWindow {
    id: app
    width: 840
    height: 520
    color: "#000"
    visible: true
    x: isMobile ? 0 : 100
    y: isMobile ? 0 : 100
    title: AppDspName + " " + AppVersion

    //
    // Global variables
    //
    property int spacing: 8

    //
    // Show window correctly on mobile devices
    //
    Component.onCompleted: {
        if (isMobile)
            showMaximized()

        Material.theme = Material.Dark
        Material.accent = Material.Teal
        Universal.theme = Universal.Dark
    }

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
    // QCCTV signals/slots
    //
    Connections {
        target: QCCTVStation
        onGroupCountChanged: {
            tabs.model = 0
            tabs.model = QCCTVStation.groupCount()
            loadingScreen.opacity = QCCTVStation.cameraCount() > 0 ? 0 : 1
        }
    }

    //
    // Standard camera grid and group selector
    //
    Page {
        anchors.fill: parent
        opacity: fullscreenCamera.enabled ? 0 : 1

        Behavior on opacity { NumberAnimation{} }

        //
        // Camera grid
        //
        GroupView {
            id: groupView
            anchors.fill: parent
            enabled: !fullscreenCamera.enabled
        }

        //
        // Group selector
        //
        footer: TabBar {
            opacity: 0.62

            Repeater {
                id: tabs
                delegate: TabButton {
                    onClicked: groupView.group = index
                    font.capitalization: Font.AllUppercase
                    text: QCCTVStation.getGroupName (index)
                }
            }
        }
    }

    //
    // Fullscreen camera (shown when user clicks on a camera)
    //
    FullscreenCamera {
        id: fullscreenCamera
        anchors.fill: parent
        Component.onCompleted: hideCamera()
    }

    //
    // QCCTV loading screen (shown when there are no cameras)
    //
    LoadingScreen {
        id: loadingScreen
        anchors.fill: parent
        opacity: QCCTVStation.cameraCount() > 0 ? 0 : 1

        Behavior on opacity { NumberAnimation{} }
    }
}
