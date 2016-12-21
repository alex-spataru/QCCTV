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

Page {
    //
    // Re-draws the camera grid if required
    //
    function generateGrid() {
        tabs.model = 0
        groupView.group = 0
        tabs.model = QCCTVStation.groupCount()
        loadingScreen.opacity = QCCTVStation.cameraCount() > 0 ? 0 : 1

        if (QCCTVStation.cameraCount() > 0) {
            groupView.redraw()
            fullscreenCamera.hideCamera()
        }
    }

    //
    // Allows the application to display a fullscreen camera from anywhere
    //
    function showCamera (camera) {
        fullscreenCamera.showCamera (camera)
    }

    //
    // QCCTV signals/slots
    //
    Connections {
        target: QCCTVStation
        onGroupCountChanged: generateGrid()
        onCameraCountChanged: generateGrid()
    }

    //
    // Set background item
    //
    background: Rectangle {
        color: app.backgroundColor
    }

    //
    // Camera grid & group selector
    //
    Page {
        anchors.fill: parent
        opacity: fullscreenCamera.enabled ? 0 : 1

        Behavior on opacity { NumberAnimation{} }

        ColumnLayout {
            spacing: 0
            anchors.fill: parent

            GroupView {
                id: groupView
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            Rectangle {
                height: 1
                color: Material.accent
                Layout.fillWidth: true
                visible: tabBar.visible
            }

            TabBar {
                id: tabBar
                visible: opacity > 0
                Layout.fillWidth: true
                opacity: tabs.model > 1 ? 1 : 0

                Behavior on opacity { NumberAnimation{} }

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
    }

    //
    // Fullscreen camera
    //
    FullscreenCamera {
        id: fullscreenCamera
        anchors.fill: parent
        Component.onCompleted: hideCamera()
    }

    //
    // Loading screen (shown when no cameras are connected)
    //
    LoadingScreen {
        id: loadingScreen
        anchors.fill: parent
        opacity: QCCTVStation.cameraCount() > 0 ? 0 : 1

        Behavior on opacity { NumberAnimation{} }
    }
}
