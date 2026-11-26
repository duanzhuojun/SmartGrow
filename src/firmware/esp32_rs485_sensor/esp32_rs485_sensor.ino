/*
 * SmartGrow - ESP32 RS485 (Modbus RTU) sensor reader
 *
 * Reads a Modbus RTU sensor (e.g. a soil moisture / temperature / EC / pH / NPK
 * probe) through a TTL <-> RS485 converter and prints the values to the
 * Serial Monitor (115200 baud).
 *
 * No extra libraries needed: Modbus frames and CRC are built by hand so the
 * whole protocol is visible for teaching.
 *
 * Wiring (see README.md in this folder for details):
 *   ESP32 GPIO16 (RX)  <- converter RO  (or "RXD"/"R")
 *   ESP32 GPIO17 (TX)  -> converter DI  (or "TXD"/"D")
 *   ESP32 GPIO4        -> converter DE + RE (tied together)  [MAX485-type only]
 *   ESP32 3V3 / GND    -> converter VCC / GND
 *   Converter A / B    -> sensor A (often yellow) / B (often blue)
 *   Sensor power       -> its own supply (usually 5-24 V DC), GND shared with ESP32
 */

#include <Arduino.h>

// ======================= CONFIGURATION — edit these ========================

#define RS485_RX_PIN      16    // ESP32 pin receiving from converter RO/RXD
#define RS485_TX_PIN      17    // ESP32 pin sending to converter DI/TXD
#define RS485_DE_RE_PIN   4     // DE+RE pin for MAX485 boards; set to -1 for
                                // "auto direction" boards that only have TXD/RXD

#define RS485_BAUD        4800  // Most soil sensors ship at 4800; others use 9600
#define SENSOR_ADDRESS    0x01  // Modbus slave address (factory default is usually 1)
#define MODBUS_FUNCTION   0x03  // 0x03 = Read Holding Registers (most common),
                                // 0x04 = Read Input Registers (some sensors)

#define READ_INTERVAL_MS     2000
#define RESPONSE_TIMEOUT_MS  500

// Set to true if you don't know the sensor's baud rate / address.
// The ESP32 will try common baud rates and addresses and report what answers.
#define RUN_SCAN_ON_BOOT  false
#define SCAN_MAX_ADDRESS  32

// Register map: CHECK YOUR SENSOR'S DATASHEET — these vary between vendors.
// The map below matches the common "7-in-1 soil sensor" (registers 0x0000-0x0006).
// Remove lines for values your sensor doesn't have.
struct Channel {
  const char* name;
  uint16_t    reg;       // register address
  float       scale;     // raw value * scale = real value
  bool        isSigned;  // true for values that can go negative (temperature)
  const char* unit;
};

const Channel CHANNELS[] = {
  //  name           reg     scale  signed  unit
  { "Moisture",    0x0000,  0.1f,  false, "%"     },
  { "Temperature", 0x0001,  0.1f,  true,  "C"     },
  { "EC",          0x0002,  1.0f,  false, "uS/cm" },
  { "pH",          0x0003,  0.1f,  false, ""      },
  { "Nitrogen",    0x0004,  1.0f,  false, "mg/kg" },
  { "Phosphorus",  0x0005,  1.0f,  false, "mg/kg" },
  { "Potassium",   0x0006,  1.0f,  false, "mg/kg" },
};
const size_t NUM_CHANNELS = sizeof(CHANNELS) / sizeof(CHANNELS[0]);

// ===========================================================================

HardwareSerial RS485(1);  // UART1, remapped to the pins above (works on ESP32, S3, C3)

enum ModbusResult {
  MB_OK = 0,
  MB_TIMEOUT,
  MB_BAD_CRC,
  MB_WRONG_ADDRESS,
  MB_EXCEPTION,
  MB_BAD_LENGTH,
};

const char* resultToString(ModbusResult r) {
  switch (r) {
    case MB_OK:            return "OK";
    case MB_TIMEOUT:       return "no response (check wiring, A/B swap, power, baud, address)";
    case MB_BAD_CRC:       return "CRC error (noise or wrong baud rate)";
    case MB_WRONG_ADDRESS: return "reply from unexpected address";
    case MB_EXCEPTION:     return "sensor returned Modbus exception (register not supported?)";
    case MB_BAD_LENGTH:    return "unexpected reply length";
  }
  return "unknown";
}

// Standard Modbus CRC-16 (polynomial 0xA001, init 0xFFFF)
uint16_t modbusCRC(const uint8_t* data, size_t len) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (int b = 0; b < 8; b++) {
      if (crc & 0x0001) crc = (crc >> 1) ^ 0xA001;
      else              crc >>= 1;
    }
  }
  return crc;
}

void printHex(const char* label, const uint8_t* data, size_t len) {
  Serial.print(label);
  for (size_t i = 0; i < len; i++) {
    if (data[i] < 0x10) Serial.print('0');
    Serial.print(data[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
}

void setTransmit(bool transmit) {
  if (RS485_DE_RE_PIN >= 0) digitalWrite(RS485_DE_RE_PIN, transmit ? HIGH : LOW);
}

// Read `count` registers starting at `startReg` from slave `addr`.
// Values are written to `out` (must hold `count` entries).
ModbusResult readRegisters(uint8_t addr, uint8_t func, uint16_t startReg,
                           uint16_t count, uint16_t* out,
                           uint32_t timeoutMs = RESPONSE_TIMEOUT_MS,
                           bool debug = false) {
  // ---- Build request: [addr][func][regHi][regLo][cntHi][cntLo][crcLo][crcHi]
  uint8_t req[8];
  req[0] = addr;
  req[1] = func;
  req[2] = startReg >> 8;
  req[3] = startReg & 0xFF;
  req[4] = count >> 8;
  req[5] = count & 0xFF;
  uint16_t crc = modbusCRC(req, 6);
  req[6] = crc & 0xFF;   // CRC is sent low byte first
  req[7] = crc >> 8;

  // ---- Send
  while (RS485.available()) RS485.read();   // discard stale bytes
  setTransmit(true);
  RS485.write(req, sizeof(req));
  RS485.flush();                             // wait until last bit is on the wire
  setTransmit(false);
  if (debug) printHex("  TX: ", req, sizeof(req));

  // ---- Receive: [addr][func][byteCount][data...][crcLo][crcHi]
  const size_t expected = 5 + 2 * count;
  uint8_t buf[5 + 2 * 125];
  if (expected > sizeof(buf)) return MB_BAD_LENGTH;

  size_t len = 0;
  uint32_t start = millis();
  while (len < expected && millis() - start < timeoutMs) {
    if (RS485.available()) {
      buf[len++] = RS485.read();
      // An exception reply is only 5 bytes long
      if (len == 5 && (buf[1] & 0x80)) break;
    }
  }
  if (debug && len) printHex("  RX: ", buf, len);

  if (len == 0) return MB_TIMEOUT;
  if (len < 5)  return MB_BAD_LENGTH;

  uint16_t rxCrc = buf[len - 2] | (buf[len - 1] << 8);
  if (modbusCRC(buf, len - 2) != rxCrc) return MB_BAD_CRC;
  if (buf[0] != addr)                   return MB_WRONG_ADDRESS;
  if (buf[1] & 0x80)                    return MB_EXCEPTION;
  if (len != expected || buf[2] != 2 * count) return MB_BAD_LENGTH;

  for (uint16_t i = 0; i < count; i++) {
    out[i] = (buf[3 + 2 * i] << 8) | buf[4 + 2 * i];
  }
  return MB_OK;
}

void beginRS485(uint32_t baud) {
  RS485.end();
  RS485.begin(baud, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
  delay(50);
}

// Try common baud rates and addresses to find a sensor
void scanBus() {
  const uint32_t bauds[] = { 4800, 9600, 2400, 19200, 38400, 115200 };
  Serial.println("\n=== Scanning RS485 bus ===");
  bool found = false;
  for (uint32_t baud : bauds) {
    Serial.printf("Baud %lu: ", (unsigned long)baud);
    beginRS485(baud);
    for (uint8_t addr = 1; addr <= SCAN_MAX_ADDRESS; addr++) {
      uint16_t val;
      ModbusResult r = readRegisters(addr, MODBUS_FUNCTION, CHANNELS[0].reg, 1, &val, 100);
      // An exception still proves a device is there at this baud/address
      if (r == MB_OK || r == MB_EXCEPTION) {
        Serial.printf("\n  >>> Device found: address %u at %lu baud (%s)\n",
                      addr, (unsigned long)baud, resultToString(r));
        found = true;
      }
      delay(10);
    }
    Serial.println();
  }
  if (!found) Serial.println("No devices found. Check power, GND, A/B wiring (try swapping A and B).");
  Serial.println("=== Scan done. Set RS485_BAUD / SENSOR_ADDRESS accordingly. ===\n");
  beginRS485(RS485_BAUD);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\nSmartGrow ESP32 RS485 sensor reader");

  if (RS485_DE_RE_PIN >= 0) {
    pinMode(RS485_DE_RE_PIN, OUTPUT);
    setTransmit(false);  // default to receive mode
  }
  beginRS485(RS485_BAUD);

  if (RUN_SCAN_ON_BOOT) scanBus();

  Serial.printf("Reading address 0x%02X at %d baud, function 0x%02X\n",
                SENSOR_ADDRESS, RS485_BAUD, MODBUS_FUNCTION);
}

void loop() {
  Serial.println("---------------------------------");
  bool anyOk = false;

  for (size_t i = 0; i < NUM_CHANNELS; i++) {
    const Channel& ch = CHANNELS[i];
    uint16_t raw;
    ModbusResult r = readRegisters(SENSOR_ADDRESS, MODBUS_FUNCTION, ch.reg, 1, &raw);

    if (r == MB_OK) {
      anyOk = true;
      float value = ch.isSigned ? (int16_t)raw * ch.scale : raw * ch.scale;
      Serial.printf("%-12s: %8.1f %s   (raw %u)\n", ch.name, value, ch.unit, raw);
    } else {
      Serial.printf("%-12s: ERROR - %s\n", ch.name, resultToString(r));
    }
    delay(50);  // short gap between requests; some sensors need it
  }

  // If nothing answered, show the raw bytes once to help debugging
  if (!anyOk) {
    uint16_t dummy;
    Serial.println("Debug frame:");
    readRegisters(SENSOR_ADDRESS, MODBUS_FUNCTION, CHANNELS[0].reg, 1, &dummy,
                  RESPONSE_TIMEOUT_MS, true);
  }

  delay(READ_INTERVAL_MS);
}
