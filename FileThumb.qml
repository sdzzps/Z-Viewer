import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

Rectangle {
    id: root
    width: 120
    height: 80
    clip: true
    radius: 6
    /*
    color: mouseArea.pressed
                ? "#D8D8D8"
                : (mouseArea.containsMouse ? "#E5E5E5" : "#F2F2F2")*/
    color: "transparent"
        
    signal thumbClicked() //Signal to notify when the thumbnail is clicked
    signal revealFile() //Signal to reveal current file in its location

    //Content Properties
    default property alias content: contentItem.data //the second child is the content area
    property int itemIndex: -1 //default -1
    property string nameLabel: "" // Primary text, default blank
    property string typeLabel: "" //secondary text
    property url imageUrl: "" //QUrl for image

    //State Properties
    property bool hovered: false
    property bool selected: false

    ColumnLayout {
        id: contentItem
        anchors.fill: parent
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "transparent"
            Image {
                id: sourceImage
                source: root.imageUrl
                anchors.fill: parent
                fillMode: Image.PreserveAspectCrop
                visible: false //let mask render
            }
            Rectangle {
                id: maskShape
                anchors.fill: sourceImage
                radius: root.radius + 1 //hardcoded: +1 to fix rendering glitch
                color: "white"
                visible: false
                layer.enabled: true //allow as rendering texture
            }

            MultiEffect {
                anchors.fill: sourceImage
                source: sourceImage
                maskEnabled: true
                maskSource: maskShape
            }

        }
    }

    //extension mark
    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        topLeftRadius: parent.radius
        bottomRightRadius: parent.radius
        width: 40
        height: 20
        color: "#6f6f6f"
        opacity: 0.6
        clip:true
        Label {
            anchors.centerIn: parent
            text: root.typeLabel.toUpperCase()
            font.pointSize: 10 * FontScale * FontScale //hardcoded
            font.family: "Roboto"
            font.weight: 600
            color: "#ffffff"
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }

    }

    //name mark
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        topLeftRadius: parent.radius
        bottomRightRadius: parent.radius
        width: 70
        height: 20
        color: "#6f6f6f"
        opacity: 0.6
        clip: true
        Label {
            anchors.fill: parent
            text: root.nameLabel
            //font.family: "Source Han Sans SC Normal"
            font.pointSize: 10 * FontScale * FontScale //hardcoded
            color: "#ffffff"
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideLeft
        }
    }

    //selection mark
    Rectangle {
        //anchors.centerIn: parent
        //width: parent.width+2
        //height: parent.height+2
        anchors.fill: parent
        radius: parent.radius
        color: "transparent"
        //Edge Highlight when selected
        border.color: root.selected ? "#2175ff" : "transparent"
        border.width: root.selected ? 3 : 0
        Behavior on border.width {
            NumberAnimation { duration: 80; easing.type: Easing.InOutQuad }
        }
    }


    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onClicked: (mouse) => { //onClicked: pressed and release without movement
                       if (mouse.button === Qt.LeftButton) {
                           root.thumbClicked(); //send out index
                           //console.log("thumbnail clicked");//for debug purposes
                       }
        }

        onReleased: (mouse) => { //show menu after right button up
            if (mouse.button === Qt.RightButton) {
                menuLoader.active = true;
                menuLoader.item.popup(mouse.x, mouse.y);
                mouse.accepted = true;
            }
        }
    }
    //RAII: only load as needed
    Loader {
        id: menuLoader
        active: false//default not loaded
        asynchronous: false //sync load
        sourceComponent: menuComp
    }
    Component {
        id: menuComp
        //copy menu
        Menu {
            id: fileMenu
            implicitHeight: 40
            implicitWidth: 150
            background: Rectangle {
                radius: 4
                color: "transparent"
            }

            MenuItem {
                id: revealFile
                enabled: filePath.length > 0
                onTriggered: {
                    root.revealFile();
                    menuLoader.active = false;
                }
                background: Rectangle {
                    radius: 4
                    color: revealFile.hovered? "#BBBBBB":"#acacac"
                }
                contentItem: Rectangle {
                    color: "transparent"
                    implicitHeight: 20
                    implicitWidth: 150
                    Image {
                        id: icon
                        source: "qrc:/img/resources/folder_open.svg"
                        anchors.left: parent.left
                        anchors.margins: 2 //hardcoded
                        anchors.verticalCenter: parent.verticalCenter
                        fillMode: Image.PreserveAspectFit
                        width: 20
                        height: 20
                    }
                    Label {
                        text: "Open File Location"
                        color: "#1A1A1A"
                        font.pointSize: 10 * FontScale
                        anchors.left: icon.right
                        anchors.margins: 6 //hardcoded
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }
        }
    }
}
