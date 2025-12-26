import QtQuick
import QtQuick.Controls

//Edit Menu for a text field to enable right click copy/paste
Menu {
    id: root
    property var textField: null
    implicitHeight: 120
    implicitWidth: 80
    background: Rectangle {
        radius: 4
        color: "transparent"
    }

    MenuItem {
        id: copySelected
        onTriggered: {
            if (textField.selectedText.length > 0) {
                textField.copy();
                return;
            }
            textField.selectAll();
            textField.copy();
        }

        background: Rectangle {
            topLeftRadius: 4
            topRightRadius: 4
            color: copySelected.hovered? "#BBBBBB":"#acacac"
        }
        contentItem: Rectangle {
            color: "transparent"
            implicitHeight: 20
            implicitWidth: 80
            Image {
                id: icon
                source: "qrc:/img/resources/copy.svg"
                anchors.left: parent.left
                anchors.margins: 2 //hardcoded
                anchors.verticalCenter: parent.verticalCenter
                fillMode: Image.PreserveAspectFit
                width: 20
                height: 20
            }

            Label {
                text: "Copy"
                color: "#1A1A1A"
                font.pointSize: 10 * FontScale
                anchors.left: icon.right
                anchors.margins: 6 //hardcoded
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    MenuItem {
        id: paste
        onTriggered: {
            textField.paste();
        }

        background: Rectangle {
            bottomLeftRadius: 4
            bottomRightRadius: 4
            color: paste.hovered? "#BBBBBB":"#acacac"
        }
        contentItem: Rectangle {
            color: "transparent"
            implicitHeight: 20
            implicitWidth: 80
            Image {
                id: pasteIcon
                source: "qrc:/img/resources/paste.svg"
                anchors.left: parent.left
                anchors.margins: 2 //hardcoded
                anchors.verticalCenter: parent.verticalCenter
                fillMode: Image.PreserveAspectFit
                width: 20
                height: 20
            }
            Label {
                text: "Paste"
                color: "#1A1A1A"
                font.pointSize: 10 * FontScale
                anchors.left: pasteIcon.right
                anchors.margins: 6 //hardcoded
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }
}
