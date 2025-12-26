pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQml
import CppComm 
//in VS editor CppComm may false alarm as bug
//import QtQuick.Controls.Material

ZBorderlessWindow {
    id: mainWindow
    width: 800
    height: 600
    //title: "Z Viewer"
    //color: "grey"

    /*
    //designate theme
    Material.theme: Material.Dark
    Material.accent: Material.Blue
    */

    //Private Properties and Logics
    Item {
        id: priv //define private variables here
        property string text1: "Waiting for signals. ";//for testing purposes
        property int globalTextSize: 15 //point size of info text
    }

    //Initialize Backend Class to communicate with C++ backend and exiftool
    Backend {
    id: exiftool //connect with C++ backend
        onMySignal: {
            priv.text1 = "Received signal from exiftool. "//for testing purposes
        }
        onCurrentIndexChanged: {
            infoPanel.searchVisible = false;
        }
    }

    DropArea {
        anchors.fill: parent

        onDropped: (drop) => {
            exiftool.importFiles(drop.urls)
        }
    }

    FileDialog {
        id: openDialog
        title: "Choose media files"
        fileMode: FileDialog.OpenFiles //enable multiple files
        nameFilters: ["All files (*)",
            "Image files (*.jpg *.png *.nef *.cr2 *.cr3 *.dng *.arw *.heic *.tif *.tiff)",
            "Video files (*.mov *.mp4 *.nev *.crw *m4v *.crm *.mpg *.mpeg *.avi *.mkv)"]
        onAccepted:{
            //infoLogic.filePath = selectedFile //for single file testing purposes
            //console.log(infoLogic.filePath);//for debug purposes
            exiftool.importFiles(selectedFiles);
        }
    }

    //Thumbnail Area
    Rectangle { //thumbnail area
        id: thumbnailArea
        width: 128
        anchors.left: parent.left
        anchors.top: topArea.bottom
        anchors.bottom: parent.bottom
        bottomLeftRadius: 8
        color: "#6f6f6f"
        clip: true

        ThumbPanel {
            id:thumbPanel
            anchors.fill: parent
            anchors.margins: 4
            displayModel: exiftool.fileListModel //In VS editor this may false alarm as bug
            currentIndex: exiftool.currentIndex
            //set index when new item selected
            onSelected: function(selectedIndex){ //pass selectedIndex to function
                exiftool.setCurrentIndex(selectedIndex); //send selectedIndex to backend
                console.log(selectedIndex + "sent");//for debug purposes
            }
            onRevealFilePath:  function(tpPath){//pass tpPath to function
                exiftool.revealInFileManager(tpPath); //Invokable method of Backend class
            }
        }
    }

    //Top Panel, needs to cover thumbnail
    Rectangle {
        id: topArea
        anchors.top: parent.top
        anchors.left: parent.left
        width: parent.width - 128
        height: 50
        topLeftRadius: 8
        color: "transparent"
        clip: true

        Rectangle {
            id: zViewerTitle
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 128
            topLeftRadius: 8
            color: aboutButton.containsMouse ? "#5f5f5f" : "transparent"
            Label {
                id: titleLabel
                anchors.fill: parent
                text: "Z VIEWER"
                font.family: "Roboto"
                font.pointSize: 16*FontScale
                font.weight: 900
                color: "#dedede"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            MouseArea {
                id: aboutButton
                anchors.fill: parent
                onClicked: {
                    aboutWindow.visible = true;
                }
            }
            AboutWindow {
                id: aboutWindow
                backend: exiftool
            }
        }

        Item {
            id: searchPanel
            anchors.left: zViewerTitle.right
            anchors.right: parent.right
            height: parent.height
            RowLayout {
                anchors.fill: parent
                spacing: 0

                Rectangle {
                    id: searchBar
                    Layout.preferredHeight: 30
                    color: "#424242"
                    radius: 8
                    Layout.fillWidth: true
                    Layout.maximumWidth: 300
                    Layout.margins: 10
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    ZSwitch {
                        id: searchFieldSwitch
                        anchors.left: parent.left
                    }
                    TextField {
                        id: searchText
                        height: 30
                        anchors.left: searchFieldSwitch.right
                        anchors.right: searchButton.left
                        //anchors.margins: 10
                        anchors.verticalCenter: parent.verticalCenter
                        clip: true
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        selectionColor: "#525252"
                        //font.family: "Source Han Sans SC Normal"
                        font.pointSize: 10 * FontScale
                        color: "#f0f0f0"
                        background: null

                        onAccepted: searchBar.search()
                        //capture right click
                        MouseArea {
                            anchors.fill: parent
                            acceptedButtons: Qt.RightButton
                            propagateComposedEvents: true
                            onReleased: (mouse) => {
                                if (mouse.button === Qt.RightButton) {
                                    editMenu.popup(mouse.x, mouse.y)
                                    mouse.accepted = true
                                }
                            }
                        }
                        EditMenu {
                            id: editMenu
                            textField: searchText
                        }
                    }

                    ZButtonIcon {
                        id: searchButton
                        implicitHeight: 30
                        implicitWidth: 30
                        radius:8
                        defaultColor: "#6a6a6a"
                        hoveredColor: "#7a7a7a"
                        pressedColor: "#737373"
                        anchors.right: parent.right
                        contentItem: Image {
                            anchors.centerIn: parent
                            fillMode: Image.PreserveAspectFit
                            source: "qrc:/img/resources/search.svg"
                        }
                        onClicked: searchBar.search()
                    }

                    function search() { //send search query to exiftool backend
                        const keyword = searchText.text.trim();
                        if (keyword.length === 0) {
                                return;
                        }
                        exiftool.searchKeyword = keyword;
                        exiftool.searchField = searchFieldSwitch.checked
                                    ? Backend.SearchField.Value
                                    : Backend.SearchField.Tag;
                        exiftool.applySearchToProxy();
                        infoPanel.searchText = searchText.text;
                        infoPanel.searchVisible = true; //show search result
                    }
                }
            }
        }
    }

    ColumnLayout { //right area: info area and control area
        spacing: 2
        anchors.left: thumbnailArea.right
        anchors.right: parent.right
        anchors.top: topArea.bottom
        anchors.bottom: parent.bottom


        Rectangle { //info area
            color: "#303030"
            Layout.fillWidth: true //occupy all available width
            Layout.fillHeight: true //occupy all available height
            clip: true

            /*
            Item {
                //file loading logics
                //loading filePath and GroupName array for infoPanel
                //for single file view testing purposes
                //consider implement multi files loading
                id: infoLogic

                property string filePath: ""
                //property var groupNames: [] // all group names of current files

                //run exiftool pipeline when filePath changes
                onFilePathChanged: {
                    if (filePath !== "") {
                        exiftool.loadExifFromFile(filePath)
                        //console.log(filePath)//for debug
                    }
                }
            }
            */

            //set info panel to display exif data
            InfoPanel {
                id: infoPanel
                anchors.fill: parent
                anchors.topMargin: 4
                anchors.rightMargin: 4
                anchors.leftMargin: 4
                //groupNames: infoLogic.groupNames //bind group names to infopanel
                currentModel: exiftool.exifGroupsModel //bind current model, in VS editor this may false alarm as bug
                searchResult: exiftool.exifProxyModel //bind search result proxy model
                textSize: priv.globalTextSize

                //send toggled signal
                onPanelFolded: function(iGroupIndex){
                    exiftool.toggleFoldStatus(thumbPanel.currentIndex ,iGroupIndex);
                    console.log(iGroupIndex + "sent");
                }
            }
        }

        Rectangle {
            id: bottomPanel
            height: 80
            Layout.fillWidth: true
            color: "#525252"
            bottomRightRadius: 8

            BasicInfo {
                id: basicInfo
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: controlArea.left
                source: exiftool.basicInfo //pass in QVariantMap of basic info
            }

            Rectangle { //control area
                id: controlArea
                color: "transparent"
                anchors.right: parent.right //put right anchor to the right of the rectangle
                anchors.verticalCenter: parent.verticalCenter
                implicitWidth: 140

                ZButtonIcon {
                    id: importButton
                    defaultColor: "#6a6a6a"
                    hoveredColor: "#7a7a7a"
                    pressedColor: "#737373"
                    implicitHeight: 40
                    implicitWidth: 40
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.rightMargin: 20
                    //margin around the button
                    contentItem: Image {
                        anchors.centerIn: parent
                        fillMode: Image.PreserveAspectFit
                        source: "qrc:/img/resources/import.svg"
                    }

                    onClicked: {
                        openDialog.open()
                    }
                }
                /*
                ZButton {
                    id: copyButton
                    text: "Copy"
                    defaultColor: "#6a6a6a"
                    hoveredColor: "#7a7a7a"
                    pressedColor: "#737373"
                    implicitHeight: 40
                    implicitWidth: 40
                    anchors.right: importButton.left
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.rightMargin: 20
                    onClicked: {
                       exiftool.myFunction();//for testing purposes
                       console.log("Copy button clicked")
                   }
                }
                */
            }
        }
    }
}
