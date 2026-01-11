#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "include/gui.h"
#include "include/dependencies.h"

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE prevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Show dependency installer first
    showDependencyInstaller(hinstance);
    
    // Then show main GUI
    return startgui(hinstance);
}
