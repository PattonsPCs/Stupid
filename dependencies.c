#include <windows.h>
#include <stdio.h>
#include <io.h>

#define ID_BUTTON_INSTALL_VLC 201
#define ID_BUTTON_SKIP 202
#define ID_BUTTON_CHECK 203
#define ID_STATIC_VLC_STATUS 204
#define ID_STATIC_MPV_STATUS 205
#define ID_STATIC_TITLE 206
#define ID_STATIC_INFO 207
#define ID_STATIC_NOTE 208

// Check if a file exists
static int fileExists(const char* path) {
    return (_access(path, 0) == 0);
}

// Check if VLC is installed
static int checkVLC(void) {
    const char* vlcPaths[] = {
        "C:\\Program Files\\VideoLAN\\VLC\\vlc.exe",
        "C:\\Program Files (x86)\\VideoLAN\\VLC\\vlc.exe"
    };
    
    for (int i = 0; i < 2; i++) {
        if (fileExists(vlcPaths[i])) {
            return 1;
        }
    }
    
    // Check if in PATH
    char pathEnv[4096];
    if (GetEnvironmentVariableA("PATH", pathEnv, sizeof(pathEnv))) {
        // Simple check - try to find vlc.exe in PATH
        STARTUPINFOA si = {0};
        PROCESS_INFORMATION pi = {0};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        
        if (CreateProcessA(NULL, "vlc.exe --version", NULL, NULL, FALSE, 
                          CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
            WaitForSingleObject(pi.hProcess, 1000);
            DWORD exitCode;
            GetExitCodeProcess(pi.hProcess, &exitCode);
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
            if (exitCode == STILL_ACTIVE || exitCode == 0) {
                return 1;
            }
        }
    }
    
    return 0;
}

// Check if mpv is installed
static int checkMPV(void) {
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    
    if (CreateProcessA(NULL, "mpv.exe --version", NULL, NULL, FALSE, 
                      CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, 1000);
        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return (exitCode == STILL_ACTIVE || exitCode == 0);
    }
    
    return 0;
}

// Update status text
static void updateStatus(HWND hwnd) {
    HWND vlcStatus = GetDlgItem(hwnd, ID_STATIC_VLC_STATUS);
    HWND mpvStatus = GetDlgItem(hwnd, ID_STATIC_MPV_STATUS);
    HWND installBtn = GetDlgItem(hwnd, ID_BUTTON_INSTALL_VLC);
    
    if (checkVLC()) {
        SetWindowTextA(vlcStatus, "VLC Media Player: INSTALLED ✓");
        EnableWindow(installBtn, FALSE);
    } else {
        SetWindowTextA(vlcStatus, "VLC Media Player: NOT INSTALLED ✗");
        EnableWindow(installBtn, TRUE);
    }
    
    if (checkMPV()) {
        SetWindowTextA(mpvStatus, "MPV Player: INSTALLED ✓");
    } else {
        SetWindowTextA(mpvStatus, "MPV Player: NOT INSTALLED (Optional)");
    }
}

// Open VLC download page
static void openVLCDownload(void) {
    ShellExecuteA(NULL, "open", "https://www.videolan.org/vlc/", NULL, NULL, SW_SHOWNORMAL);
}

static HWND g_depsWindow = NULL;

LRESULT CALLBACK depsProc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam) {
    switch (uMsg) {
        case WM_CREATE:
            updateStatus(hwnd);
            return 0;
        case WM_COMMAND:
            if (HIWORD(wparam) == BN_CLICKED) {
                switch (LOWORD(wparam)) {
                    case ID_BUTTON_INSTALL_VLC:
                        openVLCDownload();
                        MessageBoxA(hwnd, 
                            "VLC download page opened in your browser.\n\n"
                            "After installing VLC, click 'Check Again' to verify.\n\n"
                            "VLC provides the fastest, instant video playback.",
                            "Install VLC", MB_OK | MB_ICONINFORMATION);
                        break;
                    case ID_BUTTON_CHECK:
                        updateStatus(hwnd);
                        if (checkVLC()) {
                            MessageBoxA(hwnd, "All dependencies are installed! You can now use the program.", 
                                       "Dependencies OK", MB_OK | MB_ICONINFORMATION);
                        }
                        break;
                    case ID_BUTTON_SKIP:
                        DestroyWindow(hwnd);
                        break;
                }
            }
            return 0;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            g_depsWindow = NULL;
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wparam, lparam);
}

// Show dependency installer dialog
int showDependencyInstaller(HINSTANCE hInstance) {
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = depsProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hbrBackground = GetStockObject(WHITE_BRUSH);
    wc.lpszClassName = "DependencyInstallerClass";
    
    RegisterClassEx(&wc);
    
    g_depsWindow = CreateWindowEx(0, "DependencyInstallerClass", "Dependency Installer",
                              WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                              CW_USEDEFAULT, CW_USEDEFAULT, 470, 320, NULL, NULL, hInstance, NULL);
    
    if (!g_depsWindow) {
        return 0;
    }
    
    // Create controls
    CreateWindowExA(0, "STATIC", "Dependency Status", WS_VISIBLE | WS_CHILD,
                   20, 20, 200, 20, g_depsWindow, (HMENU)ID_STATIC_TITLE, hInstance, NULL);
    
    CreateWindowExA(0, "STATIC", "VLC Media Player: Checking...", WS_VISIBLE | WS_CHILD,
                   20, 50, 420, 20, g_depsWindow, (HMENU)ID_STATIC_VLC_STATUS, hInstance, NULL);
    
    CreateWindowExA(0, "STATIC", "MPV Player: Checking...", WS_VISIBLE | WS_CHILD,
                   20, 75, 420, 20, g_depsWindow, (HMENU)ID_STATIC_MPV_STATUS, hInstance, NULL);
    
    CreateWindowExA(0, "STATIC", "VLC Media Player is recommended for instant video playback.", 
                   WS_VISIBLE | WS_CHILD,
                   20, 110, 420, 40, g_depsWindow, (HMENU)ID_STATIC_INFO, hInstance, NULL);
    
    CreateWindowExA(0, "BUTTON", "Install VLC", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                   20, 160, 120, 30, g_depsWindow, (HMENU)ID_BUTTON_INSTALL_VLC, hInstance, NULL);
    
    CreateWindowExA(0, "BUTTON", "Check Again", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                   150, 160, 120, 30, g_depsWindow, (HMENU)ID_BUTTON_CHECK, hInstance, NULL);
    
    CreateWindowExA(0, "BUTTON", "Skip", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                   280, 160, 120, 30, g_depsWindow, (HMENU)ID_BUTTON_SKIP, hInstance, NULL);
    
    CreateWindowExA(0, "STATIC", "Note: You can skip this, but video playback will be slower without VLC.", 
                   WS_VISIBLE | WS_CHILD,
                   20, 210, 420, 40, g_depsWindow, (HMENU)ID_STATIC_NOTE, hInstance, NULL);
    
    ShowWindow(g_depsWindow, SW_SHOWNORMAL);
    UpdateWindow(g_depsWindow);
    
    // Modal message loop - runs until window is destroyed
    MSG msg;
    BOOL bRet;
    while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
        if (bRet == -1) {
            // Error occurred
            break;
        } else {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    return 1;
}

