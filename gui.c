#include <windows.h>
#include <stdio.h>
#include "include/operations.h"

#define ID_BUTTON_ON 101
#define ID_BUTTON_OFF 102
#define ID_COMBO_CHANCE 103
#define ID_STATIC_STATUS 104
#define ID_STATIC_CHANCE_LABEL 105

static const int width = 350;
static const int height = 220;
static HWND g_hwnd = NULL;
static HWND g_statusText = NULL;
static HWND g_comboChance = NULL;

LRESULT CALLBACK winProc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam) {
    switch (uMsg) {
        case WM_DESTROY:
            stopChecking();
            PostQuitMessage(0);
            return 0;
        case WM_COMMAND:
            if (HIWORD(wparam) == BN_CLICKED) {
                if (LOWORD(wparam) == ID_BUTTON_ON) {
                    // Get selected chance from combo box
                    int selectedIndex = (int)SendMessageA(g_comboChance, CB_GETCURSEL, 0, 0);
                    if (selectedIndex == CB_ERR) {
                        selectedIndex = 0;  // Default to first item
                    }
                    int denominator = selectedIndex + 2;  // 2 to 101 (but we'll cap at 100)
                    if (denominator > 100) denominator = 100;
                    
                    // Set the chance
                    setChanceDenominator(denominator);
                    
                    // Update status message
                    char statusMsg[128];
                    sprintf_s(statusMsg, sizeof(statusMsg), "Program starting with 1/%d chance", denominator);
                    SetWindowTextA(g_statusText, statusMsg);
                    
                    // Start checking every second
                    startChecking();
                } else if (LOWORD(wparam) == ID_BUTTON_OFF) {
                    // Stop checking and terminate program
                    stopChecking();
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
    
    g_hwnd = hwnd;

    // Create chance selection label
    CreateWindowExA(0, "STATIC", "Chance (1/X):", WS_VISIBLE | WS_CHILD,
                   20, 20, 100, 20, hwnd, (HMENU)ID_STATIC_CHANCE_LABEL, hInstance, NULL);
    
    // Create combo box for chance selection (1/2 to 1/100)
    g_comboChance = CreateWindowExA(0, "COMBOBOX", "", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL,
                                    130, 18, 80, 200, hwnd, (HMENU)ID_COMBO_CHANCE, hInstance, NULL);
    
    // Populate combo box with options from 1/2 to 1/100
    char chanceText[32];
    for (int i = 2; i <= 100; i++) {
        sprintf_s(chanceText, sizeof(chanceText), "1/%d", i);
        SendMessageA(g_comboChance, CB_ADDSTRING, 0, (LPARAM)chanceText);
    }
    // Set default selection to 1/5 (index 3, since we start at 2)
    SendMessageA(g_comboChance, CB_SETCURSEL, 3, 0);
    
    // Create status text area
    g_statusText = CreateWindowExA(0, "STATIC", "", WS_VISIBLE | WS_CHILD | SS_LEFT,
                                   20, 50, width - 40, 30, hwnd, (HMENU)ID_STATIC_STATUS, hInstance, NULL);
    
    // Create ON and OFF buttons
    int buttonWidth = 100;
    int buttonHeight = 40;
    int spacing = 20;
    int totalWidth = (buttonWidth * 2) + spacing;
    int startX = (width - totalWidth) / 2;
    int buttonY = height - buttonHeight - 30;

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