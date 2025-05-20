import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3

ApplicationWindow {
    id: window
    visible: true
    width: 1000
    height: 700
    title: "FlashUp - Firmware Updater"
    
    // Notification popup
    Popup {
        id: notificationPopup
        width: 400
        height: 100
        x: (parent.width - width) / 2
        y: 100
        modal: false
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        
        property string notificationTitle: ""
        property string notificationMessage: ""
        property int notificationType: 0 // 0=info, 1=warning, 2=error, 3=success
        
        background: Rectangle {
            color: {
                switch (notificationPopup.notificationType) {
                    case 0: return "#f0f0f0"; // info
                    case 1: return "#fff3cd"; // warning
                    case 2: return "#f8d7da"; // error
                    case 3: return "#d4edda"; // success
                    default: return "#f0f0f0";
                }
            }
            border.color: {
                switch (notificationPopup.notificationType) {
                    case 0: return "#d0d0d0"; // info
                    case 1: return "#ffeeba"; // warning
                    case 2: return "#f5c6cb"; // error
                    case 3: return "#c3e6cb"; // success
                    default: return "#d0d0d0";
                }
            }
            radius: 5
        }
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            
            Label {
                text: notificationPopup.notificationTitle
                font.bold: true
                font.pixelSize: 16
                color: {
                    switch (notificationPopup.notificationType) {
                        case 0: return "#333333"; // info
                        case 1: return "#856404"; // warning
                        case 2: return "#721c24"; // error
                        case 3: return "#155724"; // success
                        default: return "#333333";
                    }
                }
            }
            
            Label {
                Layout.fillWidth: true
                text: notificationPopup.notificationMessage
                wrapMode: Text.WordWrap
                color: {
                    switch (notificationPopup.notificationType) {
                        case 0: return "#333333"; // info
                        case 1: return "#856404"; // warning
                        case 2: return "#721c24"; // error
                        case 3: return "#155724"; // success
                        default: return "#333333";
                    }
                }
            }
            
            Item {
                Layout.fillHeight: true
            }
            
            RowLayout {
                Layout.alignment: Qt.AlignRight
                
                Button {
                    text: "Close"
                    onClicked: notificationPopup.close()
                }
            }
        }
        
        Timer {
            id: autoCloseTimer
            interval: 5000
            running: false
            repeat: false
            onTriggered: notificationPopup.close()
        }
        
        onOpened: {
            autoCloseTimer.start();
        }
    }
    
    // File dialogs
    FileDialog {
        id: openFirmwareDialog
        title: "Select Firmware File"
        folder: shortcuts.home
        nameFilters: ["Firmware files (*.bin *.fw *.hex)", "All files (*)"]
        onAccepted: {
            if (flashupGui.loadFirmware(fileUrl)) {
                // Success - notification handled by signal
            }
        }
    }
    
    FileDialog {
        id: saveLogsDialog
        title: "Save Logs"
        folder: shortcuts.home
        nameFilters: ["Log files (*.log *.txt)", "All files (*)"]
        selectExisting: false
        onAccepted: {
            if (flashupGui.saveLogs(fileUrl)) {
                showNotification("Logs Saved", "Log file saved successfully", 3);
            } else {
                showNotification("Error", "Failed to save logs", 2);
            }
        }
    }
    
    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            
            Label {
                text: "FlashUp"
                font.pixelSize: 20
                font.bold: true
                Layout.leftMargin: 10
            }
            
            Item {
                Layout.fillWidth: true
            }
            
            Button {
                text: "Refresh Devices"
                onClicked: flashupGui.refreshDevices()
            }
            
            Button {
                text: "Open Firmware"
                onClicked: openFirmwareDialog.open()
            }
            
            Button {
                text: "Save Logs"
                onClicked: saveLogsDialog.open()
            }
            
            Button {
                text: "Clear Logs"
                onClicked: flashupGui.clearLogs()
            }
        }
    }
    
    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal
        
        // Left panel (Device list and Firmware info)
        Pane {
            SplitView.preferredWidth: 300
            SplitView.minimumWidth: 200
            
            ColumnLayout {
                anchors.fill: parent
                spacing: 10
                
                // Device list
                GroupBox {
                    title: "Devices"
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    
                    DeviceList {
                        anchors.fill: parent
                    }
                }
                
                // Firmware info
                GroupBox {
                    title: "Firmware"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 200
                    
                    FirmwareInfo {
                        anchors.fill: parent
                    }
                }
            }
        }
        
        // Right panel (Update status and Logs)
        Pane {
            SplitView.fillWidth: true
            
            ColumnLayout {
                anchors.fill: parent
                spacing: 10
                
                // Update panel
                GroupBox {
                    title: "Update"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 150
                    
                    UpdatePanel {
                        anchors.fill: parent
                    }
                }
                
                // Log view
                GroupBox {
                    title: "Logs"
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    
                    LogView {
                        anchors.fill: parent
                    }
                }
            }
        }
    }
    
    // Connect signals
    Connections {
        target: flashupGui
        
        function onNotification(title, message, type) {
            showNotification(title, message, type);
        }
    }
    
    function showNotification(title, message, type) {
        notificationPopup.notificationTitle = title;
        notificationPopup.notificationMessage = message;
        notificationPopup.notificationType = type;
        notificationPopup.open();
    }
} 