#include <stdbool.h>
#include <stdio.h>
#include <windows.h>
#include "include/globals.h"
#include "include/operations.h"

#define ID_BUTTON_ON 101
#define ID_BUTTON_OFF 102

static const int width = 1200;
static const int length = 700;
volatile bool appRunning = false;

LRESULT CALLBACK winProc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam) {
    static HANDLE runThread = NULL;
    switch (uMsg) {
        case WM_DESTROY:
            if (runThread != NULL && appRunning) {
                appRunning = false;
                WaitForSingleObject(runThread, INFINITE);
            }
            CloseHandle(runThread);
            PostQuitMessage(0);
            return 0;
        case WM_COMMAND:
            if (HIWORD(wparam) == BN_CLICKED) {
                switch (LOWORD(wparam)) {
                    case ID_BUTTON_ON:
                        if (!appRunning) {
                            if (runThread != NULL) {
                                CloseHandle(runThread);
                            }
                            appRunning = true;
                            runThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)run, NULL, 0, NULL);
                        }
                        break;
                    case ID_BUTTON_OFF:
                        appRunning = false;
                        break;
                }
            }
            return 0;
        default:
            return DefWindowProc(hwnd, uMsg, wparam, lparam);
    }
}

typedef HBRUSH WindowColor;
typedef HCURSOR CursorLook;
typedef HICON WindowIcon;
typedef struct {

    const char* windowName;
    void (*event)(void);
    WindowColor brush;
    CursorLook cursor;
    WindowIcon winIcon;
} guiWindow;

int startgui(HINSTANCE hInstance) {
    guiWindow gui;
    WNDCLASSEX window;
    MSG msg;

    gui.winIcon = LoadIcon(NULL, IDI_APPLICATION);
    gui.cursor = LoadCursor(NULL, IDC_ARROW);
    gui.brush = GetStockObject(WHITE_BRUSH);

    window.hCursor = gui.cursor;
    window.hIcon = gui.winIcon;
    window.hbrBackground = gui.brush;
    window.lpfnWndProc = winProc;
    window.cbSize = sizeof(WNDCLASSEX);
    window.hInstance = hInstance;
    window.lpszClassName = "SpookyClass";
    window.style = CS_HREDRAW | CS_VREDRAW;


    RegisterClassEx(&window);

    HWND hwnd = CreateWindowEx(0, window.lpszClassName, "Spooky Menu", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, length, NULL, NULL, hInstance, NULL);

    CreateWindowEx(0, "BUTTON", "ON", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 10, 10, 100, 30, hwnd, (HMENU)ID_BUTTON_ON, hInstance, NULL);
    CreateWindowEx(0, "BUTTON", "OFF", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 100, 10, 100, 30, hwnd, (HMENU)ID_BUTTON_OFF, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}