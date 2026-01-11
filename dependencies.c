#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <shlobj.h>
#include <urlmon.h>
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "shell32.lib")

#define ID_BUTTON_INSTALL_VLC 201
#define ID_BUTTON_SKIP 202
#define ID_BUTTON_CHECK 203
#define ID_STATIC_VLC_STATUS 204
#define ID_STATIC_MPV_STATUS 205
#define ID_STATIC_TITLE 206
#define ID_STATIC_INFO 207
#define ID_STATIC_NOTE 208
#define ID_STATIC_PROGRESS 209

// Global window handle for dependency installer
static HWND g_depsWindow = NULL;

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

// Check if running as administrator
static BOOL isRunningAsAdmin(void) {
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    
    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                 DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    
    return isAdmin;
}

// Download VLC installer
static int downloadVLCInstaller(char* outPath, size_t pathSize) {
    // Get temp directory
    char tempPath[MAX_PATH];
    if (GetTempPathA(MAX_PATH, tempPath) == 0) {
        return 0;
    }
    
    // Ensure temp path ends with backslash
    size_t len = strlen(tempPath);
    if (len > 0 && tempPath[len - 1] != '\\') {
        sprintf_s(outPath, pathSize, "%s\\vlc_installer.exe", tempPath);
    } else {
        sprintf_s(outPath, pathSize, "%svlc_installer.exe", tempPath);
    }
    
    // VLC download URL (latest Windows 64-bit installer)
    // This URL redirects to the latest version
    const char* vlcUrl = "https://get.videolan.org/vlc/last/win64/vlc-3.0.21-win64.exe";
    
    // Alternative: use the main download page if direct link fails
    // We'll try direct download first, fallback to manual download if needed
    
    HWND statusWnd = GetDlgItem(g_depsWindow, ID_STATIC_PROGRESS);
    if (statusWnd) {
        SetWindowTextA(statusWnd, "Downloading VLC installer...");
    }
    
    // Download the file
    HRESULT hr = URLDownloadToFileA(NULL, vlcUrl, outPath, 0, NULL);
    
    // Wait a moment for file to be written
    Sleep(500);
    
    // Verify file exists and has reasonable size (> 1MB)
    if (hr == S_OK && fileExists(outPath)) {
        // Check file size to ensure download completed
        WIN32_FILE_ATTRIBUTE_DATA fileInfo;
        if (GetFileAttributesExA(outPath, GetFileExInfoStandard, &fileInfo)) {
            LARGE_INTEGER fileSize;
            fileSize.LowPart = fileInfo.nFileSizeLow;
            fileSize.HighPart = fileInfo.nFileSizeHigh;
            
            // VLC installer should be at least 1MB
            if (fileSize.QuadPart > 1024 * 1024) {
                if (statusWnd) {
                    SetWindowTextA(statusWnd, "Download complete!");
                }
                return 1;
            }
        }
    }
    
    if (statusWnd) {
        SetWindowTextA(statusWnd, "Download failed. Trying alternative method...");
    }
    
    // Clean up partial download if it exists
    if (fileExists(outPath)) {
        DeleteFileA(outPath);
    }
    
    // Fallback: try to open download page
    ShellExecuteA(NULL, "open", "https://www.videolan.org/vlc/download-windows.html", NULL, NULL, SW_SHOWNORMAL);
    return 0;
}

// Install VLC with admin privileges
static void installVLCWithElevation(HWND hwnd) {
    char installerPath[MAX_PATH];
    
    HWND statusWnd = GetDlgItem(hwnd, ID_STATIC_PROGRESS);
    if (statusWnd) {
        SetWindowTextA(statusWnd, "Preparing installation...");
    }
    
    // Download installer
    if (!downloadVLCInstaller(installerPath, sizeof(installerPath))) {
        MessageBoxA(hwnd, 
            "Failed to download VLC installer automatically.\n\n"
            "The download page has been opened in your browser.\n"
            "Please download and install VLC manually, then click 'Check Again'.",
            "Download Failed", MB_OK | MB_ICONWARNING);
        if (statusWnd) {
            SetWindowTextA(statusWnd, "");
        }
        return;
    }
    
    // Convert to absolute path if not already
    char absolutePath[MAX_PATH];
    if (GetFullPathNameA(installerPath, MAX_PATH, absolutePath, NULL) == 0) {
        strcpy_s(absolutePath, MAX_PATH, installerPath);
    }
    
    // Verify file exists before trying to run it
    if (!fileExists(absolutePath)) {
        char errorMsg[512];
        sprintf_s(errorMsg, sizeof(errorMsg), 
            "Downloaded installer file not found.\n\n"
            "Expected path: %s\n\n"
            "The file may have been deleted or download failed.",
            absolutePath);
        MessageBoxA(hwnd, errorMsg, "File Not Found", MB_OK | MB_ICONERROR);
        if (statusWnd) {
            SetWindowTextA(statusWnd, "");
        }
        return;
    }
    
    if (statusWnd) {
        SetWindowTextA(statusWnd, "Requesting administrator privileges...");
    }
    
    // Request admin privileges and run installer
    // ShellExecuteEx needs the full path
    SHELLEXECUTEINFOA sei = {0};
    sei.cbSize = sizeof(SHELLEXECUTEINFOA);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.hwnd = hwnd;
    sei.lpVerb = "runas";  // Request elevation
    sei.lpFile = absolutePath;  // Use absolute path
    sei.lpParameters = "/S /NCRC";  // Silent install, no CRC check (faster)
    sei.nShow = SW_HIDE;
    sei.lpDirectory = NULL;  // Use file's directory
    
    if (ShellExecuteExA(&sei)) {
        if (statusWnd) {
            SetWindowTextA(statusWnd, "Installing VLC (this may take a minute)...");
        }
        
        // Wait for installation to complete
        if (sei.hProcess) {
            WaitForSingleObject(sei.hProcess, INFINITE);
            CloseHandle(sei.hProcess);
        }
        
        // Clean up installer
        DeleteFileA(absolutePath);
        
        if (statusWnd) {
            SetWindowTextA(statusWnd, "Installation complete! Checking...");
        }
        
        // Wait a moment for installation to register
        Sleep(2000);
        
        // Check if installation was successful
        updateStatus(hwnd);
        
        if (checkVLC()) {
            MessageBoxA(hwnd, 
                "VLC Media Player has been successfully installed!\n\n"
                "You can now use the program with instant video playback.",
                "Installation Successful", MB_OK | MB_ICONINFORMATION);
            if (statusWnd) {
                SetWindowTextA(statusWnd, "");
            }
        } else {
            MessageBoxA(hwnd, 
                "Installation completed, but VLC was not detected.\n\n"
                "Please restart the program or click 'Check Again'.",
                "Installation Complete", MB_OK | MB_ICONINFORMATION);
            if (statusWnd) {
                SetWindowTextA(statusWnd, "");
            }
        }
    } else {
        DWORD error = GetLastError();
        if (error == ERROR_CANCELLED) {
            MessageBoxA(hwnd, 
                "Administrator privileges were denied.\n\n"
                "Installation cancelled. You can install VLC manually if needed.",
                "Installation Cancelled", MB_OK | MB_ICONINFORMATION);
        } else {
            MessageBoxA(hwnd, 
                "Failed to start installation.\n\n"
                "Error code: Installation failed. Please try installing VLC manually.",
                "Installation Failed", MB_OK | MB_ICONERROR);
        }
        if (statusWnd) {
            SetWindowTextA(statusWnd, "");
        }
    }
}

LRESULT CALLBACK depsProc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam) {
    switch (uMsg) {
        case WM_CREATE:
            updateStatus(hwnd);
            return 0;
        case WM_COMMAND:
            if (HIWORD(wparam) == BN_CLICKED) {
                switch (LOWORD(wparam)) {
                    case ID_BUTTON_INSTALL_VLC: {
                        int result = MessageBoxA(hwnd,
                            "This will download and install VLC Media Player.\n\n"
                            "Administrator privileges are required.\n\n"
                            "Do you want to continue?",
                            "Install VLC Media Player",
                            MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON1);
                        
                        if (result == IDYES) {
                            installVLCWithElevation(hwnd);
                        }
                        break;
                    }
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
                              CW_USEDEFAULT, CW_USEDEFAULT, 470, 340, NULL, NULL, hInstance, NULL);
    
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
                   20, 210, 420, 30, g_depsWindow, (HMENU)ID_STATIC_NOTE, hInstance, NULL);
    
    CreateWindowExA(0, "STATIC", "", WS_VISIBLE | WS_CHILD,
                   20, 250, 420, 20, g_depsWindow, (HMENU)ID_STATIC_PROGRESS, hInstance, NULL);
    
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

