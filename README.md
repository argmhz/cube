# Cube

Styresoftware til en fysisk **8×8×8 RGB LED-kube** ("Topper 3D"), bygget omkring en Raspberry Pi der driver kuben via SPI/skiftregistre. Softwaren kører et bibliotek af animationer, der kan skiftes og fjernstyres live over et TCP-socket med et lille JSON-baseret kommandosprog.

Den webbaserede fjernbetjening til kuben ligger i det separate repo [`cube-client`](../cube-client), som forbinder til denne server.

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
Cube (lib/Cube.h/.cpp)  ── framebuffer + bit-angle-modulation-rendering over SPI/GPIO
   │
   ▼
Fysisk LED-kube
```

- **`lib/Cube.h` / `lib/Cube.cpp`** — kernen. Holder en 8×8×8 voxel-framebuffer og driver skiftregistrene over SPI via `bcm2835`-biblioteket med bit-angle modulation (BAM) for farvedybde. Tilbyder tegnehjælpere som `set`, `line`, `plane`, `box`, `sphere`, `shift`, `rotate` osv. Kører sin egen renderer-tråd (`cube->start()`).
- **`lib/Animation.hpp`** — basisklasse for animationer. En animation implementerer `draw(Cube*)` og valgfrit `onDataUpdate(json)` til at modtage live parameterændringer (farve, hastighed, tekst osv.).
- **`lib/AniManager.cpp`** — indlæser animationer som delte biblioteker (`dlopen`/`dlsym` på `create()`/`destroy()`) og kan hot-swappe den kørende animation uden at genstarte processen.
- **`animations/*.cpp`** — ca. 30 animationer (FadeColor, ColorWheel, BouncyvTwo, Sparkles, Fireworks, DoubleHelix, Text m.fl.). Hver fil kompileres til sit eget `.so`-plugin.
- **`apps/*.cpp`** — de eksekverbare programmer (se "Kørsel" nedenfor).
- **`resources/`** — genererede data (bitmap-fonte, formdata) brugt af bl.a. `Text`-animationen.
- **`lib/Socket.h`/`.cpp`** — en simpel BSD-socket-wrapper (oprindeligt en generisk "SocketServer"-skabelon), bruges som netværkslaget i `apps/socket.cpp`.

## Mappestruktur

| Sti | Indhold |
|---|---|
| `lib/` | Kernebibliotek: `Cube`, `Animation`, `AniManager`, `Socket`, hjælpefunktioner (`helpers.h`), JSON-bibliotek (`json.hpp`, vendoret nlohmann/json) |
| `animations/` | Et `.cpp`-plugin pr. animation |
| `apps/` | Eksekverbare entry points (socket-server, enkelt-animation, cyklus, tekst) |
| `resources/` | Genererede font- og formdata |
| `bin/` | Byggeoutput (`.gitignore`'et — oprettes lokalt, se Opsætning) |
| `Makefile` | Bygger alle animationer til `.so` og alle apps til eksekverbare filer |
| `generate.sh` | Scaffolder en ny animationsfil ud fra en skabelon |
| `run.sh` | Genvej til at køre én animation direkte (`sudo bin/one bin/animations/<navn>.so`) |

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
