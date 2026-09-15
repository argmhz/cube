# Arkitekturplan

Dette dokument lægger vejen fra det nuværende setup til et mere modent setup, i faser. Hver fase er tænkt som sin egen, selvstændige omgang/PR.

## Status

- ✅ `CubeBuffer` (ren voxel-buffer/geometri) er adskilt fra `Cube` (SPI/GPIO-hardware) — se [README.md](README.md#arkitektur).
- ✅ Hardware-fri testsuite (`make check`, `tests/`), ingen `bcm2835`/`sudo`/Pi krævet.
- ✅ De 9 bugs fundet under code review er rettet og dækket af tests.
- ✅ Fase 1 (nedenfor): korrekt, matrix-baseret 3D-rotation.

## Fase 1: Korrekt 3D-matematik

`lib/Vec3.h` samler de grundlæggende vektor-/rotationsformler ét sted, testet, i stedet for hånd-udledte per-akse-formler i `CubeBuffer::rotate()` (hvor vi bl.a. fandt at AXIS_Y-rotationen brugte `y` i stedet for `z`). Rotation sker nu omkring kubens centrum `(3.5,3.5,3.5)` i stedet for hjørnet `(0,0,0)`.

Naturlige udvidelser herfra, når der er appetit på det: flere primitiver (cylinder, torus), en `Vec3`-baseret variant af de eksisterende tegnefunktioner, eller double buffering (klassen hedder allerede `frontBuffer` — der er aldrig blevet en `backBuffer`).

## Fase 2: Protokol & sikkerhed

- Udtræk JSON-kommando-dispatchen i `apps/socket.cpp`s `incoming()` (som i dag blander parsing, det globale `manager`/`cube`-state, og selve socket-I/O) til en selvstændig klasse/funktion der tager en kommando ind og returnerer/udfører en handling — testbar uden en levende socket. Nævnt allerede i test-infrastruktur-omgangen som en naturlig fase 2.
- Valider `select`-kommandoens `animation`-felt: i dag sendes en rå sti direkte til `dlopen()`. Skift til kun at acceptere et animations-*navn* (uden sti/`.so`), som serveren selv slår op i `./bin/animations/` — undgår at en vilkårlig sti kan blive åbnet, og matcher allerede hvordan `cube-client` viser animationsnavne (stripped for sti/endelse).

## Fase 3: Driftssikkerhed

- `apps/socket.cpp` accepterer i dag præcis én TCP-forbindelse, én gang — falder `cube-client` fra, skal serveren genstartes manuelt. Skal løkke tilbage til `accept()` efter disconnect.
- En `systemd`-service (`deploy/cube.service`) med `Restart=on-failure` og start ved boot, så serveren ikke kræver manuel opstart efter en Pi-genstart eller et crash.
- Rigtig logging (kommandoer modtaget, connect/disconnect, fejl) i stedet for tavse `catch(json::exception&){}` og den i dag ubrugte `response()`-funktion.

## Fase 4: Konfiguration & tilstand

- Flyt hardkodede værdier (`localhost:1234`, `./bin/animations`) til en lille konfiguration (fil eller kommandolinje-argumenter).
- Husk sidst valgte animation og dens sidste parametre på tværs af genstart, i stedet for altid at boote ind i IP-visnings-animationen.

## Fase 5: Build & CI

- Overvej CMake i stedet for den håndrullede `Makefile` — bedre header-afhængighedssporing (i dag ingen, så en ændret `.h`-fil genbygger ikke altid det der burde), og mulighed for at cross-kompilere mod Raspberry Pi'en fra en almindelig maskine via en toolchain-fil, i stedet for kun at kunne bygge på selve Pi'en.
- En GitHub Actions-workflow der kører `make check` på hvert push/PR.
- Byg videre på den `bcm2835.h`-stub der er brugt manuelt et par gange undervejs i dette projekt, til et permanent CI-tjek af at alle animationer/apps stadig kompilerer (mod en rigtig ARM-krydskompiler, hvis muligt) — uden at røre den fysiske Pi.
