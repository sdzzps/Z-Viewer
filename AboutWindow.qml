import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ZBorderlessWindow {
    id: root
    width: 300
    height: 400
    backgroundColor: "#606060"
    minimumWidth: width
    minimumHeight: height
    maximumHeight: height
    maximumWidth: width
    visible: false
    property var backend: null
    isMainWindow: false

    ColumnLayout {
        anchors.fill: parent
        Image {
            id: logo
            source: "qrc:/img/resources/ZViewerIcon1_2.png"
            Layout.margins: 20
            Layout.preferredHeight: 150
            Layout.preferredWidth: 150
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            fillMode: Image.PreserveAspectFit

        }
        Label {
            id: titleLabel
            Layout.preferredHeight: 40
            Layout.preferredWidth: 150
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            text: "Z VIEWER"
            font.family: "Roboto"
            font.pointSize: 20 * FontScale * FontScale
            font.weight: 900
            font.bold: true
            color: "#dedede"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignTop
        }
        Text {
            font.family: "Roboto"
            Layout.preferredHeight: 20
            Layout.preferredWidth: 225
            color: "#dedede"
            font.pointSize: 12 * FontScale * FontScale
            font.weight: 200
            text: "Developed by sdzzps"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        }
        Text {
            font.family: "Roboto"
            Layout.preferredHeight: 20
            Layout.preferredWidth: 225
            color: "#dedede"
            font.pointSize: 12 * FontScale * FontScale
            font.weight: 200
            text: "Based on ExifTool by Phil Harvey"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        }
        Text {
            font.family: "Roboto"
            Layout.preferredHeight: 20
            Layout.preferredWidth: 225
            color: "#dedede"
            font.pointSize: 12 * FontScale * FontScale
            font.weight: 200
            text: "v1.0.0   2025.12.27"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        }
        ZButtonIcon {
            id: clearAndQuit
            defaultColor: "#6a6a6a"
            hoveredColor: "#ab3030"
            pressedColor: "#ab4545"
            implicitHeight: 30
            implicitWidth: 170
            Layout.margins: 20
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            //margin around the button
            contentItem: Rectangle {
                color: "transparent"
                implicitHeight: 40
                implicitWidth: 120
                Image {
                    id: icon
                    source: "qrc:/img/resources/clear_cache.svg"
                    anchors.left: parent.left
                    anchors.margins: 2 //hardcoded
                    anchors.verticalCenter: parent.verticalCenter
                    fillMode: Image.PreserveAspectFit
                    width: 20
                    height: 20
                }

                Label {
                    text: "Clear cache and quit"
                    color: "#1A1A1A"
                    font.pointSize: 10 * FontScale * FontScale
                    anchors.left: icon.right
                    anchors.margins: 6 //hardcoded
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            onClicked: {
                backend.clearCacheFolder();
                Qt.quit();
            }
        }

    }
}
