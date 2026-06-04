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
    height: 80
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    // Property to expose the text field for focusing
    property alias captureInput: taskInput

    // ── Glassmorphic Background ──
    background: Rectangle {
        color: "#151525"
        opacity: 0.85
        radius: 16
        border.color: "#33ffffff"
        border.width: 1

        // Inner subtle glow
        layer.enabled: true
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowBlur: 1.5
            shadowColor: "#00e0ff"
            shadowOpacity: 0.15
        }
    }

    // ── Input Field ──
    contentItem: TextField {
        id: taskInput
        anchors.fill: parent
        anchors.margins: 4
        
        placeholderText: qsTr("Capture a task... (Press Enter)")
        font.pixelSize: 18
        font.family: "Inter, Segoe UI, sans-serif"
        color: "#ffffff"
        
        // Minimalist transparent styling
        background: Rectangle {
            color: "transparent"
        }
        
        onAccepted: {
            if (text.trim() !== "") {
                timerViewModel.submitTodo(text)
                text = ""
                popup.close()
            }
        }
    }

    onOpened: {
        taskInput.forceActiveFocus()
    }
}
