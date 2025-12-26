import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Basic

Switch {
    id: root
    implicitWidth: 60
    implicitHeight: 30
    checked: false //false = Tag, true = Value
    property int radius: 8
    property color leftSelectedColor: "#894444"
    property color leftBackgroundColor: "#5b3232"
    property color rightSelectedColor: "#5a5a5a"
    property color rightBackgroundColor: "#3a3a3a"

    indicator: null

    background: Rectangle {
        radius: root.radius
        color: root.checked ? root.leftBackgroundColor : root.rightBackgroundColor
        //border.color: "#555555"
        Behavior on color {
            ColorAnimation {
                duration: 160;
                easing.type: Easing.OutQuad;
            }
        }

    }

    contentItem: Item {
        anchors.fill: parent

        Rectangle {
            id: thumb
            width: parent.width / 2
            height: parent.height
            radius: root.radius
            x: root.checked ? parent.width / 2 : 0
            color: root.checked ? root.rightSelectedColor : root.leftSelectedColor
            Behavior on x {
                NumberAnimation { duration: 160;
                easing.type: Easing.OutQuad;}
            }

            Behavior on color {
                ColorAnimation {
                    duration: 160;
                    easing.type: Easing.OutQuad;
                }
            }
        }

        RowLayout {
            id: iconLayout
            anchors.fill: parent
            spacing: 0
            property int iconSize: 15

            Image {
                id: tagIcon
                Layout.preferredHeight: iconLayout.iconSize
                Layout.preferredWidth: iconLayout.iconSize
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                fillMode: Image.PreserveAspectFit
                source: root.checked ? "qrc:/img/resources/tag_black.svg" : "qrc:/img/resources/tag_white.svg"
            }

            Image {
                id: valueIcon
                Layout.preferredHeight: iconLayout.iconSize
                Layout.preferredWidth: iconLayout.iconSize
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                fillMode: Image.PreserveAspectFit
                source: root.checked ? "qrc:/img/resources/value_white.svg" : "qrc:/img/resources/value_black.svg"
            }
        }
    }
}
