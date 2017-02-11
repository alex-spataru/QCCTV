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

    //
    // Window options
    //
    width: 840
    height: 520
    visible: true
    x: isMobile ? 0 : 100
    y: isMobile ? 0 : 100
    color: backgroundColor
    title: AppDspName + " " + AppVersion

    //
    // Global variables
    //
    property int spacing: 8

    //
    // Custom styling colors
    //
    property string backgroundColor: {
        if (Material.theme === Material.Light ||
            Universal.theme === Universal.Light)
            return "#efefef"
        else
            return "#0c0c0c"
    }
    property string disabledForegroundColor: {
        if (Material.theme === Material.Light ||
            Universal.theme === Universal.Light)
            return "#7c7c7c"
        else
            return "#cccccc"
    }

    //
    // Allows the application to display a fullscreen camera from anywhere
    //
    function showCamera (camera) {
        liveFeed.showCamera (camera)
        swipeView.currentIndex = 0
    }

    //
    // Returns the correct path for the given icon
    //
    function getIcon (name) {
        if (Material.theme === Material.Light ||
            Universal.theme === Universal.Light)
            return "qrc:/images/light/" + name
        else
            return "qrc:/images/dark/" + name
    }

    //
    // Show window and set application style on launch
    //
    Component.onCompleted: {
        if (isMobile)
            showMaximized()

        Material.theme = Material.Dark
        Universal.theme = Universal.Dark

        if (Material.theme === Material.Light)
            Material.accent = "#12752d"
        else
            Material.accent = "#8fc859"

        if (Universal.theme === Universal.Light)
            Universal.accent = "#12752d"
        else
            Universal.accent = "#8fc859"
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
    // Application TabBar
    //
    header: TabBar {
        id: tabBar

        Component.onCompleted: {
            if (Material.theme === Material.Light ||
                 Universal.theme === Universal.Light) {
                Material.foreground = "#8c8c8c"
                Material.background = "#dedede"
                Universal.foreground = "#8c8c8c"
                Universal.background = "#dedede"
            }
        }

        TabButton {
            text: qsTr ("Live Feeds")
            onClicked: swipeView.currentIndex = 0
        }

        TabButton {
            text: qsTr ("Search")
            onClicked: {
                swipeView.currentIndex = 1
                search.focusSearchBox()
            }
        }

        TabButton {
            text: qsTr ("Setup")
            onClicked: swipeView.currentIndex = 2
        }
    }

    //
    // Pages
    //
    SwipeView {
        id: swipeView
        currentIndex: 0
        anchors.fill: parent
        onCurrentIndexChanged: tabBar.currentIndex = currentIndex

        LiveFeed {
            clip: true
            id: liveFeed
        }

        SearchPage {
            clip: true
            id: search
        }

        SettingsPage {
            clip: true
            id: settings
        }
    }
}
