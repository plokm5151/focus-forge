import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: root
    width: 320
    height: 220
    modal: true
    focus: true
    anchors.centerIn: Overlay.overlay

    // Signal emitted when the user successfully confirms deletion
    signal clearConfirmed()

    background: Rectangle {
        color: "#1a1a2e"
        radius: 12
        border.width: 1
        border.color: "#33ffffff"
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 16

        Text {
            text: qsTr("Clear All Tasks?")
            font.pixelSize: 18
            font.weight: Font.Bold
            color: "#ff4444"
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: qsTr("This action cannot be undone. All tasks in history and active focus will be permanently deleted.")
            font.pixelSize: 13
            color: "#aaaaaa"
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
        }

        TextField {
            id: confirmInput
            Layout.fillWidth: true
            placeholderText: qsTr("Type 'clean' to confirm")
            placeholderTextColor: "#88ffffff"
            font.pixelSize: 14
            color: "#ffffff"
            horizontalAlignment: TextInput.AlignHCenter
            background: Rectangle {
                color: "#10ffffff"
                radius: 6
                border.width: 1
                border.color: confirmInput.activeFocus ? "#ff4444" : "#33ffffff"
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 8
            spacing: 12

            Button {
                text: qsTr("Cancel")
                Layout.fillWidth: true
                font.pixelSize: 14
                background: Rectangle {
                    color: "transparent"
                    border.width: 1
                    border.color: "#555577"
                    radius: 6
                }
                contentItem: Text {
                    text: parent.text
                    color: "#ffffff"
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    root.close()
                }
            }

            Button {
                id: clearBtn
                text: qsTr("Clear")
                Layout.fillWidth: true
                enabled: confirmInput.text === "clean"
                font.pixelSize: 14
                font.weight: Font.Bold
                background: Rectangle {
                    color: parent.enabled ? "#ef4444" : "#55ef4444"
                    radius: 6
                }
                contentItem: Text {
                    text: parent.text
                    color: parent.enabled ? "#ffffff" : "#88ffffff"
                    font.pixelSize: 14
                    font.weight: Font.Bold
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    root.clearConfirmed()
                    root.close()
                }
            }
        }
    }

    onOpened: {
        confirmInput.text = ""
        confirmInput.forceActiveFocus()
    }
}
