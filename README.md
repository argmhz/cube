# Cube

Styresoftware til en fysisk **8×8×8 RGB LED-kube** ("Topper 3D"), bygget omkring en Raspberry Pi der driver kuben via SPI/skiftregistre. Softwaren kører et bibliotek af animationer, der kan skiftes og fjernstyres live over et TCP-socket med et lille JSON-baseret kommandosprog.

Den webbaserede fjernbetjening til kuben ligger i det separate repo [`cube-client`](../cube-client), som forbinder til denne server.

Se [ARCHITECTURE.md](ARCHITECTURE.md) for status og den planlagte vej mod et mere modent setup.

## Arkitektur

```
cube-client (Node/Express, port 3000)
   │  WebSocket
   ▼
apps/socket.cpp  ── TCP-server, localhost:1234, JSON-protokol
   │
   ▼
AniManager  ── indlæser/skifter animationer (dlopen af .so-filer)
   │
   ▼
Animation-plugins (animations/*.cpp)  ── tegner ind i Cube'ens buffer
   │
   ▼
Cube (lib/core/Cube.h + CubeBuffer.h/.cpp)  ── framebuffer + bit-angle-modulation-rendering over SPI/GPIO
   │
   ▼
Fysisk LED-kube
```

- **`lib/core/Cube.h`** — hardware-laget: driver skiftregistrene over SPI via `bcm2835`-biblioteket med bit-angle modulation (BAM) for farvedybde, arver al voxel-buffer/geometri fra `CubeBuffer`. Kører sin egen renderer-tråd (`cube->start()`).
- **`lib/core/CubeBuffer.h`/`.cpp`** — den rene, hardware-fri 8×8×8 voxel-buffer og geometri (`set`, `line`, `plane`, `box`, `sphere`, `shift`, `rotate` osv.) — ingen `bcm2835`, testes uden Pi (se `tests/`).
- **`lib/core/Vec3.h`** — rotationsmatricer brugt af `CubeBuffer::rotate()`.
- **`lib/animation/Animation.h`** — basisklasse for animationer. En animation implementerer `draw(Cube*)` og valgfrit `onDataUpdate(json)` til at modtage live parameterændringer (farve, hastighed, tekst osv.).
- **`lib/animation/AniManager.h`** — indlæser animationer som delte biblioteker (`dlopen`/`dlsym` på `create()`/`destroy()`) og kan hot-swappe den kørende animation uden at genstarte processen.
- **`lib/animation/Font.h`** — bitmap-font-rendering, bruges af `Text`/`ColorText`-animationerne.
- **`animations/*.cpp`** — ca. 30 animationer (FadeColor, ColorWheel, BouncyvTwo, Sparkles, Fireworks, DoubleHelix, Text m.fl.). Hver fil kompileres til sit eget `.so`-plugin. `Spectrum` er en mikrofon-analysator: browseren laver FFT'en og streamer fire frekvensbånd, som tegnes som ringe ud fra kubens midte -- bas i midten, diskant ved ydervæggen.
- **`apps/*.cpp`** — de eksekverbare programmer (se "Kørsel" nedenfor).
- **`resources/`** — bitmap-font-data brugt af `lib/animation/Font.h`.
- **`lib/net/`** — `Socket.h`/`.cpp` (BSD-socket-wrapper), `CommandHandler.h`/`AnimationCommandHandler.h` (JSON-protokol-dispatch), `ConnectionLoop.h` (server-en-klient-løkken), `Config.h` (kommandolinje-flag). Se [ARCHITECTURE.md](ARCHITECTURE.md) for detaljer.

## Mappestruktur

| Sti | Indhold |
|---|---|
| `lib/core/` | Selve kube-motoren: `Cube`, `CubeBuffer`, `Vec3` |
| `lib/animation/` | Animations-plugin-systemet: `Animation`, `AniManager`, `Font` |
| `lib/net/` | Netværk/protokol: `Socket`, `CommandHandler`, `AnimationCommandHandler`, `ConnectionLoop`, `Config` |
| `lib/vendor/` | Tredjeparts, vendorede biblioteker: `json.hpp` (nlohmann/json), `doctest.h` |
| `lib/` (roden) | Generelle hjælpere der ikke hører til i noget af ovenstående: `helpers.h`, `Log.h`, `remotehelpers.h` |
| `animations/` | Et `.cpp`-plugin pr. animation |
| `apps/` | Eksekverbare entry points (socket-server, enkelt-animation, cyklus, tekst) |
| `resources/` | Font-bitmap-data |
| `tests/` | Hardware-fri testsuite (`make check`) |
| `deploy/` | `systemd`-service til drift på Pi'en |
| `sim/` | Simulator — kør rigtige animationer og se dem i 3D uden fysisk kube (`make sim`) |
| `bin/` | Byggeoutput (`.gitignore`'et — oprettes lokalt, se Opsætning) |
| `Makefile` | Bygger alle animationer til `.so` og alle apps til eksekverbare filer, samt testsuiten (`make check`) |
| `generate.sh` | Scaffolder en ny animationsfil ud fra en skabelon |
| `run.sh` | Genvej til at køre én animation direkte (`sudo bin/one bin/animations/<navn>.so`) |

Hvorfor `lib/core/CubeBuffer` og `lib/net/Socket` har både en `.h` og en `.cpp`, mens resten af `lib/` kun har `.h`-filer: se [ARCHITECTURE.md](ARCHITECTURE.md#header-cpp-organisering).

## Bit-angle modulation (BAM)

En LED kan kun være helt tændt eller helt slukket — der er ingen analog dæmpning. `Cube::run()` (`lib/core/Cube.h`) skaber alligevel 16 lysstyrkeniveauer pr. farvekanal ved at blinke hver LED så hurtigt, at øjet ikke kan følge med, og styre *hvor længe* den er tændt inden for hvert blink.

`cube->set(x, y, z, r, g, b)` tager `r`/`g`/`b` som tal 0-15 — altså 4 bits pr. farve. `createFrame()` splitter det tal op i sine fire enkelte bits ("bit-planer"). Værdien 7 (`0111`) bliver til:

```
bit 0 (vægt 1): tændt
bit 1 (vægt 2): tændt
bit 2 (vægt 4): tændt
bit 3 (vægt 8): slukket
```

I `run()`'s render-løkke løber en tæller (`bam_counter`) fra 0 til 14 — 15 tidsslots i alt — og de fire bit-planer vises ikke lige længe:

| bit-plan | vægt | tidsslots |
|---|---|---|
| bit 0 | 1 | 1 |
| bit 1 | 2 | 2 |
| bit 2 | 4 | 4 |
| bit 3 | 8 | 8 |

*(1+2+4+8 = 15, det samlede antal tidsslots.)*

En pixel med værdien 7 er tændt i de slots hvor bit 0, 1 og 2 vises (1+2+4 = 7 af de 15 slots) og slukket når bit 3 vises (8 slots) — øjet ser den som "styrke 7 ud af 15". Det er derfor det hedder bit-**angle**-modulation: hvert bit i det binære tal får sin egen "vinkel" (tidsandel) af cyklussen, dobbelt så meget som det forrige.

Fordelen frem for bare at tælle lige langsomt op til 15: hardwaren skal kun opdateres 4 gange (én gang pr. bit-plan) for at nå 16 niveauer, i stedet for 15 gange — vigtigt når SPI-bussen i forvejen skal nå 512 voxels × 3 farver × 8 lag mange gange i sekundet.

Selve laget vælges også ved multipleksning: kuben har kun ledninger til at styre ét Y-lag ad gangen (`layers[8] = {128,64,32,16,8,4,2,1}` vælger ét lag via GPIO). Så for hvert af de 15 tidsslots løbes alle 8 lag igennem, og hvert lag får sin egen 4-bit-plan sendt over SPI — hele cyklussen sker så hurtigt, at man hverken ser lag-skiftet eller blinket, kun den samlede, jævnt dæmpede farve.

## Hardware & afhængigheder

- Raspberry Pi med SPI aktiveret, forbundet til skiftregistrene der driver kuben (lag-select på GPIO `P1_11`/latch og `P1_15`/output-enable, se `Cube::initBcm2835`).
- [`libbcm2835`](http://www.airspayce.com/mikem/bcm2835/) til GPIO/SPI-adgang — skal være installeret på target-Raspberry Pi'en.
- `g++` med C++17-understøttelse, POSIX-tråde, `dl` (dynamisk linking) og `stdc++fs`.
- SPI-transfers og GPIO-adgang kræver typisk root, derfor køres apps med `sudo`.

## Opsætning & build

`bin/` er git-ignoreret og skal oprettes manuelt før første build:

```bash
mkdir -p bin/animations
make build
```

`Makefile` finder automatisk alle filer i `animations/*.cpp` og `apps/*.cpp`:

- Hver animation bygges som et delt bibliotek: `bin/animations/<navn>.so`
- Hver app bygges som en selvstændig eksekverbar: `bin/<navn>`

Byg skal køres direkte på (eller krydskompileret til) en Raspberry Pi, da koden linker mod `libbcm2835` og bruger dens GPIO/SPI-kald.

## Kørsel

| Kommando | Beskrivelse |
|---|---|
| `sudo bin/socket` | Starter netværksserveren — den primære driftsform. Lytter på TCP `localhost:1234` og fjernstyres via JSON-kommandoer (fx fra `cube-client`). |
| `./run.sh <navn>` | Kører én enkelt animation direkte (`sudo bin/one bin/animations/<navn>.so`), uden netværk. |
| `sudo bin/all` | Cykler igennem alle byggede animationer i `bin/animations/`. |
| `sudo bin/ColorText "tekst"` | Viser en rullende tekststreng i farveskiftende gradient. |
| `sudo bin/test` | Demoprogram, roterer en linje om X-aksen. |
| `./bin/sim-socket --animations-dir ./bin/sim-animations --stream-port 8421` | Samme server, bygget til din egen maskine uden Pi. Se [`sim/README.md`](sim/README.md). |

`bin/socket` tager valgfrie flag, hvis standardværdierne (`localhost:1234`, `./bin/animations`) ikke passer:

```bash
sudo bin/socket --host 0.0.0.0 --port 1234 --animations-dir ./bin/animations
```

Se [deploy/README.md](deploy/README.md) for at køre serveren som en `systemd`-service.

## Simulator

Se hvordan en animation ser ud i 3D uden den fysiske kube — nyttigt når man udvikler eller fejlsøger uden at stå ved Pi'en:

```bash
make sim
./bin/simulator --animation Prism --seconds 10 --out /tmp/prism.jsonl
node sim/viewer/server.js
```

Åbn derefter `http://localhost:8420` — en helt selvstændig, offline-kørende 3D-afspiller (ingen Claude, ingen internet nødvendig). Se [sim/README.md](sim/README.md) for detaljer.

## Netværksprotokol

`apps/socket.cpp` binder en TCP-server på `localhost:1234` og accepterer én klientforbindelse ad gangen. Beskeder er JSON-objekter med et `"action"`-felt:

**Klient → server**

```json
{"action": "options"}
{"action": "select", "animation": "./bin/animations/ColorWheel.so"}
{"action": "set", "speed": 20000}
```

- `select` — skifter den kørende animation til den angivne `.so`-fil.
- `set` — videresendes til den aktive animations `onDataUpdate(json)`, så individuelle parametre (farve, hastighed, tekst osv.) kan opdateres live. Hvilke felter der er gyldige afhænger af den enkelte animation.
- `options` — beder serveren svare med listen af tilgængelige animations-`.so`-filer.

**Server → klient**

```json
{"action": "options", "animations": ["./bin/animations/ColorWheel.so", "./bin/animations/FadeColor.so", ...]}
```

`cube-client` fungerer som en TCP↔WebSocket-bro, så en browser (der ikke kan åbne rå TCP-sockets) kan sende og modtage præcis disse JSON-beskeder.

## Lav en ny animation

```bash
./generate.sh MinNyeAnimation
```

Genererer `animations/MinNyeAnimation.cpp` ud fra en skabelon med en tom `draw(Cube *cube)`. Enhver klasse der arver fra `Animation` og implementerer `draw()` (og valgfrit `onDataUpdate()`) kan bygges som et `.so` og vælges/fjernstyres via `select`/`set`-kommandoerne uden at genstarte serveren.

## Relateret projekt

[`cube-client`](../cube-client) er en lille Node.js/Express-webflade der:

1. Åbner en TCP-forbindelse til denne servers `localhost:1234`.
2. Serverer en statisk webside (Bootstrap 5) med en dropdown over animationer og dynamisk genererede kontrolformularer (slidere, farvevælgere osv.) ud fra JSON-skemaer pr. animation.
3. Bro-forbinder browserens WebSocket til TCP-socket'en, så UI'ets valg og parameterændringer sendes videre som de samme `select`/`set`/`options`-JSON-beskeder som beskrevet ovenfor.

De to repos forventes at køre sammen på samme maskine (begge peger på `localhost`).
