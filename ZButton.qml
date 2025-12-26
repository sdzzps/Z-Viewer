import QtQuick
import QtQuick.Controls

Button {
    id: root
    text: "Button"
    //margin around the button
    property int radius: 6
    property color defaultColor: "#424242"
    property color hoveredColor: "#3e3e3e"
    property color pressedColor: "#404040"
    property color textColor: "#f0f0f0"

    background: Rectangle{
        radius: root.radius
        color: root.pressed ? root.pressedColor :
                                      root.hovered
                                      ? root.hoveredColor
                                      : root.defaultColor
    }

    contentItem: Text{
        text: root.text
        font.pointSize: 10 * FontScale
        color: root.textColor
        //font.family: "Segoe UI"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
