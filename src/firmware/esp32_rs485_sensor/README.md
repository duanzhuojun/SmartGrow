# ESP32 RS485 Sensor Reader

Reads a Modbus RTU sensor (soil moisture, temperature, EC, pH, NPK, etc.) over RS485 using an ESP32 and a TTL↔RS485 converter. No extra libraries needed.

## Wiring

| ESP32 | Converter (MAX485 type) | Converter (auto-direction type) |
|-------|-------------------------|---------------------------------|
| GPIO16 (RX) | RO | RXD |
| GPIO17 (TX) | DI | TXD |
| GPIO4 | DE **and** RE (tied together) | — (set `RS485_DE_RE_PIN` to `-1`) |
| 3V3 | VCC | VCC |
| GND | GND | GND |

| Converter | Sensor |
|-----------|--------|
| A | A / 485+ (often yellow) |
| B | B / 485− (often blue) |

The sensor's power wires (often brown = V+, black = GND) usually need **5–24 V DC** from a separate supply; check the label. **The supply's GND must be connected to the ESP32 GND.**

> Power the converter from **3.3 V** if it supports it. A 5 V MAX485 board outputs 5 V on RO, which can damage the ESP32's RX pin; add a voltage divider (e.g. 1 kΩ / 2 kΩ) if you must run it at 5 V.

## Use it

1. Open `esp32_rs485_sensor.ino` in the Arduino IDE (board: your ESP32 model).
2. Edit the **CONFIGURATION** block at the top:
   - `RS485_BAUD`, `SENSOR_ADDRESS`: from the sensor datasheet (usually 4800 or 9600, address 1).
   - `CHANNELS[]`: the register map from the datasheet. **These differ between vendors.**
3. Upload and open the Serial Monitor at **115200** baud.

Not sure about the baud rate or address? Set `RUN_SCAN_ON_BOOT` to `true`. The sketch will try common settings and report which one the sensor answers on.

## Troubleshooting

| Symptom | Try |
|---------|-----|
| `no response` | Swap A and B. Check sensor power and the shared GND. Check baud and address. Run the scan. |
| `CRC error` | Wrong baud rate, or noise. Use twisted-pair wire for long cable runs. |
| `Modbus exception` | Wiring is fine. That register isn't supported, or try `MODBUS_FUNCTION 0x04`. |
| Values look wrong (e.g. 10× too big) | Fix `scale` for that channel in `CHANNELS[]`. |
