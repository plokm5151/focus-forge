/**
 * @file TodoListPanel.qml
 * @brief Focus Tasks panel with inline confirmation, auto-scroll,
 *        pin-to-focus, empty state guide, and undo toast.
 */
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: "#801a1a2e"
    radius: 12
    border.width: 1
    border.color: "#1affffff"

    // Undo toast state
    property bool showUndoToast: false

    // Dynamic height: min 80, grows with content, max 5 items
    readonly property int itemHeight: 44
    readonly property int maxVisibleItems: 5
    readonly property int headerHeight: 40
    readonly property int panelPadding: 32
    implicitHeight: {
        let contentH = headerHeight + panelPadding + Math.min(todoListModel.rowCount(), maxVisibleItems) * (itemHeight + 8)
        return Math.max(80, contentH)
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 10

        Text {
            text: qsTr("📝 Focus Tasks")
            font.pixelSize: 16
            font.weight: Font.DemiBold
            font.family: "Inter"
            color: "#ffffff"
            Layout.bottomMargin: 8
        }

        // ── Empty State Guide ──
        Text {
            visible: todoListModel.rowCount() === 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            text: qsTr("任務已清空\n按下 Cmd+T 捕捉下一個靈感 💡")
            font.pixelSize: 14
            font.family: "Inter"
            color: "#555577"
            opacity: 0.7
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
        }

        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 8
            visible: todoListModel.rowCount() > 0
            
            model: todoListModel

            // Custom scrollbar that appears only when needed
            ScrollBar.vertical: ScrollBar {
                policy: listView.contentHeight > listView.height ? ScrollBar.AsNeeded : ScrollBar.AlwaysOff
                width: 4

                contentItem: Rectangle {
                    implicitWidth: 4
                    radius: 2
                    color: "#33ffffff"
                    opacity: parent.active ? 1.0 : 0.0
                    Behavior on opacity { NumberAnimation { duration: 300 } }
                }
            }

            delegate: Rectangle {
                id: delegateRoot
                width: listView.width
                height: root.itemHeight
                color: root.confirmingIndex === index ? "#1500e0ff" : "#00000000"
                radius: 8
                border.width: root.confirmingIndex === index ? 1 : 0
                border.color: "#4400e0ff"

                Behavior on color { ColorAnimation { duration: 200 } }
                Behavior on border.width { NumberAnimation { duration: 200 } }

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 4
                    spacing: 8

                    // Pin Button (visible on hover)
                    Button {
                        id: pinBtn
                        text: todoListModel.pinnedIndex === index ? "📌" : "📍"
                        font.pixelSize: 12
                        Layout.preferredWidth: 28
                        Layout.preferredHeight: 28
                        background: Rectangle { color: "transparent" }
                        opacity: todoListModel.pinnedIndex === index ? 1.0 : (pinBtn.hovered ? 0.7 : 0.0)

                        onClicked: {
                            if (todoListModel.pinnedIndex === index) {
                                todoListModel.unpinTask()
                            } else {
                                todoListModel.pinTask(index)
                            }
                        }

                        Behavior on opacity { NumberAnimation { duration: 150 } }
                    }

                    // CheckBox (Custom implementation for reliable clicking)
                    Item {
                        width: 32
                        height: 32
                        Layout.alignment: Qt.AlignVCenter
                        
                        Rectangle {
                            id: indicatorRect
                            width: 20
                            height: 20
                            anchors.centerIn: parent
                            radius: 4
                            color: isCompleted ? "#6d28d9" : "#10ffffff"
                            border.color: isCompleted ? "#a855f7" : "#40ffffff"

                            Text {
                                text: "✔"
                                color: "#ffffff"
                                font.pixelSize: 14
                                visible: isCompleted
                                anchors.centerIn: parent
                            }

                            Behavior on color { ColorAnimation { duration: 150 } }
                        }

                        SequentialAnimation {
                            id: clickAnim
                            NumberAnimation { target: indicatorRect; property: "scale"; to: 0.8; duration: 50; easing.type: Easing.OutQuad }
                            NumberAnimation { target: indicatorRect; property: "scale"; to: 1.1; duration: 100; easing.type: Easing.OutBack }
                            NumberAnimation { target: indicatorRect; property: "scale"; to: 1.0; duration: 100; easing.type: Easing.OutQuad }
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                audioController.playClickSound()
                                todoListModel.toggleTask(index)
                                clickAnim.start()
                            }
                        }
                    }

                        // Normal mode: editable text field
                        TextField {
                            id: taskTextField
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            text: display
                            font.pixelSize: 14
                            font.family: "Inter"
                            color: isCompleted ? "#8888aa" : "#ffffff"
                            font.strikeout: isCompleted
                            verticalAlignment: TextInput.AlignVCenter
                            background: Rectangle { color: "transparent" }
                            readOnly: isCompleted

                            onEditingFinished: {
                                if (text.trim() !== display) {
                                    todoListModel.updateTaskWithNLP(index, text.trim())
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
                        color: priority === 3 ? "#33ef4444" : (priority === 2 ? "#33f59e0b" : "#333b82f6")
                        border.width: 1
                        border.color: priority === 3 ? "#88ef4444" : (priority === 2 ? "#88f59e0b" : "#883b82f6")
                        
                        Text {
                            id: priorityText
                            anchors.centerIn: parent
                            text: priority === 3 ? "High" : (priority === 2 ? "Med" : "Low")
                            color: priority === 3 ? "#fca5a5" : (priority === 2 ? "#fcd34d" : "#93c5fd")
                            font.pixelSize: 11
                            font.weight: Font.DemiBold
                            font.family: "Inter"
                        }
                    }

                    // Due Date Badge
                    Rectangle {
                        visible: dueDate !== ""
                        width: dateText.implicitWidth + 16
                        height: 20
                        radius: 10
                        color: "#1affffff"
                        border.width: 1
                        border.color: "#33ffffff"
                        
                        Text {
                            id: dateText
                            anchors.centerIn: parent
                            text: "📅 " + dueDate
                            color: "#d1d5db"
                            font.pixelSize: 11
                            font.family: "Inter"
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
                            todoListModel.deleteTask(index)
                            root.showUndoToast = true
                            undoTimer.restart()
                        }
                        
                        Behavior on opacity { NumberAnimation { duration: 150 } }
                    }
                }
                
                // Hover effect
                Rectangle {
                    anchors.fill: parent
                    color: "#0affffff"
                    radius: 8
                    visible: mouseArea.containsMouse && !taskTextField.activeFocus && root.confirmingIndex !== index
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    propagateComposedEvents: true
                    onClicked: (mouse) => {
                        mouse.accepted = false;
                    }
                }
            }
        }
    }

    // ── Undo Toast ──
    Rectangle {
        id: undoToast
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 8
        width: undoRow.implicitWidth + 24
        height: 36
        radius: 18
        color: "#2a2a4a"
        border.width: 1
        border.color: "#22ffffff"
        visible: root.showUndoToast
        opacity: root.showUndoToast ? 1.0 : 0.0
        z: 100

        Behavior on opacity { NumberAnimation { duration: 300 } }

        RowLayout {
            id: undoRow
            anchors.centerIn: parent
            spacing: 8

            Text {
                text: "已刪除任務"
                color: "#aaaacc"
                font.pixelSize: 12
                font.family: "Inter"
            }

            Button {
                text: "復原"
                font.pixelSize: 12
                font.weight: Font.Bold
                background: Rectangle { color: "transparent" }

                contentItem: Text {
                    text: "復原"
                    color: "#00e0ff"
                    font.pixelSize: 12
                    font.weight: Font.Bold
                }

                onClicked: {
                    todoListModel.undoDelete()
                    root.showUndoToast = false
                    todoListModel.loadTasks()
                }
            }
        }
    }

    Timer {
        id: undoTimer
        interval: 5000
        onTriggered: {
            root.showUndoToast = false
        }
    }
}
