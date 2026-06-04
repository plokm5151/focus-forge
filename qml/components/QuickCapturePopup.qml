/**
 * @file QuickCapturePopup.qml
 * @brief Glassmorphic quick capture popup for logging tasks to Obsidian.
 *
 * Activated via global shortcut (Cmd+T / Ctrl+T). Provides a sleek,
 * distraction-free input field for task capture. Supports keyboard-based
 * metadata parsing (e.g., !h for High Priority, !t for Today).
 */
import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Layouts

Popup {
    id: popup
    anchors.centerIn: Overlay.overlay
    width: 460
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
        
        placeholderText: qsTr("Capture... (Use !h/m/l for priority, !t/tmrw for date)")
        font.pixelSize: 18
        font.family: "Inter, Segoe UI, sans-serif"
        color: "#ffffff"
        
        background: Rectangle { color: "transparent" }
        
        onAccepted: {
            let rawText = text.trim()
            if (rawText !== "") {
                // Parse Priority
                let prio = 0
                if (rawText.match(/\s!(high|h)\b/i) || rawText.match(/^!(high|h)\b/i)) {
                    prio = 3
                    rawText = rawText.replace(/\s*!(high|h)\b/ig, "")
                } else if (rawText.match(/\s!(med|m)\b/i) || rawText.match(/^!(med|m)\b/i)) {
                    prio = 2
                    rawText = rawText.replace(/\s*!(med|m)\b/ig, "")
                } else if (rawText.match(/\s!(low|l)\b/i) || rawText.match(/^!(low|l)\b/i)) {
                    prio = 1
                    rawText = rawText.replace(/\s*!(low|l)\b/ig, "")
                }

                // Parse Date
                let dateStr = ""
                if (rawText.match(/\s!(today|t)\b/i) || rawText.match(/^!(today|t)\b/i)) {
                    let d = new Date()
                    let year = d.getFullYear()
                    let month = String(d.getMonth() + 1).padStart(2, '0')
                    let day = String(d.getDate()).padStart(2, '0')
                    dateStr = `${year}-${month}-${day}`
                    rawText = rawText.replace(/\s*!(today|t)\b/ig, "")
                } else if (rawText.match(/\s!(tomorrow|tmrw)\b/i) || rawText.match(/^!(tomorrow|tmrw)\b/i)) {
                    let d = new Date()
                    d.setDate(d.getDate() + 1)
                    let year = d.getFullYear()
                    let month = String(d.getMonth() + 1).padStart(2, '0')
                    let day = String(d.getDate()).padStart(2, '0')
                    dateStr = `${year}-${month}-${day}`
                    rawText = rawText.replace(/\s*!(tomorrow|tmrw)\b/ig, "")
                }

                timerViewModel.submitTodoWithMetadata(rawText.trim(), prio, dateStr)
                text = ""
                popup.close()
            }
        }
    }

    onOpened: {
        taskInput.forceActiveFocus()
    }
}
