# Refresh timing (CrowPanel 5.79", measured 2026-10-30)

Measured with `pio run -e diag -t upload`; the board logs to serial at 115200.

| Operation | Time |
|---|---|
| Full refresh (clear pass + full-quality draw, `EPD_Update`) | ~6.0 s |
| Partial refresh (`EPD_HW_RESET` + `EPD_Display` + `EPD_PartUpdate`) | ~1.3 s (10 of 10 runs 1296-1297 ms) |

Consequences:
- Updates block the main loop for the duration, so a press shorter than the refresh can be missed unless it is latched (see Task 6 notes).
- Timer cadence: digits at most once per minute, bar every few seconds, last minute once per second at most (1.3 s refresh floor).
- Ghosting after repeated partial refreshes has not been judged yet (needs eyes on the panel); `kPartialsBeforeFull` is now 500 (no ghosting seen on the panel; the full refresh was disruptive).

## After optimisation (hardware SPI + bulk transfer + direct pixel pack, 2026-10-31)

| Phase of a partial refresh | Before | After |
|---|---|---|
| Pixel copy (canvas to driver buffer) | 305 ms | 18 ms |
| Send new image over SPI | 185 ms | 49 ms |
| Panel waveform (`EPD_PartUpdate`) | 371 ms | 370 ms (hardware floor) |
| Sync old-image RAM | n/a | 24 ms |
| **Total** | **1021 ms** | **461 ms** |

Full refresh: 3.7 s with the clear pass (boot and every 30 updates), 1.8 s without. Each update also writes the
shown image into the controller's old-image RAM so the next partial diffs against what is on the glass.
Windowed (partial-area) updates would not help: the waveform time is the same for any window.
