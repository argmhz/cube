# Simulator

Kør en rigtig, uændret animation uden den fysiske kube, og se resultatet i 3D — til at afprøve nye animationer eller genkende bugs, uden at skulle stå ved Pi'en.

## Sådan virker det

`lib/core/Cube.h` er den eneste fil der kræver rigtig hardware (`bcm2835`). `sim/fake_bcm2835/` er en rigtig, linkbar udgave af de samme 9 funktioner, der bare ikke gør noget — så `Cube.h`, `AniManager` og selve animationsfilerne kan bygges og køres helt uændret på en almindelig maskine. `sim/simulator.cpp` loader én animation (samme `dlopen`-mekanisme som den rigtige server bruger) og optager ~30 frames i sekundet til en `.jsonl`-fil — én linje pr. frame, med alle 512 voxels' farver.

## Sådan bruger du det

**1. Byg simulatoren** (kun første gang, eller efter en kodeændring):

```bash
make sim
```

Bygger hver animation som en almindelig x86-`.so` i `bin/sim-animations/`, samt selve `bin/simulator`.

**2. Optag en animation:**

```bash
./bin/simulator --animation Prism --seconds 10 --out /tmp/prism.jsonl
```

- `--animation <navn>` — skal matche en fil i `animations/` (uden `.cpp`), fx `Prism`, `ColorWheel`, `FadeColor`.
- `--seconds <N>` — hvor længe der optages (standard 10).
- `--out <sti>` — hvor `.jsonl`-optagelsen gemmes. Udelades den, skrives til terminalen i stedet.

**3. Se optagelsen i 3D:**

Åbn [Cube Playback](https://claude.ai/artifact/BiwxqNsU6iw7SapEsNCqnR) og klik "Indlæs .jsonl" — vælg filen du lige optog. Træk for at rotere, scroll for at zoome, brug afspil/pause/scrub-linjen til at gå frem og tilbage i optagelsen.

## Begrænsninger

- Kun til at *se* en animation — den fysiske kube, `apps/socket.cpp` og `cube-client` er slet ikke involveret.
- `Cube::run()`s render-tråd spinner så hurtigt den kan (ingen rigtig SPI-hastighed at vente på), og bruger derfor en hel CPU-kerne mens simulatoren kører. Fint til korte optagelser, ikke tænkt til at køre i timevis.
- `bin/sim-animations/*.so` er bygget til din maskine (x86), ikke til Pi'en — bland dem aldrig sammen med de rigtige `.so`-filer i `bin/animations/`.
