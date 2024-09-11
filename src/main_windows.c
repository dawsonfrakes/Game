/* kernel32 */
typedef struct HINSTANCE__* HINSTANCE;

HINSTANCE GetModuleHandleW(u16*);
void Sleep(u32);
void ExitProcess(u32);

/* user32 */
#define IDI_WARNING (cast(void*) cast(u64) 32515)
#define IDC_CROSS (cast(void*) cast(u64) 32515)
#define CS_OWNDC 0x0020
#define WS_MAXIMIZEBOX 0x00010000
#define WS_MINIMIZEBOX 0x00020000
#define WS_THICKFRAME 0x00040000
#define WS_SYSMENU 0x00080000
#define WS_CAPTION 0x00C00000
#define WS_VISIBLE 0x10000000
#define WS_OVERLAPPEDWINDOW (WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)
#define CW_USEDEFAULT (cast(s32) 0x80000000)
#define PM_REMOVE 0x0001
#define WM_CREATE 0x0001
#define WM_DESTROY 0x0002
#define WM_SIZE 0x0005
#define WM_PAINT 0x000F
#define WM_QUIT 0x0012
#define WM_ERASEBKGND 0x0014
#define WM_ACTIVATEAPP 0x001C
#define WM_KEYDOWN 0x0100
#define WM_KEYUP 0x0101
#define WM_SYSKEYDOWN 0x0104
#define WM_SYSKEYUP 0x0105
#define WM_SYSCOMMAND 0x0112
#define SC_KEYMENU 0xF100
#define GWL_STYLE (-16)
#define MONITOR_DEFAULTTOPRIMARY 0x00000001
#define VK_RETURN 0x0D
#define VK_ESCAPE 0x1B
#define VK_F4 0x73
#define VK_F10 0x79
#define VK_F11 0x7A

typedef struct HDC__* HDC;
typedef struct HWND__* HWND;
typedef struct HMENU__* HMENU;
typedef struct HICON__* HICON;
typedef struct HBRUSH__* HBRUSH;
typedef struct HCURSOR__* HCURSOR;
typedef struct HMONITOR__* HMONITOR;
typedef s64 (*WNDPROC)(HWND, u32, u64, s64);
typedef struct {
    s32 x;
    s32 y;
} POINT;
typedef struct {
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;
} RECT;
typedef struct {
    u32 cbSize;
    u32 style;
    WNDPROC lpfnWndProc;
    s32 cbClsExtra;
    s32 cbWndExtra;
    HINSTANCE hInstance;
    HICON hIcon;
    HCURSOR hCursor;
    HBRUSH hbrBackground;
    u16* lpszMenuName;
    u16* lpszClassName;
    HICON hIconSm;
} WNDCLASSEXW;
typedef struct {
    HWND hwnd;
    u32 message;
    u64 wParam;
    s64 lParam;
    u32 time;
    POINT pt;
    u32 lPrivate;
} MSG;

s32 SetProcessDPIAware(void);
HICON LoadIconW(HINSTANCE, u16*);
HCURSOR LoadCursorW(HINSTANCE, u16*);
u16 RegisterClassExW(WNDCLASSEXW*);
HWND CreateWindowExW(u32, u16*, u16*, u32, s32, s32, s32, s32, HWND, HMENU, HINSTANCE, void*);
s32 PeekMessageW(MSG*, HWND, u32, u32, u32);
s32 TranslateMessage(MSG*);
s64 DispatchMessageW(MSG*);
HDC GetDC(HWND);
s64 DefWindowProcW(HWND, u32, u64, s64);
void PostQuitMessage(s32);
s32 DestroyWindow(HWND);

/* dwmapi */
typedef enum {
    DWMWA_USE_IMMERSIVE_DARK_MODE = 20,
    DWMWA_WINDOW_CORNER_PREFERENCE = 33,
} DWMWINDOWATTRIBUTE;
typedef enum {
    DWMWCP_DEFAULT = 0,
    DWMWCP_DONOTROUND = 1,
    DWMWCP_ROUND = 2,
    DWMWCP_ROUNDSMALL = 3,
} DWM_WINDOW_CORNER_PREFERENCE;

u32 DwmSetWindowAttribute(HWND, u32, void*, u32);

/* winmm */
u32 timeBeginPeriod(u32);

static HINSTANCE platform_hinstance;
static HWND platform_hwnd;
static HDC platform_hdc;
static u16 platform_screen_width;
static u16 platform_screen_height;

#if RENDERER_OPENGL
#include "opengl.c"
#endif

static void update_cursor_clip(void) {

}

static void toggle_fullscreen(void) {

}

static s64 window_proc(HWND hwnd, u32 message, u64 wParam, s64 lParam) {
    switch (message) {
    case WM_ACTIVATEAPP: {
        if (wParam != 0) update_cursor_clip();
        return 0;
    }
    case WM_SIZE: {
        platform_screen_width = cast(u16) cast(u64) lParam;
        platform_screen_height = cast(u16) (cast(u64) lParam >> 16);

        opengl_resize();
        return 0;
    }
    case WM_CREATE: {
        platform_hwnd = hwnd;
        platform_hdc = GetDC(hwnd);

        u32 dark_mode = 1;
        DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark_mode, 4);
        u32 round_mode = DWMWCP_DONOTROUND;
        DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &round_mode, 4);

        opengl_init();
        return 0;
    }
    case WM_DESTROY: {
        opengl_deinit();

        PostQuitMessage(0);
        return 0;
    }
    default: {
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }
    }
}

void WinMainCRTStartup(void) {
    platform_hinstance = GetModuleHandleW(null);

    bool sleep_is_granular = timeBeginPeriod(1) == 0;

    SetProcessDPIAware();
    WNDCLASSEXW wndclass = {0};
    wndclass.cbSize = size_of(WNDCLASSEXW);
    wndclass.style = CS_OWNDC;
    wndclass.lpfnWndProc = window_proc;
    wndclass.hInstance = platform_hinstance;
    wndclass.hIcon = LoadIconW(null, IDI_WARNING);
    wndclass.hCursor = LoadCursorW(null, IDC_CROSS);
    wndclass.lpszClassName = L"A";
    RegisterClassExW(&wndclass);
    CreateWindowExW(0, wndclass.lpszClassName, L"CAVEMAN",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        null, null, platform_hinstance, null);

    for (;;) {
        MSG msg;
        while (PeekMessageW(&msg, null, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            switch (msg.message) {
            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP: {
                bool pressed = (cast(u64) msg.lParam & cast(u64) 1 << 31) == 0;
                bool repeat = pressed && (cast(u64) msg.lParam & cast(u64) 1 << 30) != 0;
                bool sys = msg.message == WM_SYSKEYDOWN || msg.message == WM_SYSKEYUP;
                bool alt = sys && (cast(u64) msg.lParam & cast(u64) 1 << 29) != 0;

                if (!repeat && (!sys || alt || msg.wParam == VK_F10)) {
                    if (pressed) {
                        if (msg.wParam == VK_F4 && alt) DestroyWindow(platform_hwnd);
                        if (DEVELOPER && msg.wParam == VK_ESCAPE) DestroyWindow(platform_hwnd);
                        if (msg.wParam == VK_F11 || (msg.wParam == VK_RETURN && alt)) toggle_fullscreen();
                    }
                }
                break;
            }
            case WM_QUIT: {
                goto game_loop_end;
            }
            default: {
                DispatchMessageW(&msg);
            }
            }
        }

        opengl_present();

        if (sleep_is_granular) {
            Sleep(1);
        }
    }
game_loop_end:

    ExitProcess(0);
}
