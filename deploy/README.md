# Deploy

Guide til at sætte `cube.service` op på en Pi hvor den ikke kører som systemd-service endnu.

## 0. Find ud af hvad der auto-starter den i dag

Du nævnte at kuben allerede starter automatisk ved boot i dag — men den nye `cube.service` er ikke installeret endnu. Det betyder der er en anden mekanisme i spil (fx et `@reboot`-cronjob, en linje i `/etc/rc.local`, eller en anden systemd-service sat op manuelt). **Find og sluk den, før du installerer den nye service** — ellers kan to ting ende med at kæmpe om at binde port 1234 samtidig.

```bash
# Findes der allerede en systemd-service for cube?
systemctl status cube 2>&1
systemctl list-units --type=service --all | grep -i cube

# Cronjob?
crontab -l 2>&1
sudo crontab -l 2>&1

# rc.local?
cat /etc/rc.local 2>&1
```

Find du noget af det, så sluk/fjern det (fx `sudo systemctl disable --now <navn>` for en gammel service, eller slet linjen fra crontab/`rc.local`) før du fortsætter.

## 1. Byg

```bash
cd <sti-til-cube-repoet>
mkdir -p bin/animations
make build
```

## 2. Installér `cube.service`

1. **Ret stien først** — `deploy/cube.service` bruger `/home/pi/cube` som pladsholder for `WorkingDirectory`/`ExecStart`. Ret den til hvor `cube`-repoet rent faktisk ligger på jeres Pi.
2. Installér:
   ```bash
   sudo cp deploy/cube.service /etc/systemd/system/cube.service
   sudo systemctl daemon-reload
   sudo systemctl enable --now cube
   ```
3. Tjek status og log:
   ```bash
   sudo systemctl status cube
   journalctl -u cube -f
   ```

Efter en kodeændring: `sudo systemctl restart cube` for at genindlæse den nybyggede `bin/socket`.

## 3. Vent reelt på netværket ved boot

`cube.service` venter på `network-online.target`, så `bin/socket` ikke starter (og dermed forsøger at vise IP-adressen) før netværket reelt er oppe — vigtigt særligt på WiFi, hvor det kan tage nogle sekunder efter boot. Det kræver at Raspberry Pi OS' egen "vent på netværk"-tjeneste er aktiveret:

```bash
# Tjek først hvilken du har:
systemctl status NetworkManager 2>&1 | head -3
systemctl status dhcpcd 2>&1 | head -3

# Aktivér den relevante:
sudo systemctl enable NetworkManager-wait-online.service   # NetworkManager (standard på nyere Raspberry Pi OS)
# eller:
sudo systemctl enable systemd-networkd-wait-online.service # dhcpcd/systemd-networkd
```

`getIpAddress()` (i `lib/remotehelpers.h`) prøver desuden selv igen i op til ~10 sekunder, som et sikkerhedsnet, hvis netværket alligevel er lidt langsomt om at komme op.

## 4. Test hele vejen igennem

```bash
sudo reboot
```

Vent til Pi'en er oppe igen, og tjek:

```bash
systemctl status cube          # skal være "active (running)"
journalctl -u cube -b          # log fra dette boot -- se om den ventede/fandt IP hurtigt
```

Kuben bør vise en gyldig IP-adresse på Text-animationen ved opstart, uden en lang tom/blank periode.

## Om fallback-WiFi'et

Hvis Pi'en selv opretter sit eget WiFi-hotspot når den ikke kan logge på det konfigurerede netværk (fx via NetworkManager's egen AP-fallback), er det værd at vide: `getIpAddress()` viser i så fald hotspottets egen IP (typisk noget i stil med `10.42.0.1`), ikke en adresse på jeres hjemmenetværk — det er forventet, ikke en fejl. `network-online.target` bliver i den situation opfyldt af hotspot-forbindelsen (den *er* jo "online", bare på sit eget netværk), så boot-sekvensen ikke hænger og venter på et netværk der aldrig dukker op.
