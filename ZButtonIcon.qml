import QtQuick
import QtQuick.Controls

Button {
    id: root
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

}
