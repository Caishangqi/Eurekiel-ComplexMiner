#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in VERY few places (and .CPPs only)
#include <crtdbg.h>
#include <iostream>

#include "Framework/App.hpp"
#include "Engine/Core/EngineCommon.hpp"

// Uncomment here if you want my cute console
//#define CONSOLE_HANDLER HANDLE


extern App*   g_theApp;
extern HANDLE g_consoleHandle;


//-----------------------------------------------------------------------------------------------
// Processes all Windows messages (WM_xxx) for this app that have queued up since last frame.
// For each message in the queue, our WindowsMessageHandlingProcedure (or "WinProc") function
//	is called, telling us what happened (key up/down, minimized/restored, gained/lost focus, etc.)
//
// #SD1ToDo: Eventually, we will move this function to a more appropriate place later on... (Engine/Renderer/Window.cpp)
//
void RunMessagePump()
{
    MSG queuedMessage;
    for (;;)
    {
        const BOOL wasMessagePresent = PeekMessage(&queuedMessage, nullptr, 0, 0, PM_REMOVE);
        if (!wasMessagePresent)
        {
            break;
        }

        TranslateMessage(&queuedMessage);
        DispatchMessage(&queuedMessage);
        // This tells Windows to call our "WindowsMessageHandlingProcedure" (a.k.a. "WinProc") function
    }
}

#ifdef CONSOLE_HANDLER
HANDLE g_consoleHandle = nullptr;
void   CreateConsole()
{
    
    AllocConsole();
    FILE* stream;
    freopen_s(&stream, "CONOUT$", "w", stdout);
    freopen_s(&stream, "CONOUT$", "w", stderr);
    freopen_s(&stream, "CONIN$", "r", stdin);

    g_consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    if (g_consoleHandle == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Failed to get console handle!" << '\n';
    }
    else
    {
        puts("[/] Initialize......");
        puts("   _____ _             _    _____ _     _        ");
        puts("  / ____| |           | |  / ____| |   (_)       ");
        puts(" | (___ | |_ __ _ _ __| |_| (___ | |__  _ _ __   ");
        puts("  \\___ \\| __/ _` | '__| __|\\___ \\| '_ \\| | '_ \\       Running v2.3-BETA (v1_20)");
        puts("  ____) | || (_| | |  | |_ ____) | | | | | |_) |      Platform Windows (11)");
        puts(" |_____/ \\__\\__,_|_|   \\__|_____/|_| |_|_| .__/  ");
        puts("                                         | |     ");
        puts("                                         |_|     ");
        puts("             Developed by Caizii                 ");
        puts("\n");
    }

    if (g_consoleHandle)
    {
        SetConsoleTextAttribute(g_consoleHandle, BACKGROUND_BLUE | FOREGROUND_INTENSITY);
    }
}
#endif
//-----------------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int)
{
    UNUSED(applicationInstanceHandle)
    UNUSED(commandLineString)

#ifdef CONSOLE_HANDLER
    // Temporary Console, in SD-4 will draw by opengl
    CreateConsole();
#endif

    g_theApp = new App();
    g_theApp->Startup();

    // Program main loop; keep running frames until it's time to quit
    while (!g_theApp->IsQuitting()) // #SD1ToDo: ...becomes:  !g_theApp->IsQuitting()
    {
        //Sleep(100); // Temporary code to "slow down" our app to ~60Hz until we have proper frame timing in
        // #SD1ToDo: This call will move to Window::BeginFrame() once we have a Window engine system
        // Process OS messages (keyboard/mouse button clicked, application lost/gained focus, etc.)
        RunMessagePump(); // calls our own WindowsMessageHandlingProcedure() function for us!

        g_theApp->RunFrame();
    }

    g_theApp->Shutdown();
    delete g_theApp;
    g_theApp = nullptr;

    return 0;
}
