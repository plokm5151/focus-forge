/**
 * @file Main.qml
 * @brief Root QML view for the Brain Maintenance Dashboard.
 *
 * Integrates the BreathingOrb, countdown display, control buttons,
 * TodoListPanel, SessionReviewPopup, and all 24-point UX improvements.
 */
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "components"

ApplicationWindow {
    id: root

    width: 520
    height: 780
    visible: true
    title: qsTr("Brain Maintenance Dashboard")
    color: "#0a0a1a"

    // Always on top state
    property bool isAlwaysOnTop: false
    onIsAlwaysOnTopChanged: {
        if (isAlwaysOnTop) {
            root.flags = root.flags | Qt.WindowStaysOnTopHint
        } else {
            root.flags = root.flags & ~Qt.WindowStaysOnTopHint
        }
    }

    // Break health tips
    readonly property var breakTips: [
        "Stand up and walk around, drink some water",
        "Look at something 20 feet away",
        "Take three deep breaths, relax shoulders",
        "Stretch your wrists and fingers",
        "Close your eyes and rest for a moment"
    ]
    property int currentTipIndex: 0

    // ── Background Gradient ──
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#0a0a1a" }
            GradientStop { position: 0.5; color: "#0d0d2b" }
            GradientStop { position: 1.0; color: "#0a0a1a" }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 32
        spacing: 0

        // ── Top Bar (Pin, Title, Volume, Todo) ──
        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 16
            
            // Pin (Always on top) Toggle
            Button {
                id: pinBtn
                icon.source: "qrc:/qt/qml/FocusForgeApp/assets/icons/pin.svg"
                icon.color: root.isAlwaysOnTop ? "#00e0ff" : "#8888aa"
                icon.width: 20
                icon.height: 20
                background: Rectangle { color: "transparent" }
                opacity: root.isAlwaysOnTop ? 1.0 : 0.6
                onClicked: {
                    root.isAlwaysOnTop = !root.isAlwaysOnTop
                }
            }

            // App Title
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                spacing: 8
                
                Image {
                    source: "qrc:/qt/qml/FocusForgeApp/assets/icons/activity.svg"
                    sourceSize.width: 24
                    sourceSize.height: 24
                    Layout.alignment: Qt.AlignVCenter
                }
                
                Text {
                    text: qsTr("Brain Maintenance")
                    font.pixelSize: 22
                    font.weight: Font.DemiBold
                    font.family: "Inter"
                    color: "#8888aa"
                    opacity: 0.9
                    Layout.alignment: Qt.AlignVCenter
                }
            }

            // Mute Button
            Button {
                id: muteBtn
                icon.source: (audioController.isMuted || audioController.volume === 0) ? "qrc:/qt/qml/FocusForgeApp/assets/icons/volume-x.svg" : "qrc:/qt/qml/FocusForgeApp/assets/icons/volume.svg"
                icon.color: (audioController.isMuted || audioController.volume === 0) ? "#666688" : "#8888aa"
                icon.width: 20
                icon.height: 20
                background: Rectangle { color: "transparent" }
                opacity: 0.8
                onClicked: {
                    audioController.toggleMute()
                }
            }
            
            // Volume Slider
            Slider {
                id: volumeSlider
                Layout.preferredWidth: 80
                from: 0.0
                to: 1.0
                value: audioController.volume
                onValueChanged: {
                    if (pressed) {
                        audioController.volume = value;
                    }
                }
                
                background: Rectangle {
                    x: volumeSlider.leftPadding
                    y: volumeSlider.topPadding + volumeSlider.availableHeight / 2 - height / 2
                    width: volumeSlider.availableWidth
                    height: 4
                    radius: 2
                    color: "#33ffffff"
                    
                    Rectangle {
                        width: volumeSlider.visualPosition * parent.width
                        height: parent.height
                        color: "#ffffff"
                        radius: 2
                    }
                }
                
                handle: Rectangle {
                    x: volumeSlider.leftPadding + volumeSlider.visualPosition * (volumeSlider.availableWidth - width)
                    y: volumeSlider.topPadding + volumeSlider.availableHeight / 2 - height / 2
                    width: 12
                    height: 12
                    radius: 6
                    color: "#ffffff"
                    border.color: "#333333"
                }
            }

            // Todo List Toggle
            Button {
                id: todoBtn
                icon.source: "qrc:/qt/qml/FocusForgeApp/assets/icons/list.svg"
                icon.color: todoListPanel.visible ? "#00e0ff" : "#8888aa"
                icon.width: 20
                icon.height: 20
                background: Rectangle { color: "transparent" }
                opacity: todoListPanel.visible ? 1.0 : 0.6
                onClicked: {
                    todoListPanel.visible = !todoListPanel.visible
                }
            }
        }

        // ── Spacer ──
        Item { Layout.fillHeight: true; Layout.maximumHeight: 40 }

        // ── Breathing Orb (Central Visual — clickable for play/pause) ──
        BreathingOrb {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: todoListPanel.visible ? 160 : 300
            Layout.preferredHeight: todoListPanel.visible ? 160 : 300

            stateName: timerViewModel.currentStateName
            remainingSeconds: timerViewModel.remainingSeconds

            Behavior on Layout.preferredWidth { NumberAnimation { duration: 400; easing.type: Easing.OutQuart } }
            Behavior on Layout.preferredHeight { NumberAnimation { duration: 400; easing.type: Easing.OutQuart } }
        }

        // ── Pinned Focus Objective ──
        Text {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 4
            visible: todoListModel.pinnedTaskText !== ""
            text: "FOCUS: " + todoListModel.pinnedTaskText.toUpperCase()
            font.pixelSize: 14
            font.weight: Font.Medium
            font.family: "Inter"
            color: "#00e0ff"
            opacity: 0.8
            maximumLineCount: 1
            elide: Text.ElideRight
            width: 400
        }

        // ── Spacer ──
        Item { Layout.preferredHeight: 16 }

        // ── Countdown Timer with +/- 5min adjusters ──
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 8

            // -5 min (visible on hover)
            Button {
                id: minusBtn
                visible: timerViewModel.currentStateName === "Idle"
                text: "-5"
                font.pixelSize: 12
                font.family: "Inter"
                opacity: minusBtn.hovered ? 0.8 : 0.0
                background: Rectangle { color: "transparent" }
                contentItem: Text {
                    text: minusBtn.text
                    color: "#66ffffff"
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: timerViewModel.adjustTime(-5)
                Behavior on opacity { NumberAnimation { duration: 200 } }
            }

            // Main Timer Display
            Text {
                text: {
                    if (timerViewModel.isOvertime) {
                        // Overtime: count UP with + prefix
                        const secs = timerViewModel.overtimeSeconds;
                        const mins = Math.floor(secs / 60);
                        const s = secs % 60;
                        return "+" + String(mins).padStart(2, '0') + ":"
                             + String(s).padStart(2, '0');
                    } else {
                        const totalSec = timerViewModel.remainingSeconds;
                        const mins = Math.floor(totalSec / 60);
                        const secs = totalSec % 60;
                        return String(mins).padStart(2, '0') + ":"
                             + String(secs).padStart(2, '0');
                    }
                }
                font.pixelSize: todoListPanel.visible ? 48 : 64
                font.weight: Font.Light
                font.family: "JetBrains Mono, SF Mono, Consolas, monospace"
                font.letterSpacing: 4
                color: timerViewModel.isOvertime ? "#ff8c00" : "#ffffff"
                opacity: 0.95

                Behavior on font.pixelSize { NumberAnimation { duration: 400; easing.type: Easing.OutQuart } }
                Behavior on color { ColorAnimation { duration: 600 } }
                Behavior on opacity { NumberAnimation { duration: 400 } }
            }

            // +5 min (visible on hover)
            Button {
                id: plusBtn
                visible: timerViewModel.currentStateName === "Idle"
                text: "+5"
                font.pixelSize: 12
                font.family: "Inter"
                opacity: plusBtn.hovered ? 0.8 : 0.0
                background: Rectangle { color: "transparent" }
                contentItem: Text {
                    text: plusBtn.text
                    color: "#66ffffff"
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: timerViewModel.adjustTime(5)
                Behavior on opacity { NumberAnimation { duration: 200 } }
            }
        }

        // ── State Label ──
        Text {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 8
            text: {
                if (timerViewModel.currentStateName === "Overtime") return "FLOW STATE"
                return timerViewModel.currentStateName.toUpperCase()
            }
            font.pixelSize: 14
            font.weight: Font.Medium
            font.letterSpacing: 6
            font.family: "Inter"
            color: {
                switch (timerViewModel.currentStateName) {
                case "Focusing": return "#00e0ff";
                case "Overtime": return "#ff8c00";
                case "CoolDown": return "#a855f7";
                default:         return "#555577";
                }
            }

            Behavior on color {
                ColorAnimation { duration: 600 }
            }
        }

        // ── Break Health Tip (only during CoolDown) ──
        Text {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 12
            visible: timerViewModel.currentStateName === "CoolDown"
            text: root.breakTips[root.currentTipIndex]
            font.pixelSize: 16
            font.family: "Inter"
            color: "#a855f7"
            opacity: 0.7

            Behavior on opacity { NumberAnimation { duration: 500 } }
        }

        // ── Todo List Panel ──
        TodoListPanel {
            id: todoListPanel
            z: 10
            visible: false
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 10
            Layout.maximumHeight: 250
        }

        // ── Spacer ──
        Item { Layout.preferredHeight: 10 }

        // ── Control Buttons ──
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 12
            spacing: 20

            // Main Action Button
            Button {
                id: startBtn
                text: {
                    switch (timerViewModel.currentStateName) {
                    case "Idle": return "Start Focus"
                    case "Focusing": return "Pause"
                    case "Overtime": return "Finish & Review"
                    case "Paused": return "Resume"
                    case "CoolDown": return "Pause Break"
                    default: return "Start Focus"
                    }
                }
                enabled: true
                scale: pressed ? 0.95 : 1.0

                onClicked: {
                    switch (timerViewModel.currentStateName) {
                    case "Idle":
                        timerViewModel.startFocus();
                        break;
                    case "Focusing":
                        timerViewModel.pauseFocus();
                        break;
                    case "Overtime":
                        timerViewModel.finishFocusEarly();
                        break;
                    case "Paused":
                        timerViewModel.resume();
                        break;
                    case "CoolDown":
                        timerViewModel.pauseCoolDown();
                        break;
                    }
                }

                Behavior on scale { NumberAnimation { duration: 100 } }

                contentItem: Text {
                    text: startBtn.text
                    font.pixelSize: 15
                    font.weight: Font.Medium
                    font.family: "Inter"
                    color: startBtn.enabled ? "#ffffff" : "#888888"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    implicitWidth: 170
                    implicitHeight: 48
                    radius: 12
                    color: {
                        if (timerViewModel.currentStateName === "Overtime")
                            return startBtn.pressed ? "#b45309" : (startBtn.hovered ? "#f59e0b" : "#d97706")
                        if (timerViewModel.currentStateName === "CoolDown")
                            return startBtn.pressed ? "#6d28d9" : (startBtn.hovered ? "#7c3aed" : "#5b21b6")
                        return startBtn.pressed ? "#007a8c" : (startBtn.hovered ? "#00c8e0" : "#00a0c0")
                    }
                    border.width: 1
                    border.color: "#3300e0ff"

                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
                }
            }

            // Stop Button — Instant Reset
            Button {
                id: stopBtn
                visible: timerViewModel.currentStateName !== "Idle"
                scale: pressed ? 0.95 : 1.0

                text: "⏹️ Reset"

                onClicked: {
                    timerViewModel.stopFocus() // Universally aborts to Idle
                }

                contentItem: Text {
                    text: stopBtn.text
                    font.pixelSize: 15
                    font.weight: Font.Medium
                    font.family: "Inter"
                    color: "#ffffff"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    implicitWidth: 120
                    implicitHeight: 48
                    radius: 12
                    color: stopBtn.pressed ? "#991b1b" : (stopBtn.hovered ? "#ef4444" : "#dc2626")
                    border.width: 1
                    border.color: "#33ff0000"
                    
                    Behavior on color { ColorAnimation { duration: 150 } }
                }
            }
        }

        // ── Secondary Action: Skip to Break (subtle text link) ──
        Text {
            Layout.alignment: Qt.AlignHCenter
            visible: timerViewModel.currentStateName === "Idle" ||
                     timerViewModel.currentStateName === "Focusing" ||
                     timerViewModel.currentStateName === "Overtime"
            text: {
                switch (timerViewModel.currentStateName) {
                case "Idle": return "Skip to Break"
                case "Focusing": return "Finish Early"
                case "Overtime": return "End Flow State"
                default: return ""
                }
            }
            font.pixelSize: 12
            font.family: "Inter"
            color: "#555577"
            opacity: skipArea.containsMouse ? 0.9 : 0.4

            Behavior on opacity { NumberAnimation { duration: 200 } }

            MouseArea {
                id: skipArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (timerViewModel.currentStateName === "Idle") {
                        timerViewModel.startCoolDown()
                    } else {
                        timerViewModel.finishFocusEarly()
                    }
                }
            }
        }

        // ── Session Dots (daily progress) ──
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 8
            Layout.bottomMargin: 8
            spacing: 6
            visible: timerViewModel.sessionsCompletedToday > 0

            Repeater {
                model: timerViewModel.sessionsCompletedToday
                delegate: Rectangle {
                    width: 8
                    height: 8
                    radius: 4
                    color: "#00e0ff"
                    opacity: 0.7

                    SequentialAnimation on opacity {
                        loops: Animation.Infinite
                        NumberAnimation { from: 0.5; to: 1.0; duration: 2000 + index * 300; easing.type: Easing.InOutSine }
                        NumberAnimation { from: 1.0; to: 0.5; duration: 2000 + index * 300; easing.type: Easing.InOutSine }
                    }
                }
            }
        }

        // ── Duration Info ──
        Text {
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 16
            text: qsTr("Focus: %1 min  ·  Break: %2 min")
                  .arg(timerViewModel.focusDurationMinutes)
                  .arg(timerViewModel.coolDownDurationMinutes)
            font.pixelSize: 12
            font.family: "Inter"
            color: "#444466"
        }
    }

    // ── Quick Capture Shortcut & Popup ──
    Shortcut {
        sequences: ["Ctrl+T", "Meta+T"]
        onActivated: {
            root.raise()
            root.requestActivate()
            quickCapturePopup.open();
        }
    }

    // ── Global Volume Control Shortcuts ──
    Shortcut {
        sequences: ["Ctrl+Up", "Meta+Up"]
        onActivated: {
            audioController.volume = Math.min(1.0, audioController.volume + 0.1)
            showVolumeToast()
        }
    }

    Shortcut {
        sequences: ["Ctrl+Down", "Meta+Down"]
        onActivated: {
            audioController.volume = Math.max(0.0, audioController.volume - 0.1)
            showVolumeToast()
        }
    }

    // Volume Toast
    function showVolumeToast() {
        volumeToast.opacity = 1.0
        volumeToastHideTimer.restart()
    }

    Rectangle {
        id: volumeToast
        width: 160
        height: 40
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 30
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#d91a1a2e"
        radius: 20
        border.width: 1
        border.color: "#33ffffff"
        opacity: 0.0
        z: 100

        Behavior on opacity { NumberAnimation { duration: 200 } }

        RowLayout {
            anchors.centerIn: parent
            spacing: 8
            
            Text {
                text: audioController.isMuted || audioController.volume === 0 ? "🔇" : "🔊"
                font.pixelSize: 14
            }
            
            Rectangle {
                width: 80
                height: 6
                radius: 3
                color: "#33ffffff"
                
                Rectangle {
                    width: parent.width * audioController.volume
                    height: parent.height
                    radius: 3
                    color: "#00e0ff"
                }
            }
            
            Text {
                text: Math.round(audioController.volume * 100) + "%"
                color: "#ffffff"
                font.pixelSize: 12
                font.family: "Inter"
            }
        }

        Timer {
            id: volumeToastHideTimer
            interval: 1500
            onTriggered: volumeToast.opacity = 0.0
        }
    }

    QuickCapturePopup {
        id: quickCapturePopup
        
        onClosed: {
            todoListModel.loadTasks()
            todoListPanel.visible = true
        }
    }

    // ── Session Review Popup ──
    SessionReviewPopup {
        id: sessionReviewPopup
    }

    // Listen for session review requests from the ViewModel
    Connections {
        target: timerViewModel
        function onSessionReviewRequested() {
            sessionReviewPopup.open()
            // Cycle health tips for break screen
            root.currentTipIndex = (root.currentTipIndex + 1) % root.breakTips.length
        }
    }
}
