# Testing and troubleshooting

## Expected behavior on low-voltage test

1. Keep AC mains disconnected.
2. Power ESP32 from USB.
3. Confirm the RC522 version is plausible (previously `0xB2`) and the card UID is read.
4. Register each teacher card UID in `AUTHORIZED_UIDS`, then upload again.
5. Scan any registered card: relay toggles ON or OFF. Any registered teacher can toggle the shared bulb state.
6. Scan an unregistered card: relay state must not change.

## Known failure

When AC is connected to the bulb circuit, the ESP32/RFID stops responding even if the relay is OFF. After the first authorized scan switches the relay ON, the bulb and relay remain ON and card scanning stops. Pressing EN/RESET did not recover it; disconnecting and reconnecting USB did. This is unresolved and does not prove a software bug.

## Safety and next steps

Keep AC disconnected. A qualified electrician should inspect the actual relay contacts, load rating, mains wiring, isolation, enclosure, strain relief, and separation from low-voltage wiring. After inspection, qualified troubleshooting can check for supply disturbances and electromagnetic interference with suitable equipment. Do not repeat mains tests or experimentally modify the AC circuit.

## If RFID fails even on USB

Check RC522 3.3 V power and the SPI connections: SS=5, SCK=18, MOSI=23, MISO=19, RST=22. Replace loose jumper wires.
