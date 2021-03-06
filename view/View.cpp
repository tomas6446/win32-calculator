#include "View.h"

#define BUTTON_WIDTH 50
#define BUTTON_HEIGHT 50
#define BUTTON_TEXT_HEIGHT 30
#define BUTTON_TEXT_WIDTH 12

#define WINDOW_WIDTH 220
#define WINDOW_HEIGHT 350

#define TEXT_HEIGHT 23
#define TEXT_WIDTH 12

#define Button(name, num, x, y) CreateWindow("BUTTON", (name), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, \
(x), (y), BUTTON_WIDTH, BUTTON_HEIGHT, hWnd, (HMENU) (num), hInstance, nullptr)

Observer *View::observer{};
HWND View::textField{};
HWND View::hWnd{};
HINSTANCE View::hInstance{};

HFONT View::hFont;
LOGFONT View::font;

const char *View::character[20] = {
        "0", "1", "2", "3", "4",
        "5", "6", "7", "8", "9",
        ".", "(", ")", "/", "*",
        "-", "+", "=", "C", "X"
};
std::pair<int, int> View::pos[20] = {
        {60,  280},
        {10,  230},
        {60,  230},
        {110, 230},
        {10,  180},
        {60,  180},
        {110, 180},
        {10,  130},
        {60,  130},
        {110, 130},
        {10,  80},
        {60,  80},
        {110, 80},
        {160, 80},
        {160, 130},
        {160, 180},
        {160, 230},
        {160, 280},
        {10,  280},
        {110, 280},
};

View::View() {
    std::cout << "Creating Window\n";
    HINSTANCE hInst(GetModuleHandle(nullptr));

    hInstance = hInst;
    this->wndClass.lpszClassName = "Window Class";
    this->wndClass.hInstance = hInstance;
    this->wndClass.hbrBackground = (HBRUSH) (COLOR_WINDOW + 3);
    this->wndClass.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
    this->wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    this->wndClass.lpfnWndProc = windowProcess;

    if (!RegisterClass(&this->wndClass)) {
        std::cout << "'Class View' failed to create window" << std::endl;
        MessageBox(nullptr, TEXT("This program requires Windows NT!"),
                   "Window Class", MB_ICONERROR);
    }

    createWindow();
    createButtons();
    createTextField();
}

View::~View() {
    UnregisterClass("Window Class", this->hInstance);
    delete observer;
    delete hInstance;
    delete textField;
    delete hWnd;
}

void View::setObserver(Observer *obs) {
    observer = obs;
}

void View::Render() {
    while (true) {
        if (this->processMessage()) {
            break;
        }
    }
}

bool View::processMessage() {
    MSG msg = {};

    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_QUIT) {
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return true;
}

void View::colorBackground(WPARAM wparam) {
    SetTextColor((HDC) wparam, RGB(255, 255, 255));
    SetBkMode((HDC) wparam, TRANSPARENT);
}

void View::colorButton(WPARAM wparam, LPARAM lparam) {
    LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT) lparam;

    // Button
    if (wparam >= 10 && wparam <= 12) {
        SetDCBrushColor(lpDIS->hDC, RGB(128, 128, 128)); // silver grey
    } else if (wparam >= 13 && wparam <= 17) {
        SetDCBrushColor(lpDIS->hDC, RGB(255, 127, 80)); // orange
    } else {
        SetDCBrushColor(lpDIS->hDC, RGB(40, 79, 79)); // grey
    }

    SelectObject(lpDIS->hDC, GetStockObject(DC_BRUSH));
    FillRect(lpDIS->hDC, &lpDIS->rcItem, CreateSolidBrush(0));
    RoundRect(lpDIS->hDC, lpDIS->rcItem.left, lpDIS->rcItem.top,
              lpDIS->rcItem.right, lpDIS->rcItem.bottom, BUTTON_WIDTH, BUTTON_HEIGHT);

    // Text
    SelectObject(lpDIS->hDC, CreateFont(BUTTON_TEXT_HEIGHT, BUTTON_TEXT_WIDTH, 0, 0,
                                        FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
                                        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                        DEFAULT_QUALITY, DEFAULT_PITCH, "Arial"));
    SetTextColor(lpDIS->hDC, RGB(211, 211, 211));
    SetBkMode(lpDIS->hDC, TRANSPARENT);
    DrawTextA(lpDIS->hDC, character[wparam], 1, &lpDIS->rcItem,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

LRESULT CALLBACK View::windowProcess(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DRAWITEM:
            colorButton(wParam, lParam);
            return true;
        case WM_CTLCOLOREDIT:
            colorBackground(wParam);
            return (LRESULT) CreateSolidBrush(BLACK_BRUSH);
        case WM_CHAR:
            keyboardInput(wParam);
            break;
        case WM_COMMAND:
            buttonInput(wParam);
            break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void View::keyboardInput(WPARAM wparam) {
    char ch = static_cast<char>(wparam);

    std::string str;
    str.push_back(ch);

    if ((ch >= '0' && ch <= '9') || (ch == '.')) {
        observer->inputQuery(str);
    } else if ((ch >= '*' && ch <= '+') || (ch == '-') || (ch == '/')) {
        observer->inputQuery(" " + str + " ");
    } else if (ch == '(') {
        observer->inputQuery(str + " ");
    } else if (ch == ')') {
        observer->inputQuery(" " + str);
    } else if (str == "=" || wparam == VK_RETURN) {
        observer->resultQuery();
    } else if (wparam == VK_BACK) {
        observer->removeQuery();
    }

    if (!str.empty()) {
        updateField(observer->getInputQuery());
    }
}

void View::buttonInput(WPARAM wparam) {
    if (wparam >= 0 && wparam <= 9) {
        observer->inputQuery(std::to_string(wparam));
    }
    switch (wparam) {
        case 10:
            observer->inputQuery(".");
            break;
        case 11:
            observer->inputQuery("( ");
            break;
        case 12:
            observer->inputQuery(" )");
            break;
        case 13:
            observer->inputQuery(" / ");
            break;
        case 14:
            observer->inputQuery(" * ");
            break;
        case 15:
            observer->inputQuery(" - ");
            break;
        case 16:
            observer->inputQuery(" + ");
            break;
        case 17:
            observer->resultQuery();
            break;
        case 18:
            observer->clearQuery();
            break;
        case 19:
            observer->removeQuery();
            break;
        default:
            break;
    }
    updateField(observer->getInputQuery());
    SetFocus(hWnd);
}

void View::updateField(const char *fieldText) {
    SetWindowTextA(textField, fieldText);
    SendMessage(textField, WM_SETFONT, (WPARAM) hFont, TRUE);
}

void View::createTextField() {
    textField = CreateWindow(
            "EDIT",
            "",
            WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL,
            0,
            10,
            WINDOW_WIDTH,
            60,
            hWnd,
            (HMENU) 21,
            hInstance,
            nullptr
    );

    ZeroMemory(&font, sizeof(LOGFONT));
    font.lfCharSet = DEFAULT_CHARSET;
    font.lfHeight = TEXT_HEIGHT;
    font.lfWeight = TEXT_WIDTH;
    font.lfQuality = TEXT_HEIGHT;
    hFont = CreateFontIndirect(&font);

    SendMessage(textField, WM_SETFONT, (WPARAM) hFont, TRUE);
}

void View::createButtons() {
    for (long long i = 0; i < 20; i++) {
        Button("", (HMENU) i, pos[i].first, pos[i].second);
    }
}

void View::createWindow() {
    RECT rect;
    rect.left = 250;
    rect.top = 250;
    rect.right = rect.left + WINDOW_WIDTH;
    rect.bottom = rect.top + WINDOW_HEIGHT;
    AdjustWindowRect(
            &rect,
            WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
            false);
    hWnd = CreateWindow (
            "Window Class",
            "win32-Calculator",
            WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
            rect.left,
            rect.top,
            rect.right - rect.left,
            rect.bottom - rect.top,
            nullptr,
            nullptr,
            hInstance,
            nullptr
    );

    ShowWindow(hWnd, SW_SHOW);
}

