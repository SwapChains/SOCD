#include <windows.h>

#define DIRECTION_LEFT 0
#define DIRECTION_RIGHT 1
#define DIRECTION_UP 2
#define DIRECTION_DOWN 3

#define IS_DOWN 1
#define IS_UP 0

#define KEY_UP 0x57
#define KEY_LEFT 0x41
#define KEY_DOWN 0x53
#define KEY_RIGHT 0x44

int real[4] = { IS_UP , IS_UP , IS_UP , IS_UP };
int virtual[4] = { IS_UP , IS_UP , IS_UP , IS_UP };

int GetOpposingKey(int key) 
{
    switch (key)
    {
    case KEY_LEFT:
        return KEY_RIGHT;
    case KEY_RIGHT:
        return KEY_LEFT;
    case KEY_UP:
        return KEY_DOWN;
    case KEY_DOWN:
        return KEY_UP;
    default:
        return -1;
    }
}

int GetIndex(int key) 
{
    switch (key)
    {
    case KEY_LEFT:
        return DIRECTION_LEFT;
    case KEY_RIGHT:
        return DIRECTION_RIGHT;
    case KEY_UP:
        return DIRECTION_UP;
    case KEY_DOWN:
        return DIRECTION_DOWN;
    default:
        return -1;
    };
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) 
{
    KBDLLHOOKSTRUCT* kbInput = (KBDLLHOOKSTRUCT*)lParam;

    if (nCode != HC_ACTION || kbInput->flags & LLKHF_INJECTED) 
    {
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }

    int key = kbInput->vkCode;
    int opposing = GetOpposingKey(key);

    if (opposing < 0) 
    {
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }

    int index = GetIndex(key);
    int opposing_index = GetIndex(opposing);

    opposing = MapVirtualKeyW(opposing, MAPVK_VK_TO_VSC_EX);

    INPUT input;
    input.type = INPUT_KEYBOARD;

    if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) 
    {
        real[index] = IS_DOWN;
        virtual[index] = IS_DOWN;
        if (real[opposing_index] == IS_DOWN && virtual[opposing_index] == IS_DOWN) 
        {
            input.ki = (KEYBDINPUT){ 0, opposing, KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE, 0, 0 };
            SendInput(1, &input, sizeof(INPUT));
            virtual[opposing_index] = IS_UP;
        }
    }
    else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) 
    {
        real[index] = IS_UP;
        virtual[index] = IS_UP;
        if (real[opposing_index] == IS_DOWN) 
        {
            input.ki = (KEYBDINPUT){ 0, opposing, KEYEVENTF_SCANCODE, 0, 0 };
            SendInput(1, &input, sizeof(INPUT));
            virtual[opposing_index] = IS_DOWN;
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    HHOOK hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, (HINSTANCE)GetModuleHandle(NULL), 0);
    if (hHook == NULL) 
    {
        return 1;
    }

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) 
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hHook);
    return 0;
}
