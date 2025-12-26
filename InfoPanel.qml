pragma ComponentBehavior: Bound //allow access outer layer from nested delegates for repeaters

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQml.Models   // DelegateModel

Rectangle {
    id: root
    //implicitWidth: allGroupsListView.implicitWidth
    //implicitHeight: allGroupsListView.implicitHeight
    //width: parent.width
    color: "#303030"

    property int uiSize: 30 //linked to entry size

    //content properties
    property var currentModel: null //current model being displayed

    property var searchResult: null //proxy model of search results

    property string searchText: ""

    //UI properties
    property int textSize: 15 //allow setting text size of info, default value 15

    //search result display status
    property bool searchVisible: false

    signal panelFolded(int iGroupIndex)

    //scroll to top when search results are out
    onSearchVisibleChanged: {
        if (searchVisible) {
            Qt.callLater(()=>{
                groupsScroll.contentY=0;
            })
        }
    }

    Flickable {
        id:groupsScroll
        anchors.fill: parent
        clip: true

        contentWidth: width
        contentHeight: groupsRepeater.contentHeight

        ScrollBar.vertical: ScrollBar { }
        onContentYChanged: Qt.callLater(update) //avoid glitch

        //For RAII loading
        property real viewTop: contentY
        property real viewBottom: contentY + height

        ListView {
            id: groupsRepeater
            model: root.currentModel
            width: root.width
            implicitHeight: contentHeight
            spacing: 2 //listview only
            //header of search results
            headerPositioning: ListView.InlineHeader
            header: CollapsedPart {
                visible: root.searchVisible
                width: root.width
                title: "SEARCH RESULTS OF: " + root.searchText
                height: visible ? (collapsedStatus ? titleBarHeight + 2 : (titleBarHeight + searchResultList.height + 2)) :
                                  0 //when not visible, disable content height
                onCollapseClicked: {collapsedStatus = !collapsedStatus}
                ListView {
                    id: searchResultList
                    width: parent.width
                    implicitHeight: contentItem.height //do not use explicit height
                    boundsBehavior: Flickable.StopAtBounds

                    model: root.searchResult

                    //delegates for entries
                    delegate: EntryRow {
                        required property string tag
                        required property string value
                        width: root.width
                        entrySize: root.uiSize

                        typeText: tag
                        infoText: value
                    }
                }
            }
            //info by groups
            delegate: CollapsedPart {
                id: groupPanel
                //do not use parent.width for item in delegate, parent may not resolve
                implicitWidth: root.width //do not use explicit width, rendering glitch

                required property string groupName
                required property var entriesModel
                required property bool folded
                required property int groupLength
                required property int groupIndex

                height: collapsedStatus ? titleBarHeight : (titleBarHeight + groupLength * (root.uiSize + 4))
                //set title:
                title: groupName
                collapsedStatus: folded

                //send folded signal
                onCollapseClicked: {
                    root.panelFolded(groupIndex);
                    console.log("toggled " + groupIndex);//for debug
                }

                //RAII loading when in viewport to conserve memory
                //add contentItem.y (delegtate position) in case of header exists.
                readonly property real yInFlick: groupsRepeater.y + groupsRepeater.contentItem.y + y //groupPanel Y in GroupsScroll coordinate
                readonly property real flickTop: groupsScroll.contentY //  viewport top of groupsScroll
                readonly property real flickBottom: groupsScroll.contentY + groupsScroll.height // viewport bottom of groupsScroll

                readonly property bool visibleInViewport: (!collapsedStatus) &&
                                                         ((yInFlick + height) > flickTop) &&
                                                         (yInFlick < flickBottom)
                //set ListView for in-group info:
                Loader {
                    id: entriesLoader
                    active: groupPanel.visibleInViewport //load listview only when not folded
                    asynchronous: true //optional
                    sourceComponent: entryListComp
                }
                Component {
                    id: entryListComp

                    ListView {
                        id: entryListView
                        height: groupPanel.groupLength * (root.uiSize + 4) //for UI acceleration
                        width: parent.width
                        reuseItems: true //reuse delegate items to save memory

                        //clip: true //important for group entries filtering
                        boundsBehavior: Flickable.StopAtBounds

                        model: groupPanel.entriesModel

                        property string currentGroup: groupPanel.title

                        //delegates for entries
                        //RAII loading
                        delegate: Item {
                            id: rowItem
                            required property string tag
                            required property string value
                            width: root.width
                            height: root.uiSize + 4

                            //RAII calculation
                            readonly property real flickTop: groupPanel.flickTop
                            readonly property real flickBottom: groupPanel.flickBottom
                            readonly property real yInFlick: groupPanel.yInFlick + y
                            readonly property real yBuffer: 300 //up and down buffer //covers titlebar height

                            readonly property bool visibleInViewport:groupPanel.visibleInViewport &&
                                                                     ((yInFlick + height + yBuffer) > flickTop) &&
                                                                     ((yInFlick - yBuffer) < flickBottom)
                            //loader of entry row component
                            Loader {
                                id: rowLoader
                                visible: rowItem.visibleInViewport
                                asynchronous: true
                                sourceComponent: rowComp
                            }
                            Component {
                                id: rowComp
                                EntryRow {
                                    width: root.width
                                    entrySize: root.uiSize
                                    /*
                                    //group entries filtering: only display within-group items
                                    property bool matchGroup: (group === entryListView.currentGroup)
                                    visible: groupPanel.collapsedStatus ? false : matchGroup
                                    height: matchGroup ? implicitHeight : 0
                                    */
                                    typeText: rowItem.tag
                                    infoText: rowItem.value
                                }
                            }
                        }
                    }
                }//end of component
            }
        }
    }
    //items below are for testing purposes

    /*
    CollapsedPart {
        //id: secondPart
        Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
        //title: "3rd List"
        //anchors.top:firstPart.bottom

        Rectangle {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop // crucial, otherwise position wrong
            Layout.preferredWidth: parent.width//crucial
            implicitHeight: text2.implicitHeight + 10 //crucial
            color: "orange"

            Text {
                id:text2
                text: "This is the content inside the third CollapsedPart." +
                "For most of the time you dont need long text. "+
                "but when the text is long enought. " +
                "cropping and wrapping mechanism is crucial. "+
                "Lorem ipsum dolor sit amet, consectetur adipiscing elit. " +
                "Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. " +
                "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. " +
                "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. " +
                "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
                anchors.left: parent.left
                font.pointSize: 14
                wrapMode: Text.WordWrap
                width: parent.width - 20 //leave some padding
            }
        }
    }
    */ //items above are for testing purposes
}
