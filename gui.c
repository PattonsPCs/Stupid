#include <windows.h>
#include "include/operations.h"

#define ID_BUTTON_ON 101
#define ID_BUTTON_OFF 102

static const int width = 300;
static const int height = 150;

LRESULT CALLBACK winProc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam) {
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_COMMAND:
            if (HIWORD(wparam) == BN_CLICKED) {
                if (LOWORD(wparam) == ID_BUTTON_ON) {
                    // Check 1 in 100 chance
                    if (shouldPlayVideo()) {
                        playVideo();
                    }
                } else if (LOWORD(wparam) == ID_BUTTON_OFF) {
                    // Terminate program
                    PostQuitMessage(0);
                }
            }
            return 0;
        default:
            return DefWindowProc(hwnd, uMsg, wparam, lparam);
    }
}

int startgui(HINSTANCE hInstance) {
    WNDCLASSEX window = {0};
    MSG msg;

    window.cbSize = sizeof(WNDCLASSEX);
    window.style = CS_HREDRAW | CS_VREDRAW;
    window.lpfnWndProc = winProc;
    window.hInstance = hInstance;
    window.hCursor = LoadCursor(NULL, IDC_ARROW);
    window.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    window.hbrBackground = GetStockObject(WHITE_BRUSH);
    window.lpszClassName = "RandomVideoClass";
    window.lpszMenuName = NULL;

    if (!RegisterClassEx(&window)) {
        return 1;
    }

    HWND hwnd = CreateWindowEx(0, "RandomVideoClass", "Random Video", WS_OVERLAPPEDWINDOW, 
                               CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, hInstance, NULL);
    
    if (!hwnd) {
        return 1;
    }

    // Create ON and OFF buttons
    int buttonWidth = 100;
    int buttonHeight = 40;
    int spacing = 20;
    int totalWidth = (buttonWidth * 2) + spacing;
    int startX = (width - totalWidth) / 2;
    int buttonY = (height - buttonHeight) / 2;

    CreateWindowEx(0, "BUTTON", "ON", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 
                   startX, buttonY, buttonWidth, buttonHeight, hwnd, (HMENU)ID_BUTTON_ON, hInstance, NULL);
    
    CreateWindowEx(0, "BUTTON", "OFF", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 
                   startX + buttonWidth + spacing, buttonY, buttonWidth, buttonHeight, hwnd, (HMENU)ID_BUTTON_OFF, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}