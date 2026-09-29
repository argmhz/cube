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
- `--params '<json>'` — sendes til animationens `onDataUpdate()` inden den starter, altså præcis samme vej som når en slider flyttes i cube-client. Gør det muligt at prøve indstillinger af uden hverken Pi eller socket:

  ```bash
  ./bin/simulator --animation Halo --seconds 10 \
    --params '{"rings":3,"thickness":0.4,"spread":1.2}' --out /tmp/halo.jsonl
  ```

**3. Se optagelsen i 3D:**

```bash
node sim/viewer/server.js
```

Åbn derefter `http://localhost:8420` i browseren og klik "Indlæs .jsonl" — vælg filen du lige optog. Træk for at rotere, scroll for at zoome, brug afspil/pause/scrub-linjen til at gå frem og tilbage i optagelsen. Siden åbner med en indbygget Prism-optagelse som eksempel.

`sim/viewer/` er en helt selvstændig, offline-kørende side — ingen `npm install`, ingen forbindelse til Claude eller internettet nødvendig (Three.js ligger vendoret i `sim/viewer/public/vendor/`). Et andet portnummer kan gives som argument: `node sim/viewer/server.js 3000`.

## Live: hele stakken uden en Pi

Optagelsen ovenfor kører én animation isoleret. Skal hele kæden afprøves --
browser, `cube-client`, socket-serveren og animationen -- bygger `make sim`
også `bin/sim-socket`: den rigtige, uændrede `apps/socket.cpp` bygget mod den
falske `bcm2835`.

**1. Start serveren** (den lytter på TCP 1234 præcis som på Pi'en, og sender
samtidig hvert frame videre til viewerens live-feed):

```bash
./bin/sim-socket --animations-dir ./bin/sim-animations --stream-port 8421
```

**2. Start vieweren** og klik **Live** i stedet for "Indlæs .jsonl":

```bash
node sim/viewer/server.js
```

**3. Start `cube-client`** i det andet repo og åbn `http://localhost:3000`:

```bash
npm install && node index.js
```

Vælg en animation i dropdownen, og den skifter i vieweren. Vælges `Spectrum`,
dukker et mikrofonkort op: slå det til, og browserens FFT streamer fire
frekvensbånd til kuben ~30 gange i sekundet.

`--stream-port` er slået fra som standard, så Pi'en hverken lytter eller
serialiserer et eneste frame. Frame-strømmen læser kun voxel-bufferen og kan
derfor ikke forstyrre det animationen tegner.

> Skift af animation kan tage op til ~8 sekunder første gang: opstarts-`Text`
> (den der viser IP'en) tjekker først `isRunning()` igen, når hele teksten er
> rullet færdig.

## Begrænsninger

- `bin/simulator` er kun til at *se* en animation — den fysiske kube, `apps/socket.cpp` og `cube-client` er slet ikke involveret. (Det er netop det `bin/sim-socket` ovenfor råder bod på.)
- `Cube::run()`s render-tråd spinner så hurtigt den kan (ingen rigtig SPI-hastighed at vente på), og bruger derfor en hel CPU-kerne mens simulatoren kører. Fint til korte optagelser, ikke tænkt til at køre i timevis.
- `bin/sim-animations/*.so` er bygget til din maskine (x86), ikke til Pi'en — bland dem aldrig sammen med de rigtige `.so`-filer i `bin/animations/`.
