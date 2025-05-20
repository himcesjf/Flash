import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    // Property to easily check if firmware is loaded
    property bool hasFirmware: Object.keys(flashupGui.firmwareInfo).length > 0
    
    ScrollView {
        anchors.fill: parent
        clip: true
        
        GridLayout {
            width: parent.width
            columns: 2
            rowSpacing: 6
            columnSpacing: 12
            visible: hasFirmware
            
            Label { 
                text: "Name:" 
                font.bold: true 
            }
            Label { 
                text: flashupGui.firmwareInfo.name || "Unknown" 
                Layout.fillWidth: true
                elide: Text.ElideRight
            }
            
            Label { 
                text: "Version:" 
                font.bold: true 
            }
            Label { 
                text: flashupGui.firmwareInfo.version || "Unknown" 
                Layout.fillWidth: true
            }
            
            Label { 
                text: "Target:" 
                font.bold: true 
            }
            Label { 
                text: flashupGui.firmwareInfo.target || "Unknown" 
                Layout.fillWidth: true
            }
            
            Label { 
                text: "Timestamp:" 
                font.bold: true 
            }
            Label { 
                text: flashupGui.firmwareInfo.timestamp || "Unknown" 
                Layout.fillWidth: true
            }
            
            Label { 
                text: "SHA-256:" 
                font.bold: true 
            }
            Label { 
                text: flashupGui.firmwareInfo.sha256 || "Unknown" 
                Layout.fillWidth: true
                elide: Text.ElideMiddle
                font.family: "monospace"
                font.pixelSize: 10
            }
        }
        
        // Empty state
        ColumnLayout {
            anchors.centerIn: parent
            spacing: 10
            visible: !hasFirmware
            
            Label {
                text: "No firmware loaded"
                font.pixelSize: 16
                Layout.alignment: Qt.AlignHCenter
            }
            
            Button {
                text: "Open Firmware"
                Layout.alignment: Qt.AlignHCenter
                onClicked: openFirmwareDialog.open()
            }
        }
    }
} 