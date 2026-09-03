#include <iostream>
#include <vector>
#include <string>
#include <windows.h>
#include <tlhelp32.h>
#include <conio.h>

void SetColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void ShowCursor(bool show) {
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = show;
    SetConsoleCursorInfo(out, &cursorInfo);
}

namespace Sound {
    void Tick() { Beep(800, 20); }
    void Select() { Beep(1200, 50); }
    void Error() { Beep(260, 120); }
    void Success() {
        Beep(523, 70);
        Beep(659, 70);
        Beep(784, 120);
    }
}

int SelectMenu(const std::string& title, const std::vector<std::string>& options) {
    ShowCursor(false);
    int selected = 0;
    int total = (int)options.size();

    while (true) {
        system("cls");
        SetColor(11);
        std::cout << "\n===============================================================\n";
        std::cout << " " << title << "\n";
        std::cout << "===============================================================\n\n";
        SetColor(7);

        for (int i = 0; i < total; i++) {
            if (i == selected) {
                SetColor(14);
                std::cout << "  ►  [ " << options[i] << " ]  ◄\n";
            } else {
                SetColor(8);
                std::cout << "     " << options[i] << "\n";
            }
        }

        SetColor(3);
        std::cout << "\n---------------------------------------------------------------\n";
        std::cout << "  [↑ / ↓] : Gezin  |  [ENTER] : Onayla\n";
        std::cout << "---------------------------------------------------------------\n";
        SetColor(7);

        int key = _getch();
        if (key == 0 || key == 224) {
            int arrow = _getch();
            if (arrow == 72) {
                selected--;
                if (selected < 0) selected = total - 1;
                Sound::Tick();
            } else if (arrow == 80) {
                selected++;
                if (selected >= total) selected = 0;
                Sound::Tick();
            }
        } else if (key == 13) {
            Sound::Select();
            ShowCursor(true);
            return selected;
        }
    }
}

DWORD FindProcessByName(const char* processName) {
    DWORD pid = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(snapshot, &pe)) {
            do {
                if (_stricmp(pe.szExeFile, processName) == 0) {
                    pid = pe.th32ProcessID;
                    break;
                }
            } while (Process32Next(snapshot, &pe));
        }
        CloseHandle(snapshot);
    }
    return pid;
}

bool PerformInjection(DWORD pid, const char* dllName) {
    char fullDllPath[MAX_PATH];
    if (!GetFullPathNameA(dllName, MAX_PATH, fullDllPath, NULL)) {
        SetColor(12);
        std::cerr << "[-] Hata: DLL tam dosya yolu cozumlenemedi!\n";
        SetColor(7);
        return false;
    }

    if (GetFileAttributesA(fullDllPath) == INVALID_FILE_ATTRIBUTES) {
        SetColor(12);
        std::cerr << "[-] Hata: '" << dllName << "' dosyasi klasorde bulunamadi!\n";
        SetColor(7);
        return false;
    }

    std::cout << "\n[1] Hedef proses aciliyor (PID: " << pid << ")...\n";
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) {
        SetColor(12);
        std::cerr << "[-] OpenProcess basarisiz! (Yonetici olarak calistirmayi deneyebilirsiniz). Kod: " << GetLastError() << "\n";
        SetColor(7);
        return false;
    }
    SetColor(10);
    std::cout << "    [OK] Proses tam erisim yetkisiyle acildi.\n";
    SetColor(7);

    std::cout << "[2] Hedef proseste bellek tahsis ediliyor (VirtualAllocEx)...\n";
    size_t pathLen = strlen(fullDllPath) + 1;
    LPVOID pRemoteMem = VirtualAllocEx(hProcess, NULL, pathLen, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pRemoteMem) {
        SetColor(12);
        std::cerr << "[-] VirtualAllocEx basarisiz! Kod: " << GetLastError() << "\n";
        SetColor(7);
        CloseHandle(hProcess);
        return false;
    }
    SetColor(10);
    std::cout << "    [OK] Hedef bellekte " << pathLen << " bayt yer ayrildi.\n";
    SetColor(7);

    std::cout << "[3] DLL yolu hedefin hafizasina yaziliyor (WriteProcessMemory)...\n";
    if (!WriteProcessMemory(hProcess, pRemoteMem, fullDllPath, pathLen, NULL)) {
        SetColor(12);
        std::cerr << "[-] WriteProcessMemory basarisiz! Kod: " << GetLastError() << "\n";
        SetColor(7);
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }
    SetColor(10);
    std::cout << "    [OK] DLL yolu basariyla yazildi: " << fullDllPath << "\n";
    SetColor(7);

    std::cout << "[4] Uzak thread baslatiliyor (CreateRemoteThread -> LoadLibraryA)...\n";
    LPVOID pLoadLibrary = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    HANDLE hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibrary, pRemoteMem, 0, NULL);
    if (!hRemoteThread) {
        SetColor(12);
        std::cerr << "[-] CreateRemoteThread basarisiz! Kod: " << GetLastError() << "\n";
        SetColor(7);
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }
    SetColor(10);
    std::cout << "    [OK] Uzak Thread basariyla tetiklendi!\n";
    SetColor(7);

    WaitForSingleObject(hRemoteThread, INFINITE);
    VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
    CloseHandle(hRemoteThread);
    CloseHandle(hProcess);

    SetColor(14);
    std::cout << "\n===============================================================\n";
    std::cout << "  ★★★ TEBRIKLER: DLL BASARIYLA ENJEKTE EDILDI! ★★★\n";
    std::cout << "  Oyun ekranina donup [G], [M], [K], [P], [L] tuslarini kullanin!\n";
    std::cout << "===============================================================\n";
    SetColor(7);
    Sound::Success();

    return true;
}

int main() {
    SetConsoleOutputCP(65001);
    SetConsoleTitleA("DLL Injector - Gelismis Yonetim Paneli");

    const char* targetExeName = "oyun.exe";
    const char* dllName = "hack_dll.dll";

    while (true) {
        std::vector<std::string> menuOptions = {
            "1. Arkada Oyun Acik mi Tara (Otomatik Algila)",
            "2. Manuel Process ID (PID) Girerek Enjekte Et",
            "3. Cikis"
        };

        int choice = SelectMenu("GELISMIS DLL INJECTOR PANELI", menuOptions);

        if (choice == 0) {
            system("cls");
            SetColor(11);
            std::cout << "\n[+] Arkada calisan '" << targetExeName << "' araniyor...\n";
            SetColor(7);
            Sleep(400);

            DWORD pid = FindProcessByName(targetExeName);
            if (pid != 0) {
                SetColor(10);
                std::cout << "[+] OYUN BULUNDU! Hedef Proses ID (PID): " << pid << "\n";
                SetColor(7);
                PerformInjection(pid, dllName);
                std::cout << "\nMenuye donmek icin herhangi bir tusa basin...";
                _getch();
            } else {
                Sound::Error();
                SetColor(12);
                std::cout << "\n===============================================================\n";
                std::cout << " [!] OYUN BULUNAMADI!\n";
                std::cout << " Lutfen once 'oyun.exe'yi calistirin ve tekrar deneyin,\n";
                std::cout << " veya ana menuden manuel olarak Process ID (PID) girin.\n";
                std::cout << "===============================================================\n";
                SetColor(7);
                std::cout << "\nMenuye donmek icin herhangi bir tusa basin...";
                _getch();
            }

        } else if (choice == 1) {
            system("cls");
            SetColor(14);
            std::cout << "\n--- MANUEL PID ILE ENJEKSIYON ---\n";
            SetColor(7);
            std::cout << "Hedef programin Process ID'sini (PID) girin (Cikis icin 0): ";
            DWORD pid = 0;
            if (std::cin >> pid && pid != 0) {
                PerformInjection(pid, dllName);
                std::cout << "\nMenuye donmek icin herhangi bir tusa basin...";
                _getch();
            } else {
                std::cin.clear();
                std::cin.ignore(1000, '\n');
            }

        } else if (choice == 2) {
            std::cout << "\nInjector sonlandirildi.\n";
            break;
        }
    }

    return 0;
}
