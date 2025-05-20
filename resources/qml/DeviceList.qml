import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    ListView {
        id: deviceListView
        anchors.fill: parent
        model: flashupGui.deviceList
        clip: true
        
        delegate: ItemDelegate {
            width: parent.width
            height: 60
            highlighted: flashupGui.selectedDevice === modelData
            
            onClicked: {
                flashupGui.selectedDevice = modelData;
            }
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 4
                
                Label {
                    text: modelData
                    font.bold: true
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
                
                Label {
                    property var info: flashupGui.getDeviceInfo(modelData)
                    text: info ? (info.type + (info.port ? ": " + info.port : info.ip ? ": " + info.ip : "")) : ""
                    font.pixelSize: 12
                    color: "#555555"
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
            }
        }
        
        ScrollBar.vertical: ScrollBar {}
        
        // Empty state
        Rectangle {
            anchors.fill: parent
            color: "#f5f5f5"
            visible: deviceListView.count === 0
            
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 10
                
                Label {
                    text: "No devices found"
                    font.pixelSize: 16
                    Layout.alignment: Qt.AlignHCenter
                }
                
                Button {
                    text: "Refresh"
                    Layout.alignment: Qt.AlignHCenter
                    onClicked: flashupGui.refreshDevices()
                }
            }
        }
    }
} 