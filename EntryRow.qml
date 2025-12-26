import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    //content
    property string typeText: ""
    property string infoText: ""
    //graphics
    property int entrySize: 40 //unified size variable, equals to row height
    property color typeColor: "#703a3a"//"#894444"
    property color typePressedColor: "#6a3838"//"#804040"
    property color infoColor: "#424242"
    property color textColor: "#c2c2c2"
    //property real backgroundOpacity: 0.6 //disabled
    property int spacing: 4 //spacing between edges and objects
    property int typeLength: 200
    //size
    implicitHeight: entrySize + spacing //outer size
    implicitWidth: 1200
    //let inner rectangles to decide size
    //implicitHeight: entryRow.implicitHeight + spacing
    //implicitWidth: entryRow.implicitWidth + spacing

    RowLayout {
        id: entryRow
        anchors.fill: parent
        anchors.margins: root.spacing * 0.5
        spacing: root.spacing

        Rectangle {
            id: typeRect
            property bool extended: false //allow click to show full length
            Layout.minimumWidth: root.typeLength
            Layout.preferredWidth: extended ? (typeText.implicitWidth + root.spacing*4) : root.typeLength
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignLeft
            color: typeClick.pressed ? root.typePressedColor : root.typeColor
            radius: root.spacing
            //opacity: root.backgroundOpacity
            clip: true

            Label {
                id: extendMark //show arrow when extension available
                visible: (typeText.implicitWidth+root.spacing*2)>root.typeLength ? (typeRect.extended ? false
                                                                                                      : true ) : false
                text: "▶"
                anchors.right: parent.right
                anchors.margins: root.spacing
                anchors.verticalCenter: parent.verticalCenter
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignRight
                color: root.textColor
                font.pointSize: root.entrySize * 0.2 * FontScale
            }

            MouseArea {
                id: typeClick
                anchors.fill: parent
                onClicked: {
                    typeRect.extended = !typeRect.extended;
                }
            }

            TextEdit {
                id: typeText
                anchors.left: parent.left
                anchors.margins: root.spacing*2 + 2 //hardcoded
                anchors.verticalCenter: parent.verticalCenter
                width: (implicitWidth+root.spacing*2)>root.typeLength ? ( typeRect.extended ? implicitWidth
                                                                           : root.typeLength-root.spacing*8 )
                                                                      : implicitWidth //when overlength, truncate
                clip: true
                text: root.typeText
                readOnly: true
                selectByMouse: true
                verticalAlignment: TextEdit.AlignVCenter
                color: root.textColor
                selectionColor: "#525252"
                font.pointSize: root.entrySize * 0.4 * FontScale
            }
        }

        Rectangle {
            id: infoRect
            Layout.alignment: Qt.AlignLeft
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: root.infoColor
            radius: root.spacing
            //opacity: root.backgroundOpacity
            clip: true
            TextEdit {
                id: infoText
                anchors.left: parent.left
                anchors.margins: root.spacing*2 +2
                anchors.verticalCenter: parent.verticalCenter

                text: root.infoText
                readOnly: true
                selectByMouse: true
                verticalAlignment: TextEdit.AlignVCenter
                color: root.textColor
                selectionColor: "#525252"
                font.pointSize: root.entrySize * 0.4 * FontScale
            }
        }
    }

    //right click mouse area
    MouseArea {
        anchors.fill: entryRow
        acceptedButtons: Qt.RightButton
        propagateComposedEvents: true

        onReleased: (mouse) => {
            if (mouse.button !== Qt.RightButton) return;
            menuLoader.active = true; //load menu component
            menuLoader.item.popup(mouse.x,mouse.y);
            mouse.accepted = true;
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
            id: copyMenu
            implicitHeight: 40
            implicitWidth: 80
            background: Rectangle {
                radius: 4
                color: "transparent"
            }

            MenuItem {
                id: copySelected
                onTriggered: {
                    if (typeText.selectedText.length > 0) {
                        typeText.copy();
                        menuLoader.active=false;
                        return;
                    }
                    if (infoText.selectedText.length > 0) {
                        infoText.copy();
                        menuLoader.active=false;
                        return;
                    }
                    infoText.selectAll();
                    infoText.copy();
                    menuLoader.active=false;
                }

                background: Rectangle {
                    radius: 4
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
        }
    }//end of optional component
}
