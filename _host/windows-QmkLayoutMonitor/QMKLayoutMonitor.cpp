#include <windows.h>
#include <setupapi.h>
#include <hidsdi.h>
#include <hidpi.h>
#include <dbt.h>
#include <shlobj.h>  // Для создания ярлыков и работы с путями %APPDATA%
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include "Helpers.h"

// Линковка необходимых системных библиотек
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "hid.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib") // Для работы с COM (IShellLink)
#pragma comment(lib, "uuid.lib")  // Для CLSID_ShellLink и IID_IShellLinkW

#define WM_APP_CHECKLAYOUT (WM_APP + 1)
#undef DEBUG

// Глобальные переменные
WORD g_vid = 0;
WORD g_pid = 0;
WORD g_usagePage = 0;
HANDLE g_hKeyboard = INVALID_HANDLE_VALUE;
std::wofstream g_logFile;

HWND g_hwndHidden = NULL;
HHOOK g_hKeyboardHook = NULL;
HWINEVENTHOOK g_hWinEventHook = NULL;
WORD g_lastLangId = 0;

#ifdef DEBUG
// Функция для записи логов
void _LogMessage(const std::wstring& msg) {
    if (g_logFile.is_open()) {
        SYSTEMTIME st;
        GetLocalTime(&st);
        g_logFile << L"[" << st.wYear << L"-"
            << std::setfill(L'0') << std::setw(2) << st.wMonth << L"-"
            << std::setfill(L'0') << std::setw(2) << st.wDay << L" "
            << std::setfill(L'0') << std::setw(2) << st.wHour << L":"
            << std::setfill(L'0') << std::setw(2) << st.wMinute << L":"
            << std::setfill(L'0') << std::setw(2) << st.wSecond << L"] "
            << msg << std::endl;
    }
}
#endif

#ifdef DEBUG
#define LogMessage(X) _LogMessage(X)
#else
#define LogMessage(X)
#endif

// Подключение к клавиатуре QMK
void ConnectKeyboard() {
    if (g_hKeyboard != INVALID_HANDLE_VALUE) {
        CloseHandle(g_hKeyboard);
        g_hKeyboard = INVALID_HANDLE_VALUE;
    }

    GUID hidGuid;
    HidD_GetHidGuid(&hidGuid);

    HDEVINFO hDevInfo = SetupDiGetClassDevs(&hidGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (hDevInfo == INVALID_HANDLE_VALUE) return;

    SP_DEVICE_INTERFACE_DATA devInfoData;
    devInfoData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    for (int i = 0; SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &hidGuid, i, &devInfoData); ++i) {
        DWORD detailSize = 0;
        SetupDiGetDeviceInterfaceDetail(hDevInfo, &devInfoData, NULL, 0, &detailSize, NULL);

        std::vector<BYTE> detailDataBuf(detailSize);
        PSP_DEVICE_INTERFACE_DETAIL_DATA detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)detailDataBuf.data();
        detailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

        if (SetupDiGetDeviceInterfaceDetail(hDevInfo, &devInfoData, detailData, detailSize, NULL, NULL)) {
            HANDLE hDevice = CreateFile(detailData->DevicePath, GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

            if (hDevice != INVALID_HANDLE_VALUE) {
                HIDD_ATTRIBUTES attr;
                attr.Size = sizeof(HIDD_ATTRIBUTES);
                if (HidD_GetAttributes(hDevice, &attr)) {
                    if (attr.VendorID == g_vid && attr.ProductID == g_pid) {
                        PHIDP_PREPARSED_DATA preparsedData;
                        if (HidD_GetPreparsedData(hDevice, &preparsedData)) {
                            HIDP_CAPS caps;
                            HidP_GetCaps(preparsedData, &caps);
                            HidD_FreePreparsedData(preparsedData);

                            if (caps.UsagePage == g_usagePage) {
                                g_hKeyboard = hDevice;
                                LogMessage(L"QMK Keyboard connected successfully.");
                                SetupDiDestroyDeviceInfoList(hDevInfo);
                                return;
                            }
                        }
                    }
                }
                CloseHandle(hDevice);
            }
        }
    }
    SetupDiDestroyDeviceInfoList(hDevInfo);
}

// Отправка данных в клавиатуру
void SendLayoutToQMK(WORD langId) {
    if (g_hKeyboard == INVALID_HANDLE_VALUE) {
        ConnectKeyboard();
    }

    if (g_hKeyboard != INVALID_HANDLE_VALUE) {
        const char* layoutName = langIdToLangName(langId);
        BYTE report[33] = { 0 };
        report[0] = 0;         // Report ID
        report[1] = 0xfc; // command code not conflicting with Vial
        report[2] = 0x42; // my command code for keyboard layout
        report[3] = layoutName[0];
        report[4] = layoutName[1];
        report[5] = (strcmp(layoutName, "EN") == 0)? 0 : 1; // should the keyboard turn backlight on
        report[6] = 0;

        DWORD bytesWritten = 0;
        if (!WriteFile(g_hKeyboard, report, sizeof(report), &bytesWritten, NULL)) {
            LogMessage(L"Failed to write. Device disconnected?");
            CloseHandle(g_hKeyboard);
            g_hKeyboard = INVALID_HANDLE_VALUE;
        }
        else {
            std::wstringstream wss;
            wss << L"Sent language ID: 0x" << std::hex << std::setfill(L'0') << std::setw(4) << std::uppercase << langId << L" to QMK.";
            LogMessage(wss.str());
        }
    }
    else {
        LogMessage(L"Cannot send layout. Keyboard not connected.");
    }
}

// Проверка текущего языка в системе
void CheckLayout() {
    HWND hForeground = GetForegroundWindow();
    if (!hForeground) return;

    DWORD threadId = GetWindowThreadProcessId(hForeground, NULL);
    HKL hkl = GetKeyboardLayout(threadId);
    WORD langId = LOWORD(hkl);

    // Если язык изменился
    if (langId != g_lastLangId && langId != 0) {
        g_lastLangId = langId;

        WCHAR langName[256];
        std::wstringstream wss;
        wss << std::hex << std::setfill(L'0') << std::setw(4) << std::uppercase << langId;

        if (GetLocaleInfoW(langId, LOCALE_SENGLISHDISPLAYNAME, langName, 256)) {
            LogMessage(std::wstring(L"Layout changed to: ") + langName + L" (ID: 0x" + wss.str() + L")");
        }
        else {
            LogMessage(L"Layout changed to ID: 0x" + wss.str());
        }

        SendLayoutToQMK(langId);
    }
}

// Установка: Копирование в %APPDATA% и создание ярлыка в Автозагрузке
bool InstallAndCreateShortcut(LPCWSTR vid, LPCWSTR pid, LPCWSTR usagePage) {
    WCHAR exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH); // Путь текущего запущенного .exe

    WCHAR appDataPath[MAX_PATH];
    if (FAILED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appDataPath))) {
        LogMessage(L"Error: Failed to get %APPDATA% path.");
        return false;
    }

    // Создаем папку в %APPDATA%
    std::wstring destFolder = std::wstring(appDataPath) + L"\\QMKLayoutMonitor";
    CreateDirectoryW(destFolder.c_str(), NULL);

    // Копируем сам .exe файл
    std::wstring destExe = destFolder + L"\\QMKLayoutMonitor.exe";
    if (!CopyFileW(exePath, destExe.c_str(), FALSE)) {
        if (GetLastError() != ERROR_FILE_EXISTS) {
            LogMessage(L"Error: Failed to copy executable to APPDATA. Code: " + std::to_wstring(GetLastError()));
            return false;
        }
        else {
            LogMessage(L"Executable already exists in APPDATA. Overwriting...");
            CopyFileW(exePath, destExe.c_str(), TRUE);
        }
    }
    LogMessage(L"Executable successfully copied to: " + destExe);

    // Получаем путь к системной папке "Автозагрузка"
    WCHAR startupPath[MAX_PATH];
    if (FAILED(SHGetFolderPathW(NULL, CSIDL_STARTUP, NULL, 0, startupPath))) {
        LogMessage(L"Error: Failed to get STARTUP folder path.");
        return false;
    }

    // Создаем COM объект ярлыка (IShellLink)
    std::wstring shortcutPath = std::wstring(startupPath) + L"\\QMKLayoutMonitor.lnk";
    std::wstring args = std::wstring(vid) + L" " + pid + L" " + usagePage;

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    bool success = false;
    if (SUCCEEDED(hr)) {
        IShellLinkW* psl;
        if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (LPVOID*)&psl))) {
            psl->SetPath(destExe.c_str());             // Путь к скопированному EXE
            psl->SetArguments(args.c_str());           // Аргументы (VID, PID, UsagePage)
            psl->SetWorkingDirectory(destFolder.c_str()); // Рабочая папка (туда будут писаться логи)
            psl->SetDescription(L"QMK Layout Monitor Auto-Start");

            IPersistFile* ppf;
            if (SUCCEEDED(psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf))) {
                if (SUCCEEDED(ppf->Save(shortcutPath.c_str(), TRUE))) {
                    success = true;
                    LogMessage(L"Shortcut created successfully in Startup folder: " + shortcutPath);
                }
                else {
                    LogMessage(L"Error: Failed to save shortcut.");
                }
                ppf->Release();
            }
            psl->Release();
        }
        CoUninitialize();
    }
    return success;
}

// Хук: переключение между окнами
void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime) {
    if (event == EVENT_SYSTEM_FOREGROUND && g_hwndHidden) {
        PostMessageW(g_hwndHidden, WM_APP_CHECKLAYOUT, 0, 0);
    }
}

// Хук: нажатие клавиш-модификаторов
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            KBDLLHOOKSTRUCT* pkb = (KBDLLHOOKSTRUCT*)lParam;
            DWORD vk = pkb->vkCode;
            if (vk == VK_LSHIFT || vk == VK_RSHIFT || vk == VK_SHIFT ||
                vk == VK_LMENU || vk == VK_RMENU || vk == VK_MENU ||
                vk == VK_LCONTROL || vk == VK_RCONTROL || vk == VK_CONTROL ||
                vk == VK_LWIN || vk == VK_RWIN || vk == VK_SPACE || vk == VK_CAPITAL) {
                if (g_hwndHidden) {
                    PostMessageW(g_hwndHidden, WM_APP_CHECKLAYOUT, 0, 0);
                }
            }
        }
    }
    return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
}

// Оконная процедура скрытого окна
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_APP_CHECKLAYOUT:
        SetTimer(hwnd, 1, 100, NULL);
        return 0;

    case WM_TIMER:
        if (wParam == 1) {
            KillTimer(hwnd, 1);
            CheckLayout();
        }
        return 0;

    case WM_DEVICECHANGE:
        if (wParam == DBT_DEVICEARRIVAL) {
            PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR)lParam;
            if (pHdr && pHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {
                LogMessage(L"HID Device connected. Re-checking...");
                if (g_hKeyboard == INVALID_HANDLE_VALUE) {
                    ConnectKeyboard();
                    if (g_lastLangId != 0) SendLayoutToQMK(g_lastLangId);
                }
            }
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// Точка входа
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    // --- ЗАЩИТА ОТ ДВОЙНОГО ЗАПУСКА ---
    // Создаем мьютекс с уникальным именем (можно придумать любое)
    HANDLE hMutex = CreateMutexW(NULL, TRUE, L"QMKLayoutMonitor_SingleInstance_Mutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        // Мьютекс уже существует, значит программа уже запущена.
        // Закрываем хэндл и тихо завершаем работу новой копии.
        CloseHandle(hMutex);
        return 0;
    }
    // ----------------------------------

    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
#ifdef DEBUG
    g_logFile.open(L"layout_monitor.log", std::ios::out | std::ios::app);
#endif
    LogMessage(L"--- Starting QMK Layout Monitor ---");

    if (argc < 4) {
        LogMessage(L"Error: Missing arguments. Usage: app.exe <VID> <PID> <UsagePage> [debug|install]");
        LocalFree(argv);
        return 1;
    }

    g_vid = (WORD)std::wcstoul(argv[1], NULL, 16);
    g_pid = (WORD)std::wcstoul(argv[2], NULL, 16);
    g_usagePage = (WORD)std::wcstoul(argv[3], NULL, 16);

    // Проверка опционального четвертого аргумента
    if (argc >= 5) {
        if (_wcsicmp(argv[4], L"test") == 0) {
            LogMessage(L"Running in DEBUG mode...");
            ConnectKeyboard();
            CheckLayout(); // Считает текущую раскладку и сразу отправит (так как g_lastLangId = 0)
            if (g_hKeyboard != INVALID_HANDLE_VALUE) CloseHandle(g_hKeyboard);
            LogMessage(L"DEBUG mode finished. Exiting.");
            LocalFree(argv);
            g_logFile.close();
            return 0; // Сразу завершаем программу
        }
        else if (_wcsicmp(argv[4], L"install") == 0) {
            LogMessage(L"Running in INSTALL mode...");
            InstallAndCreateShortcut(argv[1], argv[2], argv[3]);
            LogMessage(L"INSTALL mode finished. Exiting.");
            LocalFree(argv);
            g_logFile.close();
            return 0; // Сразу завершаем программу
        }
    }

    LocalFree(argv);
    ConnectKeyboard();

    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"QMKLayoutMonitorHiddenWindow";
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(WS_EX_TOOLWINDOW, wc.lpszClassName, L"QMKLayoutMonitor", WS_POPUP, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
    if (!hwnd) {
        LogMessage(L"Failed to create hidden window.");
        return 1;
    }
    g_hwndHidden = hwnd;

    GUID hidGuid;
    HidD_GetHidGuid(&hidGuid);
    DEV_BROADCAST_DEVICEINTERFACE filter = { 0 };
    filter.dbcc_size = sizeof(filter);
    filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    filter.dbcc_classguid = hidGuid;
    RegisterDeviceNotification(hwnd, &filter, DEVICE_NOTIFY_WINDOW_HANDLE);

    g_hWinEventHook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL, WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
    g_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInstance, 0);

    CheckLayout(); // Читаем язык при старте

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (g_hWinEventHook) UnhookWinEvent(g_hWinEventHook);
    if (g_hKeyboardHook) UnhookWindowsHookEx(g_hKeyboardHook);
    if (g_hKeyboard != INVALID_HANDLE_VALUE) CloseHandle(g_hKeyboard);

    LogMessage(L"Exiting...");
    g_logFile.close();

    return (int)msg.wParam;
}