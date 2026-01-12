#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <windows.h>
#include <process.h>

// Get video file path - looks in same directory as exe, then falls back to hardcoded path
static void getVideoFilePath(char* outPath, size_t pathSize) {
    // First, try to find video in the same directory as the executable
    char exePath[MAX_PATH];
    if (GetModuleFileNameA(NULL, exePath, MAX_PATH) != 0) {
        // Get directory of exe
        char* lastSlash = strrchr(exePath, '\\');
        if (lastSlash) {
            *lastSlash = '\0';
            sprintf_s(outPath, pathSize, "%s\\video.mp4", exePath);
            if (_access(outPath, 0) == 0) {
                return;  // Found video in exe directory
            }
        }
    }
    
    // Fallback: use hardcoded path (for development)
    strcpy_s(outPath, pathSize, "C:\\Users\\antho\\Downloads\\Voicy_foxy jumpscare fnaf2.mp4");
}

// Global flag to control the background thread
volatile int g_running = 0;
// Global chance denominator (e.g., 5 means 1/5 chance)
volatile int g_chanceDenominator = 5;

// Set the chance denominator (2 to 100)
void setChanceDenominator(int denominator) {
    if (denominator >= 2 && denominator <= 100) {
        g_chanceDenominator = denominator;
    }
}

// Get the current chance denominator
int getChanceDenominator(void) {
    return g_chanceDenominator;
}

// Check if we should play the video (configurable chance)
// Returns 1 if video should play, 0 otherwise
int shouldPlayVideo(void) {
    static int initialized = 0;
    if (!initialized) {
        srand((unsigned int)time(NULL));
        initialized = 1;
    }
    
    int roll = rand() % g_chanceDenominator;
    return (roll == 0);  // 1 in g_chanceDenominator chance
}

// Data structure for window enumeration
struct EnumWindowData {
    DWORD processId;
    HWND hwnd;
};

// Callback function for EnumWindows to find VLC window
static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    struct EnumWindowData* data = (struct EnumWindowData*)lParam;
    DWORD windowProcessId;
    GetWindowThreadProcessId(hwnd, &windowProcessId);
    
    if (windowProcessId == data->processId && IsWindowVisible(hwnd)) {
        char className[256];
        GetClassNameA(hwnd, className, sizeof(className));
        // VLC uses "QWidget" class name or similar
        if (strstr(className, "QWidget") != NULL || 
            strstr(className, "VLC") != NULL ||
            GetWindowTextLengthA(hwnd) > 0) {
            data->hwnd = hwnd;
            return FALSE;  // Stop enumeration
        }
    }
    return TRUE;  // Continue enumeration
}

// Callback to find any visible window from a process
static BOOL CALLBACK FindAnyVisibleWindowProc(HWND hwnd, LPARAM lParam) {
    struct EnumWindowData* data = (struct EnumWindowData*)lParam;
    DWORD windowProcessId;
    GetWindowThreadProcessId(hwnd, &windowProcessId);
    
    if (windowProcessId == data->processId && IsWindowVisible(hwnd)) {
        data->hwnd = hwnd;
        return FALSE;  // Stop enumeration
    }
    return TRUE;  // Continue enumeration
}

// Helper function to find window by process ID
static HWND FindWindowByProcessId(DWORD processId) {
    struct EnumWindowData data = { processId, NULL };
    EnumWindows(EnumWindowsProc, (LPARAM)&data);
    return data.hwnd;
}

// Force window to foreground (aggressive method)
static void ForceWindowToForeground(HWND hwnd) {
    if (hwnd == NULL) return;
    
    // Allow this process to set foreground window (needed for background threads)
    DWORD currentThreadId = GetCurrentThreadId();
    DWORD foregroundThreadId = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
    
    if (foregroundThreadId != currentThreadId) {
        AttachThreadInput(foregroundThreadId, currentThreadId, TRUE);
    }
    
    // Multiple methods to ensure it comes to front
    ShowWindow(hwnd, SW_RESTORE);
    ShowWindow(hwnd, SW_SHOW);
    ShowWindow(hwnd, SW_SHOWMAXIMIZED);
    SetForegroundWindow(hwnd);
    BringWindowToTop(hwnd);
    SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
    SetActiveWindow(hwnd);
    
    // Force focus
    SetFocus(hwnd);
    
    // Try multiple times to ensure it stays on top
    for (int i = 0; i < 3; i++) {
        SetForegroundWindow(hwnd);
        BringWindowToTop(hwnd);
        SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
        Sleep(50);
    }
    
    if (foregroundThreadId != currentThreadId) {
        AttachThreadInput(foregroundThreadId, currentThreadId, FALSE);
    }
}

// Play the video file instantly using VLC (fastest) or fallback method
void playVideo(void) {
    char videoPath[MAX_PATH];
    getVideoFilePath(videoPath, sizeof(videoPath));
    
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOWMAXIMIZED;  // Show maximized instead of hiding
    
    char cmd[1024];
    
    // Try VLC in common locations (fastest - instant fullscreen, no GUI)
    const char* vlcPaths[] = {
        "C:\\Program Files\\VideoLAN\\VLC\\vlc.exe",
        "C:\\Program Files (x86)\\VideoLAN\\VLC\\vlc.exe",
        "vlc.exe"  // If in PATH
    };
    
    for (int i = 0; i < 3; i++) {
        sprintf_s(cmd, sizeof(cmd), "\"%s\" --intf=dummy --play-and-exit --fullscreen --no-video-deco \"%s\"", 
                 vlcPaths[i], videoPath);
        
        // Remove CREATE_NO_WINDOW so window can be shown and brought to front
        if (CreateProcessA(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            // Wait for VLC window to appear and try multiple times
            HWND vlcWindow = NULL;
            for (int attempt = 0; attempt < 10 && vlcWindow == NULL; attempt++) {
                Sleep(100);  // Wait 100ms between attempts
                
                // Find VLC window by process ID
                vlcWindow = FindWindowByProcessId(pi.dwProcessId);
                if (vlcWindow == NULL) {
                    // Try finding by class name as fallback
                    vlcWindow = FindWindowA("QWidget", NULL);
                    if (vlcWindow == NULL) {
                        // Try any visible window from this process
                        struct EnumWindowData data = { pi.dwProcessId, NULL };
                        EnumWindows(FindAnyVisibleWindowProc, (LPARAM)&data);
                        vlcWindow = data.hwnd;
                    }
                }
            }
            
            if (vlcWindow != NULL) {
                ForceWindowToForeground(vlcWindow);
            }
            
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
            return;  // Success!
        }
    }
    
    // Fallback: Try mpv if available (also very fast)
    sprintf_s(cmd, sizeof(cmd), "mpv.exe --fs --no-terminal --no-osc \"%s\"", videoPath);
    if (CreateProcessA(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        Sleep(200);
        HWND mpvWindow = FindWindowByProcessId(pi.dwProcessId);
        if (mpvWindow != NULL) {
            ForceWindowToForeground(mpvWindow);
        }
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return;
    }
    
    // Last resort: Use Windows default player (slower but works)
    ShellExecuteA(NULL, "open", videoPath, NULL, NULL, SW_SHOWMAXIMIZED);
}

// Background thread that checks every second
unsigned int __stdcall checkLoop(void* param) {
    srand((unsigned int)time(NULL));
    
    while (g_running) {
        Sleep(1000);  // Wait 1 second
        
        if (g_running && shouldPlayVideo()) {
            playVideo();
        }
    }
    
    return 0;
}

// Start the background checking loop
void startChecking(void) {
    if (!g_running) {
        g_running = 1;
        _beginthreadex(NULL, 0, checkLoop, NULL, 0, NULL);
    }
}

// Stop the background checking loop
void stopChecking(void) {
    g_running = 0;
}
