import QtQuick
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: root

    title: i18nc("@title:window", "Echoes")
    width: 480
    height: 720
    visible: true

    pageStack.initialPage: Kirigami.Page {
        title: i18nc("@title", "Echoes")

        Controls.Label {
            anchors.centerIn: parent
            text: coreGreeting
        }
    }
}
