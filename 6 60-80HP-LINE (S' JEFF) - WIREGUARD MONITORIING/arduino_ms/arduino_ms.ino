/*
  Material Setter Sensor & Interlock Controller - Arduino Mega
  =============================================================
  Reads 7 IR sensors, controls 7 relay outputs, and interfaces with
  three R35D-B QR scanners via Serial1 (MS ID), Serial2 (MS Kitting),
  and Serial3 (P1 ID). P1 Kitting scanner relay/sensor on this board,
  but scanner serial data read by Cycle Time Arduino via its Serial1.
  
  Pin Mapping (Arduino Mega):
  -----------------------------------------------
  Pin 2       - INPUT  : IR Sensor 1 (MS ID detection)
  Pin 3       - OUTPUT : Relay 2 (MS solenoid interlock)
  Pin 4       - OUTPUT : Relay 4 (R35D-B ID scanner power relay)
  Pin 5       - INPUT  : IR Sensor 5 (Kitting QR code detection at MS)
  Pin 6       - OUTPUT : Relay 5 (Kitting QR scanner power relay)
  Pin 7       - INPUT  : IR Sensor 2 (dunpla at Standby)
  Pin 8       - OUTPUT : Standby Relay (Standby interlock)
  Pin 9       - INPUT  : IR Sensor 3 (P1 ID detection)
  Pin 10      - OUTPUT : Relay 7 (P1 Scanner 3 power relay)
  Pin 11      - INPUT  : IR Sensor 5 (P1 kitting detection)
  Pin 12      - OUTPUT : Relay 8 (P1 Scanner 4 power relay)
  Pin 13      - OUTPUT : Relay 9 (P1 solenoid interlock)
  Pin 14 (TX3)- OUTPUT : P1 R35D-B Scanner TTL-RX (hardware Serial3 TX)
  Pin 15 (RX3)- INPUT  : P1 R35D-B Scanner TTL-TX (hardware Serial3 RX)
  Pin 16 (TX2)- OUTPUT : Kitting R35D-B Scanner TTL-RX (hardware Serial2 TX)
  Pin 17 (RX2)- INPUT  : Kitting R35D-B Scanner TTL-TX (hardware Serial2 RX)
  Pin 18 (TX1)- OUTPUT : ID R35D-B Scanner TTL-RX (hardware Serial1 TX)
  Pin 19 (RX1)- INPUT  : ID R35D-B Scanner TTL-TX (hardware Serial1 RX)
  Pin 22      - INPUT  : IR Sensor 6 (dunpla at Process 1)
  Pin 23      - INPUT  : IR Sensor 7 (P2 ID detection)
  Pin 27      - OUTPUT : Relay 4 (P2 solenoid interlock)
  Pin 28      - INPUT  : IR Sensor 8 (dunpla at Process 2)
  -----------------------------------------------
  
  Wiring:
    Pin 2  : IR sensor OUT → Pin 2. Sensor LOW = object detected.
    Pin 3  : Pin 3 → Relay IN. HIGH = relay energized (MS interlock open).
    Pin 4  : Pin 4 → Relay 4 IN. LOW = relay energized (ID scanner power ON).
             R35D-B GND → Relay 4 COM. Relay 4 NO → Arduino GND.
    Pin 5  : IR sensor OUT → Pin 5. Sensor LOW = kitting dunpla detected.
    Pin 6  : Pin 6 → Relay 5 IN. LOW = relay energized (Kitting scanner power ON).
             R35D-B GND → Relay 5 COM. Relay 5 NO → Arduino GND.
    Pin 7  : IR sensor OUT → Pin 7. Sensor LOW = object detected.
    Pin 8  : Pin 8 → Relay IN. HIGH = relay energized (Standby interlock open).
    Pin 9  : IR sensor OUT → Pin 9. Sensor LOW = object detected.
    Pin 10 : Pin 10 → Relay 7 IN. LOW = relay energized (P1 Scanner 3 power ON).
    Pin 11 : IR sensor OUT → Pin 11. Sensor LOW = kitting dunpla detected.
    Pin 12 : Pin 12 → Relay 8 IN. LOW = relay energized (P1 Scanner 4 power ON).
    Pin 13 : Pin 13 → Relay 9 IN. HIGH = relay energized (P1 interlock open).
    Pin 14 : Pin 14 (TX3) → P1 R35D-B TTL-RX. Arduino sends trigger commands.
    Pin 15 : P1 R35D-B TTL-TX → Pin 15 (RX3). Scanner sends scanned data.
    Pin 16 : Pin 16 (TX2) → Kitting R35D-B TTL-RX. Arduino sends trigger commands.
    Pin 17 : Kitting R35D-B TTL-TX → Pin 17 (RX2). Scanner sends scanned data.
    Pin 18 : Pin 18 (TX1) → ID R35D-B TTL-RX. Arduino sends trigger commands.
    Pin 19 : ID R35D-B TTL-TX → Pin 19 (RX1). Scanner sends scanned data.
    Pin 22 : IR sensor OUT → Pin 22. Sensor LOW = dunpla detected at Process 1.
    Pin 23 : IR sensor OUT → Pin 23. Sensor LOW = P2 ID detected.
    Pin 27 : Pin 27 → Relay IN. HIGH = relay energized (P2 interlock open).
    Pin 28 : IR sensor OUT → Pin 28. Sensor LOW = dunpla detected at Process 2.
  
  Serial Output (Arduino → PC):
    MS_LOAD_ID               - ID detected by Sensor 1 (Pin 2 LOW)
    MS_UNLOAD_ID             - ID removed from Sensor 1 (Pin 2 HIGH)
    P1_ID_LOAD               - ID detected by Sensor 3 (Pin 9 LOW)
    P1_ID_UNLOAD             - ID removed from Sensor 3 (Pin 9 HIGH)
    STANDBY_MS               - Dunpla detected by Sensor 2 (Pin 7 LOW)
    MS_SCAN:<data>           - Scanned QR data from MS ID R35D-B scanner (Serial1)
    MS_KITTING_LOAD          - Kitting dunpla detected (Pin 5 LOW)
    MS_KITTING_UNLOAD        - Kitting dunpla removed (Pin 5 HIGH)
    MS_KITSCAN:<data>        - Scanned QR data from MS Kitting R35D-B scanner (Serial2)
    MS_KIT_SCANNER_READY     - MS Kitting scanner booted and ready
    P1_SCANNER_READY         - P1 ID scanner (Scanner 3) booted and ready
    P1_SCANNER_RELAY_ON_CONFIRM  - Scanner 3 relay ON (Pin 10 LOW)
    P1_SCANNER_RELAY_OFF_CONFIRM - Scanner 3 relay OFF (Pin 10 HIGH)
    P1_SCAN:<data>           - Scanned QR data from P1 ID R35D-B scanner (Serial3)
    P1_KITTING_LOAD          - P1 kitting dunpla detected (Pin 11 LOW)
    P1_KITTING_UNLOAD        - P1 kitting dunpla removed (Pin 11 HIGH)
    P1_KIT_SCANNER_READY     - P1 Kitting scanner (Scanner 4) booted and ready
    P1_KIT_RELAY_ON_CONFIRM  - Scanner 4 relay ON (Pin 12 LOW)
    P1_KIT_RELAY_OFF_CONFIRM - Scanner 4 relay OFF (Pin 12 HIGH)
    P1_KITSCAN:<data>        - Scanned QR data from P1 Kitting R35D-B scanner (Serial3)
    P1_DONE                  - Dunpla detected at Process 1 (Pin 22 LOW sustained)
    MS_OPEN_INTERLOCK        - Confirmation: Relay 2 activated (Pin 3 HIGH)
    MS_CLOSE_INTERLOCK       - Confirmation: Relay 2 deactivated (Pin 3 LOW)
    P1_OPEN_INTERLOCK        - Confirmation: Relay 9 activated (Pin 13 HIGH)
    P1_CLOSE_INTERLOCK       - Confirmation: Relay 9 deactivated (Pin 13 LOW)
    STANDBY_OPEN_INTERLOCK   - Confirmation: Standby relay activated (Pin 8 HIGH)
    STANDBY_CLOSE_INTERLOCK  - Confirmation: Standby relay deactivated (Pin 8 LOW)
  
  Serial Input (PC → Arduino):
    RELAY2_ON           - Set Pin 3 HIGH (open MS interlock)
    RELAY2_OFF          - Set Pin 3 LOW (close MS interlock)
    RELAY_P1_ON         - Set Pin 13 HIGH (open P1 interlock)
    RELAY_P1_OFF        - Set Pin 13 LOW (close P1 interlock)
    STANDBY_RELAY_ON    - Set Pin 8 HIGH (open Standby interlock)
    STANDBY_RELAY_OFF   - Set Pin 8 LOW (close Standby interlock)
    SCAN_TRIGGER        - Trigger MS ID R35D-B scanner to scan once (Serial1)
    SCANNER_RELAY_ON    - Set Pin 4 LOW (power on MS ID scanner)
    SCANNER_RELAY_OFF   - Set Pin 4 HIGH (power off MS ID scanner)
    KIT_RELAY_ON        - Set Pin 6 LOW (power on MS Kitting scanner)
    KIT_RELAY_OFF       - Set Pin 6 HIGH (power off MS Kitting scanner)
    KIT_SCAN_TRIGGER    - Trigger MS Kitting R35D-B scanner to scan once (Serial2)
    P1_SCANNER_RELAY_ON   - Set Pin 10 LOW (power on P1 ID scanner)
    P1_SCANNER_RELAY_OFF  - Set Pin 10 HIGH (power off P1 ID scanner)
    P1_SCAN_TRIGGER       - Trigger P1 ID R35D-B scanner to scan once (Serial3)
    P1_KIT_RELAY_ON       - Set Pin 12 LOW (power on P1 Kitting scanner)
    P1_KIT_RELAY_OFF      - Set Pin 12 HIGH (power off P1 Kitting scanner)
    P1_KIT_SCAN_TRIGGER   - Trigger P1 Kitting R35D-B scanner to scan once (Serial3)
*/

// Pin definitions (Arduino Mega)
const int SENSOR1_PIN = 2;        // Digital Pin 2  (IR Sensor 1 - MS ID detection)
const int RELAY2_PIN = 3;         // Digital Pin 3  (Relay 2 - MS solenoid interlock)
const int SCANNER_RELAY_PIN = 4;  // Digital Pin 4  (Relay 4 - R35D-B ID scanner power)
const int KITTING_SENSOR_PIN = 5; // Digital Pin 5  (IR Sensor 5 - Kitting QR detection at MS)
const int KITTING_RELAY_PIN = 6;  // Digital Pin 6  (Relay 5 - Kitting QR scanner power)
const int SENSOR2_PIN = 7;        // Digital Pin 7  (IR Sensor 2 - dunpla at Standby)
const int STANDBY_RELAY_PIN = 8;  // Digital Pin 8  (Standby Relay - Standby interlock)
const int SENSOR_P1_PIN = 9;           // Digital Pin 9  (IR Sensor 3 - P1 ID detection)
const int P1_SCANNER_RELAY_PIN = 10;   // Digital Pin 10 (Relay 7 - P1 Scanner 3 power)
const int P1_KITTING_SENSOR_PIN = 11;  // Digital Pin 11 (IR Sensor 5 - P1 kitting detection)
const int P1_KITTING_RELAY_PIN = 12;   // Digital Pin 12 (Relay 8 - P1 Scanner 4 power)
const int RELAY_P1_PIN = 13;           // Digital Pin 13 (Relay 9 - P1 solenoid interlock)
const int SENSOR_P2_PIN = 22;          // Digital Pin 22 (IR Sensor 6 - dunpla at Process 1)
const int SENSOR_P2_ID_PIN = 23;       // Digital Pin 23 (IR Sensor 7 - P2 ID detection)
const int RELAY_P2_PIN = 27;           // Digital Pin 27 (Relay 4 - P2 solenoid interlock)
const int SENSOR_P3_PIN = 28;          // Digital Pin 28 (IR Sensor 8 - dunpla at Process 2)

// Scanner state
String scannerBuffer = "";
bool scannerEnabled = false;  // Only accept scan data when enabled
bool scanComplete = false;    // Prevents re-triggering until ID is removed
unsigned long unloadStartTime = 0;        // When sensor went HIGH (for stable unload detection)
const unsigned long STABLE_UNLOAD_TIME = 100;  // 500ms of continuous HIGH required to reset scanComplete
                                              // YOU CAN TRY 1500 OR 1000 FOR 

// Scanner power relay boot delay (scanner needs time to initialize after power-on)
bool scannerPoweringUp = false;           // true while waiting for scanner to boot
unsigned long scannerPowerOnTime = 0;     // When relay was turned ON
const unsigned long SCANNER_BOOT_DELAY = 2000; // 2s boot time for ID R35D-B after power restore
const unsigned long KITTING_SCANNER_BOOT_DELAY = 2000; // 2s boot time for Kitting R35D-B
bool scannerWasOff = true;                // Scanner starts OFF, will power on at first MS1LOAD
unsigned long lastScanTriggerTime = 0;    // When last scan trigger was sent (for retry)
const unsigned long SCAN_RETRY_MS = 3000; // Re-trigger scanner every 3s if no scan received

// Kitting QR scanner state (2nd R35D-B on Serial2, mirrors ID scanner logic)
String kitScannerBuffer = "";
bool kitScannerEnabled = false;           // Only accept kitting scan data when enabled
bool kitScanComplete = false;             // Prevents re-triggering until dunpla removed
bool kitScannerPoweringUp = false;        // true while waiting for kitting scanner to boot
unsigned long kitScannerPowerOnTime = 0;  // When kitting relay was turned ON
bool kitScannerWasOff = true;             // Kitting scanner starts OFF
unsigned long lastKitScanTriggerTime = 0; // When last kitting trigger was sent (for retry)
unsigned long kitLastByteTime = 0;        // When last byte received on Serial2 (for timeout detection)
const unsigned long KIT_BUFFER_TIMEOUT = 50; // 50ms silence = complete message (no CR/LF needed)
bool prevKittingSensorState = HIGH;       // Previous state for edge detection
unsigned long kittingUnloadPendingStart = 0; // For sustained HIGH check on removal
const unsigned long KITTING_UNLOAD_SUSTAIN_MS = 500; // 500ms sustained HIGH to confirm removal

// P1 ID scanner state (3rd R35D-B on Serial3, mirrors MS ID scanner logic)
String p1ScannerBuffer = "";
bool p1ScannerEnabled = false;           // Only accept P1 scan data when enabled
bool p1ScanComplete = false;             // Prevents re-triggering until ID is removed
bool p1ScannerPoweringUp = false;        // true while waiting for P1 scanner to boot
unsigned long p1ScannerPowerOnTime = 0;  // When P1 scanner relay was turned ON
bool p1ScannerWasOff = true;             // P1 scanner starts OFF
unsigned long p1LastScanTriggerTime = 0; // When last P1 scan trigger was sent (for retry)

// P1 Kitting relay/sensor state (scanner serial data read by Cycle Time Arduino)
bool p1KitScannerPoweringUp = false;     // true while waiting for P1 kitting scanner to boot
unsigned long p1KitScannerPowerOnTime = 0;  // When P1 kitting relay was turned ON
bool p1KitScannerWasOff = true;          // P1 kitting scanner starts OFF
bool p1KitScanCycleActive = false;       // true while relay is locked for boot+scan (immune to sensor glitch)
unsigned long p1KitScanCycleStart = 0;   // When scan cycle started (for safety timeout)
const unsigned long P1_KIT_SCAN_TIMEOUT = 10000; // 10s safety timeout: auto-off if scan never completes
bool prevP1KittingSensorState = HIGH;    // Previous state for P1 kitting edge detection
unsigned long p1KittingUnloadPendingStart = 0; // For sustained HIGH check on P1 kitting removal
const unsigned long P1_KITTING_UNLOAD_SUSTAIN_MS = 500; // 500ms sustained HIGH to confirm removal

// Previous sensor states for edge detection (HIGH = no object due to pull-up)
bool prevSensor1State = HIGH;

// State-based detection for Pin 9 (P1 ID sensor)
bool p1IsLoaded = false;                  // P1_ID_LOAD fired — reset on sustained HIGH
unsigned long sensorP1LoadStart = 0;      // When LOW first seen (for sustained load check)
unsigned long sensorP1UnloadStart = 0;    // When HIGH first seen (for sustained unload check)
const unsigned long P1_SUSTAIN_MS = 500;  // 500ms sustained to confirm load/unload

// State-based detection for dunpla sensors (Pin 7 and Pin 22)
bool dunplaAtStandby = false;  // STANDBY_MS fired — reset when sensor goes HIGH
bool dunplaAtP1 = false;       // P1_DONE fired — reset when sensor goes HIGH
bool dunplaAtP2 = false;       // P2_DONE fired — reset when sensor goes HIGH

// State-based detection for Pin 23 (P2 ID sensor)
bool p2IsLoaded = false;                  // P2_ID_LOAD fired — reset on sustained HIGH
unsigned long sensorP2IdLoadStart = 0;    // When LOW first seen (for sustained load check)
unsigned long sensorP2IdUnloadStart = 0;  // When HIGH first seen (for sustained unload check)

// Debounce timing
unsigned long lastSensor1Debounce = 0;
const unsigned long DEBOUNCE_DELAY = 250; // 250ms debounce for sensors

// Sustained-LOW confirmation for dunpla sensors (reject vibration/noise spikes)
unsigned long sensor2PendingStart = 0;   // 0 = not pending, >0 = millis() when LOW first seen
unsigned long sensorP2PendingStart = 0;  // 0 = not pending, >0 = millis() when LOW first seen
unsigned long sensorP3PendingStart = 0;  // 0 = not pending, >0 = millis() when LOW first seen (Pin 28)
const unsigned long SUSTAIN_CONFIRM_MS = 2000; // Sensor must stay LOW for 2s to confirm real detection (Pin 7)
const unsigned long SENSOR_P2_SUSTAIN_MS = 500;  // Pin 22: 500ms sustained LOW (faster than standby)
const unsigned long SENSOR_P3_SUSTAIN_MS = 500;  // Pin 28: 500ms sustained LOW
const unsigned long INTERLOCK_SUSTAIN_MS = 100;  // Pin 7 fast detection when interlock is open (100ms)
unsigned int sensor2LowCount = 0;                // Count of LOW readings when interlock open (accumulator)
const unsigned int INTERLOCK_LOW_THRESHOLD = 3;  // Fire STANDBY_MS after 3 LOW readings (non-consecutive OK)
unsigned long sensor2HighStart = 0;              // 0 = not pending, >0 = millis() when HIGH first seen
const unsigned long SENSOR2_HIGH_DEBOUNCE = 200; // Pin 7 must stay HIGH for 200ms to confirm real removal (rejects relay EMI noise)
bool interlockOpen = false;                      // true while RELAY2_ON active — suppresses Pin 2 edge detection

// Sustained-HIGH confirmation for MS1UNLOAD.
// Keep a short 200ms stability check to reject electrical noise, but do NOT wait 2 seconds.
// This check is also allowed while the ID scanner is ON so removing the ID cancels scanning.
unsigned long sensor1UnloadPendingStart = 0; // 0 = not pending, >0 = millis() when HIGH first seen
const unsigned long UNLOAD_SUSTAIN_MS = 200; // Fast unload response with brief noise filtering

// Scanner trigger command for R35D-B (SYN + T + CR + LF)
// Change this if your scanner uses a different trigger sequence
const byte SCANNER_TRIGGER[] = {0x16, 0x54, 0x0D, 0x0A};
const int SCANNER_TRIGGER_LEN = 4;

// Scanner stop command for R35D-B (SYN + U + CR + LF)
const byte SCANNER_STOP[] = {0x16, 0x55, 0x0D, 0x0A};
const int SCANNER_STOP_LEN = 4;

// Serial receive buffer
String serialBuffer = "";

bool isCleanIdScan(const String &data) {
  for (unsigned int i = 0; i < data.length(); ++i) {
    byte c = data.charAt(i);
    if (c < 32 || c == 127) return false;
  }
  return true;
}

void setup() {
  // Start serial communication at 9600 baud
  Serial.begin(9600);
  
  // Start hardware Serial1 for ID R35D-B scanner at 115200 baud (Pin 19=RX1, Pin 18=TX1)
  Serial1.begin(115200);
  
  // Start hardware Serial2 for Kitting R35D-B scanner at 115200 baud (Pin 17=RX2, Pin 16=TX2)
  Serial2.begin(115200);
  
  // Start hardware Serial3 for P1 scanners (shared) at 115200 baud (Pin 15=RX3, Pin 14=TX3)
  Serial3.begin(115200);
  
  // Set sensor pins as INPUT with internal pull-up resistor
  pinMode(SENSOR1_PIN, INPUT_PULLUP);
  pinMode(SENSOR_P1_PIN, INPUT_PULLUP);
  pinMode(SENSOR2_PIN, INPUT_PULLUP);
  pinMode(SENSOR_P2_PIN, INPUT_PULLUP);
  pinMode(SENSOR_P2_ID_PIN, INPUT_PULLUP);
  pinMode(SENSOR_P3_PIN, INPUT_PULLUP);
  pinMode(KITTING_SENSOR_PIN, INPUT_PULLUP);
  pinMode(P1_KITTING_SENSOR_PIN, INPUT_PULLUP);
  
  // Set relay pins as OUTPUT, initially LOW (interlock closed)
  pinMode(RELAY2_PIN, OUTPUT);
  digitalWrite(RELAY2_PIN, LOW);
  pinMode(RELAY_P1_PIN, OUTPUT);
  digitalWrite(RELAY_P1_PIN, LOW);
  // Standby interlock relay: initially LOW (interlock closed)
  pinMode(STANDBY_RELAY_PIN, OUTPUT);
  digitalWrite(STANDBY_RELAY_PIN, LOW);
  // P2 interlock relay: initially LOW (interlock closed)
  pinMode(RELAY_P2_PIN, OUTPUT);
  digitalWrite(RELAY_P2_PIN, LOW);
  // ID Scanner power relay: initially HIGH (relay OFF = scanner OFF at boot, no sensor interference)
  pinMode(SCANNER_RELAY_PIN, OUTPUT);
  digitalWrite(SCANNER_RELAY_PIN, HIGH);
  // Kitting Scanner power relay: initially HIGH (relay OFF = scanner OFF at boot)
  pinMode(KITTING_RELAY_PIN, OUTPUT);
  digitalWrite(KITTING_RELAY_PIN, HIGH);
  // P1 Scanner 3 power relay: initially HIGH (relay OFF = scanner OFF at boot)
  pinMode(P1_SCANNER_RELAY_PIN, OUTPUT);
  digitalWrite(P1_SCANNER_RELAY_PIN, HIGH);
  // P1 Scanner 4 (kitting) power relay: initially HIGH (relay OFF = scanner OFF at boot)
  pinMode(P1_KITTING_RELAY_PIN, OUTPUT);
  digitalWrite(P1_KITTING_RELAY_PIN, HIGH);
  
  // Pre-allocate String buffers to prevent heap fragmentation
  scannerBuffer.reserve(110);
  kitScannerBuffer.reserve(110);
  p1ScannerBuffer.reserve(110);
  serialBuffer.reserve(32);
  
  delay(1000);
  Serial.println("ESP_READY");
}

void loop() {
  unsigned long currentTime = millis();
  
  // --- Stable Unload Detection: Reset scanComplete only after sensor HIGH for 2+ seconds ---
  if (scanComplete && !interlockOpen && digitalRead(SENSOR1_PIN) == HIGH) {
    if (unloadStartTime == 0) {
      unloadStartTime = millis();
    } else if (millis() - unloadStartTime >= STABLE_UNLOAD_TIME) {
      scanComplete = false;
      unloadStartTime = 0;
    }
  } else {
    unloadStartTime = 0;
  }

  // --- Scanner Boot Delay: Enable scanning only after scanner has had time to initialize ---
  if (scannerPoweringUp && (currentTime - scannerPowerOnTime >= SCANNER_BOOT_DELAY)) {
    scannerPoweringUp = false;
    scannerEnabled = true;
    // The scanner may already have decoded the ID during boot; keep queued bytes.
    if (Serial1.available() == 0) {
      Serial1.write(SCANNER_TRIGGER, SCANNER_TRIGGER_LEN);
    }
    lastScanTriggerTime = currentTime;
    Serial.println("MS_SCANNER_READY");
  }

  // --- Scanner Retry: Re-trigger if no scan data received within timeout ---
  if (scannerEnabled && !scanComplete && !scannerWasOff && !scannerPoweringUp &&
      lastScanTriggerTime > 0 && (currentTime - lastScanTriggerTime >= SCAN_RETRY_MS)) {
    if (Serial1.available() == 0 && scannerBuffer.length() == 0) {
      Serial1.write(SCANNER_TRIGGER, SCANNER_TRIGGER_LEN);
    }
    lastScanTriggerTime = currentTime;
  }

  // --- P1 Scanner 3 Boot Delay: Enable scanning only after scanner has had time to initialize ---
  if (p1ScannerPoweringUp && (currentTime - p1ScannerPowerOnTime >= SCANNER_BOOT_DELAY)) {
    p1ScannerPoweringUp = false;
    p1ScannerEnabled = true;
    // Preserve any ID decoded while Scanner 3 was booting.
    if (Serial3.available() == 0) {
      Serial3.write(SCANNER_TRIGGER, SCANNER_TRIGGER_LEN);
    }
    p1LastScanTriggerTime = currentTime;
    Serial.println("P1_SCANNER_READY");
  }

  // --- P1 Scanner 3 Retry: Re-trigger if no scan data received within timeout ---
  if (p1ScannerEnabled && !p1ScanComplete && !p1ScannerWasOff && !p1ScannerPoweringUp &&
      p1LastScanTriggerTime > 0 && (currentTime - p1LastScanTriggerTime >= SCAN_RETRY_MS)) {
    if (Serial3.available() == 0 && p1ScannerBuffer.length() == 0) {
      Serial3.write(SCANNER_TRIGGER, SCANNER_TRIGGER_LEN);
    }
    p1LastScanTriggerTime = currentTime;
  }

  // --- Sensor 1: ID Detection (Pin 2) ---
  // Falling edge (HIGH→LOW) = ID detected → MS1LOAD (fires immediately)
  // Rising edge (LOW→HIGH) = ID removed → MS1UNLOAD (requires 500ms sustained HIGH)
  bool sensor1 = digitalRead(SENSOR1_PIN);
  if (sensor1 == LOW && prevSensor1State == HIGH) {
    // Falling edge: ID placed — fire MS1LOAD immediately (with debounce)
    if ((currentTime - lastSensor1Debounce) > DEBOUNCE_DELAY) {
      lastSensor1Debounce = currentTime;
      prevSensor1State = LOW;
      sensor1UnloadPendingStart = 0;
      Serial.println("MS_LOAD_ID");
      // Enable scanner reception when ID is detected (only once per ID placement)
      if (!scanComplete) {
        if (scannerWasOff) {
          // Scanner was powered off — power ON and wait for boot
          digitalWrite(SCANNER_RELAY_PIN, LOW);
          scannerPoweringUp = true;
          scannerPowerOnTime = currentTime;
          scannerWasOff = false;
          // scannerEnabled will be set after SCANNER_BOOT_DELAY
        } else {
          // Scanner already powered — enable immediately and trigger
          scannerEnabled = true;
          scannerBuffer = "";
          while (Serial1.available()) Serial1.read(); // Flush stale data
          Serial1.write(SCANNER_TRIGGER, SCANNER_TRIGGER_LEN);
          lastScanTriggerTime = currentTime;
        }
      }
    }
  } else if (sensor1 == HIGH && prevSensor1State == LOW && !interlockOpen) {
    // Rising edge: ID removed. Confirm HIGH briefly, even if the scanner is still ON.
    // This lets an unscanned ID cancel the Scan ID popup/scanner instead of waiting forever.
    if (sensor1UnloadPendingStart == 0) {
      sensor1UnloadPendingStart = currentTime;
    } else if ((currentTime - sensor1UnloadPendingStart) >= UNLOAD_SUSTAIN_MS) {
      if ((currentTime - lastSensor1Debounce) > DEBOUNCE_DELAY) {
        lastSensor1Debounce = currentTime;
        prevSensor1State = HIGH;
        sensor1UnloadPendingStart = 0;

        // Cancel any pending scanner boot, retry, or active scan immediately.
        scannerPoweringUp = false;
        scannerEnabled = false;
        lastScanTriggerTime = 0;
        scannerBuffer = "";

        // If the scanner is still powered, stop the laser and turn the relay OFF.
        if (!scannerWasOff) {
          Serial1.write(SCANNER_STOP, SCANNER_STOP_LEN);
          digitalWrite(SCANNER_RELAY_PIN, HIGH); // Active LOW relay: HIGH = scanner OFF
          scannerWasOff = true;
        }

        // Removal ends the current ID cycle, whether a scan succeeded or was cancelled.
        scanComplete = false;
        unloadStartTime = 0;
        Serial.println("MS_UNLOAD_ID");
      }
    }
  } else {
    // Sensor is stable (matches prevState) — reset pending unload if sensor went back LOW
    if (sensor1 == LOW) {
      sensor1UnloadPendingStart = 0;
    }
  }
  
  // --- Sensor P1: ID Detection (Pin 9) --- STATE-BASED ---
  // Sensor LOW for 500ms AND not already loaded → P1_ID_LOAD
  // Sensor HIGH for 500ms AND was loaded → P1_ID_UNLOAD
  // Blanked while scanner relay is ON (crosstalk)
  bool sensorP1 = digitalRead(SENSOR_P1_PIN);
  if (!scannerWasOff) {
    // Scanner is ON — blank Pin 9 (crosstalk makes it unreliable)
    sensorP1LoadStart = 0;
    sensorP1UnloadStart = 0;
  } else if (sensorP1 == LOW && !p1IsLoaded) {
    // Sensor LOW, not yet loaded — check 500ms sustained
    if (sensorP1LoadStart == 0) {
      sensorP1LoadStart = currentTime;
    } else if ((currentTime - sensorP1LoadStart) >= P1_SUSTAIN_MS) {
      p1IsLoaded = true;
      sensorP1LoadStart = 0;
      Serial.println("P1_ID_LOAD");
      // Enable P1 scanner (Scanner 3) when ID is detected (mirrors MS1LOAD → MS scanner)
      if (!p1ScanComplete) {
        if (p1ScannerWasOff) {
          // Scanner 3 was OFF — power ON and wait for boot
          digitalWrite(P1_SCANNER_RELAY_PIN, LOW); // Active LOW: LOW = relay ON
          p1ScannerPoweringUp = true;
          p1ScannerPowerOnTime = currentTime;
          p1ScannerWasOff = false;
        } else {
          // Scanner 3 already powered — enable immediately and trigger
          p1ScannerEnabled = true;
          p1ScannerBuffer = "";
          while (Serial3.available()) Serial3.read(); // Flush stale data
          Serial3.write(SCANNER_TRIGGER, SCANNER_TRIGGER_LEN);
          p1LastScanTriggerTime = currentTime;
        }
      }
    }
    sensorP1UnloadStart = 0;
  } else if (sensorP1 == HIGH && p1IsLoaded && !interlockOpen) {
    // Sensor HIGH, was loaded — check 500ms sustained
    if (sensorP1UnloadStart == 0) {
      sensorP1UnloadStart = currentTime;
    } else if ((currentTime - sensorP1UnloadStart) >= P1_SUSTAIN_MS) {
      p1IsLoaded = false;
      sensorP1UnloadStart = 0;

      // Cancel any pending P1 scanner boot, retry, or active scan
      p1ScannerPoweringUp = false;
      p1ScannerEnabled = false;
      p1LastScanTriggerTime = 0;
      p1ScannerBuffer = "";

      // If P1 scanner is still powered, stop and power OFF
      if (!p1ScannerWasOff) {
        Serial3.write(SCANNER_STOP, SCANNER_STOP_LEN);
        digitalWrite(P1_SCANNER_RELAY_PIN, HIGH); // Active LOW: HIGH = OFF
        p1ScannerWasOff = true;
      }

      p1ScanComplete = false;
      Serial.println("P1_ID_UNLOAD");
    }
    sensorP1LoadStart = 0;
  } else {
    // Stable state — reset pending timers
    sensorP1LoadStart = 0;
    sensorP1UnloadStart = 0;
  }
  
  // --- Sensor 2: Dunpla Detection at Standby (Pin 7) --- STATE-BASED ---
  // Sensor LOW for 2s AND not already detected → STANDBY_MS
  // Sensor HIGH → reset flag (allow re-detection)
  // Blanked while scanner relay is ON (no state corruption)
  bool sensor2 = digitalRead(SENSOR2_PIN);
  if (!scannerWasOff) {
    // Scanner is ON — blank Pin 7 (interference makes it unreliable)
    sensor2PendingStart = 0;
    sensor2HighStart = 0;
  } else if (sensor2 == LOW && !dunplaAtStandby) {
    // Sensor LOW and not yet fired
    sensor2HighStart = 0;
    if (interlockOpen) {
      // Count-based detection: accumulate LOW readings (resilient to EMI gaps)
      sensor2LowCount++;
      if (sensor2LowCount >= INTERLOCK_LOW_THRESHOLD) {
        dunplaAtStandby = true;
        sensor2LowCount = 0;
        sensor2PendingStart = 0;
        Serial.println("STANDBY_MS");
      }
    } else {
      // Normal: timer-based sustained LOW (2s)
      if (sensor2PendingStart == 0) {
        sensor2PendingStart = currentTime;
      } else if ((currentTime - sensor2PendingStart) >= SUSTAIN_CONFIRM_MS) {
        dunplaAtStandby = true;
        sensor2PendingStart = 0;
        Serial.println("STANDBY_MS");
      }
    }
  } else if (sensor2 == HIGH) {
    if (interlockOpen) {
      // Interlock open: ignore HIGH readings (relay EMI)
      // Do NOT reset sensor2LowCount — let it accumulate across EMI gaps
      sensor2HighStart = 0;
      // Also check pre-started timer (dunpla was present before relay ON)
      if (!dunplaAtStandby && sensor2PendingStart > 0 &&
          (currentTime - sensor2PendingStart) >= INTERLOCK_SUSTAIN_MS) {
        dunplaAtStandby = true;
        sensor2PendingStart = 0;
        sensor2LowCount = 0;
        Serial.println("STANDBY_MS");
      }
    } else {
      // Normal: debounce HIGH to confirm real removal
      if (sensor2HighStart == 0) {
        sensor2HighStart = currentTime;
      } else if ((currentTime - sensor2HighStart) >= SENSOR2_HIGH_DEBOUNCE) {
        // Confirmed: sensor stayed HIGH for 200ms (real removal, not noise)
        dunplaAtStandby = false;
        sensor2PendingStart = 0;
        sensor2HighStart = 0;
        sensor2LowCount = 0;
      }
    }
  }
  
  // --- Sensor P2: Dunpla Detection at Process 1 (Pin 22) --- STATE-BASED ---
  // Sensor LOW for 500ms AND not already detected → P1_DONE
  // Sensor HIGH → reset flag (allow re-detection)
  bool sensorP2 = digitalRead(SENSOR_P2_PIN);
  if (sensorP2 == LOW && !dunplaAtP1) {
    // Sensor LOW and not yet fired — check sustained
    if (sensorP2PendingStart == 0) {
      sensorP2PendingStart = currentTime;
    } else if ((currentTime - sensorP2PendingStart) >= SENSOR_P2_SUSTAIN_MS) {
      // Confirmed: sensor stayed LOW for 500ms (real dunpla)
      dunplaAtP1 = true;
      sensorP2PendingStart = 0;
      Serial.println("P1_DONE");
    }
  } else if (sensorP2 == HIGH) {
    // Sensor HIGH — reset flag and timer (dunpla removed, allow re-detection)
    dunplaAtP1 = false;
    sensorP2PendingStart = 0;
  }
  
  // --- Sensor P2 ID: ID Detection (Pin 23) --- STATE-BASED ---
  // Sensor LOW for 500ms AND not already loaded → P2_ID_LOAD
  // Sensor HIGH for 500ms AND was loaded → P2_ID_UNLOAD
  bool sensorP2Id = digitalRead(SENSOR_P2_ID_PIN);
  if (sensorP2Id == LOW && !p2IsLoaded) {
    if (sensorP2IdLoadStart == 0) {
      sensorP2IdLoadStart = currentTime;
    } else if ((currentTime - sensorP2IdLoadStart) >= P1_SUSTAIN_MS) {
      p2IsLoaded = true;
      sensorP2IdLoadStart = 0;
      Serial.println("P2_ID_LOAD");
    }
    sensorP2IdUnloadStart = 0;
  } else if (sensorP2Id == HIGH && p2IsLoaded) {
    if (sensorP2IdUnloadStart == 0) {
      sensorP2IdUnloadStart = currentTime;
    } else if ((currentTime - sensorP2IdUnloadStart) >= P1_SUSTAIN_MS) {
      p2IsLoaded = false;
      sensorP2IdUnloadStart = 0;
      Serial.println("P2_ID_UNLOAD");
    }
    sensorP2IdLoadStart = 0;
  } else {
    sensorP2IdLoadStart = 0;
    sensorP2IdUnloadStart = 0;
  }

  // --- Sensor P3: Dunpla Detection at Process 2 (Pin 28) --- STATE-BASED ---
  // Sensor LOW for 500ms AND not already detected → P2_DONE
  // Sensor HIGH → reset flag (allow re-detection)
  bool sensorP3 = digitalRead(SENSOR_P3_PIN);
  if (sensorP3 == LOW && !dunplaAtP2) {
    if (sensorP3PendingStart == 0) {
      sensorP3PendingStart = currentTime;
    } else if ((currentTime - sensorP3PendingStart) >= SENSOR_P3_SUSTAIN_MS) {
      dunplaAtP2 = true;
      sensorP3PendingStart = 0;
      Serial.println("P2_DONE");
    }
  } else if (sensorP3 == HIGH) {
    dunplaAtP2 = false;
    sensorP3PendingStart = 0;
  }

  // --- Kitting Sensor (Pin 5): Dunpla Detection for Kitting QR scan ---
  // Falling edge (HIGH→LOW) = kitting dunpla placed → KITTING_LOAD, power on kitting scanner
  // Rising edge (LOW→HIGH) sustained = dunpla removed → KITTING_UNLOAD, reset for next cycle
  bool kittingSensor = digitalRead(KITTING_SENSOR_PIN);
  if (!scannerWasOff) {
    // ID Scanner is ON — blank Pin 5 (crosstalk from adjacent Pin 4 scanner relay)
    kittingUnloadPendingStart = 0;
  } else if (kittingSensor == LOW && prevKittingSensorState == HIGH) {
    // Falling edge: kitting dunpla placed
    prevKittingSensorState = LOW;
    kittingUnloadPendingStart = 0;
    Serial.println("MS_KITTING_LOAD");
    // Enable kitting scanner (same flow as ID scanner on MS1LOAD)
    if (!kitScanComplete) {
      if (kitScannerWasOff) {
        // Kitting scanner was OFF — power ON and wait for boot
        digitalWrite(KITTING_RELAY_PIN, LOW); // Active LOW: LOW = relay ON
        kitScannerPoweringUp = true;
        kitScannerPowerOnTime = currentTime;
        kitScannerWasOff = false;
      } else {
        // Kitting scanner already powered — enable immediately and trigger
        kitScannerEnabled = true;
        kitScannerBuffer = "";
        kitLastByteTime = 0;
        // Flush any idle noise (scanner was STOPped, not booting — no valid data waiting)
        while (Serial2.available()) Serial2.read();
        Serial2.write(SCANNER_TRIGGER, SCANNER_TRIGGER_LEN);
        lastKitScanTriggerTime = currentTime;
      }
    }
  } else if (kittingSensor == HIGH && prevKittingSensorState == LOW) {
    // Rising edge: kitting dunpla removed — start sustained HIGH check
    if (kittingUnloadPendingStart == 0) {
      kittingUnloadPendingStart = currentTime;
    } else if ((currentTime - kittingUnloadPendingStart) >= KITTING_UNLOAD_SUSTAIN_MS) {
      prevKittingSensorState = HIGH;
      kittingUnloadPendingStart = 0;
      Serial.println("MS_KITTING_UNLOAD");
      // Turn off kitting scanner when dunpla removed (same as ID scanner on unload)
      kitScannerEnabled = false;
      kitScanComplete = false;
      kitScannerPoweringUp = false;
      kitScannerBuffer = "";
      kitLastByteTime = 0;
      lastKitScanTriggerTime = 0;
      // Stop scanner laser and power OFF relay
      if (!kitScannerWasOff) {
        Serial2.write(SCANNER_STOP, SCANNER_STOP_LEN);
        digitalWrite(KITTING_RELAY_PIN, HIGH); // Active LOW: HIGH = OFF
        kitScannerWasOff = true;
      }
      // Safe place to discard any residual bytes: the physical kitting dunpla is already removed.
      while (Serial2.available()) Serial2.read();
    }
  } else if (kittingSensor == LOW) {
    kittingUnloadPendingStart = 0; // Still LOW — reset pending unload
  }

  // --- Kitting Scanner Boot Delay ---
  if (kitScannerPoweringUp && (currentTime - kitScannerPowerOnTime >= KITTING_SCANNER_BOOT_DELAY)) {
    kitScannerPoweringUp = false;
    kitScannerEnabled = true;
    kitScannerBuffer = "";
    kitLastByteTime = 0;

    // IMPORTANT: Do NOT flush Serial2 here.
    // If the R35D-B already scanned the QR while booting, the valid bytes may already be queued.
    // Only send a trigger when no scanner data is already waiting.
    if (Serial2.available() == 0) {
      Serial2.write(SCANNER_TRIGGER, SCANNER_TRIGGER_LEN);
    }
    lastKitScanTriggerTime = currentTime;
    Serial.println("MS_KIT_SCANNER_READY");
  }

  // --- Kitting Scanner Retry: Re-trigger only when no QR bytes are waiting ---
  if (kitScannerEnabled && !kitScanComplete && !kitScannerWasOff && !kitScannerPoweringUp &&
      lastKitScanTriggerTime > 0 && (currentTime - lastKitScanTriggerTime >= SCAN_RETRY_MS)) {
    // Do NOT clear kitScannerBuffer and do NOT flush Serial2 here.
    // Flushing here can delete the QR before the read section processes it.
    if (Serial2.available() == 0 && kitScannerBuffer.length() == 0) {
      Serial2.write(SCANNER_TRIGGER, SCANNER_TRIGGER_LEN);
      lastKitScanTriggerTime = currentTime;
    }
  }

  // --- Kitting R35D-B Scanner: Read scanned data via hardware Serial2 ---
  // Read only after the scanner is enabled. This preserves QR bytes that may arrive during boot.
  if (!kitScannerWasOff && kitScannerEnabled) {
    while (Serial2.available() > 0) {
      char c = Serial2.read();
      kitLastByteTime = currentTime;

      // R35D-B terminates the QR with CR/LF. Either one can complete the message.
      if (c == '\n' || c == '\r') {
        kitScannerBuffer.trim();

        if (kitScannerBuffer.length() > 0 && !kitScanComplete) {
          // Forward scanned data to PC with the exact KITSCAN: prefix expected by the bridge.
          Serial.println("MS_KITSCAN:" + kitScannerBuffer);

          // One-shot scan: stop accepting data until Pin 5 is unloaded and loaded again.
          kitScannerEnabled = false;
          kitScanComplete = true;

          // Stop scanner laser but keep relay ON to avoid 2s boot delay on next cycle
          Serial2.write(SCANNER_STOP, SCANNER_STOP_LEN);
          lastKitScanTriggerTime = 0;
          kitLastByteTime = 0;
        }

        kitScannerBuffer = "";
      } else {
        // Keep printable ASCII only. This rejects possible scanner boot/control garbage.
        if (c >= 32 && c <= 126 && kitScannerBuffer.length() < 100) {
          kitScannerBuffer += c;
        }
      }
    }

    // Fallback: if a scanner configuration does not send CR/LF,
    // 100 ms of silence after printable data is treated as end-of-message.
    if (kitScannerBuffer.length() > 0 && !kitScanComplete && kitLastByteTime > 0 &&
        (currentTime - kitLastByteTime >= 100)) {
      kitScannerBuffer.trim();

      if (kitScannerBuffer.length() > 0) {
        Serial.println("MS_KITSCAN:" + kitScannerBuffer);
        kitScannerEnabled = false;
        kitScanComplete = true;
        // Stop scanner laser but keep relay ON to avoid 2s boot delay on next cycle
        Serial2.write(SCANNER_STOP, SCANNER_STOP_LEN);
        lastKitScanTriggerTime = 0;
      }

      kitScannerBuffer = "";
      kitLastByteTime = 0;
    }
  }

  // --- ID R35D-B Scanner: Read scanned data via hardware Serial1 ---
  // Only process Serial1 when scanner is powered ON (Pin 19 floats when OFF, causing noise floods)
  if (!scannerWasOff && scannerEnabled) {
  while (Serial1.available() > 0) {
    char c = Serial1.read();
    if (c == '\n' || c == '\r') {
      bool cleanFrame = isCleanIdScan(scannerBuffer);
      scannerBuffer.trim();
      if (scannerBuffer.length() > 0 && scannerEnabled && cleanFrame) {
        // Forward scanned data to PC with SCAN: prefix
        Serial.println("MS_SCAN:" + scannerBuffer);
        // Disable scanner after successful scan (one-shot)
        scannerEnabled = false;
        scanComplete = true;
        // Stop scanner laser after successful read
        Serial1.write(SCANNER_STOP, SCANNER_STOP_LEN);
        // Power OFF scanner via relay (active LOW: HIGH = OFF)
        digitalWrite(SCANNER_RELAY_PIN, HIGH);
        scannerWasOff = true;
      }
      scannerBuffer = "";
    } else {
      if (scannerBuffer.length() < 100) { // Prevent buffer overflow
        scannerBuffer += c;
      }
    }
  }
  } // end if (!scannerWasOff)

  // --- P1 Kitting Sensor (Pin 11): Dunpla Detection for P1 Kitting QR scan ---
  // Falling edge (HIGH→LOW) = kitting dunpla placed → P1_KITTING_LOAD, power on Scanner 4 relay
  // Rising edge (LOW→HIGH) sustained = dunpla removed → P1_KITTING_UNLOAD
  // NOTE: Scanner 4 serial data is read by the Cycle Time Arduino (Serial1 on that board).
  //       This board only controls the relay (Pin 12) and detects the sensor (Pin 11).
  //       Relay is LOCKED during scan cycle (p1KitScanCycleActive) to survive sensor noise.
  bool p1KittingSensor = digitalRead(P1_KITTING_SENSOR_PIN);
  if (p1KittingSensor == LOW && prevP1KittingSensorState == HIGH) {
    // Falling edge: P1 kitting dunpla placed
    prevP1KittingSensorState = LOW;
    p1KittingUnloadPendingStart = 0;
    Serial.println("P1_KITTING_LOAD");
    // Power on Scanner 4 relay if it was OFF, start scan cycle lock
    if (p1KitScannerWasOff && !p1KitScanCycleActive) {
      digitalWrite(P1_KITTING_RELAY_PIN, LOW); // Active LOW: LOW = relay ON
      p1KitScannerPoweringUp = true;
      p1KitScannerPowerOnTime = currentTime;
      p1KitScannerWasOff = false;
      p1KitScanCycleActive = true;
      p1KitScanCycleStart = currentTime;
    }
  } else if (p1KittingSensor == HIGH && prevP1KittingSensorState == LOW) {
    // Rising edge: P1 kitting dunpla removed — start sustained HIGH check
    if (p1KittingUnloadPendingStart == 0) {
      p1KittingUnloadPendingStart = currentTime;
    } else if ((currentTime - p1KittingUnloadPendingStart) >= P1_KITTING_UNLOAD_SUSTAIN_MS) {
      prevP1KittingSensorState = HIGH;
      p1KittingUnloadPendingStart = 0;
      Serial.println("P1_KITTING_UNLOAD");
      if (!p1KitScanCycleActive) {
        // No active scan cycle — safe to power off relay immediately
        digitalWrite(P1_KITTING_RELAY_PIN, HIGH); // Active LOW: HIGH = OFF
        p1KitScannerWasOff = true;
        p1KitScannerPoweringUp = false;
      }
      // If scan cycle is active, relay stays ON — will be powered off by P1_KIT_SCAN_DONE or timeout
    }
  } else if (p1KittingSensor == LOW) {
    p1KittingUnloadPendingStart = 0; // Still LOW — reset pending unload
  }

  // --- P1 Kitting Scanner 4 Boot Delay ---
  // After boot delay, send P1_KIT_SCANNER_READY so Flask can trigger scan on CT Arduino
  if (p1KitScannerPoweringUp && (currentTime - p1KitScannerPowerOnTime >= KITTING_SCANNER_BOOT_DELAY)) {
    p1KitScannerPoweringUp = false;
    Serial.println("P1_KIT_SCANNER_READY");
  }

  // --- P1 Kitting Scan Cycle Safety Timeout ---
  // If scan cycle has been active for too long (scanner failed), auto-power off relay
  if (p1KitScanCycleActive && (currentTime - p1KitScanCycleStart >= P1_KIT_SCAN_TIMEOUT)) {
    p1KitScanCycleActive = false;
    p1KitScannerPoweringUp = false;
    digitalWrite(P1_KITTING_RELAY_PIN, HIGH); // Active LOW: HIGH = OFF
    p1KitScannerWasOff = true;
    Serial.println("P1_KIT_SCAN_TIMEOUT");
  }

  // --- P1 ID Scanner 3: Read scanned data via hardware Serial3 (P1 ID only) ---
  if (!p1ScannerWasOff && p1ScannerEnabled) {
    while (Serial3.available() > 0) {
      char c = Serial3.read();
      if (c == '\n' || c == '\r') {
        bool cleanFrame = isCleanIdScan(p1ScannerBuffer);
        p1ScannerBuffer.trim();
        if (p1ScannerBuffer.length() > 0 && !p1ScanComplete && cleanFrame) {
          Serial.println("P1_SCAN:" + p1ScannerBuffer);
          p1ScannerEnabled = false;
          p1ScanComplete = true;
          Serial3.write(SCANNER_STOP, SCANNER_STOP_LEN);
          digitalWrite(P1_SCANNER_RELAY_PIN, HIGH); // Active LOW: HIGH = OFF
          p1ScannerWasOff = true;
          p1LastScanTriggerTime = 0;
        }
        p1ScannerBuffer = "";
      } else {
        if (p1ScannerBuffer.length() < 100) {
          p1ScannerBuffer += c;
        }
      }
    }
  }

  // --- Serial Receive: Commands from Python ---
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      serialBuffer.trim();
      if (serialBuffer.length() > 0) {
        handleSerialCommand(serialBuffer);
        serialBuffer = "";
      }
    } else {
      serialBuffer += c;
    }
  }
  
  // Small delay to prevent excessive CPU usage (1ms = fast Pin 7 sampling)
  delay(1);
}

// Handle incoming serial commands from Python
void handleSerialCommand(String cmd) {
  if (cmd == "RELAY2_ON") {
    // Read Pin 7 BEFORE energizing relay (no EMI yet = reliable reading)
    bool dunplaPresent = (digitalRead(SENSOR2_PIN) == LOW);
    digitalWrite(RELAY2_PIN, HIGH);
    interlockOpen = true;
    // Re-arm Pin 7 so STANDBY_MS can fire again and close the interlock.
    // Mirrors Python's "dunpla flag reset" — without this the latch deadlocks
    // whenever the dunpla is still covering Pin 7 when the interlock opens.
    dunplaAtStandby = false;
    sensor2PendingStart = dunplaPresent ? millis() : 0;
    sensor2LowCount = dunplaPresent ? INTERLOCK_LOW_THRESHOLD : 0;
    sensor2HighStart = 0;
    sensorP1UnloadStart = 0;
    Serial.println("MS_OPEN_INTERLOCK");
  } else if (cmd == "RELAY2_OFF") {
    digitalWrite(RELAY2_PIN, LOW);
    interlockOpen = false;
    Serial.println("MS_CLOSE_INTERLOCK");
  } else if (cmd == "RELAY_P1_ON") {
    digitalWrite(RELAY_P1_PIN, HIGH);
    Serial.println("P1_OPEN_INTERLOCK");
  } else if (cmd == "RELAY_P1_OFF") {
    digitalWrite(RELAY_P1_PIN, LOW);
    Serial.println("P1_CLOSE_INTERLOCK");
  } else if (cmd == "RELAY_P2_ON") {
    digitalWrite(RELAY_P2_PIN, HIGH);
    Serial.println("P2_OPEN_INTERLOCK");
  } else if (cmd == "RELAY_P2_OFF") {
    digitalWrite(RELAY_P2_PIN, LOW);
    Serial.println("P2_CLOSE_INTERLOCK");
  } else if (cmd == "STANDBY_RELAY_ON") {
    digitalWrite(STANDBY_RELAY_PIN, HIGH);
    Serial.println("STANDBY_OPEN_INTERLOCK");
  } else if (cmd == "STANDBY_RELAY_OFF") {
    digitalWrite(STANDBY_RELAY_PIN, LOW);
    Serial.println("STANDBY_CLOSE_INTERLOCK");
  } else if (cmd == "SCAN_TRIGGER") {
    triggerScanner();
  } else if (cmd == "SCANNER_RELAY_ON") {
    digitalWrite(SCANNER_RELAY_PIN, LOW);  // Active LOW: LOW = relay ON = scanner ON
    scannerWasOff = false;
    Serial.println("MS_SCANNER_RELAY_ON_CONFIRM");
  } else if (cmd == "SCANNER_RELAY_OFF") {
    digitalWrite(SCANNER_RELAY_PIN, HIGH); // Active LOW: HIGH = relay OFF = scanner OFF
    scannerWasOff = true;
    Serial.println("MS_SCANNER_RELAY_OFF_CONFIRM");
  } else if (cmd == "KIT_RELAY_ON") {
    digitalWrite(KITTING_RELAY_PIN, LOW);  // Active LOW: LOW = relay ON = kitting scanner ON
    kitScannerWasOff = false;
    Serial.println("MS_KIT_RELAY_ON_CONFIRM");
  } else if (cmd == "KIT_RELAY_OFF") {
    digitalWrite(KITTING_RELAY_PIN, HIGH); // Active LOW: HIGH = relay OFF = kitting scanner OFF
    kitScannerWasOff = true;
    Serial.println("MS_KIT_RELAY_OFF_CONFIRM");
  } else if (cmd == "KIT_SCAN_TRIGGER") {
    triggerKitScanner();
  } else if (cmd == "P1_SCANNER_RELAY_ON") {
    digitalWrite(P1_SCANNER_RELAY_PIN, LOW);  // Active LOW: LOW = relay ON = P1 scanner ON
    p1ScannerWasOff = false;
    Serial.println("P1_SCANNER_RELAY_ON_CONFIRM");
  } else if (cmd == "P1_SCANNER_RELAY_OFF") {
    digitalWrite(P1_SCANNER_RELAY_PIN, HIGH); // Active LOW: HIGH = relay OFF = P1 scanner OFF
    p1ScannerWasOff = true;
    Serial.println("P1_SCANNER_RELAY_OFF_CONFIRM");
  } else if (cmd == "P1_SCAN_TRIGGER") {
    triggerP1Scanner();
  } else if (cmd == "P1_KIT_RELAY_ON") {
    digitalWrite(P1_KITTING_RELAY_PIN, LOW);  // Active LOW: LOW = relay ON = P1 kitting scanner ON
    p1KitScannerWasOff = false;
    Serial.println("P1_KIT_RELAY_ON_CONFIRM");
  } else if (cmd == "P1_KIT_RELAY_OFF") {
    digitalWrite(P1_KITTING_RELAY_PIN, HIGH); // Active LOW: HIGH = relay OFF = P1 kitting scanner OFF
    p1KitScannerWasOff = true;
    Serial.println("P1_KIT_RELAY_OFF_CONFIRM");
  } else if (cmd == "P1_KIT_SCAN_DONE") {
    // Scan completed on CT Arduino — end scan cycle, power off relay
    p1KitScanCycleActive = false;
    p1KitScannerPoweringUp = false;
    digitalWrite(P1_KITTING_RELAY_PIN, HIGH); // Active LOW: HIGH = OFF
    p1KitScannerWasOff = true;
    Serial.println("P1_KIT_SCAN_DONE_CONFIRM");
  } else if (cmd == "REPORT_STATE") {
    // Re-emit current sensor states so Flask can sync after restart.
    // Only emit LOAD events — Flask initializes all states to false,
    // so only the true (loaded) states need correction.
    if (prevSensor1State == LOW) {
      Serial.println("MS_LOAD_ID");
    }
    if (p1IsLoaded) {
      Serial.println("P1_ID_LOAD");
    }
  } else if (cmd == "WIRE_CHECK") {
    // Report digital state of all monitored sensor/input pins for wiring diagnostics.
    // H = HIGH (no detection / wire open), L = LOW (detected / wire grounded).
    String status = "WIRE_STATUS:";
    status += "P2=";  status += (digitalRead(SENSOR1_PIN) == HIGH ? "H" : "L");
    status += ",P5="; status += (digitalRead(KITTING_SENSOR_PIN) == HIGH ? "H" : "L");
    status += ",P7="; status += (digitalRead(SENSOR2_PIN) == HIGH ? "H" : "L");
    status += ",P9="; status += (digitalRead(SENSOR_P1_PIN) == HIGH ? "H" : "L");
    status += ",P11="; status += (digitalRead(P1_KITTING_SENSOR_PIN) == HIGH ? "H" : "L");
    status += ",P22="; status += (digitalRead(SENSOR_P2_PIN) == HIGH ? "H" : "L");
    status += ",P23="; status += (digitalRead(SENSOR_P2_ID_PIN) == HIGH ? "H" : "L");
    status += ",P28="; status += (digitalRead(SENSOR_P3_PIN) == HIGH ? "H" : "L");
    Serial.println(status);
  }
}

// Trigger ID R35D-B scanner to accept one scan (called via SCAN_TRIGGER command)
void triggerScanner() {
  if (scanComplete) return;
  scannerEnabled = true;
  if (Serial1.available() == 0 && scannerBuffer.length() == 0) {
    Serial1.write(SCANNER_TRIGGER, SCANNER_TRIGGER_LEN);
  }
}

// Trigger Kitting R35D-B scanner to accept one scan (called via KIT_SCAN_TRIGGER command)
void triggerKitScanner() {
  if (kitScanComplete) return;
  kitScannerEnabled = true;
  kitScannerBuffer = "";
  kitLastByteTime = 0;
  // Do NOT flush Serial2 here; queued bytes may be valid QR data.
  if (Serial2.available() == 0) {
    Serial2.write(SCANNER_TRIGGER, SCANNER_TRIGGER_LEN); // Send hardware trigger to Kitting R35D-B
  }
  lastKitScanTriggerTime = millis();
}

// Trigger P1 ID R35D-B scanner to accept one scan (called via P1_SCAN_TRIGGER command)
void triggerP1Scanner() {
  if (p1ScanComplete) return;
  p1ScannerEnabled = true;
  if (Serial3.available() == 0 && p1ScannerBuffer.length() == 0) {
    Serial3.write(SCANNER_TRIGGER, SCANNER_TRIGGER_LEN);
  }
}

