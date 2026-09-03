# DLL Enjeksiyon ve Oyun Bellek Manipülasyonu

Bu proje, Windows işletim sistemi üzerinde C++ ile geliştirilmiş bir konsol RPG oyunu (oyun.exe), bu oyuna harici olarak çalışma zamanında kod/kütüphane enjekte eden bir DLL enjektörü (injector.exe) ve oyunun bellek alanına dahil olarak çeşitli hile özellikleri sunan dinamik bağlantı kütüphanesini (hack_dll.dll) içerir.

## Proje Yapısı

- **Game / Hacks**:
  - oyun.cpp / oyun.exe: Konsol tabanlı, menülü, ses efektli ve kayıt (save/load) destekli mini RPG oyunu. GetPlayerInstance fonksiyonunu dışa aktararak (export) bellek manipülasyonuna olanak tanır.
  - injector.cpp / injector.exe: Windows API (OpenProcess, VirtualAllocEx, WriteProcessMemory, CreateRemoteThread) fonksiyonlarını kullanarak DLL dosyasını hedef prosesin belleğine yükleyen enjektör aracı. Otomatik PID bulma veya manuel PID girişi destekler.
  - hack_dll.cpp / hack_dll.dll: Hedef prosese enjekte edildiğinde ayrı bir thread (CheatThread) başlatarak oyun verilerini manipüle eden ve klavye kısayollarını dinleyen DLL.

## Hile Kısayolları (In-Game)

DLL oyuna enjekte edildikten sonra oyun penceresinde aşağıdaki tuş kombinasyonları kullanılabilir:

| Tuş | Açıklama |
|---|---|
| **G** | God Mode (Ölümsüzlük) Aç / Kapat |
| **M** | Sınırsız / Yüksek Miktarda Para Ekle (+50.000 Altın) |
| **K** | Kill Power (Saldırı ve Defans Değerlerini Arttırır) |
| **P** | Can ve Mana İksirlerini Yeniler, Manayı Tazeler |
| **L** | Seviyeyi 50 Yapar ve Nitelikleri Arttırır |

## Derleme (MinGW / GCC)

Projeyi kaynak koddan derlemek isterseniz aşağıdaki komutları kullanabilirsiniz:

`ash
# Oyunun Derlenmesi
g++ -O2 -o oyun.exe oyun.cpp

# DLL Dosyasının Derlenmesi
g++ -O2 -shared -o hack_dll.dll hack_dll.cpp

# Injector'ın Derlenmesi
g++ -O2 -o injector.exe injector.cpp
`

## Kullanım

1. oyun.exe dosyasını çalıştırın ve yeni bir oyuna başlayın ya da mevcut kaydınızı yükleyin.
2. injector.exe dosyasını çalıştırın.
3. Menüden **1** numaralı seçeneği (Arkada Oyun Açık mı Tara) seçerek otomatik enjeksiyonu gerçekleştirin ya da oyunun PID numarasını manuel girin.
4. Başarılı enjeksiyon uyarısından sonra oyun penceresine dönerek kısayol tuşlarını kullanabilirsiniz.
