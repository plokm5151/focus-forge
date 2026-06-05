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
        "站起來走走，喝杯水 💧",
        "看向 6 公尺外的遠方 👀",
        "深呼吸三次，放鬆肩膀 🧘",
        "伸展手腕和手指 ✋",
        "閉上眼睛休息一下 😌"
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
                text: "📌"
                font.pixelSize: 18
                background: Rectangle { color: "transparent" }
                opacity: root.isAlwaysOnTop ? 1.0 : 0.4
                onClicked: {
                    root.isAlwaysOnTop = !root.isAlwaysOnTop
                }
            }

            // App Title
            Text {
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                text: qsTr("🧠 Brain Maintenance")
                font.pixelSize: 24
                font.weight: Font.DemiBold
                font.family: "Inter"
                color: "#8888aa"
                opacity: 0.9
                horizontalAlignment: Text.AlignHCenter
            }

            // Mute Button
            Button {
                id: muteBtn
                text: audioController.isMuted ? "🔇" : "🔊"
                font.pixelSize: 18
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
                    color: "#ffffff33"
                    
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
                text: "📝"
                font.pixelSize: 18
                background: Rectangle { color: "transparent" }
                opacity: todoListPanel.visible ? 1.0 : 0.4
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
            Layout.preferredWidth: 300
            Layout.preferredHeight: 300

            stateName: timerViewModel.currentStateName
            remainingSeconds: timerViewModel.remainingSeconds
        }

        // ── Pinned Focus Objective ──
        Text {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 4
            visible: todoListModel.pinnedTaskText !== ""
            text: "🎯 " + todoListModel.pinnedTaskText
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
                text: "-5"
                font.pixelSize: 12
                font.family: "Inter"
                opacity: minusBtn.hovered ? 0.8 : 0.0
                background: Rectangle { color: "transparent" }
                contentItem: Text {
                    text: minusBtn.text
                    color: "#ffffff66"
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
                font.pixelSize: 64
                font.weight: Font.Light
                font.family: "JetBrains Mono, SF Mono, Consolas, monospace"
                font.letterSpacing: 4
                color: timerViewModel.isOvertime ? "#ff8c00" : "#ffffff"
                opacity: 0.95

                Behavior on color { ColorAnimation { duration: 600 } }
                Behavior on opacity { NumberAnimation { duration: 400 } }
            }

            // +5 min (visible on hover)
            Button {
                id: plusBtn
                text: "+5"
                font.pixelSize: 12
                font.family: "Inter"
                opacity: plusBtn.hovered ? 0.8 : 0.0
                background: Rectangle { color: "transparent" }
                contentItem: Text {
                    text: plusBtn.text
                    color: "#ffffff66"
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
                if (timerViewModel.currentStateName === "Overtime") return "FLOW STATE ✨"
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
            Layout.margins: 10
            Layout.preferredHeight: implicitHeight
            Layout.maximumHeight: 300
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
                        timerViewModel.startFocus();
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
                    border.color: "#00e0ff33"

                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
                }
            }

            // Stop Button — LONG PRESS (1.5s) to prevent accidental reset
            Button {
                id: stopBtn
                visible: timerViewModel.currentStateName === "Paused"
                scale: pressed ? 0.95 : 1.0
                property real holdProgress: 0.0
                property bool holdActive: false

                text: "⏹️ Hold to Stop"

                onPressed: {
                    holdActive = true
                    holdProgress = 0
                    holdTimer.start()
                }
                onReleased: {
                    holdActive = false
                    holdTimer.stop()
                    holdProgress = 0
                }
                onCanceled: {
                    holdActive = false
                    holdTimer.stop()
                    holdProgress = 0
                }

                Timer {
                    id: holdTimer
                    interval: 50
                    repeat: true
                    onTriggered: {
                        stopBtn.holdProgress += 50.0 / 1500.0
                        if (stopBtn.holdProgress >= 1.0) {
                            holdTimer.stop()
                            stopBtn.holdProgress = 0
                            stopBtn.holdActive = false
                            timerViewModel.stopFocus()
                        }
                    }
                }

                Behavior on scale { NumberAnimation { duration: 100 } }

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
                    implicitWidth: 150
                    implicitHeight: 48
                    radius: 12
                    color: "#dc2626"
                    border.width: 1
                    border.color: "#ff000033"

                    // Hold progress overlay
                    Rectangle {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        width: parent.width * stopBtn.holdProgress
                        radius: 12
                        color: "#ffffff33"
                    }
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
                case "Idle": return "☕️ Skip to Break"
                case "Focusing": return "⏭️ Finish Early"
                case "Overtime": return "⏭️ End Flow State"
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
