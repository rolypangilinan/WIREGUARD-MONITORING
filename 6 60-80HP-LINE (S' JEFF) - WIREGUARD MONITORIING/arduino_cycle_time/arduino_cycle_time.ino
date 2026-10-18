/*
  Cycle Time Monitoring - Arduino Mega Button Controller
  ======================================================
  Reads physical buttons for 9 processes (START/STOP each).
  Sends serial messages to the computer when a button is pressed.

  Pin Mapping:
  -----------------------------------------------
  Process 1: START=41, STOP=42
  Process 2: START=43, STOP=44
  Process 3: START=45, STOP=46
  Process 4: START=47, STOP=48
  Process 5: START=49, STOP=50
  Process 6: START=3,  STOP=4
  Process 7: START=5,  STOP=6
  Process 8: START=7,  STOP=8
  Process 9: START=9,  STOP=10
  -----------------------------------------------

  Wiring: Each button connects the pin to GND.
  Internal pull-up resistors are used (LOW = pressed).

  P1 Kitting Scanner 4 (R35D-B) via Serial1 (Pin 18 TX, Pin 19 RX):
    Relay and sensor on Material Setter Arduino.
    This board reads scanner serial data and handles scan triggers.

  Serial Output Examples:
    P1START, P1STOP, P2START, P2STOP, ... P9START, P9STOP
    P1_KITSCAN:<data> - Scanned QR from P1 Kitting Scanner 4

  Serial Input (PC → Arduino):
    P1_KIT_SCAN_TRIGGER - Trigger Scanner 4 to scan once (Serial1)
*/

// Number of processes
const int NUM_PROCESSES = 9;

// Pin arrays: index 0 = Process 1, index 8 = Process 9
const int startPins[NUM_PROCESSES] = {41, 43, 45, 47, 49, 3, 5, 7, 9};
const int stopPins[NUM_PROCESSES]  = {42, 44, 46, 48, 50, 4, 6, 8, 10};

// Previous button states for edge detection (HIGH = not pressed due to pull-up)
bool prevStartState[NUM_PROCESSES];
bool prevStopState[NUM_PROCESSES];

// Debounce timing
unsigned long lastStartDebounce[NUM_PROCESSES];
unsigned long lastStopDebounce[NUM_PROCESSES];
const unsigned long DEBOUNCE_DELAY = 250; // 250ms debounce (increased to filter noise)

// --- P1 Kitting Scanner 4 (R35D-B on Serial1, relay controlled by MS Arduino) ---
// Scanner trigger command for R35D-B (SYN + T + CR + LF)
const byte SCANNER_TRIGGER[] = {0x16, 0x54, 0x0D, 0x0A};
const int SCANNER_TRIGGER_LEN = 4;
// Scanner stop command for R35D-B (SYN + U + CR + LF)
const byte SCANNER_STOP[] = {0x16, 0x55, 0x0D, 0x0A};
const int SCANNER_STOP_LEN = 4;

String p1KitScannerBuffer = "";
bool p1KitScannerEnabled = false;           // Only accept scan data when enabled
bool p1KitScanComplete = false;             // Prevents re-triggering until reset
unsigned long p1KitLastScanTriggerTime = 0; // When last trigger was sent (for retry)
unsigned long p1KitLastByteTime = 0;        // When last byte received (for timeout)
const unsigned long SCAN_RETRY_MS = 3000;   // Re-trigger if no data within 3s

// Serial receive buffer for commands from PC
String serialBuffer = "";


void setup() {
  // Start serial communication at 9600 baud
  Serial.begin(9600);

  // P1 Kitting Scanner 4 via Serial1 (Pin 18 TX, Pin 19 RX)
  Serial1.begin(115200);

  // Pre-allocate String buffers
  p1KitScannerBuffer.reserve(110);
  serialBuffer.reserve(32);

  for (int i = 0; i < NUM_PROCESSES; i++) {
    // Set all button pins as INPUT with internal pull-up resistor
    // When button is NOT pressed, pin reads HIGH
    // When button IS pressed (connected to GND), pin reads LOW
    pinMode(startPins[i], INPUT_PULLUP);
    pinMode(stopPins[i], INPUT_PULLUP);

    // Initialize previous states as HIGH (not pressed)
    prevStartState[i] = HIGH;
    prevStopState[i] = HIGH;

    // Initialize debounce timers
    lastStartDebounce[i] = 0;
    lastStopDebounce[i] = 0;
  }

  Serial.println("ARDUINO_READY");
}

void loop() {
  unsigned long currentTime = millis();

  for (int i = 0; i < NUM_PROCESSES; i++) {
    // --- Read START button ---
    bool currentStartState = digitalRead(startPins[i]);

    // Detect falling edge (HIGH -> LOW) = button just pressed
    if (currentStartState == LOW && prevStartState[i] == HIGH) {
      if ((currentTime - lastStartDebounce[i]) > DEBOUNCE_DELAY) {
        // Double-read verification: read again after 5ms to filter noise
        delay(5);
        if (digitalRead(startPins[i]) == LOW) {
          lastStartDebounce[i] = currentTime;

          // Send START signal for this process
          Serial.print("P");
          Serial.print(i + 1);
          Serial.println("START");
        }
      }
    }
    prevStartState[i] = currentStartState;

    // --- Read STOP button ---
    bool currentStopState = digitalRead(stopPins[i]);

    // Detect falling edge (HIGH -> LOW) = button just pressed
    if (currentStopState == LOW && prevStopState[i] == HIGH) {
      if ((currentTime - lastStopDebounce[i]) > DEBOUNCE_DELAY) {
        // Double-read verification: read again after 5ms to filter noise
        delay(5);
        if (digitalRead(stopPins[i]) == LOW) {
          lastStopDebounce[i] = currentTime;

          // Send STOP signal for this process
          Serial.print("P");
          Serial.print(i + 1);
          Serial.println("STOP");
        }
      }
    }
    prevStopState[i] = currentStopState;
  }

  // --- P1 Kitting Scanner 4: Read scanned data via hardware Serial1 ---
  if (p1KitScannerEnabled && !p1KitScanComplete) {
    while (Serial1.available() > 0) {
      char c = Serial1.read();
      p1KitLastByteTime = currentTime;
      if (c == '\n' || c == '\r') {
        p1KitScannerBuffer.trim();
        if (p1KitScannerBuffer.length() > 0) {
          Serial.println("P1_KITSCAN:" + p1KitScannerBuffer);
          p1KitScannerEnabled = false;
          p1KitScanComplete = true;
          Serial1.write(SCANNER_STOP, SCANNER_STOP_LEN);
          p1KitLastScanTriggerTime = 0;
          p1KitLastByteTime = 0;
        }
        p1KitScannerBuffer = "";
      } else {
        if (c >= 32 && c <= 126 && p1KitScannerBuffer.length() < 100) {
          p1KitScannerBuffer += c;
        }
      }
    }

    // Fallback: 100ms silence after printable data = end-of-message
    if (p1KitScannerBuffer.length() > 0 && p1KitLastByteTime > 0 &&
        (currentTime - p1KitLastByteTime >= 100)) {
      p1KitScannerBuffer.trim();
      if (p1KitScannerBuffer.length() > 0) {
        Serial.println("P1_KITSCAN:" + p1KitScannerBuffer);
        p1KitScannerEnabled = false;
        p1KitScanComplete = true;
        Serial1.write(SCANNER_STOP, SCANNER_STOP_LEN);
        p1KitLastScanTriggerTime = 0;
      }
      p1KitScannerBuffer = "";
      p1KitLastByteTime = 0;
    }
  }

  // --- P1 Kitting Scanner 4 Retry: Re-trigger if no scan data within timeout ---
  if (p1KitScannerEnabled && !p1KitScanComplete &&
      p1KitLastScanTriggerTime > 0 && (currentTime - p1KitLastScanTriggerTime >= SCAN_RETRY_MS)) {
    if (Serial1.available() == 0 && p1KitScannerBuffer.length() == 0) {
      Serial1.write(SCANNER_TRIGGER, SCANNER_TRIGGER_LEN);
      p1KitLastScanTriggerTime = currentTime;
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

  // Small delay to prevent excessive CPU usage
  delay(5);
}

// Handle incoming serial commands from Python
void handleSerialCommand(String cmd) {
  if (cmd == "P1_KIT_SCAN_TRIGGER") {
    // Trigger Scanner 4 to scan once — called when Flask forwards after boot delay
    // Reset scan-complete flag for new cycle (flag was set by previous scan)
    p1KitScanComplete = false;
    p1KitScannerEnabled = true;
    p1KitScannerBuffer = "";
    p1KitLastByteTime = 0;
    // Flush any boot garbage before triggering
    while (Serial1.available()) Serial1.read();
    Serial1.write(SCANNER_TRIGGER, SCANNER_TRIGGER_LEN);
    p1KitLastScanTriggerTime = millis();
  }
}
