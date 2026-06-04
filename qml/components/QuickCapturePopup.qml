/**
 * @file QuickCapturePopup.qml
 * @brief Glassmorphic quick capture popup for logging tasks to Obsidian.
 *
 * Activated via global shortcut (Cmd+T / Ctrl+T). Provides a sleek,
 * distraction-free input field for task capture.
 */
import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Layouts

Popup {
    id: popup
    anchors.centerIn: Overlay.overlay
    width: 400
    height: 140
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    // Property to expose the text field for focusing
    property alias captureInput: taskInput
    
    // State properties for new task metadata
    property int selectedPriority: 0 // 0=None, 1=Low, 2=Med, 3=High
    property string selectedDueDate: ""

    // ── Glassmorphic Background ──
    background: Rectangle {
        color: "#151525"
        opacity: 0.85
        radius: 16
        border.color: "#33ffffff"
        border.width: 1

        layer.enabled: true
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowBlur: 1.5
            shadowColor: "#00e0ff"
            shadowOpacity: 0.15
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        // ── Input Field ──
        TextField {
            id: taskInput
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            
            placeholderText: qsTr("Capture a task... (Press Enter)")
            font.pixelSize: 18
            font.family: "Inter, Segoe UI, sans-serif"
            color: "#ffffff"
            
            background: Rectangle { color: "transparent" }
            
            onAccepted: {
                if (text.trim() !== "") {
                    timerViewModel.submitTodoWithMetadata(text.trim(), popup.selectedPriority, popup.selectedDueDate)
                    text = ""
                    popup.selectedPriority = 0
                    popup.selectedDueDate = ""
                    popup.close()
                }
            }
        }

        // ── Metadata Controls ──
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
                text: "Priority:"
                color: "#8888aa"
                font.pixelSize: 12
            }

            ComboBox {
                id: priorityCombo
                model: ["None", "Low", "Medium", "High"]
                currentIndex: 0
                Layout.preferredHeight: 30
                Layout.preferredWidth: 100
                onActivated: {
                    if (currentIndex === 1) popup.selectedPriority = 1;
                    else if (currentIndex === 2) popup.selectedPriority = 2;
                    else if (currentIndex === 3) popup.selectedPriority = 3;
                    else popup.selectedPriority = 0;
                }
            }

            Text {
                text: "Date:"
                color: "#8888aa"
                font.pixelSize: 12
                Layout.leftMargin: 12
            }

            ComboBox {
                id: dateCombo
                model: ["No Date", "Today", "Tomorrow"]
                currentIndex: 0
                Layout.preferredHeight: 30
                Layout.preferredWidth: 110
                onActivated: {
                    if (currentIndex === 0) {
                        popup.selectedDueDate = "";
                    } else {
                        const d = new Date();
                        if (currentIndex === 2) {
                            d.setDate(d.getDate() + 1);
                        }
                        const year = d.getFullYear();
                        const month = String(d.getMonth() + 1).padStart(2, '0');
                        const day = String(d.getDate()).padStart(2, '0');
                        popup.selectedDueDate = `${year}-${month}-${day}`;
                    }
                }
            }
        }
    }

    onOpened: {
        taskInput.forceActiveFocus()
    }
}
