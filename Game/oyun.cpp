#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <windows.h>
#include <conio.h>
#include <cstdlib>
#include <ctime>

// ==========================================
// 1. OYUNCU VE ISTATISTIK VERI YAPISI
// ==========================================
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

    // Istatistikler
    int monstersKilled;
    int bossesKilled;
    int totalDamageDealt;
    int totalGoldEarned;
    bool godMode; // Hile / Mod tarafindan kilitlenebilir
};

// Global oyuncu ornegi
Player g_player;

// DLL Injection icin fonksiyonu disa aktariyoruz (Export)
extern "C" __declspec(dllexport) Player* GetPlayerInstance() {
    return &g_player;
}

// ==========================================
// 2. KONSOL GORSEL VE IMLEC AYARLARI
// ==========================================
void ShowConsoleCursor(bool show) {
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = show;
    SetConsoleCursorInfo(out, &cursorInfo);
}

void SetColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void ResetCursor() {
    COORD coord = { 0, 0 };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// ==========================================
// 3. SES EFEKTLERI SISTEMI (Windows Beep)
// ==========================================
namespace Sound {
    void MenuTick() {
        Beep(800, 20); // Ok tusuna basinca cikan hos menu sesi
    }
    void MenuSelect() {
        Beep(1200, 50); // Enter sesi
    }
    void MenuError() {
        Beep(250, 100);
    }
    void Attack() {
        Beep(400, 35);
        Beep(280, 45);
    }
    void Lightning() {
        Beep(1200, 30);
        Beep(900, 40);
        Beep(1500, 60);
    }
    void CriticalHit() {
        Beep(700, 40);
        Beep(1100, 60);
        Beep(1400, 80);
    }
    void MonsterAttack() {
        Beep(160, 60);
        Beep(120, 80);
    }
    void Heal() {
        Beep(523, 70);
        Beep(659, 70);
        Beep(784, 100);
    }
    void Coin() {
        Beep(1046, 50);
        Beep(1318, 90);
    }
    void LevelUp() {
        Beep(523, 80);
        Beep(659, 80);
        Beep(784, 80);
        Beep(1046, 200);
    }
    void Victory() {
        Beep(659, 90);
        Beep(659, 90);
        Beep(659, 90);
        Beep(523, 70);
        Beep(659, 100);
        Beep(784, 250);
    }
    void Defeat() {
        Beep(300, 150);
        Beep(220, 200);
        Beep(160, 300);
    }
}

// ==========================================
// 4. OK TUSLARI ILE ANIMASYONLU MENÜ SISTEMI
// ==========================================
int SelectMenu(const std::string& title, const std::vector<std::string>& options, int defaultSelected = 0) {
    ShowConsoleCursor(false);
    int selected = defaultSelected;
    int total = (int)options.size();

    while (true) {
        system("cls");

        // Baslik Yazdir
        SetColor(11);
        std::cout << "\n===============================================================\n";
        std::cout << " " << title << "\n";
        std::cout << "===============================================================\n\n";
        SetColor(7);

        // Secenekleri listele
        for (int i = 0; i < total; i++) {
            if (i == selected) {
                SetColor(14); // Parlak Sari
                std::cout << "  ►  [ " << options[i] << " ]  ◄\n";
            } else {
                SetColor(8); // Gri / Soluk
                std::cout << "     " << options[i] << "\n";
            }
        }

        SetColor(3);
        std::cout << "\n---------------------------------------------------------------\n";
        std::cout << "  [↑ / ↓] : Secim Yap  |  [ENTER] : Onayla\n";
        std::cout << "---------------------------------------------------------------\n";
        SetColor(7);

        // Tus girisini yakala
        int key = _getch();
        if (key == 0 || key == 224) { // Yon tuslari
            int arrow = _getch();
            if (arrow == 72) { // YUKARI OK
                selected--;
                if (selected < 0) selected = total - 1;
                Sound::MenuTick();
            } else if (arrow == 80) { // ASAGI OK
                selected++;
                if (selected >= total) selected = 0;
                Sound::MenuTick();
            }
        } else if (key == 13) { // ENTER
            Sound::MenuSelect();
            ShowConsoleCursor(true);
            return selected;
        }
    }
}

// ==========================================
// 5. SAGLIK VE GORSEL EFEKTLER
// ==========================================
void DrawHealthBar(int current, int max, int barLength = 22) {
    if (current < 0) current = 0;
    float ratio = (float)current / (float)max;
    int filled = (int)(ratio * barLength);
    if (filled > barLength) filled = barLength;

    std::cout << "[";
    if (ratio > 0.5f) SetColor(10); // Yesil
    else if (ratio > 0.25f) SetColor(14); // Sari
    else SetColor(12); // Kirmizi

    for (int i = 0; i < filled; i++) std::cout << "=";
    SetColor(8);
    for (int i = filled; i < barLength; i++) std::cout << "-";
    SetColor(7);
    std::cout << "] " << current << "/" << max;
}

void FlashScreen(int colorCode, const std::string& message) {
    for (int i = 0; i < 2; i++) {
        SetColor(colorCode);
        std::cout << "\r   >>> " << message << " <<<   " << std::flush;
        Sleep(60);
        SetColor(7);
        std::cout << "\r                                     " << std::flush;
        Sleep(40);
    }
    std::cout << "\n";
}

// ==========================================
// 6. KAYIT (SAVE & LOAD)
// ==========================================
bool SaveGame() {
    std::ofstream file("savegame.dat", std::ios::binary);
    if (!file) return false;
    file.write((char*)&g_player, sizeof(Player));
    file.close();
    return true;
}

bool HasSaveGame() {
    std::ifstream file("savegame.dat", std::ios::binary);
    return file.good();
}

bool LoadGame() {
    std::ifstream file("savegame.dat", std::ios::binary);
    if (!file) return false;
    file.read((char*)&g_player, sizeof(Player));
    file.close();
    return true;
}

void InitPlayer(const std::string& name) {
    memset(&g_player, 0, sizeof(Player));
    strncpy_s(g_player.name, name.c_str(), sizeof(g_player.name) - 1);
    g_player.level = 1;
    g_player.exp = 0;
    g_player.expToNext = 100;
    g_player.hp = 120;
    g_player.maxHp = 120;
    g_player.mana = 60;
    g_player.maxMana = 60;
    g_player.gold = 75;
    g_player.attack = 20;
    g_player.defense = 8;
    g_player.healthPotions = 3;
    g_player.manaPotions = 2;
    g_player.godMode = false;
}

void CheckLevelUp() {
    while (g_player.exp >= g_player.expToNext) {
        g_player.exp -= g_player.expToNext;
        g_player.level++;
        g_player.expToNext = (int)(g_player.expToNext * 1.5f);
        g_player.maxHp += 30;
        g_player.hp = g_player.maxHp;
        g_player.maxMana += 20;
        g_player.mana = g_player.maxMana;
        g_player.attack += 7;
        g_player.defense += 5;

        SetColor(14);
        std::cout << "\n***************************************************\n";
        std::cout << ">>> TEBRIKLER! SEVIYE ATLADINIZ! (SEVIYE " << g_player.level << ") <<<\n";
        std::cout << "Maksimum Can ve Mana artti! Gucun yukseldi!\n";
        std::cout << "***************************************************\n";
        SetColor(7);
        Sound::LevelUp();
        Sleep(1000);
    }
}

// ==========================================
// 7. HUD VE BILGI PANELI
// ==========================================
void ShowHUD() {
    if (g_player.godMode) {
        g_player.hp = g_player.maxHp;
    }

    SetColor(11);
    std::cout << "===============================================================\n";
    std::cout << " KAHRAMAN: " << g_player.name << "  |  SEVIYE: " << g_player.level;
    if (g_player.godMode) {
        SetColor(14);
        std::cout << "  ★ [GOD MODE AKTIF] ★";
        SetColor(11);
    }
    std::cout << "\n";
    SetColor(7);

    std::cout << " CAN  : ";
    DrawHealthBar(g_player.hp, g_player.maxHp);
    std::cout << "  |  MANA: " << g_player.mana << "/" << g_player.maxMana << "\n";

    std::cout << " GUC  : Saldiri " << g_player.attack 
              << " | Defans " << g_player.defense 
              << " | Altin: " << g_player.gold << " G\n";
    std::cout << " CANTA: " << g_player.healthPotions << "x Can Iksiri | " 
              << g_player.manaPotions << "x Mana Iksiri | EXP: " << g_player.exp << "/" << g_player.expToNext << "\n";
    SetColor(11);
    std::cout << "===============================================================\n";
    SetColor(7);
}

// ==========================================
// 8. CANAVARLAR VE SAVAS
// ==========================================
struct Monster {
    std::string name;
    int hp;
    int maxHp;
    int attack;
    int defense;
    int goldDrop;
    int expDrop;
    bool isBoss;
};

void Battle(Monster m) {
    system("cls");
    SetColor(12);
    if (m.isBoss) {
        std::cout << "\n>>> !!! ZINDAN KORUYUCUSU UYANDI: " << m.name << " !!! <<<\n";
        Beep(220, 200);
        Beep(180, 300);
    } else {
        std::cout << "\n[!] Pusudan cikan " << m.name << " ile karsilasildi!\n";
    }
    SetColor(7);
    Sleep(800);

    while (m.hp > 0 && (g_player.hp > 0 || g_player.godMode)) {
        if (g_player.godMode) g_player.hp = g_player.maxHp;

        std::string battleHeader = "SAVAS: " + m.name + " (HP: " + std::to_string(m.hp) + "/" + std::to_string(m.maxHp) + ") | SENIN CANIN: " + std::to_string(g_player.hp) + "/" + std::to_string(g_player.maxHp);
        
        std::vector<std::string> battleActions = {
            "Kilic Saldirisi (Temel Vurus)",
            "Ates Topu Buyusu (15 Mana - Agir Hasar)",
            "Yildirim Carpmasi (30 Mana - Devasa Hasar)",
            "Can Iksiri Ic (" + std::to_string(g_player.healthPotions) + " Adet)",
            "Mana Iksiri Ic (" + std::to_string(g_player.manaPotions) + " Adet)",
            "Karanlik Koridorlara Kac"
        };

        int act = SelectMenu(battleHeader, battleActions, 0);

        system("cls");
        if (act == 0) { // Kilic
            int dmg = (g_player.attack + (rand() % 8)) - (m.defense / 2);
            if (dmg < 4) dmg = 4;
            bool crit = (rand() % 4 == 0);
            if (crit) {
                dmg *= 2;
                FlashScreen(14, "KRITIK VURUS!");
                Sound::CriticalHit();
            } else {
                Sound::Attack();
            }
            m.hp -= dmg;
            g_player.totalDamageDealt += dmg;
            std::cout << ">> " << m.name << " uzerine kilicini savurdun! " << dmg << " fiziksel hasar!\n";

        } else if (act == 1) { // Ates Topu
            if (g_player.mana >= 15) {
                g_player.mana -= 15;
                int dmg = (g_player.attack * 2) + (rand() % 20);
                m.hp -= dmg;
                g_player.totalDamageDealt += dmg;
                FlashScreen(14, "ATES TOPU PATLAMASI!");
                Sound::CriticalHit();
                std::cout << ">> Gokten alevler yagdirdin! " << m.name << " " << dmg << " buyu hasari aldi!\n";
            } else {
                Sound::MenuError();
                std::cout << ">> Yetersiz Mana! (En az 15 Mana gerekir)\n";
                Sleep(900);
                continue;
            }

        } else if (act == 2) { // Yildirim
            if (g_player.mana >= 30) {
                g_player.mana -= 30;
                int dmg = (g_player.attack * 3) + 25 + (rand() % 25);
                m.hp -= dmg;
                g_player.totalDamageDealt += dmg;
                FlashScreen(11, "GOK GURLEMESI VE YILDIRIM!");
                Sound::Lightning();
                std::cout << ">> Yildirim mızrağı dustu! " << m.name << " " << dmg << " elektrik hasari aldi!\n";
            } else {
                Sound::MenuError();
                std::cout << ">> Yetersiz Mana! (En az 30 Mana gerekir)\n";
                Sleep(900);
                continue;
            }

        } else if (act == 3) { // Can Iksiri
            if (g_player.healthPotions > 0) {
                g_player.healthPotions--;
                int heal = g_player.maxHp * 0.5f;
                g_player.hp += heal;
                if (g_player.hp > g_player.maxHp) g_player.hp = g_player.maxHp;
                Sound::Heal();
                std::cout << ">> Kirmizi can iksirini ictin! +" << heal << " Can dolduruldu.\n";
            } else {
                Sound::MenuError();
                std::cout << ">> Cantanda can iksiri kalmadi!\n";
                Sleep(900);
                continue;
            }

        } else if (act == 4) { // Mana Iksiri
            if (g_player.manaPotions > 0) {
                g_player.manaPotions--;
                g_player.mana = g_player.maxMana;
                Sound::Heal();
                std::cout << ">> Mavi iksiri ictin! Tum manan tamamen tazelendi!\n";
            } else {
                Sound::MenuError();
                std::cout << ">> Cantanda mana iksiri kalmadi!\n";
                Sleep(900);
                continue;
            }

        } else if (act == 5) { // Kac
            if (!m.isBoss && rand() % 2 == 0) {
                std::cout << ">> Sislerin arasina dalarak kactin!\n";
                Sleep(1000);
                return;
            } else {
                std::cout << ">> Kacamadin! Canavar onunu kesti!\n";
            }
        }

        // Canavar oldu mu?
        if (m.hp <= 0) {
            SetColor(10);
            std::cout << "\n============================================\n";
            std::cout << "ZAFER! " << m.name << " hezimete ugratildi!\n";
            std::cout << "+ " << m.goldDrop << " Altin  |  + " << m.expDrop << " Deneyim Puani\n";
            std::cout << "============================================\n";
            SetColor(7);
            Sound::Victory();

            g_player.gold += m.goldDrop;
            g_player.exp += m.expDrop;
            g_player.totalGoldEarned += m.goldDrop;
            g_player.monstersKilled++;
            if (m.isBoss) g_player.bossesKilled++;

            CheckLevelUp();
            std::cout << "\nDevam etmek icin herhangi bir tusa basin...";
            _getch();
            return;
        }

        // Dusman saldirisi
        Sleep(600);
        int mDmg = (m.attack + (rand() % 6)) - (g_player.defense / 2);
        if (mDmg < 3) mDmg = 3;

        if (g_player.godMode) {
            std::cout << "[HACK] Canavar vurdu fakat GOD MODE aktif oldugu icin hasar engellendi!\n";
        } else {
            g_player.hp -= mDmg;
            FlashScreen(12, "CANAVAR SALDIRISI!");
            Sound::MonsterAttack();
            std::cout << ">> " << m.name << " vurdu ve sana " << mDmg << " hasar verdi!\n";
        }

        if (g_player.hp <= 0 && !g_player.godMode) {
            SetColor(12);
            std::cout << "\n[XXX] Gucun tukendi... Zindanin karanliginda kayboldun!\n";
            SetColor(7);
            Sound::Defeat();
            std::cout << "\nAna menuye donmek icin herhangi bir tusa basin...";
            _getch();
            return;
        }

        Sleep(700);
    }
}

// ==========================================
// 9. KASABA VE PAZAR
// ==========================================
void TownShop() {
    while (true) {
        std::vector<std::string> shopItems = {
            "Can Iksiri Al (+%50 Can) - 25 Altin",
            "Mana Iksiri Al (Full Mana) - 20 Altin",
            "Mithril Kilic Isle (+10 Saldiri) - 120 Altin",
            "Ejderha Derisi Zirh Kusan (+8 Defans) - 150 Altin",
            "Kasabadan Ayril (Geri Don)"
        };

        int pick = SelectMenu("KASABA DEMIRCISI VE BUYU PAZARI (Mevcut Altin: " + std::to_string(g_player.gold) + " G)", shopItems, 0);

        if (pick == 0) {
            if (g_player.gold >= 25) {
                g_player.gold -= 25;
                g_player.healthPotions++;
                Sound::Coin();
            } else { Sound::MenuError(); }
        } else if (pick == 1) {
            if (g_player.gold >= 20) {
                g_player.gold -= 20;
                g_player.manaPotions++;
                Sound::Coin();
            } else { Sound::MenuError(); }
        } else if (pick == 2) {
            if (g_player.gold >= 120) {
                g_player.gold -= 120;
                g_player.attack += 10;
                Sound::Coin();
            } else { Sound::MenuError(); }
        } else if (pick == 3) {
            if (g_player.gold >= 150) {
                g_player.gold -= 150;
                g_player.defense += 8;
                Sound::Coin();
            } else { Sound::MenuError(); }
        } else if (pick == 4) {
            break;
        }
    }
}

// ==========================================
// 10. ISTATISTIKLER EKRANI
// ==========================================
void ViewStats() {
    system("cls");
    SetColor(11);
    std::cout << "===============================================================\n";
    std::cout << "              KAHRAMANIN KARIYER ISTATISTIKLERI                \n";
    std::cout << "===============================================================\n";
    SetColor(7);
    std::cout << "  Kahraman Adi            : " << g_player.name << "\n";
    std::cout << "  Mevcut Seviye           : " << g_player.level << "\n";
    std::cout << "  Yok Edilen Canavar      : " << g_player.monstersKilled << "\n";
    std::cout << "  Yenilen Kadim Boss'lar  : " << g_player.bossesKilled << "\n";
    std::cout << "  Toplam Verilen Hasar    : " << g_player.totalDamageDealt << "\n";
    std::cout << "  Toplam Kazanilan Altin  : " << g_player.totalGoldEarned << " G\n";
    std::cout << "  God Mode Durumu         : " << (g_player.godMode ? "AKTIF" : "PASIF") << "\n";
    std::cout << "\n  [Bellek Bilgisi - DLL Gelistirici Icin]\n";
    std::cout << "  g_player RAM Adresi     : " << &g_player << "\n";
    std::cout << "  GetPlayerInstance()     : " << (void*)&GetPlayerInstance << "\n";
    std::cout << "===============================================================\n";
    std::cout << "\n  Geri donmek icin herhangi bir tusa basin...";
    _getch();
}

// ==========================================
// 11. ANA DONGU VE BASLANGIC
// ==========================================
int main() {
    SetConsoleOutputCP(65001); // UTF-8
    SetConsoleTitleA("Efsanevi Golge RPG - Enhanced Edition");
    srand((unsigned int)time(NULL));

    DWORD pid = GetCurrentProcessId();

    // Baslangic Menusu (Ok Tuslari ile)
    std::string saveInfo = HasSaveGame() ? "2. Kayitli Oyunu Yukle (KAYIT MEVCUT)" : "2. Kayitli Oyunu Yukle (KAYIT BULUNAMADI)";
    std::vector<std::string> startMenu = {
        "1. Yeni Maceraya Basla",
        saveInfo,
        "3. Oyundan Cikis"
    };

    std::string banner = "EFSANEVI GOLGE RPG (PROSES ID: " + std::to_string(pid) + ")";
    int initChoice = SelectMenu(banner, startMenu, 0);

    if (initChoice == 1) {
        if (HasSaveGame() && LoadGame()) {
            std::cout << "\n[+] Kayit basariyla yuklendi! Hos geldin, " << g_player.name << "!\n";
            Sleep(800);
        } else {
            Sound::MenuError();
            std::cout << "\n[-] Kayitli oyun bulunamadi, yeni oyun aciliyor...\n";
            Sleep(1000);
            InitPlayer("Savasci");
        }
    } else if (initChoice == 0) {
        system("cls");
        ShowConsoleCursor(true);
        std::cout << "\nKahramaninizin adini girin: ";
        std::string name;
        std::cin >> name;
        InitPlayer(name);
    } else {
        return 0;
    }

    // Oyun Dongusu (Ok Tuslari ile)
    while (true) {
        std::string title = "GOLGE DIYARI - KAHRAMAN: " + std::string(g_player.name) + 
                            " (Lvl " + std::to_string(g_player.level) + ") | HP: " + 
                            std::to_string(g_player.hp) + "/" + std::to_string(g_player.maxHp) +
                            " | Altin: " + std::to_string(g_player.gold) + " G";

        std::vector<std::string> actions = {
            "1. Karanlik Zindana Gir (Yaratik Avi)",
            "2. Ejderha Tapinagina In (PATRON SAVASI)",
            "3. Kasaba Ticaret Merkezine Git",
            "4. Kamp Kur ve Dinlen (Can & Mana Fullenir)",
            "5. Kariyer Istatistiklerini Incele",
            "6. Macerayi Kaydet (Save)",
            "7. Oyundan Cikis"
        };

        int act = SelectMenu(title, actions, 0);

        if (act == 0) { // Zindan
            Monster mobs[] = {
                {"Goblin Casusu", 50, 50, 14, 4, 30, 40, false},
                {"Lanetli Iskelet", 80, 80, 20, 8, 50, 65, false},
                {"Kara Muhafiz", 120, 120, 28, 12, 90, 100, false},
                {"Golge Suikastcisi", 100, 100, 35, 6, 110, 120, false}
            };
            Battle(mobs[rand() % 4]);

        } else if (act == 1) { // Boss
            Monster boss = {"KADIM ATES EJDERHASI", 450, 450, 48, 20, 600, 500, true};
            Battle(boss);

        } else if (act == 2) { // Kasaba
            TownShop();

        } else if (act == 3) { // Dinlen
            g_player.hp = g_player.maxHp;
            g_player.mana = g_player.maxMana;
            Sound::Heal();
            FlashScreen(10, "KAMP ATESI YANDI! CAN VE MANA DOLDURULDU!");
            Sleep(800);

        } else if (act == 4) { // Istatistik
            ViewStats();

        } else if (act == 5) { // Save
            if (SaveGame()) {
                Sound::Coin();
                FlashScreen(10, "OYUN BASARIYLA KAYDEDILDI (savegame.dat)");
            } else {
                Sound::MenuError();
                FlashScreen(12, "KAYIT HATASI!");
            }
            Sleep(800);

        } else if (act == 6) { // Cikis
            std::cout << "\nMacera sonlandirildi. Gorusmek uzere!\n";
            break;
        }
    }

    return 0;
}
