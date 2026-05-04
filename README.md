CrowPanel-GSR-ADS1115-ESP32S3

# CrowPanel GSR Monitor — ESP32-S3 + ADS1115 + LovyanGFX

A real-time Galvanic Skin Response (GSR) sensor monitor running on the
Elecrow CrowPanel 7.0" HMI ESP32-S3 display (800×480 RGB). Reads
analog GSR values via an ADS1115 16-bit I2C ADC and visualises them
live on the display with a stress-level indicator.

 Hardware
- Elecrow CrowPanel 7.0" HMI (ESP32-S3, 800×480 RGB TFT)
- ADS1115 16-bit ADC module (I2C address 0x48)
- Grove GSR sensor (connected to ADS1115 A0)
- 2× 4.7kΩ pull-up resistors on SDA/SCL

Wiring
| ADS1115 | CrowPanel |

SDA     - IO19      
SCL     - IO20      
VCC     - 3.3V      
GND     - GND       
ADDR    - GND       

>  External 4.7kΩ pull-up resistors required on SDA and SCL
> (3.3V → SDA, 3.3V → SCL). Without them the I2C bus will not work.

Key Technical Fixes (Documented)
1. Touch controller disabled** in `gfx_conf.h` — LovyanGFX's GT911
   touch driver was permanently claiming IO19/IO20 via I2C_NUM_1,
   preventing Arduino Wire from accessing those pins.
2. ADS1115 initialised before `tft.begin()`** — after `tft.begin()`
   the GPIO matrix is reconfigured and Wire can no longer reach IO19/IO20.
3. `Wire.begin(19,20)` called only once** — calling it a second time
   after `tft.begin()` breaks the display.
4. **4.7kΩ pull-up resistors** — I2C is open-drain; without pull-ups
   SDA floated at 1.3V and no device was detectable.

## Dependencies
- [LovyanGFX 1.1.8](https://github.com/lovyan03/LovyanGFX)
- [Adafruit ADS1X15](https://github.com/adafruit/Adafruit_ADS1X15)
- [PCA9557 Library](https://github.com/nickcoutsos/pca9557)
- esp32 board package **2.0.14 or 2.0.15** (newer versions break display)

## Arduino IDE Board Settings
| Setting | Value |
|---------|-------|
| Board | ESP32S3 Dev Module |
| USB CDC On Boot | Enabled |
| PSRAM | OPI PSRAM |
| Flash Size | 4MB |
