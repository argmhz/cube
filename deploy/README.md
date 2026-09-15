# Deploy

## `cube.service` — kør serveren som en systemd-service

Auto-starter `bin/socket` ved boot, og genstarter den automatisk hvis den crasher.

1. **Ret stien først** — `deploy/cube.service` bruger `/home/pi/cube` som pladsholder for `WorkingDirectory`/`ExecStart`. Ret den til hvor `cube`-repoet rent faktisk ligger på jeres Pi.
2. Byg som normalt (`mkdir -p bin/animations && make build`) på Pi'en, så `bin/socket` og `bin/animations/*.so` findes.
3. Installér servicen:
   ```bash
   sudo cp deploy/cube.service /etc/systemd/system/cube.service
   sudo systemctl daemon-reload
   sudo systemctl enable --now cube
   ```
4. Tjek status og log:
   ```bash
   sudo systemctl status cube
   journalctl -u cube -f
   ```

Efter en kodeændring: `sudo systemctl restart cube` for at genindlæse den nybyggede `bin/socket`.
