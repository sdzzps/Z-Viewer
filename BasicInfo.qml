import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQml.Models

Rectangle {
    id: root
    color: "transparent"
    clip: true
    property var source: null
    property int tagRadius: 4

    property bool sourceValid: !!source //false when null or undefine or 0

    Rectangle {
        id: fileNameTag
        visible: root.sourceValid && !!root.source["fileName"]
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 10
        anchors.topMargin: 5 //hardcoded
        color: "#6f6f6f"
        implicitWidth: fileNameLabel.implicitWidth + 16
        implicitHeight: 24 //fileNameLabel.implicitHeight + 8
        radius: root.tagRadius
        Label {
            id: fileNameLabel
            visible: fileNameTag.visible
            anchors.centerIn: parent
            //anchors.margins: 8
            text: visible ? root.source["fileName"] : ""
            color: "#f0f0f0"
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            font.pointSize: 10 * FontScale * FontScale
        }
    }

    RowLayout {
        id: firstRow //displays Aperture, Shutter, ISO etc, red tags
        visible: root.sourceValid
        anchors.left: parent.left
        anchors.top: fileNameTag.bottom
        anchors.leftMargin: 10
        anchors.topMargin: 4
        spacing: 4

        property color tagColor: "#894444"
        property color textColor: "#f0f0f0"

        BasicInfoTag {
            color: firstRow.tagColor
            source: root.source
            val: "aperture"
        }

        BasicInfoTag {
            color: firstRow.tagColor
            source: root.source
            val: "shutterSpeed"
        }

        BasicInfoTag {
            color: firstRow.tagColor
            source: root.source
            val: "iso"
        }

        BasicInfoTag {
            color: firstRow.tagColor
            source: root.source
            val: "focalLength"
        }

        BasicInfoTag {
            color: firstRow.tagColor
            source: root.source
            val: "frameRate"
        }

    }

    RowLayout {
        id: secondRow //displays FileInfo, dimension, video info etc
        visible: root.sourceValid
        anchors.left: parent.left
        anchors.top: firstRow.bottom
        anchors.leftMargin: 10
        anchors.topMargin: 4
        spacing: 4
        property color tagColor: "#6f6f6f"
        property color textColor: "#f0f0f0"

        BasicInfoTag {
            color: parent.tagColor
            source: root.source
            val: "imageSize"
        }


        BasicInfoTag {
            color: parent.tagColor
            source: root.source
            val: "duration"
        }

        BasicInfoTag {
            color: parent.tagColor
            source: root.source
            val: "fileSize"
        }

        BasicInfoTag {
            color: parent.tagColor
            source: root.source
            val: "camera"
        }
        BasicInfoTag {
            color: parent.tagColor
            source: root.source
            val: "dateTaken"
        }
    }



}
