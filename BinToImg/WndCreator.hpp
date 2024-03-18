#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <memory>

#if __cplusplus < 201103L
    #ifndef _MSC_VER
        #error "C++11 or a later version is required for std::shared_ptr"
    #endif
#endif

#ifdef _MSC_VER
__pragma(warning(disable:6387))
#pragma comment(lib, "gdi32.lib")
#endif

#define SafeReleaseDC(Wnd, DC)  if((DC != NULL && DC != INVALID_HANDLE_VALUE) && Wnd != NULL) { ReleaseDC(Wnd, DC); }
#define SafeDeleteDC(DC)  if(DC != NULL && DC != INVALID_HANDLE_VALUE) { DeleteDC(DC); }
#define SafeDeleteObject(Obj)  if(Obj != NULL && Obj != INVALID_HANDLE_VALUE) { DeleteObject(Obj); }
#define SafeDestroyWindow(Wnd)  if(Wnd != NULL) { DestroyWindow(Wnd); }
#define SafeDeleteIcon(Ico)  if(Ico != NULL && Ico != INVALID_HANDLE_VALUE) { DestroyIcon(Ico); }

#ifdef _MSC_VER
__pragma(warning(default:6387))
#endif

#ifdef UNICODE
#define WndCreator WndCreatorW
#else
#define WndCreator WndCreatorA
#endif // UNICODE

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define CurrHinst ((HINSTANCE)&__ImageBase)

void(__fastcall * WndCtrlEvntProcessor)() = nullptr;

enum WndExModes : DWORD
{
    FullScreenEx = WS_EX_TOPMOST,
    BorderLessEx = 0,
    WindowedEx = 0,
    ChildEx = 0,
    Layered = WS_EX_LAYERED,
    NoTaskBarAndSmallBorder = WS_EX_TOOLWINDOW
};

enum WndModes : DWORD
{
    FullScreen = WS_POPUP | WS_VISIBLE,
    BorderLess = WS_POPUP | WS_VISIBLE,
    Windowed = WS_OVERLAPPED | WS_THICKFRAME | WS_VISIBLE | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MAXIMIZE,
    Child =  WS_CHILD | WS_VISIBLE,
    ClipChildren = WS_CLIPCHILDREN
};

enum ButtonStyles : DWORD
{
    RadioButton = BS_RADIOBUTTON,
    AutoRadio = BS_AUTORADIOBUTTON,
    PushButton = BS_PUSHBUTTON,
    New = BS_FLAT,
    Flat = BS_FLAT,
    ThreeState = BS_AUTO3STATE,
};

struct ScreenDimensions
{
    LONG Width, Height;
};

namespace ControlColors
{
    COLORREF ButtonBgClr = RGB(255, 255, 255);
    COLORREF ButtonTextClr = RGB(255, 1, 1);
}

LRESULT CALLBACK WindowProcA(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_DESTROY: // called when DestroyWindow is called
        {
            PostQuitMessage(0);
            break; // calls DefWindowProcW: which will call WM_QUIT
        }
        case WM_CLOSE: // called when user clicks x button or alt f4
        {
            PostQuitMessage(0);
            break; // calls DefWindowProcW: which will call Destroy Window 
        }
        case WM_QUIT: // closes window
        {
            PostQuitMessage(0);
            break;
        }
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WindowProcW(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_COMMAND:
        {
            if (LOWORD(wParam) == 201 && HIWORD(wParam) == BN_CLICKED)
            {
                MessageBoxW(hwnd, L"Thanks! I Needed That!", L"AAAhhhh!", MB_OK);
            }
            break;
        }
        case WM_DESTROY: // called when DestroyWindow is called
        {
            PostQuitMessage(0);
            break; // calls DefWindowProcW: which will call WM_QUIT
        }
        case WM_CLOSE: // called when user clicks x button or alt f4
        {
            PostQuitMessage(0);
            break; // calls DefWindowProcW: which will call Destroy Window 
        }
        case WM_QUIT: // closes window
        {
            PostQuitMessage(0);
            break;
        }
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

class WndIconW
{
private:
    std::shared_ptr<HICON> Ico = nullptr;
public:
    HINSTANCE Hinst = 0;
    LPCWSTR IconName = nullptr;

    WndIconW(const LPCWSTR IconName, const HINSTANCE Inst = NULL)
    {
        this->IconName = IconName;
        this->Hinst = Inst;
        this->Ico = std::make_shared<HICON>(LoadIconW(Inst, IconName));

        if (!this->Ico)
        {
            // TODO Log error
        }
    }

    WndIconW(const int ResourceID, const HINSTANCE Inst = NULL)
    {
        this->IconName = MAKEINTRESOURCEW(ResourceID);
        this->Hinst = Inst;
        this->Ico = std::make_shared<HICON>(LoadIconW(Inst, MAKEINTRESOURCEW(ResourceID)));

        if (!this->Ico || !*this->Ico)
        {
            // TODO Log error
            throw 1;
        }
    }

    ~WndIconW()
    {
        SafeDeleteIcon(*this->Ico);
    }

    bool Load(const LPCWSTR IcoName, const HINSTANCE Inst)
    {
        //if (NeedsDeleted)
        //{
        //    SafeDeleteIcon(*this->Ico);
        //}

        this->IconName = IcoName;
        this->Hinst = Inst;
        this->Ico = std::make_shared<HICON>(LoadIconW(Inst, IcoName));

        if (!*this->Ico)
        {
            return false;
        }
        return true;
    }

    bool Load(const int ResourceID, const HINSTANCE Inst)
    {
        //if (NeedsDeleted)
        //{
        //    SafeDeleteIcon(*this->Ico);
        //}

        this->IconName = MAKEINTRESOURCEW(ResourceID);
        this->Hinst = Inst;
        this->Ico = std::make_shared<HICON>(LoadIconW(Inst, MAKEINTRESOURCEW(ResourceID)));

        if (!*this->Ico)
        {
            return false;
        }
        return true;
    }

    HICON GetHICON()
    {
        return *this->Ico;
    }

    std::shared_ptr<HICON> GetSharedPointer()
    {
        return this->Ico;
    }
};

class WndIconA
{
    private:
    std::shared_ptr<HICON> Ico = nullptr;
    public:
    HINSTANCE Hinst = 0;
    LPCSTR IconName = nullptr;

    WndIconA(const LPCSTR IconName, const HINSTANCE Inst = NULL)
    {
        this->IconName = IconName;
        this->Hinst = Inst;
        this->Ico = std::make_shared<HICON>(LoadIconA(Inst, IconName));

        if (!this->Ico)
        {
            // TODO Log error
        }
    }

    WndIconA(const int ResourceID, const HINSTANCE Inst = NULL)
    {
        this->IconName = MAKEINTRESOURCEA(ResourceID);
        this->Hinst = Inst;
        this->Ico = std::make_shared<HICON>(LoadIconA(Inst, MAKEINTRESOURCEA(ResourceID)));

        if (!this->Ico || !*this->Ico)
        {
            // TODO Log error
            throw 1;
        }
    }

    ~WndIconA()
    {
        SafeDeleteIcon(*this->Ico);
    }

    bool Load(const LPCSTR IcoName, const HINSTANCE Inst)
    {
        //if (NeedsDeleted)
        //{
        //    SafeDeleteIcon(*this->Ico);
        //}

        this->IconName = IcoName;
        this->Hinst = Inst;
        this->Ico = std::make_shared<HICON>(LoadIconA(Inst, IcoName));

        if (!*this->Ico)
        {
            return false;
        }
        return true;
    }

    bool Load(const int ResourceID, const HINSTANCE Inst)
    {
        //if (NeedsDeleted)
        //{
        //    SafeDeleteIcon(*this->Ico);
        //}

        this->IconName = MAKEINTRESOURCEA(ResourceID);
        this->Hinst = Inst;
        this->Ico = std::make_shared<HICON>(LoadIconA(Inst, MAKEINTRESOURCEA(ResourceID)));

        if (!*this->Ico)
        {
            return false;
        }
        return true;
    }

    HICON GetHICON()
    {
        return *this->Ico;
    }

    std::shared_ptr<HICON> GetSharedPointer()
    {
        return this->Ico;
    }
};


class WndCreatorA
{
    public:
    WndCreatorA()
    {
        if (!DidCreate)
        {
            wc = {};
            DidCreate = false;
            Wnd = NULL;
            ErrorHandler = LogError;
        }
        else
        {
            this->~WndCreatorA();
        }
    }

    WndCreatorA(const UINT ClassStyle, const std::string_view ClassName, const std::string_view MenuName, const HCURSOR Curs, const HICON Ico, const HBRUSH BackGround, const DWORD ExFlags, const DWORD WStyle, const int x, const int y, const int Width, const int Height, HWND HwndParent = 0, const HINSTANCE hInstance = GetModuleHandleW(NULL), WNDPROC WndProc = WindowProcA)
    {
        SecureZeroMemory(&wc, sizeof(WNDCLASSEXA));

        wc = { sizeof(WNDCLASSEXA), ClassStyle, WndProc, 0L, 0L, hInstance, Ico, Curs, BackGround, MenuName.data(), ClassName.data(), NULL };

        if (!RegisterClassExA(&wc))
        {
            if (GetLastError() != 1410)
            {
                ErrorHandler("[WndCreator] Failed To Register Window Class: " + std::to_string(GetLastError()));
            }
            return;
        }

        this->Wnd = CreateWindowExA(ExFlags, ClassName.data(), MenuName.data(), WStyle, x, y, Width, Height, HwndParent, 0, hInstance, 0);

        if (!this->Wnd)
        {
            SafeDeleteObject(wc.hbrBackground);
            UnregisterClassA(wc.lpszClassName, wc.hInstance);
            ErrorHandler("[WndCreator] Failed To Create Window: " + std::to_string(GetLastError()));
            return;
        }

        this->DidCreate = true;
    }

    WndCreatorA(const HWND Wnd)
    {
        this->Wnd = Wnd;

        this->DidCreate = false;
    }

    ~WndCreatorA()
    {
        if (this->DidCreate)
        {
            SafeDestroyWindow(Wnd);
            SafeDeleteObject(wc.hbrBackground);
            UnregisterClassA(wc.lpszClassName, wc.hInstance);
        }
    }

    static void LogError(std::string ErrorMsg)
    {
        MessageBoxA(NULL, ErrorMsg.c_str(), "", MB_OK);
    }

    // Copying is not allowed
    WndCreatorA(const WndCreatorA&) = default;
    WndCreatorA& operator=(const WndCreatorA&) = default;

    // Moving is allowed
    WndCreatorA(WndCreatorA&& Rhs) noexcept
    {
        this->wc = Rhs.wc;
        this->DidCreate = Rhs.DidCreate;
        this->Wnd = Rhs.Wnd;
        this->ErrorHandler = Rhs.ErrorHandler;
        this->Children = Rhs.Children;

        Rhs.Wnd = NULL;
    }

    WndCreatorA& operator=(WndCreatorA&& Rhs) noexcept
    {
        this->wc = Rhs.wc;
        this->DidCreate = Rhs.DidCreate;
        this->Wnd = Rhs.Wnd;
        this->ErrorHandler = Rhs.ErrorHandler;
        this->Children = Rhs.Children;

        Rhs.Wnd = NULL;

        return *this;
    }

    private:
    WNDCLASSEXA wc = {};
    bool DidCreate = false;
    public:
    HWND Wnd = NULL;
    void(*ErrorHandler)(std::string) = LogError;
    std::vector<HWND> Children = {};

    const bool Hide()
    {
        if (!this->Wnd)
        {
            return false;
        }

        return ShowWindow(this->Wnd, SW_HIDE);
    }

    const bool Show()
    {
        if (!this->Wnd)
        {
            return false;
        }

        return ShowWindow(this->Wnd, SW_SHOW);
    }

    const bool Maximize()
    {
        if (!this->Wnd)
        {
            return false;
        }

        return ShowWindow(this->Wnd, SW_MAXIMIZE);
    }

    const bool Minimize()
    {
        if (!this->Wnd)
        {
            return false;
        }

        return ShowWindow(this->Wnd, SW_MINIMIZE);
    }

    const bool AddStyleFlags(const LONG_PTR StyleToAdd)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        LONG_PTR Style = GetWindowLongPtrA(this->Wnd, GWL_STYLE);

        if (!Style)
        {
            ErrorHandler("[WndCreator] Error Retrieving Window Style: " + std::to_string(GetLastError()));
            return false;
        }

        Hide();
        Style |= StyleToAdd;
        if (!SetWindowLongPtrA(this->Wnd, GWL_STYLE, Style))
        {
            ErrorHandler("[WndCreator] Error Setting Window Style: " + std::to_string(GetLastError()));
            return false;
        }
        Show();

        return true;
    }

    const bool AddStyleFlagsEx(const LONG_PTR ExStyle)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        LONG_PTR Style = GetWindowLongPtrA(this->Wnd, GWL_EXSTYLE);

        if (!Style)
        {
            ErrorHandler("[WndCreator] Error Retrieving Window Style: " + std::to_string(GetLastError()));
            return false;
        }

        Hide();

        Style |= ExStyle;

        if (!SetWindowLongPtrA(this->Wnd, GWL_EXSTYLE, Style))
        {
            ErrorHandler("[WndCreator] Error Setting Window Style: " + std::to_string(GetLastError()));
            return false;
        }

        Show();

        return true;
    }

    const bool SubStyleFlags(const LONG_PTR StyleToSub)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        LONG_PTR Style = GetWindowLongPtrA(this->Wnd, GWL_STYLE);

        if (!Style)
        {
            ErrorHandler("[WndCreator] Error Retrieving Window Style: " + std::to_string(GetLastError()));
            return false;
        }

        Style &= ~(StyleToSub);

        Hide();

        if (!SetWindowLongPtrA(this->Wnd, GWL_STYLE, Style))
        {
            ErrorHandler("[WndCreator] Error Setting Window Style: " + std::to_string(GetLastError()));
            return false;
        }

        Show();

        return true;
    }

    const bool SubStyleFlagsEx(const LONG_PTR ExStyle)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        LONG_PTR Style = GetWindowLongPtrA(this->Wnd, GWL_EXSTYLE);

        if (!Style)
        {
            ErrorHandler("[WndCreator] Error Retrieving Window Style: " + std::to_string(GetLastError()));
            return false;
        }

        Style &= ~(ExStyle);

        this->Hide();
        if (!SetWindowLongPtrA(this->Wnd, GWL_EXSTYLE, Style))
        {
            ErrorHandler("[WndCreator] Error Setting Window Style: " + std::to_string(GetLastError()));
            return false;
        }
        this->Show();

        return true;
    }

    const bool ResetStyle(const LONG_PTR NewStyle)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        this->Hide();
        SetLastError(0);
        if (!SetWindowLongPtrA(this->Wnd, GWL_STYLE, NewStyle) && GetLastError() != 0)
        {
            std::string err = std::to_string(GetLastError());
            ErrorHandler(std::string("[WndCreator] Error Setting Window Ex Style: (" + err + ")"));
            return false;
        }
        this->Show();

        return true;
    }

    const bool ResetStyleEx(const LONG_PTR NewExStyle)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        this->Hide();
        SetLastError(0);
        if (!SetWindowLongPtrA(this->Wnd, GWL_EXSTYLE, NewExStyle) && GetLastError() != 0)
        {
            std::string err = std::to_string(GetLastError());
            ErrorHandler(std::string("[WndCreator] Error Setting Window Ex Style: (" + err + ")"));
            return false;
        }
        this->Show();

        return true;
    }

    const ScreenDimensions GetClientArea()
    {
        RECT rect;
        if (GetClientRect(this->Wnd, &rect))
        {
            return{ rect.right, rect.bottom };
        }
        else
        {
            return { 0, 0 };
        }
    }

    const bool SetWndSz(const HWND WndZPos, const int X, const int Y, const int Width, const int Height, const UINT SwFlags)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        if (!SetWindowPos(this->Wnd, WndZPos, X, Y, Width, Height, SwFlags))
        {
            ErrorHandler("[WndCreator] Error with SetWindowPos: " + std::to_string(GetLastError()));
            return false;
        }

        return true;
    }

    const bool SetWndTitle(const std::string_view Str)
    {
        return SetWindowTextA(this->Wnd, Str.data());
    }

    const bool HasFocus()
    {
        if (GetFocus() == this->Wnd)
        {
            return true;
        }

        return false;
    }

    const bool SetLayeredAttributes(const COLORREF CrKey, const BYTE Alpha, const DWORD DwFlags = LWA_COLORKEY)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        if (!SetLayeredWindowAttributes(Wnd, CrKey, Alpha, DwFlags))
        {
            ErrorHandler("[WndCreator] Failed to set layered attributes: " + std::to_string(GetLastError()));
            return false;
        }

        return true;
    }

    const LRESULT SendWndMessage(const UINT MSG, WPARAM WParam, LPARAM LParam)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return -1;
        }

        return SendMessageA(this->Wnd, MSG, WParam, LParam);
    }

    void CreateChildWindow(const DWORD ExFlags, const DWORD WStyle, const std::string_view ClassName, const std::string_view WndName, const int x, const int y, const int Width, const int Height, const HMENU Hmenu)
    {
        HWND hwndButton = CreateWindowExA(ExFlags, ClassName.data(), WndName.data(), WStyle, x, y, Width, Height, this->Wnd, Hmenu, (HINSTANCE)GetWindowLongPtrW(this->Wnd, GWLP_HINSTANCE), NULL);

        this->Children.push_back(hwndButton);
    }

    void CreateButton(const DWORD ButtonStyle, std::string_view ButtonText, const int x, const int y, const int Width, const int Height, const DWORD ID)
    {
        this->CreateChildWindow(WndExModes::ChildEx, WndModes::Child | WndModes::ClipChildren | ButtonStyle, "BUTTON", ButtonText, x, y, Width, Height, (HMENU)ID);
    }

    void UpdateChildren()
    {
        for (HWND W : this->Children)
        {
            UpdateWindow(W);
        }
    }

    const void Destroy()
    {
        SafeDestroyWindow(this->Wnd);
    }
};

class WndCreatorW
{
public:
    WndCreatorW()
    {
        if (!DidCreate)
        {
            wc = {};
            DidCreate = false;
            Wnd = NULL;
            ErrorHandler = LogError;
        }
        else
        {
            this->~WndCreatorW();
        }
    }

    WndCreatorW(const UINT ClassStyle, const std::wstring_view ClassName, const std::wstring_view WindowName, const HCURSOR Curs, const HICON Ico, const HBRUSH BackGround, const DWORD ExFlags, const DWORD WStyle, const int x, const int y, const int Width, const int Height, HWND HwndParent = 0, const HINSTANCE hInstance = GetModuleHandleW(NULL), WNDPROC WndProc = WindowProcW)
    {
        SecureZeroMemory(&wc, sizeof(WNDCLASSEXW));

        wc = { sizeof(WNDCLASSEXW), ClassStyle, WndProc, 0L, 0L, hInstance, Ico, Curs, BackGround, NULL, ClassName.data(), NULL };

        if (!RegisterClassExW(&wc))
        {
            if (GetLastError() != 1410)
            {
                ErrorHandler("[WndCreator] Failed To Register Window Class: " + std::to_string(GetLastError()));
            }
            return;
        }

        this->Wnd = CreateWindowExW(ExFlags, ClassName.data(), WindowName.data(), WStyle, x, y, Width, Height, HwndParent, 0, hInstance, 0);

        if (!this->Wnd)
        {
            SafeDeleteObject(wc.hbrBackground);
            UnregisterClassW(wc.lpszClassName, wc.hInstance);
            ErrorHandler("[WndCreator] Failed To Create Window: " + std::to_string(GetLastError()));
            return;
        }

        this->DidCreate = true;
    }

    WndCreatorW(const HWND Wnd)
    {
        this->Wnd = Wnd;

        this->DidCreate = false;
    }

    ~WndCreatorW()
    {
        if (this->DidCreate)
        {
            SafeDestroyWindow(Wnd);
            SafeDeleteObject(wc.hbrBackground);
            UnregisterClassW(wc.lpszClassName, wc.hInstance);
        }
    }

    static void LogError(std::string ErrorMsg)
    {
        MessageBoxA(NULL, ErrorMsg.c_str(), "", MB_OK);
    }

    // Copying is not allowed
    WndCreatorW(const WndCreatorW&) = default;
    WndCreatorW& operator=(const WndCreatorW&) = default;

    // Moving is allowed
    WndCreatorW(WndCreatorW&& Rhs) noexcept
    {
        this->wc = Rhs.wc;
        this->DidCreate = Rhs.DidCreate;
        this->Wnd = Rhs.Wnd;
        this->ErrorHandler = Rhs.ErrorHandler;
        this->Children = Rhs.Children;

        Rhs.Wnd = NULL;
    }

    WndCreatorW& operator=(WndCreatorW&& Rhs) noexcept
    {
        this->wc = Rhs.wc;
        this->DidCreate = Rhs.DidCreate;
        this->Wnd = Rhs.Wnd;
        this->ErrorHandler = Rhs.ErrorHandler;
        this->Children = Rhs.Children;

        Rhs.Wnd = NULL;

        return *this;
    }

private:
    WNDCLASSEXW wc = {};
    bool DidCreate = false;
public:
    HWND Wnd = NULL;
    void(*ErrorHandler)(std::string) = LogError;
    std::vector<HWND> Children = {};

    const bool Hide()
    {
        if (!this->Wnd)
        {
            return false;
        }

        return ShowWindow(this->Wnd, SW_HIDE);
    }

    const bool Show()
    {
        if (!this->Wnd)
        {
            return false;
        }

        return ShowWindow(this->Wnd, SW_SHOW);
    }

    const bool Maximize()
    {
        if (!this->Wnd)
        {
            return false;
        }

        return ShowWindow(this->Wnd, SW_MAXIMIZE);
    }

    const bool Minimize()
    {
        if (!this->Wnd)
        {
            return false;
        }

        return ShowWindow(this->Wnd, SW_MINIMIZE);
    }

    const bool AddStyleFlags(const LONG_PTR StyleToAdd)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        LONG_PTR Style = GetWindowLongPtrW(this->Wnd, GWL_STYLE);

        if (!Style)
        {
            ErrorHandler("[WndCreator] Error Retrieving Window Style: " + std::to_string(GetLastError()));
            return false;
        }

        Hide();
        Style |= StyleToAdd;
        if (!SetWindowLongPtrW(this->Wnd, GWL_STYLE, Style))
        {
            ErrorHandler("[WndCreator] Error Setting Window Style: " + std::to_string(GetLastError()));
            return false;
        }
        Show();

        return true;
    }

    const bool AddStyleFlagsEx(const LONG_PTR ExStyle)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        LONG_PTR Style = GetWindowLongPtrW(this->Wnd, GWL_EXSTYLE);

        if (!Style)
        {
            ErrorHandler("[WndCreator] Error Retrieving Window Style: " + std::to_string(GetLastError()));
            return false;
        }

        Hide();

        Style |= ExStyle;

        if (!SetWindowLongPtrW(this->Wnd, GWL_EXSTYLE, Style))
        {
            ErrorHandler("[WndCreator] Error Setting Window Style: " + std::to_string(GetLastError()));
            return false;
        }

        Show();

        return true;
    }

    const bool SubStyleFlags(const LONG_PTR StyleToSub)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        LONG_PTR Style = GetWindowLongPtrW(this->Wnd, GWL_STYLE);

        if (!Style)
        {
            ErrorHandler("[WndCreator] Error Retrieving Window Style: " + std::to_string(GetLastError()));
            return false;
        }

        Style &= ~(StyleToSub);

        Hide();

        if (!SetWindowLongPtrW(this->Wnd, GWL_STYLE, Style))
        {
            ErrorHandler("[WndCreator] Error Setting Window Style: " + std::to_string(GetLastError()));
            return false;
        }

        Show();

        return true;
    }

    const bool SubStyleFlagsEx(const LONG_PTR ExStyle)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        LONG_PTR Style = GetWindowLongPtrW(this->Wnd, GWL_EXSTYLE);

        if (!Style)
        {
            ErrorHandler("[WndCreator] Error Retrieving Window Style: " + std::to_string(GetLastError()));
            return false;
        }

        Style &= ~(ExStyle);

        this->Hide();
        if (!SetWindowLongPtrW(this->Wnd, GWL_EXSTYLE, Style))
        {
            ErrorHandler("[WndCreator] Error Setting Window Style: " + std::to_string(GetLastError()));
            return false;
        }
        this->Show();

        return true;
    }

    const bool ResetStyle(const LONG_PTR NewStyle)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        this->Hide();
        SetLastError(0);
        if (!SetWindowLongPtrW(this->Wnd, GWL_STYLE, NewStyle) && GetLastError() != 0)
        {
            std::string err = std::to_string(GetLastError());
            ErrorHandler(std::string("[WndCreator] Error Setting Window Ex Style: (" + err + ")"));
            return false;
        }
        this->Show();

        return true;
    }

    const bool ResetStyleEx(const LONG_PTR NewExStyle)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        this->Hide();
        SetLastError(0);
        if (!SetWindowLongPtrW(this->Wnd, GWL_EXSTYLE, NewExStyle) && GetLastError() != 0)
        {
            std::string err = std::to_string(GetLastError());
            ErrorHandler(std::string("[WndCreator] Error Setting Window Ex Style: (" + err + ")"));
            return false;
        }
        this->Show();

        return true;
    }

    const ScreenDimensions GetClientArea()
    {
        RECT rect;
        if (GetClientRect(this->Wnd, &rect))
        {
            return{ rect.right, rect.bottom};
        }
        else
        {
            return { 0, 0 };
        }
    }

    const bool SetWndSz(const HWND WndZPos, const int X, const int Y, const int Width, const int Height, const UINT SwFlags)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        if (!SetWindowPos(this->Wnd, WndZPos, X, Y, Width, Height, SwFlags))
        {
            ErrorHandler("[WndCreator] Error with SetWindowPos: " + std::to_string(GetLastError()));
            return false;
        }

        return true;
    }

    const bool SetWndTitle(const std::wstring& Str)
    {
        return SetWindowTextW(this->Wnd, Str.c_str());
    }

    const bool HasFocus()
    {
        if (GetFocus() == this->Wnd)
        {
            return true;
        }

        return false;
    }

    const bool SetLayeredAttributes(const COLORREF CrKey, const BYTE Alpha, const DWORD DwFlags = LWA_COLORKEY)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        if (!SetLayeredWindowAttributes(Wnd, CrKey, Alpha, DwFlags))
        {
            ErrorHandler("[WndCreator] Failed to set layered attributes: " + std::to_string(GetLastError()));
            return false;
        }

        return true;
    }
     
    const LRESULT SendWndMessage(const UINT MSG, WPARAM WParam, LPARAM LParam)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return -1;
        }

        return SendMessageW(this->Wnd, MSG, WParam, LParam);
    }

    void CreateChildWindow(const DWORD ExFlags, const DWORD WStyle, const std::wstring_view ClassName, const std::wstring_view WndName, const int x, const int y, const int Width, const int Height, const HMENU Hmenu)
    {
        HWND hwndButton = CreateWindowExW(ExFlags, ClassName.data(), WndName.data(), WStyle, x, y, Width, Height, this->Wnd,Hmenu, (HINSTANCE)GetWindowLongPtrW(this->Wnd, GWLP_HINSTANCE), NULL);
        this->Children.push_back(hwndButton);
    }

    void CreateButton(const DWORD ButtonStyle, const std::wstring_view ButtonText, const int x, const int y, const int Width, const int Height, const HMENU ID)
    {
        this->CreateChildWindow(WndExModes::ChildEx, WndModes::Child | WndModes::ClipChildren | ButtonStyle, L"BUTTON", ButtonText.data(), x, y, Width, Height, ID);
    }

    void UpdateChildren()
    {
        for (HWND W : this->Children)
        {
            UpdateWindow(W);
        }
    }

    const void Destroy()
    {
        SafeDestroyWindow(this->Wnd);
    }
};
