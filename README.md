**Overview**
This prototype demonstrates a real-time lift allocation system where four lifts are simulated using ESP32 microcontrollers and 7-segment displays.
An Android app built with Kotlin sends floor requests via the HTTP protocol. The system evaluates lift positions and assigns the nearest available lift, 
indicating it with an 'H' (highlight) on the corresponding display.

**Features**
1. ESP32-based lift logic processing.
2. Android app interface for accepting user's current and destination floor requests.
3. HTTP based communication between app and hardware when device connected to the same network as Arduino's Wi-Fi module.
4. Optimal lift selection using floor difference logic.
5. Real-time display of lift assignment on 7-segment modules as well as blinking on the App.
6. Logic to randomly shut off one of the lifts every 1 minute is included and email will be sent about the lift not being in working condition.
 
**Hardware Components**
1. ESP32 (NodeMCU or DevKit)
2. 4x 7-segment displays (Common Cathode/Anode)
3. MAX7219 to control the LED segments of each 7-segment displays.
4. Jumper wires and breadboard
5. Power source to ESP32 like a power bank.

**Software components**
1. Arduino Integrated Development Environment to write the code logic and load it into the ESP32 using a USB cable.
2. Android Studio software suite to write the App code in kotlin and transfer it into the Mobile phone.
3. Necessary packages for the ESP32 like <Wifi.h>, <WebServer.h>, <LedControl.h>.

**Usage**
1. Open the Android app and enter your floor number and destination floor number.
2. Submit the request.
3. The nearest lift is selected and marked with ‘H’ on the corresponding display in the hardware as well as the corresponding Lift blinks in R-G color in the app.
4. System resets after 5 seconds.

**Limitations**
1. The logic is designed to take only proximity into account for choosing the appropriate lift. Other factors such as load,
2. As far as the prototype is concerned, since Wi-Fi is being used, the ESP32 and the mobile device must be connected to same network. Alternative protocols such as utilizing public cloud MQTT broker such as HiveMQ,
   Adafruit IO doesn't need this restriction.
