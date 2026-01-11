#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <process.h>

// Path to your video file - change this to your video file path
#define VIDEO_FILE_PATH "C:\\Users\\antho\\Downloads\\Voicy_foxy jumpscare fnaf2.mp4"

// Global flag to control the background thread
volatile int g_running = 0;

// Check if we should play the video (1 in 100 chance)
// Returns 1 if video should play, 0 otherwise
int shouldPlayVideo(void) {
    static int initialized = 0;
    if (!initialized) {
        srand((unsigned int)time(NULL));
        initialized = 1;
    }
    
    int roll = rand() % 100;
    return (roll == 0);  // 1 in 100 chance (0-99, so 0 is 1/100)
}

// Play the video file instantly using VLC (fastest) or fallback method
void playVideo(void) {
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    
    char cmd[1024];
    
    // Try VLC in common locations (fastest - instant fullscreen, no GUI)
    const char* vlcPaths[] = {
        "C:\\Program Files\\VideoLAN\\VLC\\vlc.exe",
        "C:\\Program Files (x86)\\VideoLAN\\VLC\\vlc.exe",
        "vlc.exe"  // If in PATH
    };
    
    for (int i = 0; i < 3; i++) {
        sprintf_s(cmd, sizeof(cmd), "\"%s\" --intf=dummy --play-and-exit --fullscreen --no-video-deco \"%s\"", 
                 vlcPaths[i], VIDEO_FILE_PATH);
        
        if (CreateProcessA(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
            return;  // Success!
        }
    }
    
    // Fallback: Try mpv if available (also very fast)
    sprintf_s(cmd, sizeof(cmd), "mpv.exe --fs --no-terminal --no-osc \"%s\"", VIDEO_FILE_PATH);
    if (CreateProcessA(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return;
    }
    
    // Last resort: Use Windows default player (slower but works)
    ShellExecuteA(NULL, "open", VIDEO_FILE_PATH, NULL, NULL, SW_SHOWMAXIMIZED);
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
