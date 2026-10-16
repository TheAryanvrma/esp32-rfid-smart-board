# Wiring notes

## Low-voltage side only

Disconnect USB power before changing wiring.

| Device pin | ESP32 connection |
|---|---|
| RC522 SDA/SS | GPIO 5 |
| RC522 SCK | GPIO 18 |
| RC522 MOSI | GPIO 23 |
| RC522 MISO | GPIO 19 |
| RC522 RST | GPIO 22 |
| RC522 3.3V | 3V3 |
| RC522 GND | GND |
| Relay VCC | VIN/5V only if module supports 5 V |
| Relay GND | GND |
| Relay IN1 | GPIO 26 |
| Relay IN2 | Unused |

RC522 uses 3.3 V. The sketch assumes the relay input is active-LOW. Verify the actual module and any JD-VCC jumper against its datasheet.


