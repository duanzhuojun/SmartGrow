#define RS485_RX 17
#define RS485_TX 16
#define BAUD     9600 // probe factory default per manual
#define ADDR     0x01

uint16_t crc16(const uint8_t *buf, uint8_t len) {
  uint16_t crc = 0xFFFF;
  for (uint8_t i = 0; i < len; i++) {
    crc ^= buf[i];
    for (uint8_t b = 0; b < 8; b++)
      crc = (crc & 1) ? (crc >> 1) ^ 0xA001 : crc >> 1;
  }
  return crc;
}

// Read 'qty' holding registers starting at 'reg' into out[]. Returns true on success.
bool readRegs(uint16_t reg, uint8_t qty, uint16_t *out) {
  uint8_t f[8] = {ADDR, 0x03, (uint8_t)(reg >> 8), (uint8_t)reg, 0x00, qty};
  uint16_t c = crc16(f, 6);
  f[6] = c & 0xFF; f[7] = c >> 8;

  while (Serial1.available()) Serial1.read();
  Serial1.write(f, 8);
  Serial1.flush();

  uint8_t r[32]; int n = 0;
  int expected = 5 + 2 * qty;
  uint32_t t0 = millis();
  while (millis() - t0 < 500 && n < expected) {
    if (Serial1.available()) r[n++] = Serial1.read();
  }
  delay(30);  // inter-frame gap

  bool ok = n == expected && r[0] == ADDR && r[1] == 0x03 && r[2] == 2 * qty &&
            crc16(r, n - 2) == (r[n - 2] | (r[n - 1] << 8));
  if (!ok) {
    Serial.printf("  [reg 0x%04X x%u failed, got %d bytes: ", reg, qty, n);
    for (int i = 0; i < n; i++) Serial.printf("%02X ", r[i]);
    if (n >= 3 && r[1] == 0x83) Serial.printf("<- exception %02X", r[2]);
    Serial.println("]");
    return false;
  }
  for (uint8_t i = 0; i < qty; i++) out[i] = (r[3 + 2 * i] << 8) | r[4 + 2 * i];
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial1.begin(BAUD, SERIAL_8N1, RS485_RX, RS485_TX);
  Serial.printf("Soil NPK reader - addr %d @ %d baud\n", ADDR, BAUD);
}

void loop() {
  uint16_t ph, mt[2], ec, npk[3];
  Serial.println("---------------------------------");

  if (readRegs(0x0012, 2, mt)) {
    Serial.printf("Moisture    : %.1f %%\n", mt[0] / 10.0);
    Serial.printf("Temperature : %.1f C\n", (int16_t)mt[1] / 10.0);
  }
  if (readRegs(0x0015, 1, &ec))
    Serial.printf("EC          : %u uS/cm\n", ec);
  if (readRegs(0x0006, 1, &ph))
    Serial.printf("pH          : %.2f\n", ph / 100.0);
  if (readRegs(0x001E, 3, npk)) {
    Serial.printf("Nitrogen    : %u mg/kg\n", npk[0]);
    Serial.printf("Phosphorus  : %u mg/kg\n", npk[1]);
    Serial.printf("Potassium   : %u mg/kg\n", npk[2]);
  }

  delay(2000);
}