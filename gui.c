#include <windows.h>
#include "include/operations.h"

#define ID_BUTTON_ON 101

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
                }
            }
            return 0;
        default:
            return DefWindowProc(hwnd, uMsg, wparam, lparam);
    }
}

int startgui(HINSTANCE hInstance) {
    WNDCLASSEX window;
    MSG msg;

    window.hCursor = LoadCursor(NULL, IDC_ARROW);
    window.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    window.hbrBackground = GetStockObject(WHITE_BRUSH);
    window.lpfnWndProc = winProc;
    window.cbSize = sizeof(WNDCLASSEX);
    window.hInstance = hInstance;
    window.lpszClassName = "RandomVideoClass";
    window.style = CS_HREDRAW | CS_VREDRAW;


    if (!RegisterClassEx(&window)) {
        return 1;
    }

    HWND hwnd = CreateWindowEx(0, window.lpszClassName, "Random Video", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, hInstance, NULL);
    
    if (!hwnd) {
        return 1;
    }

    // Create a centered ON button
    int buttonWidth = 100;
    int buttonHeight = 50;
    int buttonX = (width - buttonWidth) / 2;
    int buttonY = (height - buttonHeight) / 2;
    CreateWindowEx(0, "BUTTON", "ON", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, buttonX, buttonY, buttonWidth, buttonHeight, hwnd, (HMENU)ID_BUTTON_ON, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}