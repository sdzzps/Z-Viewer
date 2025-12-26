import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

//copyright sdzzps 2025.11
//a collapsible panel component
//developed for Z Viewer exiftool GUI application

Rectangle {
    id: root
    color: "transparent"
    
    //collapsedStatus property to control collapsed/expanded state
    property bool collapsedStatus: false 

    //customizable sizes
    property int titleBarHeight: 20

    //content
    property string title: "Collapsed Part Title"
    default property alias content: contentItem.data //the second child is the content area

    //Customize title bar
    property color expandedColor:  "#7B7B7B"
    property color foldedColor: "#646464"
    property color hoveredColor: "#9A9A9A"
    property color pressedColor: "#6E6E6E"
    property color textColor: "#1F1F1F"

    property color titleBarColor: collapsedStatus ? foldedColor : expandedColor

    signal collapseClicked()

    //default size, modify when instantiate
    //when used in layout, use Layout.preferred(Height&Width) to instantiate
    //when used as anchored, anchors.top to the bottom of the item above
    //width: 1000
    //implicitWidth: 1200 //do not set implicitWidth, may cause runtime rendering glitch
    //mandate height and clipping for foldability
    height: collapsedStatus ? titleBarHeight : (titleBarHeight + contentItem.implicitHeight + 4) //panelHeight, change with contentItem
    implicitHeight: height //crucial, otherwise implicitHeight remains unchanged
    clip: true //clip content when collapsed

    Behavior on height {
        NumberAnimation {
           duration: collapsedStatus ? 200 : Math.min(contentItem.height * 0.5, 800)
        }
    }

    //decide what to display on title bar
    //property string titleContent: root.collapsedStatus ? (root.title + " (folded)") : (root.title)
    property string titleContent: root.title //use icon to show folded status


    //title bar
    Rectangle {
        id: titleBar

        width: parent.width
        implicitHeight: root.titleBarHeight
        anchors.left: parent.left
        anchors.top: parent.top
        color: mouseArea.pressed
            ? root.pressedColor
            : (mouseArea.containsMouse ? root.hoveredColor : root.titleBarColor)

        radius: 4

        Label {
            id: titleLabel
            anchors.left: parent.left
            anchors.leftMargin: 12 //hardcoded to align with entry text below
            anchors.verticalCenter: parent.verticalCenter
            text: root.titleContent.toUpperCase()
            font.pointSize: 10 * FontScale * FontScale
            color: root.textColor
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            elide: Text.ElideRight //elliptical for lengthy title
            }

        Image {
            id: collapsedIcon
            visible: collapsedStatus
            anchors.left: titleLabel.right
            anchors.margins: 2
            anchors.verticalCenter: parent.verticalCenter
            fillMode: Image.PreserveAspectFit
            source: "qrc:/img/resources/bottomClosed.svg"
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true

            onClicked: {
                //root.collapsedStatus = !root.collapsedStatus
                root.collapseClicked();
            }
        }
    }

    ColumnLayout {
        id: contentItem
        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
        anchors.top: titleBar.bottom
        anchors.left: parent.left
        width: parent.width

        /*
        //Test items
        Rectangle {
            id: testRect
            width: parent.width
            height: 280
            color: "lightblue"

            Text {
                text: "This is the content inside the CollapsedPart."
                anchors.centerIn: parent
                font.pointSize: 14
            }
        }
        */
    }
}
