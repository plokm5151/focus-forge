import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: "#1a1a2e80"
    radius: 12
    border.width: 1
    border.color: "#ffffff1a"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 10

        Text {
            text: qsTr("📝 Focus Tasks")
            font.pixelSize: 16
            font.weight: Font.DemiBold
            font.family: "Inter, Segoe UI, sans-serif"
            color: "#ffffff"
            Layout.bottomMargin: 8
        }

        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 8
            
            // This references the C++ model exposed to QML via main.cpp
            model: todoListModel

            delegate: Rectangle {
                width: listView.width
                height: 40
                color: "#00000000" // Transparent
                radius: 8

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 4
                    spacing: 12

                    CheckBox {
                        id: taskCheck
                        checked: isCompleted // from role IsCompletedRole
                        
                        onClicked: {
                            // Call Q_INVOKABLE toggleTask
                            todoListModel.toggleTask(index);
                        }

                        // Glassmorphic CheckBox indicator
                        indicator: Rectangle {
                            implicitWidth: 20
                            implicitHeight: 20
                            x: taskCheck.leftPadding
                            y: parent.height / 2 - height / 2
                            radius: 4
                            color: taskCheck.checked ? "#6d28d9" : "#ffffff10"
                            border.color: taskCheck.checked ? "#a855f7" : "#ffffff40"

                            Text {
                                text: "✔"
                                color: "#ffffff"
                                font.pixelSize: 14
                                visible: taskCheck.checked
                                anchors.centerIn: parent
                            }

                            Behavior on color { ColorAnimation { duration: 150 } }
                        }
                    }

                    TextField {
                        id: taskTextField
                        Layout.fillWidth: true
                        text: display // from role DisplayRole
                        font.pixelSize: 14
                        font.family: "Inter, Segoe UI, sans-serif"
                        color: taskCheck.checked ? "#8888aa" : "#ffffff"
                        font.strikeout: taskCheck.checked
                        verticalAlignment: TextInput.AlignVCenter
                        background: Rectangle { color: "transparent" }

                        // Allow editing
                        readOnly: taskCheck.checked

                        onEditingFinished: {
                            if (text.trim() !== display) {
                                todoListModel.updateTaskText(index, text.trim());
                            }
                        }

                        Behavior on color { ColorAnimation { duration: 150 } }
                    }

                    // Priority Badge
                    Rectangle {
                        visible: priority > 0
                        width: priorityText.implicitWidth + 12
                        height: 20
                        radius: 10
                        color: priority === 3 ? "#ef444433" : (priority === 2 ? "#f59e0b33" : "#3b82f633")
                        border.width: 1
                        border.color: priority === 3 ? "#ef444488" : (priority === 2 ? "#f59e0b88" : "#3b82f688")
                        
                        Text {
                            id: priorityText
                            anchors.centerIn: parent
                            text: priority === 3 ? "High" : (priority === 2 ? "Med" : "Low")
                            color: priority === 3 ? "#fca5a5" : (priority === 2 ? "#fcd34d" : "#93c5fd")
                            font.pixelSize: 11
                            font.weight: Font.DemiBold
                            font.family: "Inter, Segoe UI, sans-serif"
                        }
                    }

                    // Due Date Badge
                    Rectangle {
                        visible: dueDate !== ""
                        width: dateText.implicitWidth + 16
                        height: 20
                        radius: 10
                        color: "#ffffff1a"
                        border.width: 1
                        border.color: "#ffffff33"
                        
                        Text {
                            id: dateText
                            anchors.centerIn: parent
                            text: "📅 " + dueDate
                            color: "#d1d5db"
                            font.pixelSize: 11
                            font.family: "Inter, Segoe UI, sans-serif"
                        }
                    }

                    // Delete Button
                    Button {
                        id: delBtn
                        text: "🗑️"
                        font.pixelSize: 14
                        Layout.preferredWidth: 32
                        Layout.preferredHeight: 32
                        background: Rectangle { color: "transparent" }
                        opacity: delBtn.hovered ? 1.0 : 0.0
                        
                        onClicked: {
                            todoListModel.deleteTask(index);
                        }
                        
                        Behavior on opacity { NumberAnimation { duration: 150 } }
                    }
                }
                
                // Hover effect
                Rectangle {
                    anchors.fill: parent
                    color: "#ffffff0a"
                    radius: 8
                    visible: mouseArea.containsMouse && !taskTextField.activeFocus
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    propagateComposedEvents: true
                    onClicked: (mouse) => {
                        mouse.accepted = false; // Let it pass to children
                    }
                }
            }
        }
    }
}
