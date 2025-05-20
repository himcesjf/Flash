# FlashUp

A cross-platform firmware/OTA updater & diagnostics tool built with modern C++ and Qt.

## Technical Stack

- **Language**: C++17
- **Framework**: Qt 5.15+/6.2+ (Core, GUI, QML, SerialPort, Network)
- **Build System**: CMake 3.16+
- **Design Patterns**: Plugin Architecture, Observer Pattern, Factory Pattern
- **Architecture**: Modular, Cross-Platform, Event-Driven

## Key Features

- **Device Support**:
  - USB-CDC devices (ESP32, STM32, etc.)
  - Network OTA devices
  - Plugin-based extensibility

- **Security**:
  - SHA-256 firmware validation
  - Cryptographic signature verification
  - Secure communication protocols

- **Update Features**:
  - Resumable chunked transfers
  - Progress monitoring
  - Error recovery
  - Cross-platform support

- **User Interface**:
  - Modern Qt/QML interface
  - Real-time progress updates
  - Device diagnostics
  - Logging system

- **Automation**:
  - Headless script mode
  - CI/CD pipeline integration
  - Command-line interface

## Supported Platforms

- Linux
- Windows
- macOS
- Android tablets

## Building

### Prerequisites

- CMake 3.16+
- Qt 5.15+ or Qt 6.2+
- C++17 compatible compiler

### Build Instructions

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

### GUI Mode

```bash
./FlashUp
```

### Headless Script Mode

```bash
./FlashUp -s -f <firmware_file> -d <device_id>
```