import QtQuick
import QtQuick.Effects

Item {
    id: root
    width: 60
    height: 80

    property color fireColor: "#00e0ff"
    property bool isActive: false

    opacity: isActive ? 1.0 : 0.0
    Behavior on opacity { NumberAnimation { duration: 1000; easing.type: Easing.InOutQuad } }

    // Core flame
    Rectangle {
        id: core
        width: 20
        height: 24
        radius: 12
        color: "#ffffff"
        opacity: 0.9
        anchors.centerIn: parent

        SequentialAnimation on scale {
            loops: Animation.Infinite
            running: root.isActive
            NumberAnimation { to: 1.1; duration: 800; easing.type: Easing.InOutSine }
            NumberAnimation { to: 0.9; duration: 900; easing.type: Easing.InOutSine }
        }

        SequentialAnimation on anchors.verticalCenterOffset {
            loops: Animation.Infinite
            running: root.isActive
            NumberAnimation { from: 5; to: -5; duration: 1200; easing.type: Easing.InOutSine }
            NumberAnimation { to: 5; duration: 1400; easing.type: Easing.InOutSine }
        }
    }

    // Inner glow
    Rectangle {
        id: innerGlow
        width: 32
        height: 38
        radius: 18
        color: root.fireColor
        opacity: 0.7
        anchors.centerIn: parent

        SequentialAnimation on scale {
            loops: Animation.Infinite
            running: root.isActive
            NumberAnimation { to: 1.3; duration: 1100; easing.type: Easing.InOutQuad }
            NumberAnimation { to: 0.8; duration: 1300; easing.type: Easing.InOutQuad }
        }
        
        SequentialAnimation on anchors.verticalCenterOffset {
            loops: Animation.Infinite
            running: root.isActive
            NumberAnimation { from: 8; to: -8; duration: 1500; easing.type: Easing.InOutSine }
            NumberAnimation { to: 8; duration: 1700; easing.type: Easing.InOutSine }
        }
    }

    // Outer aura
    Rectangle {
        id: outerAura
        width: 44
        height: 54
        radius: 25
        color: root.fireColor
        opacity: 0.3
        anchors.centerIn: parent

        SequentialAnimation on scale {
            loops: Animation.Infinite
            running: root.isActive
            NumberAnimation { to: 1.5; duration: 1500; easing.type: Easing.InOutSine }
            NumberAnimation { to: 1.0; duration: 1800; easing.type: Easing.InOutSine }
        }
    }

    // Apply intense blur to blend them into a flame
    layer.enabled: true
    layer.effect: MultiEffect {
        blurEnabled: true
        blur: 0.6
        shadowEnabled: true
        shadowColor: root.fireColor
        shadowBlur: 1.0
        shadowOpacity: 0.6
    }
}
