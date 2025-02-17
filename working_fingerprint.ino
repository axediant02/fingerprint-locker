#include <Adafruit_Fingerprint.h>

// Pin definitions
#define SOLENOID_PIN 12  // Define solenoid pin (change to your actual pin number)
SoftwareSerial mySerial(2, 3); // RX, TX (Sensor TX to pin 2, RX to pin 3)
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup() {
    Serial.begin(9600);
    mySerial.begin(57600);
    
    // Initialize solenoid pin
    pinMode(SOLENOID_PIN, OUTPUT);
    digitalWrite(SOLENOID_PIN, LOW);  // Ensure solenoid starts locked

    Serial.println("Initializing fingerprint sensor...");
    finger.begin(57600);

    if (finger.verifyPassword()) {
        Serial.println("Sensor detected!");
    } else {
        Serial.println("Fingerprint sensor not found. Check wiring.");
        while (1);
    }

    Serial.println("\n📌 Place a finger to scan or enter 'r' to register new fingerprints.");
}

void loop() {
    // Check if user wants to register fingerprints
    if (Serial.available()) {
        char input = Serial.read();
        if (input == 'r' || input == 'R') {
            registerFingerprints();  // Register multiple fingerprints
        }
    }

    // Auto-detect fingerprints
    if (finger.getImage() == FINGERPRINT_OK) {  
        Serial.println("Finger detected! Processing...");

        if (finger.image2Tz(1) != FINGERPRINT_OK) {
            Serial.println("Failed to process fingerprint.");
            return;
        }

        if (finger.fingerFastSearch() == FINGERPRINT_OK) {  
            Serial.print("✅ Fingerprint matched! ID: ");
            Serial.println(finger.fingerID);
            digitalWrite(SOLENOID_PIN, HIGH);  
            delay(5000);  // Keep it unlocked for 5 seconds
            digitalWrite(SOLENOID_PIN, LOW);   
        } else {
            Serial.println("❌ No match found.");
        }

        delay(2000);  
    }
}

void registerFingerprints() {
    int id;
    char more = 'y';

    while (more == 'y' || more == 'Y') {
        Serial.println("\n📝 Enter ID (0-127): ");
        while (!Serial.available());
        id = Serial.parseInt();
        Serial.read(); // Clear buffer

        if (id < 0 || id > 127) {
            Serial.println("❌ Invalid ID.");
            continue; // Ask for a valid ID again
        }

        Serial.println("👉 Place your finger on the sensor...");
        while (finger.getImage() != FINGERPRINT_OK);

        if (finger.image2Tz(1) != FINGERPRINT_OK) {
            Serial.println("❌ Failed to process image. Try again.");
            continue;
        }

        Serial.println("✋ Remove your finger and place it again.");
        delay(2000);

        while (finger.getImage() != FINGERPRINT_OK);
        if (finger.image2Tz(2) != FINGERPRINT_OK) {
            Serial.println("❌ Failed to process second scan. Try again.");
            continue;
        }

        if (finger.createModel() != FINGERPRINT_OK) {
            Serial.println("❌ Fingerprints did not match. Try again.");
            continue;
        }

        if (finger.storeModel(id) == FINGERPRINT_OK) {
            Serial.println("✅ Fingerprint stored successfully!");
        } else {
            Serial.println("❌ Failed to store fingerprint.");
        }

        Serial.println("Do you want to register another fingerprint? (y/n)");
        while (!Serial.available());
        more = Serial.read();
        Serial.read(); // Clear buffer
    }

    Serial.println("\n📌 Place a finger to scan or enter 'r' to register more fingerprints.");
}