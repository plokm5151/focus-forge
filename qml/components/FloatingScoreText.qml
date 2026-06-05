import QtQuick

Text {
    id: root
    font.pixelSize: 24
    font.weight: Font.Bold
    font.family: "JetBrains Mono, SF Mono, Consolas, monospace"
    opacity: 0.0

    SequentialAnimation {
        id: anim
        running: true
        
        ParallelAnimation {
            NumberAnimation { target: root; property: "y"; by: -80; duration: 1500; easing.type: Easing.OutQuart }
            SequentialAnimation {
                NumberAnimation { target: root; property: "opacity"; to: 1.0; duration: 200 }
                PauseAnimation { duration: 800 }
                NumberAnimation { target: root; property: "opacity"; to: 0.0; duration: 500 }
            }
        }
        
        ScriptAction { script: root.destroy() }
    }
}
