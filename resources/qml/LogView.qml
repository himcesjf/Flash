import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    ListView {
        id: logView
        anchors.fill: parent
        model: flashupGui.logModel
        clip: true
        verticalLayoutDirection: ListView.BottomToTop  // Newest logs at the bottom
        
        delegate: Rectangle {
            width: ListView.view.width
            height: logText.height + 12
            color: index % 2 === 0 ? "#f8f8f8" : "#ffffff"
            
            Rectangle {
                width: 4
                height: parent.height
                color: model.color
            }
            
            Text {
                id: logText
                anchors {
                    left: parent.left
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                    leftMargin: 12
                    rightMargin: 8
                }
                text: "[" + model.timestampStr + "] [" + model.levelStr + "] " + model.message
                wrapMode: Text.WordWrap
                font.family: "monospace"
                font.pixelSize: 12
                color: model.color
            }
        }
        
        ScrollBar.vertical: ScrollBar {}
        
        // Empty state
        Rectangle {
            anchors.fill: parent
            color: "#f5f5f5"
            visible: logView.count === 0
            
            Text {
                anchors.centerIn: parent
                text: "No log messages"
                font.pixelSize: 14
                color: "#888888"
            }
        }
    }
} 