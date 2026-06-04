/**
 * @file main.qml
 * @brief Minimal QML entry point for Phase 1 skeleton validation.
 *
 * This is a placeholder UI that verifies the ViewModel binding is working.
 * The full UI will be implemented in Phase 2.
 */
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root

    width: 480
    height: 640
    visible: true
    title: qsTr("Brain Maintenance Dashboard")
    color: "#1a1a2e"

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 24

        // -- App Title --
        Text {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("🧠 Brain Maintenance")
            font.pixelSize: 28
            font.bold: true
            color: "#e94560"
        }

        // -- Timer Display --
        Text {
            Layout.alignment: Qt.AlignHCenter
            text: {
                const mins = Math.floor(timerViewModel.remainingSeconds / 60);
                const secs = timerViewModel.remainingSeconds % 60;
                return String(mins).padStart(2, '0') + ":"
                     + String(secs).padStart(2, '0');
            }
            font.pixelSize: 72
            font.family: "monospace"
            color: "#ffffff"
        }

        // -- State Label --
        Text {
            Layout.alignment: Qt.AlignHCenter
            text: timerViewModel.currentStateName
            font.pixelSize: 18
            color: "#0f3460"
            opacity: 0.8
        }

        // -- Control Buttons --
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 16

            Button {
                text: qsTr("▶ Start Focus")
                onClicked: timerViewModel.startFocus()
                palette.buttonText: "#ffffff"
                background: Rectangle {
                    color: "#e94560"
                    radius: 8
                    implicitWidth: 140
                    implicitHeight: 44
                }
            }

            Button {
                text: qsTr("⏸ Pause")
                onClicked: timerViewModel.pauseFocus()
                palette.buttonText: "#ffffff"
                background: Rectangle {
                    color: "#0f3460"
                    radius: 8
                    implicitWidth: 140
                    implicitHeight: 44
                }
            }
        }
    }
}
