#include <SoftwareSerial.h>

SoftwareSerial scannerSerial(8, 9);

String buffer = "";
const long baudRates[] = {9600, 115200, 19200, 38400, 57600, 4800, 2400};
const char* baudNames[] = {"9600", "115200", "19200", "38400", "57600", "4800", "2400"};

void setup() {
  Serial.begin(9600);
  Serial.println("=== AUTO BAUD SWEEP ===");
  Serial.println("Scan your ID card NOW and keep scanning!");
  Serial.println("");

  for (int i = 0; i < 7; i++) {
    scannerSerial.end();
    scannerSerial.begin(baudRates[i]);
    Serial.print("Testing ");
    Serial.print(baudNames[i]);
    Serial.print("... ");

    buffer = "";
    bool gotData = false;
    unsigned long start = millis();

    while (millis() - start < 5000) {
      if (scannerSerial.available()) {
        char c = scannerSerial.read();
        gotData = true;
        if (c >= 32 && c <= 126) buffer += c;
        else if ((c == '\r' || c == '\n') && buffer.length() > 0) break;
      }
    }

    if (buffer.length() > 0) {
      Serial.print("\"");
      Serial.print(buffer);
      Serial.println("\"");
      Serial.println("");
      Serial.print("*** CORRECT BAUD: ");
      Serial.print(baudNames[i]);
      Serial.println(" ***");
      return;
    } else if (gotData) {
      Serial.println("garbage (wrong baud)");
    } else {
      Serial.println("no data");
    }
  }
  Serial.println("Done. No match found.");
}

void loop() {}