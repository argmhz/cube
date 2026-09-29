# Arkitekturplan

Dette dokument lægger vejen fra det nuværende setup til et mere modent setup, i faser. Hver fase er tænkt som sin egen, selvstændige omgang/PR.

## Status

- ✅ `CubeBuffer` (ren voxel-buffer/geometri) er adskilt fra `Cube` (SPI/GPIO-hardware) — se [README.md](README.md#arkitektur).
- ✅ Hardware-fri testsuite (`make check`, `tests/`), ingen `bcm2835`/`sudo`/Pi krævet.
- ✅ De 9 bugs fundet under code review er rettet og dækket af tests.
- ✅ Fase 1 (nedenfor): korrekt, matrix-baseret 3D-rotation.
- ✅ Fase 2 (nedenfor): udtrukket, testbar kommando-dispatch; path traversal i `select` lukket.
- ✅ Fase 3 (nedenfor): reconnect-loop, rigtig logging, `systemd`-service.
- ✅ Fase 4 (nedenfor): host/port/animations-mappe er nu konfigurerbare kommandolinje-flag.
- ✅ Header/cpp-organisering (nedenfor): rigtig `.h`/`.cpp`-adskillelse og undermapper i `lib/`.

## Fase 1: Korrekt 3D-matematik

`lib/core/Vec3.h` samler de grundlæggende vektor-/rotationsformler ét sted, testet, i stedet for hånd-udledte per-akse-formler i `CubeBuffer::rotate()` (hvor vi bl.a. fandt at AXIS_Y-rotationen brugte `y` i stedet for `z`). Rotation sker nu omkring kubens centrum `(3.5,3.5,3.5)` i stedet for hjørnet `(0,0,0)`.

Naturlige udvidelser herfra, når der er appetit på det: flere primitiver (cylinder, torus), en `Vec3`-baseret variant af de eksisterende tegnefunktioner, eller double buffering (klassen hedder allerede `frontBuffer` — der er aldrig blevet en `backBuffer`).

## Fase 2: Protokol & sikkerhed

`lib/net/CommandHandler.h` (ren dispatch, `handleCommand()`) og `lib/net/AnimationCommandHandler.h` (den rigtige implementering, wrapper om `AniManager`) udtrækker JSON-kommando-håndteringen fra `apps/socket.cpp`s `incoming()` til noget testbart uden en levende socket. `resolveAnimationPath()` henter kun animations-*navnet* ud af hvad `select` end sender (bart navn, `"X.so"`, eller den hidtidige fulde sti), og bygger selv den fulde sti udelukkende inden for `./bin/animations/` — en `select` med fx `"../../etc/passwd"` kan derfor aldrig give `dlopen()` en sti uden for animations-mappen. `options`-svaret sender nu samme bare navne. `cube-client` krævede ingen ændringer (den ekkoer altid `options`-svaret uændret tilbage i `select`).

## Fase 3: Driftssikkerhed

`lib/net/ConnectionLoop.h`s `serveConnection()` opdager nu korrekt når en klient disconnecter (`socket_read() <= 0`) og returnerer, så `apps/socket.cpp`s `incoming()` kan løkke tilbage til `accept()` i stedet for at kræve en manuel genstart af `bin/socket`. `lib/Log.h` giver leveled logging til stderr (fanges af `journalctl` under systemd) — modtagne handlinger, connect/disconnect, og JSON-parse-fejl logges nu i stedet for at forsvinde tavst. `deploy/cube.service` + `deploy/README.md` giver en `systemd`-service med `Restart=on-failure` og auto-start ved boot (kræver at du retter pladsholder-stien til jeres rigtige Pi-sti før installation).

## Fase 4: Konfiguration

`lib/net/Config.h`s `parseArgs()` udtrækker `--host`/`--port`/`--animations-dir` fra kommandolinjen (med samme standardværdier som før: `localhost`, `1234`, `./bin/animations`), så `apps/socket.cpp` ikke længere hardkoder dem. Se [README.md](README.md#kørsel).

Bevidst fravalgt: at huske sidst valgte animation/parametre på tværs af genstart — det er en tilstand, ikke konfiguration, og blev vurderet ikke ønsket.

Hvis `--port` eller `--host` nogensinde ændres fra standardværdien, skal `cube-client`s egen hardkodede forbindelse (i `index.js`) opdateres tilsvarende — den er stadig uden for scope for dette repo.

## Header/cpp-organisering

Bevidst fravalgt: automatisk CI/build — der bygges fortsat i hånden, direkte på Raspberry Pi'en, med `make build`.

I C++ bør en `.cpp`-fil kun kompileres ét sted og aldrig `#include`s af andre filer — det er en headers (`.h`) job at blive delt rundt. Kodebasen brød den regel i praksis: `Cube.cpp`, `AniManager.cpp`, `Animation.cpp`, `Font.cpp` og `remotehelpers.cpp` var reelt headers (kun `inline`/i-klassekrop-indhold), men hed `.cpp` og blev tekst-inkluderet — mens `CubeBuffer.cpp` og `Socket.cpp` faktisk havde rigtige, selvstændige metodedefinitioner.

Begge dele er nu rettet:

- **Rigtig `.h`/`.cpp`-adskillelse**: `lib/core/CubeBuffer.h`/`.cpp` og `lib/net/Socket.h`/`.cpp` er de eneste to steder med den klassiske opdeling (erklæring i `.h`, definition i `.cpp`) — `Makefile`s `$(animations)`/`$(apps)`-regler kompilerer nu `CubeBuffer.cpp` (og for `apps/socket.cpp`s vedkommende også `Socket.cpp`) sammen med hver enkelt animation/app, i stedet for at tekst-inkludere dem via den nu-fjernede `lib/Cube.cpp`-shim. Hver animations-`.so` får sin egen kompilerede kopi — bevidst simplere og mere robust end at dele én kopi via dynamisk symbol-opslag mellem værtsproces og `dlopen`'ede plugins.
- **De reelle headers hedder nu `.h`**: `Cube.cpp`→`Cube.h` (allerede rigtig, ingen omdøbning nødvendig), `AniManager.cpp`→`lib/animation/AniManager.h`, `Animation.hpp`→`lib/animation/Animation.h`, `Font.cpp`→`lib/animation/Font.h`, `remotehelpers.cpp`→`lib/remotehelpers.h`, `resources/*.cpp`→`.h` (arrays markeret `inline` for at være ODR-sikre). `Animation.cpp` (tilføjede intet ud over `Animation.hpp`) og `Cube.cpp` (kun en 2-liners include-shim) er fjernet helt.
- **Undermapper i `lib/`**: `core/` (kube-motoren), `animation/` (plugin-systemet), `net/` (netværk/protokol), `vendor/` (tredjeparts — `json.hpp`, `doctest.h`), resten (`helpers.h`, `Log.h`, `remotehelpers.h`) direkte i `lib/`. Se [README.md](README.md#mappestruktur).
