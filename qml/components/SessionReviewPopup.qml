/**
 * @file SessionReviewPopup.qml
 * @brief Non-intrusive session review popup that slides from the top.
 *
 * Appears when a focus session completes (enters Overtime/CoolDown).
 * The cooldown timer starts immediately — this popup does NOT block rest.
 * User can fill in their review at their leisure, or skip it entirely.
 */
import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Layouts

Popup {
    id: popup
    x: (parent ? (parent.width - width) / 2 : 0)
    y: 60
    width: 420
    height: reviewInput.implicitHeight + 100
    modal: false   // Non-blocking! Cooldown runs underneath.
    focus: false    // Don't steal focus from other elements
    closePolicy: Popup.CloseOnEscape

    enter: Transition {
        NumberAnimation { property: "y"; from: -height; to: 60; duration: 500; easing.type: Easing.OutBack }
        NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 400 }
    }

    exit: Transition {
        NumberAnimation { property: "y"; from: 60; to: -height; duration: 400; easing.type: Easing.InQuad }
        NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 300 }
    }

    background: Rectangle {
        color: "#1a1a30"
        opacity: 0.92
        radius: 16
        border.color: "#a855f744"
        border.width: 1

        layer.enabled: true
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowBlur: 2.0
            shadowColor: "#a855f7"
            shadowOpacity: 0.2
        }
    }

    contentItem: ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // Title
        Text {
            text: qsTr("做得好！🎉")
            font.pixelSize: 18
            font.weight: Font.Bold
            font.family: "Inter, Segoe UI, sans-serif"
            color: "#ffffff"
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: qsTr("這段專注時間內，您完成了什麼？")
            font.pixelSize: 13
            font.family: "Inter, Segoe UI, sans-serif"
            color: "#8888bb"
            Layout.alignment: Qt.AlignHCenter
        }

        // Input
        ScrollView {
            Layout.fillWidth: true
            Layout.preferredHeight: Math.min(reviewInput.implicitHeight + 8, 100)

            TextArea {
                id: reviewInput
                placeholderText: qsTr("寫下您的反思... (Enter 提交, Esc 略過)")
                font.pixelSize: 15
                font.family: "Inter, Segoe UI, sans-serif"
                color: "#ffffff"
                wrapMode: TextEdit.Wrap
                
                background: Rectangle {
                    color: "#ffffff0a"
                    radius: 8
                    border.color: "#ffffff1a"
                    border.width: 1
                }

                Keys.onReturnPressed: (event) => {
                    if (reviewInput.text.trim() !== "") {
                        timerViewModel.submitSessionReview(reviewInput.text.trim())
                        reviewInput.text = ""
                        popup.close()
                        event.accepted = true
                    }
                }
            }
        }

        // Bottom buttons
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 16

            // Submit
            Button {
                text: "📝 提交"
                font.pixelSize: 13

                contentItem: Text {
                    text: parent.text
                    color: "#ffffff"
                    font.pixelSize: 13
                    font.family: "Inter, Segoe UI, sans-serif"
                    horizontalAlignment: Text.AlignHCenter
                }

                background: Rectangle {
                    implicitWidth: 100
                    implicitHeight: 32
                    radius: 8
                    color: parent.pressed ? "#6d28d9" : (parent.hovered ? "#7c3aed" : "#5b21b6")
                    Behavior on color { ColorAnimation { duration: 150 } }
                }

                onClicked: {
                    let txt = reviewInput.text.trim()
                    if (txt !== "") {
                        timerViewModel.submitSessionReview(txt)
                    } else {
                        timerViewModel.submitSessionReview("")
                    }
                    reviewInput.text = ""
                    popup.close()
                }
            }

            // Skip
            Button {
                text: "略過"
                font.pixelSize: 13

                contentItem: Text {
                    text: parent.text
                    color: "#666688"
                    font.pixelSize: 13
                    font.family: "Inter, Segoe UI, sans-serif"
                    horizontalAlignment: Text.AlignHCenter
                }

                background: Rectangle {
                    implicitWidth: 80
                    implicitHeight: 32
                    radius: 8
                    color: "transparent"
                    border.color: "#ffffff1a"
                    border.width: 1
                }

                onClicked: {
                    timerViewModel.submitSessionReview("")
                    reviewInput.text = ""
                    popup.close()
                }
            }
        }
    }

    onOpened: {
        reviewInput.forceActiveFocus()
    }
}
