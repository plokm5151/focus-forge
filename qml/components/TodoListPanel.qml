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

                    Text {
                        Layout.fillWidth: true
                        text: display // from role DisplayRole
                        font.pixelSize: 14
                        font.family: "Inter, Segoe UI, sans-serif"
                        color: taskCheck.checked ? "#8888aa" : "#ffffff"
                        font.strikeout: taskCheck.checked
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight

                        Behavior on color { ColorAnimation { duration: 150 } }
                    }
                }
                
                // Hover effect
                Rectangle {
                    anchors.fill: parent
                    color: "#ffffff0a"
                    radius: 8
                    visible: mouseArea.containsMouse
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        todoListModel.toggleTask(index);
                    }
                }
            }
        }
    }
}
