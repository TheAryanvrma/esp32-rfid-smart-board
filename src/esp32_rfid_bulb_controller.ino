/*
 * ESP32 RFID Bulb Controller
 *
 * Function:
 * - Any UID in AUTHORIZED_UIDS toggles Relay 1.
 * - Add one entry per teacher card after reading its UID in Serial Monitor.
 * - Relay ON: bulb ON (when connected through a verified COM/NO contact path).
 * - Relay OFF: bulb OFF.
 *
 * Hardware:
 * - ESP32 Dev Module
 * - MFRC522 RFID reader (SPI)
 * - 5 V active-LOW relay module, IN1 on GPIO 26
 *


#include <SPI.h>
#include <MFRC522.h>

constexpr uint8_t RFID_SS_PIN = 5;
constexpr uint8_t RFID_RST_PIN = 22;
constexpr uint8_t RELAY_PIN = 26;

// This relay module was tested as active-LOW:
constexpr uint8_t RELAY_ON = LOW;
constexpr uint8_t RELAY_OFF = HIGH;

constexpr unsigned long SCAN_COOLDOWN_MS = 1200;

MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);
// Add every teacher's card UID here. Each UID can have a different length.
// The first entry is the currently known authorized card: 72 2B CE 5C.
// Example second card: {4, {0x11, 0x22, 0x33, 0x44}},
// Replace that example with the real UID printed by Serial Monitor.
struct AuthorizedCard {
  byte length;
  byte uid[10];
};

const AuthorizedCard AUTHORIZED_UIDS[] = {
  {4, {0x72, 0x2B, 0xCE, 0x5C}},
  // {4, {0x11, 0x22, 0x33, 0x44}}, // Example only — replace with a real teacher card UID
};
constexpr size_t AUTHORIZED_CARD_COUNT =
    sizeof(AUTHORIZED_UIDS) / sizeof(AUTHORIZED_UIDS[0]);

bool bulbCommandOn = false;
unsigned long lastAcceptedScanMs = 0;

bool isAuthorizedCard() {
  for (size_t card = 0; card < AUTHORIZED_CARD_COUNT; ++card) {
    if (rfid.uid.size != AUTHORIZED_UIDS[card].length) continue;

    bool matches = true;
    for (size_t i = 0; i < AUTHORIZED_UIDS[card].length; ++i) {
      if (rfid.uid.uidByte[i] != AUTHORIZED_UIDS[card].uid[i]) {
        matches = false;
        break;
      }
    }
    if (matches) return true;
  }
  return false;
}

void applyBulbCommand(bool turnOn) {
  bulbCommandOn = turnOn;
  digitalWrite(RELAY_PIN, bulbCommandOn ? RELAY_ON : RELAY_OFF);

  if (bulbCommandOn) {
    Serial.println(F("ACCESS GRANTED - RELAY ON - BULB COMMAND ON"));
  } else {
    Serial.println(F("ACCESS GRANTED - RELAY OFF - BULB COMMAND OFF"));
  }
}

void printCardUID() {
  Serial.print(F("Card UID: "));
  for (byte i = 0; i < rfid.uid.size; ++i) {
    if (rfid.uid.uidByte[i] < 0x10) Serial.print('0');
    Serial.print(rfid.uid.uidByte[i], HEX);
    if (i + 1 < rfid.uid.size) Serial.print(' ');
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // Set relay output OFF at startup (physical behavior during boot depends on hardware).
  digitalWrite(RELAY_PIN, RELAY_OFF);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_OFF);
  bulbCommandOn = false;

  SPI.begin(18, 19, 23, RFID_SS_PIN); // SCK, MISO, MOSI, SS
  rfid.PCD_Init();
  delay(100);

  byte version = rfid.PCD_ReadRegister(MFRC522::VersionReg);

  Serial.println();
  Serial.println(F("ESP32 RFID Bulb Controller"));
  Serial.print(F("RC522 version: 0x"));
  Serial.println(version, HEX);
  Serial.println(F("Startup command: relay OFF / bulb OFF"));
  Serial.print(F("Authorized teacher cards configured: "));
  Serial.println(AUTHORIZED_CARD_COUNT);
  Serial.println(F("Present an authorized teacher card to toggle bulb command."));

  if (version == 0x00 || version == 0xFF) {
    Serial.println(F("WARNING: RC522 response looks invalid; check wiring and 3.3 V supply."));
  }
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent()) {
    delay(10);
    return;
  }
  if (!rfid.PICC_ReadCardSerial()) {
    delay(10);
    return;
  }

  printCardUID();

  const unsigned long now = millis();
  const bool cooldownPassed =
      (lastAcceptedScanMs == 0) || (now - lastAcceptedScanMs >= SCAN_COOLDOWN_MS);

  if (!cooldownPassed) {
    Serial.println(F("Scan ignored during cooldown."));
  } else if (isAuthorizedCard()) {
    lastAcceptedScanMs = now;
    applyBulbCommand(!bulbCommandOn);
  } else {
    Serial.println(F("ACCESS DENIED - bulb command unchanged"));
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(50);
}
