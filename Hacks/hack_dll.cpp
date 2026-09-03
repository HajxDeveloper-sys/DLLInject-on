#include <windows.h>
#include <iostream>

// oyun.cpp icindeki Player veri yapisinin birebir aynisi
struct Player {
    char name[32];
    int level;
    int exp;
    int expToNext;
    int hp;
    int maxHp;
    int mana;
    int maxMana;
    int gold;
    int attack;
    int defense;
    int healthPotions;
    int manaPotions;
    int monstersKilled;
    int bossesKilled;
    int totalDamageDealt;
    int totalGoldEarned;
    bool godMode;
};

typedef Player* (*GetPlayerInstance_t)();

// Hile Menu ve Tus Dinleme Thread'i
DWORD WINAPI CheatThread(LPVOID lpParam) {
    // 1. Hedef oyunun modülünü al
    HMODULE hExe = GetModuleHandleA(NULL);
    if (!hExe) return 1;

    // 2. oyun.exe icinden export edilen GetPlayerInstance fonksiyonunu bul
    GetPlayerInstance_t GetPlayer = (GetPlayerInstance_t)GetProcAddress(hExe, "GetPlayerInstance");
    if (!GetPlayer) return 1;

    Player* p = GetPlayer();
    if (!p) return 1;

    // Baglanti basarili ses melodisi
    Beep(880, 80);
    Beep(1174, 120);
    Beep(1480, 160);

    // Kucuk bilgilendirici popup (Laptop klavyelerine uygun tuslar)
    CreateThread(NULL, 0, [](LPVOID) -> DWORD {
        MessageBoxA(NULL, 
            "Efsanevi Golge RPG Hile Modulu Devrede!\n\n"
            "Klavyeden basabileceginiz harfler:\n"
            "[G] : God Mode (Olumsuzluk) Ac / Kapat\n"
            "[M] : Para Ekle (+50.000 Altin)\n"
            "[K] : Kill Power (Tek Atis Saldiri: +5000)\n"
            "[P] : Iksirler & Sinirsiz Mana\n"
            "[L] : Seviye 50 & Dev Statlar\n", 
            "DLL HILE MENU (F Tuslari Gerekmez)", 
            MB_OK | MB_ICONINFORMATION);
        return 0;
    }, NULL, 0, NULL);

    // Tus dinleme dongusu
    while (true) {
        Sleep(60);

        // [G] TUSU: GOD MODE TOGGLE
        if (GetAsyncKeyState('G') & 0x8000) {
            p->godMode = !p->godMode;
            if (p->godMode) {
                p->maxHp = 99999;
                p->hp = 99999;
                Beep(1200, 100);
            } else {
                p->maxHp = 200;
                p->hp = 200;
                Beep(350, 100);
            }
            Sleep(250);
        }

        // [M] TUSU: MONEY (+50.000 ALTIN)
        if (GetAsyncKeyState('M') & 0x8000) {
            p->gold += 50000;
            p->totalGoldEarned += 50000;
            Beep(1500, 80);
            Sleep(250);
        }

        // [K] TUSU: KILL POWER (TEK ATIS GUCU)
        if (GetAsyncKeyState('K') & 0x8000) {
            p->attack += 5000;
            p->defense += 1500;
            Beep(1700, 80);
            Sleep(250);
        }

        // [P] TUSU: IKSIRLER & MANA
        if (GetAsyncKeyState('P') & 0x8000) {
            p->mana = p->maxMana = 9999;
            p->healthPotions += 50;
            p->manaPotions += 50;
            Beep(1900, 80);
            Sleep(250);
        }

        // [L] TUSU: LEVEL 50
        if (GetAsyncKeyState('L') & 0x8000) {
            p->level = 50;
            p->maxHp = 9999;
            p->hp = 9999;
            p->attack += 800;
            p->defense += 400;
            Beep(2100, 120);
            Sleep(250);
        }
    }

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, CheatThread, NULL, 0, NULL);
    }
    return TRUE;
}
