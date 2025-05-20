import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    // Property to easily check if update is ready
    property bool canUpdate: flashupGui.selectedDevice !== "" && 
                           Object.keys(flashupGui.firmwareInfo).length > 0

    ColumnLayout {
        anchors.fill: parent
        spacing: 10
        
        // Update progress
        ProgressBar {
            id: progressBar
            Layout.fillWidth: true
            from: 0
            to: 100
            value: flashupGui.updateProgress
            
            background: Rectangle {
                implicitWidth: 200
                implicitHeight: 24
                color: "#e6e6e6"
                radius: 3
            }
            
            contentItem: Rectangle {
                width: progressBar.visualPosition * parent.width
                height: parent.height
                radius: 2
                color: {
                    if (flashupGui.updateProgress < 100) {
                        return "#2196F3"; // Blue during update
                    } else {
                        return "#4CAF50"; // Green when complete
                    }
                }
            }
        }
        
        // Status text
        Label {
            text: flashupGui.updateStatus
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 14
            wrapMode: Text.WordWrap
        }
        
        // Buttons
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            
            Item { Layout.fillWidth: true }
            
            Button {
                text: "Start Update"
                enabled: canUpdate && !flashupGui.updateActive
                
                background: Rectangle {
                    implicitWidth: 100
                    implicitHeight: 40
                    color: parent.enabled ? "#4CAF50" : "#cccccc"
                    radius: 4
                }
                
                contentItem: Text {
                    text: parent.text
                    color: parent.enabled ? "white" : "#888888"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                onClicked: {
                    if (flashupGui.startUpdate()) {
                        // Started successfully
                    }
                }
            }
            
            Button {
                text: "Cancel"
                enabled: flashupGui.updateActive
                
                background: Rectangle {
                    implicitWidth: 100
                    implicitHeight: 40
                    color: parent.enabled ? "#F44336" : "#cccccc" 
                    radius: 4
                }
                
                contentItem: Text {
                    text: parent.text
                    color: parent.enabled ? "white" : "#888888"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                onClicked: {
                    if (flashupGui.cancelUpdate()) {
                        // Canceled successfully
                    }
                }
            }
            
            Item { Layout.fillWidth: true }
        }
        
        // Requirements notice for when update can't start
        Label {
            visible: !canUpdate
            text: {
                if (flashupGui.selectedDevice === "") {
                    return "Please select a device to update";
                } else if (Object.keys(flashupGui.firmwareInfo).length === 0) {
                    return "Please load a firmware file";
                } else {
                    return "";
                }
            }
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            color: "#F44336"  // Red
            font.italic: true
        }
    }
} 