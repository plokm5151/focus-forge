/**
 * @file BreathingOrb.qml
 * @brief Futuristic neon orb component driven by a GPU fragment shader.
 *
 * Renders a glowing cyan/purple orb that:
 * - Pulsates smoothly during Idle and CoolDown states
 * - Locks into a steady, intense glow during Focusing
 * - Transitions between modes with smooth easing
 *
 * All visual computation is GPU-side (zero CPU rendering load).
 * The only CPU cost is a lightweight NumberAnimation incrementing iTime.
 */
import QtQuick

Item {
    id: root

    // ── Public Interface (bound from parent / ViewModel) ──

    /** @brief Current timer state name from TimerViewModel ("Idle", "Focusing", "CoolDown"). */
    property string stateName: "Idle"

    /** @brief Remaining seconds from TimerViewModel (used for future progress mapping). */
    property int remainingSeconds: 0

    // ── Derived Animation Parameters ──

    /** @brief Pulse speed: 0 = steady (Focusing), 2.0 = breathing (Idle/CoolDown). */
    readonly property real effectivePulseSpeed: {
        switch (stateName) {
        case "Focusing": return 0.0;
        case "CoolDown": return 2.5;
        default:         return 1.8;  // Idle — gentle breathing
        }
    }

    /** @brief Glow intensity: brighter during Focusing, dimmer when Idle. */
    readonly property real effectiveGlowIntensity: {
        switch (stateName) {
        case "Focusing": return 1.0;
        case "CoolDown": return 0.75;
        default:         return 0.55;  // Idle — subdued
        }
    }

    // ── Time Accumulator (drives shader animation, zero CPU render cost) ──

    /** @brief Monotonically increasing time counter for shader input. */
    property real iTime: 0.0

    NumberAnimation on iTime {
        from: 0
        to: 36000        // 10-hour cycle before wrap (avoids precision issues)
        duration: 36000000  // 10 hours in ms — linear 1:1 second mapping
        loops: Animation.Infinite
        running: true
    }

    // ── Shader Rendering ──

    ShaderEffect {
        id: orbShader
        anchors.fill: parent

        // Uniforms passed to the fragment shader (names must match UBO members)
        property real iTime: root.iTime
        property real pulseSpeed: root.effectivePulseSpeed
        property real glowIntensity: root.effectiveGlowIntensity

        fragmentShader: "qrc:/shaders/shaders/breathing_orb.frag.qsb"

        // Smooth transitions when state changes
        Behavior on pulseSpeed {
            NumberAnimation { duration: 1000; easing.type: Easing.InOutQuad }
        }

        Behavior on glowIntensity {
            NumberAnimation { duration: 800; easing.type: Easing.InOutSine }
        }
    }
}
