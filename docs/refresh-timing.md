# Refresh timing (CrowPanel 5.79", measured 2026-10-30)

Measured with `pio run -e diag -t upload`; the board logs to serial at 115200.

| Operation | Time |
|---|---|
| Full refresh (clear pass + full-quality draw, `EPD_Update`) | ~6.0 s |
| Partial refresh (`EPD_HW_RESET` + `EPD_Display` + `EPD_PartUpdate`) | ~1.3 s (10 of 10 runs 1296-1297 ms) |

Consequences:
- Updates block the main loop for the duration, so a press shorter than the refresh can be missed unless it is latched (see Task 6 notes).
- Timer cadence: digits at most once per minute, bar every few seconds, last minute once per second at most (1.3 s refresh floor).
- Ghosting after repeated partial refreshes has not been judged yet (needs eyes on the panel); `kPartialsBeforeFull` is still the plan's initial 30.
