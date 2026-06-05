/**
 * @file BreathingOrb.qml
 * @brief Battery-friendly breathing orb using pure QML circles.
 *
 * Replaces the GPU-heavy Fragment Shader with layered QML Rectangles
 * using radius, scale, and opacity animations. Visually equivalent
 * but with near-zero power consumption.
 */
import QtQuick

Item {
    id: root

    // ── Public Interface (bound from parent / ViewModel) ──
    property string stateName: "Idle"
    property int remainingSeconds: 0

    // ── Derived Colors ──
    readonly property color orbColor: {
        switch (stateName) {
        case "Focusing": return "#00e0ff";
        case "Overtime": return "#ff8c00";
        case "CoolDown": return "#a855f7";
        default:         return "#3366aa";
        }
    }

    readonly property real breathSpeed: {
        switch (stateName) {
        case "Focusing": return 4000;   // Slow, steady breath
        case "Overtime": return 2000;   // Faster, urgent
        case "CoolDown": return 5000;   // Very slow, relaxed
        default:         return 3500;   // Idle — gentle
        }
    }

    // ── Interactive: Click to toggle play/pause ──
    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            if (timerViewModel.currentStateName === "Focusing" ||
                timerViewModel.currentStateName === "Overtime") {
                timerViewModel.pauseFocus();
            } else if (timerViewModel.currentStateName === "Paused") {
                timerViewModel.resume();
            }
        }
    }

    // ── Layer 1: Outer Glow (largest, most transparent) ──
    Rectangle {
        id: outerGlow
        anchors.centerIn: parent
        width: root.width * 0.95
        height: width
        radius: width / 2
        color: "transparent"
        border.width: 2
        border.color: root.orbColor
        opacity: 0.15

        SequentialAnimation on scale {
            loops: Animation.Infinite
            NumberAnimation { from: 0.85; to: 1.15; duration: root.breathSpeed; easing.type: Easing.InOutSine }
            NumberAnimation { from: 1.15; to: 0.85; duration: root.breathSpeed; easing.type: Easing.InOutSine }
        }

        Behavior on border.color { ColorAnimation { duration: 800 } }
    }

    // ── Layer 2: Middle Ring ──
    Rectangle {
        id: middleRing
        anchors.centerIn: parent
        width: root.width * 0.7
        height: width
        radius: width / 2
        color: "transparent"
        border.width: 3
        border.color: root.orbColor
        opacity: 0.25

        SequentialAnimation on scale {
            loops: Animation.Infinite
            NumberAnimation { from: 0.9; to: 1.1; duration: root.breathSpeed * 0.9; easing.type: Easing.InOutSine }
            NumberAnimation { from: 1.1; to: 0.9; duration: root.breathSpeed * 0.9; easing.type: Easing.InOutSine }
        }

        Behavior on border.color { ColorAnimation { duration: 800 } }
    }

    // ── Layer 3: Inner Glow Core ──
    Rectangle {
        id: innerCore
        anchors.centerIn: parent
        width: root.width * 0.45
        height: width
        radius: width / 2

        gradient: Gradient {
            GradientStop { position: 0.0; color: Qt.rgba(root.orbColor.r, root.orbColor.g, root.orbColor.b, 0.35) }
            GradientStop { position: 1.0; color: Qt.rgba(root.orbColor.r, root.orbColor.g, root.orbColor.b, 0.05) }
        }

        SequentialAnimation on scale {
            loops: Animation.Infinite
            NumberAnimation { from: 0.95; to: 1.08; duration: root.breathSpeed * 0.8; easing.type: Easing.InOutQuad }
            NumberAnimation { from: 1.08; to: 0.95; duration: root.breathSpeed * 0.8; easing.type: Easing.InOutQuad }
        }

        SequentialAnimation on opacity {
            loops: Animation.Infinite
            NumberAnimation { from: 0.4; to: 0.8; duration: root.breathSpeed; easing.type: Easing.InOutSine }
            NumberAnimation { from: 0.8; to: 0.4; duration: root.breathSpeed; easing.type: Easing.InOutSine }
        }
    }

    // ── Layer 4: Tiny Bright Center Dot ──
    Rectangle {
        anchors.centerIn: parent
        width: root.width * 0.12
        height: width
        radius: width / 2
        color: root.orbColor
        opacity: 0.6

        SequentialAnimation on opacity {
            loops: Animation.Infinite
            NumberAnimation { from: 0.4; to: 0.9; duration: root.breathSpeed * 0.7; easing.type: Easing.InOutSine }
            NumberAnimation { from: 0.9; to: 0.4; duration: root.breathSpeed * 0.7; easing.type: Easing.InOutSine }
        }

        Behavior on color { ColorAnimation { duration: 800 } }
    }
}
