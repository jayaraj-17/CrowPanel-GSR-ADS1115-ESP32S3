/*******************************************************************************
  CrowPanel 7.0" + ADS1115 + GSR Sensor
  KEY ORDER:
    1. Wire.begin(19,20)   — ONCE, never again
    2. PCA9557 init        — display reset circuit
    3. ads.begin(0x48)     — ADS1115 BEFORE tft.begin()
    4. tft.begin()         — display last
*******************************************************************************/
#include <Wire.h>
#include <SPI.h>
#include <PCA9557.h>
#include <Adafruit_ADS1X15.h>
#include "gfx_conf.h"

PCA9557          Out;
Adafruit_ADS1115 ads;
long             threshold = 0;

/* ══════════════════════════════  SETUP  ════════════════════════════════════ */
void setup()
{
    Serial.begin(115200);
    delay(2000);
    Serial.println("=== BOOT START ===");

    /* ── STEP 1: Wire ONCE — never called again ─────────────────────────── */
    Wire.begin(19, 20);
    delay(100);
    Serial.println("Step 1: Wire.begin done");

    /* ── STEP 2: PCA9557 display reset ──────────────────────────────────── */
    Out.reset();
    Out.setMode(IO_OUTPUT);
    Out.setState(IO0, IO_LOW);
    Out.setState(IO1, IO_LOW);
    delay(20);
    Out.setState(IO0, IO_HIGH);
    delay(100);
    Out.setMode(IO1, IO_INPUT);
    Wire.endTransmission();   // ← key fix from Elecrow forum
delay(100);
Wire.begin(19, 20);       // ← re-init after PCA9557
delay(100);

ads.begin(0x48); 
    Serial.println("Step 2: PCA9557 done");

    /* ── STEP 3: ADS1115 BEFORE tft.begin() ─────────────────────────────── */
    Serial.println("Step 3: ads.begin...");
    if (!ads.begin(0x48)) {
        Serial.println("ERROR: ADS1115 not found!");
        // tft not ready yet so flash LED pattern as error signal
        // Just loop — fix wiring then retry
        while (1) { delay(1000); Serial.println("ADS1115 missing - check wiring"); }
    }
    ads.setGain(GAIN_ONE);
    Serial.println("Step 3: ADS1115 OK!");

    /* ── STEP 4: Display init ────────────────────────────────────────────── */
    Serial.println("Step 4: tft.begin...");
    tft.begin();
    Serial.println("Step 4: Display OK!");

    /* ── STEP 5: Calibration UI ──────────────────────────────────────────── */
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(3);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setCursor(80, 160);
    tft.print("Calibrating GSR...");
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(80, 220);
    tft.print("Do NOT touch the electrodes!");

    // Progress bar
    tft.drawRoundRect(20, 280, 760, 30, 6, TFT_WHITE);
    long sum = 0;
    for (int i = 0; i < 300; i++) {
        sum += ads.readADC_SingleEnded(0);
        tft.fillRoundRect(22, 282, map(i, 0, 299, 0, 756), 26, 4, TFT_GREEN);
        delay(5);
    }
    threshold = sum / 300;
    Serial.print("Baseline: "); Serial.println(threshold);

    // Ready screen
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(3);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setCursor(260, 200);
    tft.print("Ready!");
    tft.setTextSize(2);
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.setCursor(200, 260);
    tft.print("Wear electrodes now.");
    delay(1500);

    /* ── STEP 6: Static UI layout ────────────────────────────────────────── */
    tft.fillScreen(TFT_BLACK);
    // Title bar
    tft.fillRect(0, 0, 800, 48, 0x1082);
    tft.setTextColor(TFT_CYAN, 0x1082);
    tft.setTextSize(2);
    tft.setCursor(20, 14);
    tft.print("GSR Monitor  |  ADS1115  |  CrowPanel 7.0");
    tft.drawFastHLine(0, 48, 800, TFT_DARKGREY);

    // Bar label
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(20, 230);
    tft.print("Skin Conductance Level:");
    tft.fillRoundRect(20, 258, 760, 34, 8, 0x2104);

    // Baseline info
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(20, 420);
    char buf[60];
    snprintf(buf, sizeof(buf), "Baseline: %ld   |   Gain: GAIN_ONE (+/-4.096V)", threshold);
    tft.print(buf);

    Serial.println("=== SETUP DONE ===");
}

/* ══════════════════════════════  LOOP  ═════════════════════════════════════ */
void loop()
{
    int16_t raw     = ads.readADC_SingleEnded(0);
    float   voltage = ads.computeVolts(raw);
    long    change  = raw - threshold;

    // Serial log
    Serial.print("Raw: ");      Serial.print(raw);
    Serial.print(" | V: ");     Serial.print(voltage, 4);
    Serial.print(" | Change: "); Serial.println(change);

    // Raw value
    tft.setTextSize(3);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setCursor(20, 75);
    tft.printf("Raw:    %-8d", raw);

    // Voltage
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(20, 120);
    tft.printf("Volt:   %.4fV  ", voltage);

    // Change
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setCursor(20, 165);
    tft.printf("Change: %-8ld", change);

    // Live bar
    long barPx = constrain(map(change, 0, 5000, 0, 756), 0, 756);
    tft.fillRoundRect(20, 258, 760, 34, 8, 0x2104);
    uint16_t barCol = (change < 300) ? TFT_GREEN :
                      (change < 1500) ? TFT_YELLOW : TFT_RED;
    if (barPx > 0) tft.fillRoundRect(20, 258, barPx, 34, 8, barCol);

    // Status
    tft.fillRect(20, 310, 760, 44, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(20, 320);
    if (change < 300) {
        tft.setTextColor(TFT_GREEN,  TFT_BLACK);
        tft.print("Status: CALM / RESTING             ");
    } else if (change < 1500) {
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.print("Status: MILD RESPONSE              ");
    } else {
        tft.setTextColor(TFT_RED,    TFT_BLACK);
        tft.print("Status: HIGH STRESS DETECTED!      ");
    }

    delay(100);
}
