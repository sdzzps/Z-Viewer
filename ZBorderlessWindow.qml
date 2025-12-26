import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

Window {
    id: root
    property color backgroundColor: "#525252"
    property int radius: 8
    property int borderThickness: 4
    property bool isMainWindow: true //if defined as main window, closing the window would quit program
    default property alias content: borderlessFrame.data
    width: 800
    height: 600
    minimumWidth: 400
    minimumHeight: 250
    color: "transparent"
    visible: true
    flags: Qt.FramelessWindowHint | Qt.Window | Qt.WindowSystemMenuHint |
           Qt.WindowMaximizeButtonHint | Qt.WindowMinimizeButtonHint

    //borderlessFrame: visible area
    Rectangle {
        id:borderlessFrame
        width: root.width
        height: root.height
        anchors.centerIn: parent
        color: root.backgroundColor
        radius: root.radius
        clip: true

        MouseArea {
            //drag top 1/3 part of window to move
            //leave margin of 2 for resize mouse area
            id: dragArea
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
            }
            anchors.margins: 2 //reserve for resize mouse area
            height: parent.height / 3 - 4
            acceptedButtons: Qt.LeftButton
            onPressed: {
                root.startSystemMove() //Qt 6: no need to pass mouse as parameter
            }
            onDoubleClicked: {//double click to change maximize status
                if (root.isMainWindow === false) return;
                if (root.visibility === Window.Maximized) {
                    root.showNormal()
                } else {
                    root.showMaximized()
                }
            }
        }

        //top right corner: close, maximize, and minimize buttons
        Row {
            id: windowButtons
            spacing: 0
            anchors.top: parent.top
            anchors.right: parent.right
            z: 2

            // minimize button
            Rectangle {
                id: minimizeButton
                visible: root.isMainWindow
                width: 40
                height: 30
                color: mouseArea1.pressed ? "#616161" : ( mouseArea1.containsMouse ? "#838080" : backgroundColor)
                Behavior on color { //do not use transparent color, will cause animation glitch
                       ColorAnimation {
                              duration: 100
                       }
                }

                Image {
                    id: minimizeIcon
                    width: 18
                    height: 18
                    anchors.centerIn: parent
                    source: "qrc:/img/resources/minimize.svg"
                }

                MouseArea {
                    id: mouseArea1
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked:
                    {
                           if (root.isMainWindow === false) return;
                           root.showMinimized();
                    }
                }

            }

            // maximize/reset button
            Rectangle {
                id: maximizeButton
                visible: root.isMainWindow
                width: 40
                height: 30
                color: mouseArea2.pressed ? "#616161" : ( mouseArea2.containsMouse ? "#838080" : backgroundColor)
                Behavior on color {
                       ColorAnimation {
                              duration: 100
                       }
                }
                Image {
                    id: maximizeIcon
                    anchors.centerIn: parent
                    width: 14
                    height: 14
                    source: root.visibility === Window.Maximized ? "qrc:/img/resources/stack.svg"
                                                                 : "qrc:/img/resources/box.svg"
                }
                MouseArea {
                    id: mouseArea2
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                           if (root.isMainWindow === false) return;
                           if (root.visibility === Window.Maximized){
                               root.showNormal()
                           } else {
                               root.showMaximized()
                           }
                     }
                }

            }

            // close button
            Rectangle {
                id: closeButton
                width: 40
                height: 30
                topRightRadius: 8 //frame does not clip corners
                color: mouseArea3.pressed
                       ? "#ab2d2d"
                       : ( mouseArea3.containsMouse ? "#ab0101" : backgroundColor)
                Behavior on color {
                       ColorAnimation {
                              duration: 100
                       }
                }
                Image {
                    id: closeIcon
                    width: 18
                    height: 18
                    anchors.centerIn: parent
                    source: "qrc:/img/resources/close.svg"
                }
                MouseArea {
                    id: mouseArea3
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                           if (root.isMainWindow === true) Qt.quit();
                           root.hide();
                    }
                }

            }
        }
    }

    //Implementation of resize by dragging corners/edges
    // top edge
   MouseArea {
       anchors {
           left: parent.left
           right: parent.right
           top: parent.top
       }
       height: borderThickness
       acceptedButtons: Qt.LeftButton
       cursorShape: Qt.SizeVerCursor
       onPressed: root.startSystemResize(Qt.TopEdge)
   }

   // bottom edge
   MouseArea {
       anchors {
           left: parent.left
           right: parent.right
           bottom: parent.bottom
       }
       height: borderThickness
       acceptedButtons: Qt.LeftButton
       cursorShape: Qt.SizeVerCursor
       onPressed: root.startSystemResize(Qt.BottomEdge)
   }

   // left edge
   MouseArea {
       anchors {
           left: parent.left
           top: parent.top
           bottom: parent.bottom
       }
       width: borderThickness
       acceptedButtons: Qt.LeftButton
       cursorShape: Qt.SizeHorCursor
       onPressed: root.startSystemResize(Qt.LeftEdge)
   }

   // right edge
   MouseArea {
       anchors {
           right: parent.right
           top: parent.top
           bottom: parent.bottom
       }
       width: borderThickness
       acceptedButtons: Qt.LeftButton
       cursorShape: Qt.SizeHorCursor
       onPressed: root.startSystemResize(Qt.RightEdge)
   }

   // top left corner
   MouseArea {
       width: borderThickness * 2
       height: borderThickness * 2
       anchors.left: parent.left
       anchors.top: parent.top
       acceptedButtons: Qt.LeftButton
       cursorShape: Qt.SizeFDiagCursor
       onPressed: root.startSystemResize(Qt.TopEdge | Qt.LeftEdge)
   }

   // top right corner
   MouseArea {
       width: borderThickness * 2
       height: borderThickness * 2
       anchors.right: parent.right
       anchors.top: parent.top
       acceptedButtons: Qt.LeftButton
       cursorShape: Qt.SizeBDiagCursor
       onPressed: root.startSystemResize(Qt.TopEdge | Qt.RightEdge)
   }

   // bottom left corner
   MouseArea {
       width: borderThickness * 2
       height: borderThickness * 2
       anchors.left: parent.left
       anchors.bottom: parent.bottom
       acceptedButtons: Qt.LeftButton
       cursorShape: Qt.SizeBDiagCursor
       onPressed: root.startSystemResize(Qt.BottomEdge | Qt.LeftEdge)
   }

   // bottom right corner
   MouseArea {
       width: borderThickness * 2
       height: borderThickness * 2
       anchors.right: parent.right
       anchors.bottom: parent.bottom
       acceptedButtons: Qt.LeftButton
       cursorShape: Qt.SizeFDiagCursor
       onPressed: root.startSystemResize(Qt.BottomEdge | Qt.RightEdge)
   }
}

