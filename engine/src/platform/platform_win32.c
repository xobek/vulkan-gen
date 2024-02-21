#include "platform/platform.h"


#if PLATFORM_WINDOWS

    #include "core/logger.h"
    #include <windows.h>
    #include <windowsx.h>
    #include <stdlib.h>

    typedef struct internal_state {
        HINSTANCE hInstance;
        HWND hWnd;
    } internal_state;

    static f64 clock_frequency;
    static LARGE_INTEGER start_time;
    LRESULT CALLBACK win32_process_message(HWND hWnd, u32 uMsg, WPARAM wParam, LPARAM lParam);

    b8 platform_startup(platform_state* pstate, const char* app_name, i32 x, i32 y, i32 w, i32 h)
    {
        pstate->internal_state = malloc(sizeof(internal_state));
        internal_state* state = (internal_state*)pstate->internal_state;
        
        state->hInstance = GetModuleHandleA(0);

        HICON hIcon = LoadIcon(state->hInstance, IDI_APPLICATION);
        HCURSOR hCursor = LoadCursor(0, IDC_ARROW);
        WNDCLASS wc = {0};
        memset(&wc, 0, sizeof(wc));
        wc.style = CS_DBLCLKS;
        wc.lpfnWndProc = win32_process_message;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = state->hInstance;
        wc.hIcon = hIcon;
        wc.hCursor = hCursor;
        wc.hbrBackground = NULL;
        wc.lpszClassName = "veng_windows_class";
        
        if (!RegisterClass(&wc)) {
            MessageBoxA(0, "Windows RegisterClass Failed", "Error", MB_OK);
            return FALSE;
        }

        u32 cl_x = x;
        u32 cl_y = y;
        u32 cl_w = w;
        u32 cl_h = h;

        u32 window_x = cl_x;
        u32 window_y = cl_y;
        u32 window_w = cl_w;
        u32 window_h = cl_h;

        u32 window_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
        u32 window_style_ex = WS_EX_APPWINDOW;

        window_style |= WS_MAXIMIZEBOX;
        window_style |= WS_MINIMIZEBOX;
        window_style |= WS_THICKFRAME;

        RECT window_rect = {0, 0, 0, 0};
        AdjustWindowRectEx(&window_rect, window_style, 0, window_style_ex);

        window_x += window_rect.left;
        window_y += window_rect.top;

        window_w += window_rect.right - window_rect.left;
        window_h += window_rect.bottom - window_rect.top;

        HWND handle = CreateWindowExA(
            window_style_ex,
            "veng_windows_class",
            app_name,
            window_style,
            window_x,
            window_y,
            window_w,
            window_h,
            0,
            0,
            state->hInstance,
            0
        );

        if (handle == 0)
        {
            MessageBoxA(0, "Windows CreateWindowEx Failed", "Error", MB_OK);
            FATAL("Windows CreateWindowEx Failed");
            return FALSE;
        } else {
            state->hWnd = handle;
        }

        b32 should_activate = 1;
        i32 show_window_command_flags = should_activate ? SW_SHOW : SW_SHOWNOACTIVATE;

        ShowWindow(state->hWnd, show_window_command_flags);

        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);
        clock_frequency = 1.0 / (f64)frequency.QuadPart;
        QueryPerformanceCounter(&start_time);

        return TRUE;
    }

    void platform_shutdown(platform_state* pstate)
    {
        internal_state* state = (internal_state*)pstate->internal_state;
        if (state->hWnd) {
            DestroyWindow(state->hWnd);
            state->hWnd = 0;
        }
    }

    b8 platform_pump_messages(platform_state* pstate)
    {
        MSG msg;
        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE)) {
            // if (msg.message == WM_QUIT) {
            //     return FALSE;
            // }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        return TRUE;
    }

    void* platform_allocate(u64 size, b8 aligned)
    {
        return malloc(size);
    }

    void platform_free(void* block, b8 aligned)
    {
        free(block);
    }

    void* platform_zero_memory(void* block, u64 size)
    {
        return memset(block, 0, size);
    }

    void* platform_copy_memory(void* dest, const void* src, u64 size)
    {
        return memcpy(dest, src, size);
    }

    void* platform_set_memory(void* dest, i32 value, u64 size)
    {
        return memset(dest, value, size);
    }

    void platform_console_write(const char* msg, u8 color)
    {

        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        static u8 levels[6] = {64, 4, 6, 2, 1, 8};
        SetConsoleTextAttribute(hConsole, levels[color]);

        OutputDebugStringA(msg);
        u64 length = strlen(msg);
        LPDWORD num_written = 0;

        WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), msg, (DWORD) length, num_written, 0);
    }

    void platform_console_write_error(const char* msg, u8 color)
    {
        HANDLE hConsole = GetStdHandle(STD_ERROR_HANDLE);
        static u8 levels[6] = {64, 4, 6, 2, 1, 8};
        SetConsoleTextAttribute(hConsole, levels[color]);

        OutputDebugStringA(msg);
        u64 length = strlen(msg);
        LPDWORD num_written = 0;

        WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), msg, (DWORD) length, num_written, 0);
    }

    f64 platform_get_absolute_time()
    {
        LARGE_INTEGER current_time;
        QueryPerformanceCounter(&current_time);
        return (f64)current_time.QuadPart * clock_frequency;
    }

    void platform_sleep(u64 ms)
    {
        Sleep(ms);
    }

    LRESULT CALLBACK win32_process_message(HWND hWnd, u32 uMsg, WPARAM wParam, LPARAM lParam) {
        switch(uMsg)
        {
            case WM_ERASEBKGND:
                return 1; // return 1 to tell windows that we are handling bg
            case WM_CLOSE:
                // TODO: Fire event for app quit.
                return 0;
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
            case WM_SIZE: {
                // RECT r;
                // GetClientRect(hWnd, &r);
                // u32 width = r.right - r.left;
                // u32 height = r.bottom - r.top;
                // TODO: Fire event for resize.
            } break;
            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP: {
                // b8 pressed = (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN);
                // TODO: process input
            } break;
            case WM_MOUSEMOVE: {
                // i32 x_pos = GET_X_LPARAM(lParam);
                // i32 y_pos = GET_Y_LPARAM(lParam);
                // TODO: process input
            } break;
            case WM_MOUSEWHEEL: {
                // i32 z_delta = GET_WHEEL_DELTA_WPARAM(wParam);
                // if (z_delta != 0) {
                //     z_delta = (z_delta < 0 ) ? -1 : 1; // map to -1 to 1 
                // }
            } break;
            case WM_LBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_MBUTTONUP:
            case WM_RBUTTONUP: {
                // b8 pressed = (uMsg == WM_LBUTTONDOWN || uMsg == WM_MBUTTONDOWN || uMsg == WM_RBUTTONDOWN);
            } break;
        }
        return DefWindowProcA(hWnd, uMsg, wParam, lParam);
    }

#endif