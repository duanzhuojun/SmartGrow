#include <Wire.h>
#include <BH1750.h>

// ---- Soil probe (RS485 via T/R converter) ----
#define RS485_RX 17   // final working pins
#define RS485_TX 16
#define BAUD     9600 // set to the baud that worked for you (4800 or 9600)
#define ADDR     0x01

// ---- Light sensor (BH1750, I2C) ----
#define I2C_SDA  19   // BH1750 DAT
#define I2C_SCL  20   // BH1750 SCL

BH1750 lightMeter(0x23);   // ADDR pin to GND or floating = 0x23; to 3.3V = 0x5C
bool lightOk = false;

uint16_t crc16(const uint8_t *buf, uint8_t len) {
  uint16_t crc = 0xFFFF;
  for (uint8_t i = 0; i < len; i++) {
    crc ^= buf[i];
    for (uint8_t b = 0; b < 8; b++)
      crc = (crc & 1) ? (crc >> 1) ^ 0xA001 : crc >> 1;
  }
  return crc;
}

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
  delay(30);

  bool ok = n == expected && r[0] == ADDR && r[1] == 0x03 && r[2] == 2 * qty &&
            crc16(r, n - 2) == (r[n - 2] | (r[n - 1] << 8));
  if (!ok) {
    Serial.printf("  [reg 0x%04X x%u failed, got %d bytes]\n", reg, qty, n);
    return false;
  }
  for (uint8_t i = 0; i < qty; i++) out[i] = (r[3 + 2 * i] << 8) | r[4 + 2 * i];
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(3000);

  Serial1.begin(BAUD, SERIAL_8N1, RS485_RX, RS485_TX);

  Wire.begin(I2C_SDA, I2C_SCL);
  lightOk = lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
  Serial.println(lightOk ? "BH1750 found" : "BH1750 NOT found - check wiring/address");

  Serial.printf("Soil + light reader - soil addr %d @ %d baud\n", ADDR, BAUD);
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

  if (lightOk) {
    float lux = lightMeter.readLightLevel();
    if (lux >= 0) Serial.printf("Light       : %.1f lux\n", lux);
    else          Serial.println("Light       : read error");
  }

  delay(2000);
}