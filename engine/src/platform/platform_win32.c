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

    typedef struct platform_state {
        HINSTANCE hInstance;
        HWND hWnd;
        VkSurfaceKHR surface;
        f64 clock_frequency;
        LARGE_INTEGER start_time;
    } platform_state;

    static platform_state *state_ptr;

    static f64 clock_frequency;
    static LARGE_INTEGER start_time;
    LRESULT CALLBACK win32_process_message(HWND hWnd, u32 uMsg, WPARAM wParam, LPARAM lParam);

b8 platform_system_startup(u64 *memory_requirement, void *state, const char *application_name, i32 x, i32 y, i32 w, i32 h) {
        *memory_requirement = sizeof(platform_state);
        if (state == 0) {
            return true;
        }
        state_ptr = state;
        state_ptr->hInstance = GetModuleHandleA(0);

        HICON icon = LoadIcon(state_ptr->hInstance, IDI_APPLICATION);
        HCURSOR hCursor = LoadCursor(0, IDC_ARROW);
        WNDCLASS wc = {0};
        memset(&wc, 0, sizeof(wc));
        wc.style = CS_DBLCLKS;
        wc.lpfnWndProc = win32_process_message;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = state_ptr->hInstance;
        wc.hIcon = icon;
        wc.hCursor = hCursor;
        wc.hbrBackground = NULL;
        wc.lpszClassName = "veng_windows_class";
        
        if (!RegisterClass(&wc)) {
            MessageBoxA(0, "Windows RegisterClass Failed", "Error", MB_OK);
            return false;
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
            window_style_ex, "veng_windows_class", application_name,
            window_style, window_x, window_y, window_w, window_h,
            0, 0, state_ptr->hInstance, 0);

        if (handle == 0)
        {
            MessageBoxA(0, "Windows CreateWindowEx Failed", "Error", MB_OK);
            FATAL("Windows CreateWindowEx Failed");
            return false;
        } else {
            state_ptr->hWnd = handle;
        }

        b32 should_activate = 1;
        i32 show_window_command_flags = should_activate ? SW_SHOW : SW_SHOWNOACTIVATE;

        ShowWindow(state_ptr->hWnd, show_window_command_flags);

        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);
        state_ptr->clock_frequency = 1.0 / (f64)frequency.QuadPart;
        QueryPerformanceCounter(&state_ptr->start_time);

        return true;
    }

    void platform_system_shutdown(void *plat_state) {
        if (state_ptr && state_ptr->hWnd) {
            DestroyWindow(state_ptr->hWnd);
            state_ptr->hWnd = 0;
        }
    }

    b8 platform_pump_messages() {
        if (state_ptr) {
            MSG message;
            while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&message);
                DispatchMessageA(&message);
            }
        }
        return true;
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
        if (state_ptr) {
            LARGE_INTEGER now_time;
            QueryPerformanceCounter(&now_time);
            return (f64)now_time.QuadPart * state_ptr->clock_frequency;
        }
        return 0;
    }

    void platform_sleep(u64 ms)
    {
        Sleep(ms);
    }

    void platform_get_required_extension_names(const char ***names_darray) {
        darray_push(*names_darray, &"VK_KHR_win32_surface");
    }

    b8 platform_create_vulkan_surface(vulkan_context* context) {
        if (!state_ptr) {
            return false;
        }
        VkWin32SurfaceCreateInfoKHR create_info = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
        create_info.hinstance = state_ptr->hInstance;
        create_info.hwnd = state_ptr->hWnd;

        VkResult result = vkCreateWin32SurfaceKHR(context->instance, &create_info, context->allocator, &state_ptr->surface);
        if (result != VK_SUCCESS) {
            FATAL("Vulkan surface creation failed.");
            return false;
        }
        context->surface = state_ptr->surface;
        return true;
    }

    LRESULT CALLBACK win32_process_message(HWND hWnd, u32 uMsg, WPARAM wParam, LPARAM lParam) {

        switch(uMsg)
        {
            case WM_ERASEBKGND:
                return 1; // return 1 to tell windows that we are handling bg
            case WM_CLOSE:
                event_context data = {};
                event_fire(EVENT_CODE_APPLICATION_QUIT, 0, data);
                return true;
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
                // KEY PRESSED / RELEASED
                b8 pressed = (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN);
                keys key = (u16)wParam;

                b8 extended = (HIWORD(lParam) & KF_EXTENDED) != KF_EXTENDED;

                if (wParam == VK_MENU) {
                    key = extended ? KEY_RALT : KEY_LALT;
                } else if (wParam == VK_SHIFT) {
                    u32 left_shift = MapVirtualKeyA(VK_LSHIFT, MAPVK_VK_TO_VSC);
                    u32 scan_code = (lParam & (0xFF << 16)) >> 16;
                    key = (scan_code == left_shift) ? KEY_LSHIFT : KEY_RSHIFT;
                } else if (wParam == VK_CONTROL) {
                    key = extended ? KEY_RCONTROL : KEY_LCONTROL;
                }

                input_process_key(key, pressed);
            } break;
            case WM_MOUSEMOVE: {
                // MOUSE MOVED
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