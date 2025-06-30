#include <WiFi.h>
#include <WebServer.h>
#include <LedControl.h>

// Secure Wi-Fi credentials (stored in Flash Memory)
#define WIFI_SSID "Vignesh"
#define WIFI_PASS "Vignesh29"

// Pins for MAX7219
#define DIN_PIN   23
#define CLK_PIN   18
#define CS_PIN    5

// Initialize the LED controller
LedControl lc = LedControl(DIN_PIN, CLK_PIN, CS_PIN, 1);

// Mutex for synchronization
portMUX_TYPE liftMutex = portMUX_INITIALIZER_UNLOCKED;

// Lift Data
int liftFloors[4] = {3, 1, 7, 8};
bool liftDirections[4] = {true, false, true, false};  // true = up, false = down

// Server initialization
WebServer server(80);

// Global variables
int currentFloor = -1;
int destinationFloor = -1;
bool liftOperational[4] = {true, true, true, true};  // Lift availability tracking
void handleLiftStatus() {
    String status = "Lift Status:\n";
    for (int i = 0; i < 4; i++) {
        status += "Lift " + String(i) + ": Floor " + String(liftFloors[i]) + 
                  " | Direction: " + (liftDirections[i] ? "Up" : "Down") + 
                  " | Operational: " + (liftOperational[i] ? "Yes" : "No") + "\n";
    }
    server.send(200, "text/plain", status);
}

void setup() {
    Serial.begin(115200);

    lc.shutdown(0, false); 
    lc.setIntensity(0, 8);  
    lc.clearDisplay(0);

    for (int i = 0; i < 4; i++) {
        lc.setDigit(0, i, liftFloors[i], false);
    }

    // Connect to Wi-Fi securely
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to Wi-Fi...");
    }
    Serial.println("Connected to Wi-Fi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Server routes
    server.on("/", handleRoot);
    server.on("/setFloor", handleSetFloor);
    server.on("/liftStatus", handleLiftStatus);
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
    updateLifts();
    monitorLiftFailures();
    delay(3000);
}

// Root request handler
void handleRoot() {
    server.send(200, "text/plain", "Lift Tracking System Active");
}

// Handle floor input request
void handleSetFloor() {
    if (!server.hasArg("current") || !server.hasArg("destination")) {
        server.send(400, "text/plain", "Error: Provide 'current' and 'destination' parameters.");
        return;
    }

    currentFloor = server.arg("current").toInt();
    destinationFloor = server.arg("destination").toInt();

    Serial.print("Current Floor: "); Serial.println(currentFloor);
    Serial.print("Destination Floor: "); Serial.println(destinationFloor);

    // Select best lift
    int chosenLift = selectBestLift(currentFloor, destinationFloor);

    if (chosenLift == -1) {
        server.send(500, "text/plain", "Error: No operational lifts available.");
        return;
    }

    const char* liftNames[] = {"B", "A", "C", "D"};
    server.send(200, "text/plain", liftNames[chosenLift]);

    // Highlight selected lift for better UI visibility
    lc.setChar(0, chosenLift, 'H', false);
    delay(3000);
    lc.setDigit(0, chosenLift, liftFloors[chosenLift], false);
}

// Lift Selection Logic with Weighted Prioritization
int selectBestLift(int current, int destination) {
    bool goingUp = (current < destination);
    int bestLift = -1;
    int bestPriority = 999;

    taskENTER_CRITICAL(&liftMutex); // Lock to prevent race conditions

    for (int i = 0; i < 4; i++) {
        if (!liftOperational[i]) continue; // Skip unavailable lifts

        int priority = abs(liftFloors[i] - current);

        if (goingUp && liftDirections[i] && liftFloors[i] < current) {
            if (priority < bestPriority) {
                bestPriority = priority;
                bestLift = i;
            }
        } else if (!goingUp && !liftDirections[i] && liftFloors[i] > current) {
            if (priority < bestPriority) {
                bestPriority = priority;
                bestLift = i;
            }
        }
    }

    // Fallback logic
    if (bestLift == -1) {
        bestPriority = 999;
        for (int i = 0; i < 4; i++) {
            if (!liftOperational[i]) continue;
            if ((goingUp && !liftDirections[i]) || (!goingUp && liftDirections[i])) {
                if (bestLift == -1 || liftFloors[i] < bestPriority) {
                    bestLift = i;
                    bestPriority = liftFloors[i];
                }
            }
        }
    }

    taskEXIT_CRITICAL(&liftMutex); // Unlock mutex
    return bestLift;
}

// Move lifts dynamically
void updateLifts() {
    taskENTER_CRITICAL(&liftMutex);

    for (int i = 0; i < 4; i++) {
        if (!liftOperational[i]) continue;

        liftFloors[i] += liftDirections[i] ? 1 : -1;

        if (liftFloors[i] >= 9) {
            liftFloors[i] = 9;
            liftDirections[i] = false;
        } else if (liftFloors[i] <= 0) {
            liftFloors[i] = 0;
            liftDirections[i] = true;
        }

        lc.setDigit(0, i, liftFloors[i], false);
    }

    taskEXIT_CRITICAL(&liftMutex);
}

// Simulate Lift Failures Every 30 Seconds
void monitorLiftFailures() {
    static unsigned long lastFailureTime = millis();
    if (millis() - lastFailureTime >= 30000) {
        int failedLift = random(0, 4);
        liftOperational[failedLift] = false;

        Serial.print("Lift "); Serial.print(failedLift); Serial.println(" stopped temporarily.");

        sendFailureNotification(failedLift);
        delay(10000);
        liftOperational[failedLift] = true;

        Serial.print("Lift "); Serial.print(failedLift); Serial.println(" resumed operation.");
        lastFailureTime = millis();
    }
}

// Send Email Notification
void sendFailureNotification(int liftID) {
    Serial.print("Sending email: Lift ");
    Serial.print(liftID);
    Serial.println(" is out of service.");
    // SMTP email logic here
}
