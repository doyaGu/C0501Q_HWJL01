#include <pthread.h>
#include "ite/itp.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

extern void BootInit(void);
extern void* BootloaderMain(void* arg);

static HWND hWnd;

static LRESULT CALLBACK
WndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wparam,
    LPARAM lparam)
{
    switch (msg)
    {
    case WM_KEYDOWN:
        switch (wparam)
        {
        case VK_ESCAPE:
            DestroyWindow(hwnd);
            return 0;

        case VK_UP:
        case VK_DOWN:
        case VK_RETURN:
        default:
            break;
        }
        break;

    case WM_CREATE:
        break;

    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

int main(void)
{
    WNDCLASS wc;
    MSG msg;
    pthread_t task;
    pthread_attr_t attr;

	// create window
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "ITE";

    if (RegisterClass(&wc) == FALSE)
		return -1;

    hWnd = CreateWindow("ITE",
                        "ITE APPLICATION", // Window name
                        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        240,
                        320,
                        NULL,
                        NULL,
                        wc.hInstance,
                        NULL);
    if (hWnd == NULL)
		return -1;

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    // boot init
    BootInit();

	// create test task	
	pthread_attr_init(&attr);
    pthread_create(&task, &attr, BootloaderMain, NULL);

	// message loop
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}
