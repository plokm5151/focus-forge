/**
 * @file main.qml
 * @brief Root QML view for the Brain Maintenance Dashboard.
 *
 * Integrates the BreathingOrb shader, a minimalistic countdown display,
 * and Start/Pause controls. All data is bound to the timerViewModel
 * context property exposed from C++.
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

        // ── App Title ──
        Text {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 16
            text: qsTr("🧠 Brain Maintenance")
            font.pixelSize: 24
            font.weight: Font.DemiBold
            font.family: "Inter, Segoe UI, Helvetica Neue, sans-serif"
            color: "#8888aa"
            opacity: 0.9
        }

        // ── Spacer ──
        Item { Layout.fillHeight: true; Layout.maximumHeight: 40 }

        // ── Breathing Orb (Central Visual) ──
        BreathingOrb {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 300
            Layout.preferredHeight: 300

            stateName: timerViewModel.currentStateName
            remainingSeconds: timerViewModel.remainingSeconds
        }

        // ── Spacer ──
        Item { Layout.preferredHeight: 24 }

        // ── Countdown Timer ──
        Text {
            Layout.alignment: Qt.AlignHCenter
            text: {
                const totalSec = timerViewModel.remainingSeconds;
                const mins = Math.floor(totalSec / 60);
                const secs = totalSec % 60;
                return String(mins).padStart(2, '0') + ":"
                     + String(secs).padStart(2, '0');
            }
            font.pixelSize: 64
            font.weight: Font.Light
            font.family: "JetBrains Mono, SF Mono, Consolas, monospace"
            font.letterSpacing: 4
            color: "#ffffff"
            opacity: 0.95

            Behavior on opacity {
                NumberAnimation { duration: 400 }
            }
        }

        // ── State Label ──
        Text {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 8
            text: timerViewModel.currentStateName.toUpperCase()
            font.pixelSize: 14
            font.weight: Font.Medium
            font.letterSpacing: 6
            font.family: "Inter, Segoe UI, Helvetica Neue, sans-serif"
            color: {
                switch (timerViewModel.currentStateName) {
                case "Focusing": return "#00e0ff";
                case "CoolDown": return "#a855f7";
                default:         return "#555577";
                }
            }

            Behavior on color {
                ColorAnimation { duration: 600 }
            }
        }

        // ── Todo List Panel ──
        TodoListPanel {
            id: todoListPanel
            z: 10
            visible: false
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 10
            Layout.preferredHeight: 150
            Layout.maximumHeight: 250
        }

        // ── Spacer ──
        Item { Layout.preferredHeight: 10 }

        // ── Control Buttons ──
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 40
            spacing: 20

            // Start / Resume Button
            Button {
                id: startBtn
                text: timerViewModel.currentStateName === "Idle" ? "Start Focus" : "Pause"
                enabled: true
                scale: pressed ? 0.95 : 1.0

                onClicked: {
                    if (timerViewModel.currentStateName === "Focusing") {
                        timerViewModel.pauseFocus();
                    } else {
                        timerViewModel.startFocus();
                    }
                }

                Behavior on scale { NumberAnimation { duration: 100 } }

                contentItem: Text {
                    text: startBtn.text
                    font.pixelSize: 15
                    font.weight: Font.Medium
                    font.family: "Inter, Segoe UI, sans-serif"
                    color: startBtn.enabled ? "#ffffff" : "#888888"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    implicitWidth: 170
                    implicitHeight: 48
                    radius: 12
                    color: startBtn.enabled
                           ? (startBtn.hovered ? "#00c8e0" : "#00a0c0")
                           : "#1a2a3a"
                    border.width: 1
                    border.color: startBtn.enabled ? "#00e0ff33" : "#ffffff10"

                    Behavior on color {
                        ColorAnimation { duration: 200 }
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
            font.family: "Inter, Segoe UI, sans-serif"
            color: "#444466"
        }
    }

    // ── Quick Capture Shortcut & Popup ──
    Shortcut {
        sequence: "Ctrl+T"
        onActivated: {
            todoListPanel.visible = !todoListPanel.visible;
            if (todoListPanel.visible) todoListPanel.forceActiveFocus();
        }
    }

    QuickCapturePopup {
        id: quickCapturePopup
        
        // Refresh todo list model after adding a task
        onClosed: {
            todoListModel.loadTasks()
        }
    }
}
