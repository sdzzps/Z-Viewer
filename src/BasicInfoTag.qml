import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Rectangle {
    id: root

    //pass in values and color
    property string val: ""
    property var source: null

    property bool sourceValid: !!source //false when null or undefine or 0

    visible: sourceValid && !!source[val]
    Layout.preferredWidth: label.implicitWidth + 12
    Layout.preferredHeight: label.implicitHeight + 4
    color: "#894444"//override when use
    radius: 4
    Label {
        id: label
        visible: parent.visible
        anchors.fill: parent
        anchors.leftMargin: 6
        anchors.topMargin: 2 //hardcoded
        text: visible ? root.source[root.val] : ""
        color: "#f0f0f0"
        //font.family: "Segoe UI"
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignLeft
        font.pointSize: 8 * FontScale * FontScale
    }
}
