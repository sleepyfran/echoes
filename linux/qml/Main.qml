import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: root

    title: i18nc("@title:window", "Echoes")
    width: 480
    height: 720
    visible: true

    pageStack.initialPage: Kirigami.Page {
        title: i18nc("@title", "Echoes")

        ColumnLayout {
            anchors.centerIn: parent
            spacing: Kirigami.Units.smallSpacing

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: Kirigami.Units.smallSpacing

                Kirigami.Icon {
                    Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                    Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                    source: "audio-waveform"
                }

                Controls.Label {
                    text: i18nc("@label", "Welcome to Echoes")
                }
            }

            Controls.Button {
                Layout.alignment: Qt.AlignHCenter
                text: i18nc("@action:button", "Connect OneDrive")
                enabled: !authBridge.isAuthenticating
                onClicked: authBridge.startAuthentication()
            }

            Controls.Label {
                Layout.alignment: Qt.AlignHCenter
                Layout.maximumWidth: parent.width
                horizontalAlignment: Text.AlignHCenter
                text: i18nc("@info", "If your browser doesn't open, copy this link and open it manually:")
                visible: authBridge.isAuthenticating && authBridge.authStartUrl.length > 0
                wrapMode: Text.WordWrap
            }

            Controls.Label {
                id: authenticationUrl

                Layout.alignment: Qt.AlignHCenter
                Layout.maximumWidth: parent.width
                color: Kirigami.Theme.linkColor
                font.underline: true
                horizontalAlignment: Text.AlignHCenter
                text: authBridge.authStartUrl
                visible: authBridge.isAuthenticating && text.length > 0
                wrapMode: Text.WrapAnywhere

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: authBridge.copyAuthenticationUrl()
                }
            }

            Controls.Label {
                Layout.alignment: Qt.AlignHCenter
                Layout.maximumWidth: parent.width
                color: Kirigami.Theme.negativeTextColor
                horizontalAlignment: Text.AlignHCenter
                text: authBridge.authErrorMessage
                visible: text.length > 0
                wrapMode: Text.WordWrap
            }
        }
    }
}
