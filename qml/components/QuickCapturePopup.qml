/**
 * @file QuickCapturePopup.qml
 * @brief Glassmorphic quick capture popup for logging tasks to Obsidian.
 *
 * Activated via global shortcut (Cmd+T / Ctrl+T). Provides a sleek,
 * distraction-free input field for task capture. Supports keyboard-based
 * metadata parsing (e.g., !h for High Priority, !t for Today).
 *
 * Upgraded to multi-line TextArea for complex thoughts.
 * All dates forced to UTC+8 (Taiwan time).
 */
import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Layouts

Popup {
    id: popup
    anchors.centerIn: Overlay.overlay
    width: 460
    height: Math.min(taskInput.implicitHeight + 32, 200) // Auto-expand, max 200
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property alias captureInput: taskInput

    // Helper: Get Taiwan date (UTC+8) regardless of system timezone
    function getTaiwanDate(offsetDays) {
        let now = new Date()
        // Get UTC time, then add 8 hours for Taiwan
        let utcMs = now.getTime() + (now.getTimezoneOffset() * 60000)
        let twMs = utcMs + (8 * 3600000)
        let tw = new Date(twMs)
        if (offsetDays) tw.setDate(tw.getDate() + offsetDays)
        let year = tw.getFullYear()
        let month = String(tw.getMonth() + 1).padStart(2, '0')
        let day = String(tw.getDate()).padStart(2, '0')
        return `${year}-${month}-${day}`
    }

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

    // ── Input Field (Multi-line TextArea) ──
    contentItem: ScrollView {
        anchors.fill: parent
        anchors.margins: 4

        TextArea {
            id: taskInput
            placeholderText: qsTr("Capture... (Use !h/m/l for priority, !t/tmrw for date)")
            font.pixelSize: 18
            font.family: "Inter, Segoe UI, sans-serif"
            color: "#ffffff"
            wrapMode: TextEdit.Wrap
            
            background: Rectangle { color: "transparent" }

            // Submit on Ctrl+Enter or Cmd+Enter (Enter creates new line)
            Keys.onReturnPressed: (event) => {
                if (event.modifiers & Qt.ControlModifier || event.modifiers & Qt.MetaModifier) {
                    submitTask()
                    event.accepted = true
                } else if (text.indexOf('\n') === -1) {
                    // If single line, Enter also submits
                    submitTask()
                    event.accepted = true
                }
            }

            function submitTask() {
                let rawText = text.trim()
                if (rawText === "") return

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

                // Parse Date (forced Taiwan timezone)
                let dateStr = ""
                if (rawText.match(/\s!(today|t)\b/i) || rawText.match(/^!(today|t)\b/i)) {
                    dateStr = popup.getTaiwanDate(0)
                    rawText = rawText.replace(/\s*!(today|t)\b/ig, "")
                } else if (rawText.match(/\s!(tomorrow|tmrw)\b/i) || rawText.match(/^!(tomorrow|tmrw)\b/i)) {
                    dateStr = popup.getTaiwanDate(1)
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
