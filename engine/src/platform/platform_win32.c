#include "platform/platform.h"


#if PLATFORM_WINDOWS

    #include "core/logger.h"
    #include "core/input.h"
    #include "core/event.h"
    #include "containers/darray.h"
    #include <windows.h>
    #include <windowsx.h>
    #include <stdlib.h>

    #include <vulkan/vulkan.h>
    #include <vulkan/vulkan_win32.h>
    #include "renderer/vulkan/vulkan_types.inl"

    typedef struct internal_state {
        HINSTANCE hInstance;
        HWND hWnd;
        VkSurfaceKHR surface;
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

    void platform_get_required_extension_names(const char ***names_darray) {
        darray_push(*names_darray, &"VK_KHR_win32_surface");
    }

    b8 platform_create_vulkan_surface(platform_state* plat_state, vulkan_context* context) {
        internal_state* state = (internal_state*)plat_state->internal_state;
        VkWin32SurfaceCreateInfoKHR create_info = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
        create_info.hinstance = state->hInstance;
        create_info.hwnd = state->hWnd;

        VK_CHECK(vkCreateWin32SurfaceKHR(context->instance, &create_info, context->allocator, &state->surface));
        context->surface = state->surface;
        return TRUE;
    }

    LRESULT CALLBACK win32_process_message(HWND hWnd, u32 uMsg, WPARAM wParam, LPARAM lParam) {
        switch(uMsg)
        {
            case WM_ERASEBKGND:
                return 1; // return 1 to tell windows that we are handling bg
            case WM_CLOSE:
                event_context data = {};
                event_fire(EVENT_CODE_APPLICATION_QUIT, 0, data);
                return TRUE;
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
            case WM_SIZE: {
                RECT r;
                GetClientRect(hWnd, &r);
                u32 width = r.right - r.left;
                u32 height = r.bottom - r.top;

                // Fire the event. The application layer should pick this up, but not handle it
                // as it shouldn be visible to other parts of the application.
                event_context context;
                context.data.u16[0] = (u16)width;
                context.data.u16[1] = (u16)height;
                event_fire(EVENT_CODE_RESIZED, 0, context);
            } break;
            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP: {
                b8 pressed = (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN);
                keys key = (u16)wParam;
                input_process_key(key, pressed);
            } break;
            case WM_MOUSEMOVE: {
                i32 x_pos = GET_X_LPARAM(lParam);
                i32 y_pos = GET_Y_LPARAM(lParam);
                input_process_mouse_move(x_pos, y_pos);
            } break;
            case WM_MOUSEWHEEL: {
                i32 z_delta = GET_WHEEL_DELTA_WPARAM(wParam);
                if (z_delta != 0) {
                    z_delta = (z_delta < 0 ) ? -1 : 1; // map to -1 to 1 
                    input_process_mouse_wheel(z_delta);
                }
            } break;
            case WM_LBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_MBUTTONUP:
            case WM_RBUTTONUP: {
                b8 pressed = (uMsg == WM_LBUTTONDOWN || uMsg == WM_MBUTTONDOWN || uMsg == WM_RBUTTONDOWN);
                buttons mouse_button = BUTTON_MAX_BUTTONS;

                switch(uMsg) {
                    case WM_LBUTTONDOWN:
                    case WM_LBUTTONUP: {
                        mouse_button = BUTTON_LEFT;
                    } break;
                    case WM_MBUTTONDOWN:
                    case WM_MBUTTONUP: {
                        mouse_button = BUTTON_MIDDLE;
                    } break;
                    case WM_RBUTTONDOWN:
                    case WM_RBUTTONUP: {
                        mouse_button = BUTTON_RIGHT;
                    } break;
                }

                if (mouse_button != BUTTON_MAX_BUTTONS) {
                    input_process_button(mouse_button, pressed);
                }
            } break;
        }
        return DefWindowProcA(hWnd, uMsg, wParam, lParam);
    }

#endif