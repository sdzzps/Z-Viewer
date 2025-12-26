pragma ComponentBehavior: Bound //allow access outer layer from nested delegates for repeaters

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
	id: root
    implicitWidth: 200
    implicitHeight: 200
    color: "transparent"
    bottomLeftRadius: 8

    //content Property
    property var displayModel: null
    property int currentIndex: -1 //only one item under current View
    //consider enable selecting multiple items in the future
    
    //send out clicked signal
    signal selected(int selectedIndex) //define result as selectedIndex
    //send out reveal file
    signal revealFilePath(string tpPath)
    ListView {
        id: thumbList
        anchors.fill: parent
        spacing: 4
        model: root.displayModel //for testing, number of thumbnails to display
        ScrollBar.vertical: ScrollBar {}
        delegate: 
        FileThumb {
            id: thumbItem
            //add thumbnail picture in the future
                
            required property int fileIndex //index of this file
            required property string baseName
            required property string fileType
            required property url thumbUrl
            required property string filePath
            //set content
            itemIndex: fileIndex
            nameLabel: baseName
            typeLabel: fileType
            imageUrl: thumbUrl
            selected: (fileIndex === root.currentIndex) //displays selected status when matched with selected item

            //send index when left clicked
            onThumbClicked: {
                    root.selected(itemIndex);
                    console.log("clicked " + itemIndex);//for debug
            }
            //send file local path out when Open file location requested
            onRevealFile: {
                if (thumbItem.filePath){
                    root.revealFilePath(thumbItem.filePath);
                }
            }
        }
    }
}
