class Webcam {
  private:
    bool enabled = false;
    int faceAttempts = 0;

  public:
    int captureTime = 0;

    // Constructor
    Webcam() {}

    // Call once in loop() to handle face auth
    bool awaitFaceRecognition() {

      if (Serial.available() > 0) {
        String msg = Serial.readStringUntil('\n');
        msg.trim();  // Remove newline/whitespace

        if (msg == "FACE_OK") {
          enabled = true;
          return true;
        } else if (msg == "FACE_NOTOK") {
          faceAttempts++;
          enabled = false;
          return false;
        } 
        else {
          return false;
        }
      }

      return false;  // Still waiting
    }

    void activateWebcam() {
      enabled = true;
    }

    void deactivateWebcam() {
      enabled = false;
    }

    void reset() {
      enabled = false;
      faceAttempts = 0;
    }

    bool isEnabled() {
      return enabled;
    }

    int getAttempts() {
      return faceAttempts;
    }
};

class Sensor {

  private:    
    int arduinoPin;
    bool enabled;

  public:
    String name;

    // Constructor
    Sensor(String sensorName, int sensorPin) {
      name = sensorName;
      arduinoPin = sensorPin;
      pinMode(arduinoPin, INPUT);
    }

    // Function to read sensor state
    int readSensorState() {
      return digitalRead(arduinoPin);
    }

};

class AlarmOutput {

  private:           

  public:
    String name;
    bool enabled;
    int arduinoPin; 

    // Constructor
    AlarmOutput(String outputName, int outputPin) {
      name = outputName;
      arduinoPin = outputPin;
      pinMode(arduinoPin, OUTPUT);
    }

};

class Buzzer : public AlarmOutput {
  private:
      int pitch;              // Frequency in Hz
      int volume;             // Simulated volume (0-255, works if using PWM/transistor)
      int currentDuration;    // How long the buzzer should play

  public:
      // Constructor
      Buzzer(String outputName, int outputPin) : AlarmOutput(outputName, outputPin) {
          pitch = 4000;         // Default pitch 1KHz
          volume = 255;         // Max simulated volume
          currentDuration = 0;  // Default duration
      }

      // Setters
      void setPitch(int newPitch) {
          pitch = newPitch;
      }

      void setVolume(int newVolume) {
          volume = constrain(newVolume, 0, 255); // Simulated volume, 0-255
      }

      void setDuration(int duration) {
          currentDuration = duration;
      }

      // Play the buzzer sound
      void playBuzzer() {
          tone(arduinoPin, pitch); // Start tone at current pitch
      }

      // Stop the buzzer
      void endBuzzer() {
          noTone(arduinoPin);  // Stop the tone
      }

      // Optionally: play for a duration
      void playForDuration(int durationMs) {
          playBuzzer();
          delay(durationMs); // Blocking delay for demo
          endBuzzer();
      }
  };


class LED : public AlarmOutput {
  private:

  public:
      int brightness;

      // Constructor
      LED(String outputName, int outputPin) : AlarmOutput(outputName, outputPin) {
        brightness = 255;
        pinMode(outputPin, OUTPUT);  // Make sure pin is set as output
      }

      // Setters
      void setBrightness(int newBrightness) {
          brightness = newBrightness;
      }

      // Turn LED on
      void turnOn() {
          digitalWrite(arduinoPin, HIGH);  // Use analogWrite for PWM brightness
      }

      // Turn LED off
      void turnOff() {
          digitalWrite(arduinoPin, LOW);  // Set to 0 brightness
      }
};

class SolenoidLock : public AlarmOutput {
  private:

  public:
      // Constructor
      SolenoidLock(String outputName, int outputPin) : AlarmOutput(outputName, outputPin) {
        pinMode(outputPin, OUTPUT);  // Make sure pin is set as output
      }

      void Lock() {
          digitalWrite(arduinoPin, HIGH);  // Lock again
      }

      void Unlock() {
          digitalWrite(arduinoPin, LOW);  // Lock again
      }
};

class MainSession {

  private:
    bool isActivated;
    bool isDeactivated;
    bool isTriggered;

    String correctPIN;

    Buzzer myBuzzer;
    SolenoidLock myLock;
    LED leds[3];
    Sensor doorSensor;
    Sensor motionSensor;
    Webcam myWebcam;

  public:
    int loggedUserID;

    MainSession() : myWebcam(), myBuzzer("MainBuzzer", 13), myLock("DoorLock", 8), leds{ LED("LED1", 51), LED("LED2", 49), LED("LED3", 47)}, doorSensor("window", 2), motionSensor("window", 30) {
        isActivated = true;
        isDeactivated = false;
        isTriggered = false;

        correctPIN = "1234";
    }

    // Function to display car details
    bool readSensors() {
        if (isActivated == true && isDeactivated == false) {

          //Read Sensors      
          return (motionSensor.readSensorState() || !doorSensor.readSensorState());
        }
        else {
          return false;
        }
    }

    bool checkForFace() {
      return myWebcam.awaitFaceRecognition();
    }

    void triggerAlarmProcedure() {
        isActivated = true;
        isDeactivated = false;
        isTriggered = true;

        myBuzzer.playBuzzer();
        myLock.Unlock();
        for (int i = 0; i < 3; i++) {
          leds[i].turnOn();   // Turn current LED on
        }

        Serial.println("ALARM_TRIGGERED");

        unsigned long startTime = millis();
        bool pinEntered = false;

        // Run this loop for 10 seconds
        while (!pinEntered & (millis() - startTime < 20000)) {
            // Check for incoming serial data
              pinEntered = readPin();              

            if (millis() - startTime > 10000) {
                myBuzzer.endBuzzer();  // Turn off buzzer after5s or upon PIN entry
            }

            // Do other non-blocking tasks if needed
            delay(10);  // Optional short delay to prevent CPU overload
        }        

        endAlarmProcedure();

        activateAllSensors();
    }

    void endAlarmProcedure() {
        isActivated = false;
        isDeactivated = false;
        isTriggered = false;

        myBuzzer.endBuzzer();
        myLock.Lock();
        for (int i = 0; i < 3; i++) {
          leds[i].turnOff();  // Then turn it off before moving to the next
        }
    }

    void deactivateAllSensors() {

        endAlarmProcedure();

        isActivated = false;
        isDeactivated = true;
        isTriggered = false;

        myLock.Unlock(); //Unlock Solenoid (since endAlarmProcedure locks it)

        Serial.println("ALARM_DEACTIVATED");
    }

    void activateAllSensors() {
        isActivated = true;
        isDeactivated = false;
        isTriggered = false;

        myLock.Lock(); //Unlock Solenoid (since endAlarmProcedure locks it)

        Serial.println("ALARM_ACTIVATED");
    }

    bool isActive() {
        return isActivated & !isDeactivated;
    }

    void logDateTime() {
        if (Serial.available()) { // Check if data is received
          String dateTime = Serial.readString(); // Read the string
          Serial.print("Received Time: ");
          Serial.println(dateTime); // Display the received time 
      }
    }

    bool readPin() {
      if (Serial.available() > 0) {
        String receivedPIN = Serial.readStringUntil('\n');
        receivedPIN.trim(); // remove newline and spaces

        if (receivedPIN == correctPIN) {
          Serial.println("ACCESS GRANTED");
          digitalWrite(LED_BUILTIN, HIGH); // Turn on LED
          return true;
        } else {
          Serial.println("ACCESS DENIED");
          digitalWrite(LED_BUILTIN, LOW);  // Turn off LED
          return false;
        }
      }

      return false;
  }

  void changePIN(String newPin) {
    correctPIN = newPin;
  }

  String waitForMATLAB() {
    String command = "";

    // Wait until something is available
    while (Serial.available() == 0) {
      // Optionally add timeout logic
      delay(10);
    }

    command = Serial.readStringUntil('\n');  // Waits for newline character
    command.trim();                          // Remove leading/trailing spaces
    return command;
  }  

  void turnOnLED(int ledIndex) {
        leds[ledIndex].turnOn(); 
    }

  void openSettingsMenu() {

    bool selectionMade = false;

    while (!selectionMade) {
      if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        input.trim();

        switch (input.toInt()) {
          case 1:
              selectionMade = true;              
              turnOnLED(1); 
              delay(10000);  
              activateAllSensors();
            break;

          case 2:            
            turnOnLED(1); 
            selectionMade = true;
            delay(1000);  
            deactivateAllSensors();
            break; 

          default:
            turnOnLED(1); 
            break;
        }
      }

      delay(10);
    }
  }
};

MainSession alarmSession;

void setup() {
  // Optional: configure pitch, volume, and duration
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  alarmSession.deactivateAllSensors();
}

void loop() {
  /*Serial.println("ALARM_DEACTIVATED");
  Serial.println("Enter a choice:");
  Serial.println("1 - Change PIN");
  Serial.println("2 - Arm the System"); */

  bool selectionMade = false;

  while (!selectionMade) {
      // Check for incoming serial data
      if (Serial.available()) {
          String input = Serial.readStringUntil('\n');
          input.trim();

          if (input == "1") {
              Serial.println("Enter new PIN:");
              while (Serial.available() == 0) {
                delay(10);
              }
              String newPin = Serial.readStringUntil('\n');
              newPin.trim();

              alarmSession.changePIN(newPin);
              selectionMade = true;
          } else if (input == "2") {              
              selectionMade = true;              
              alarmSession.turnOnLED(1); 
              delay(10000);   
              alarmSession.activateAllSensors();          
          } else {
              Serial.println("Invalid option. Please enter 1 or 2.");
          }
      }

      delay(10); // Small delay to prevent tight loop
  }

  while (alarmSession.isActive()) {
      if (alarmSession.checkForFace() == true) {         
        alarmSession.deactivateAllSensors();  

        unsigned long startTime = millis();
        bool messageReceived = false;

        while ((!messageReceived) & (millis() - startTime < 30000)) { // 60,000 ms = 60 seconds 
            alarmSession.turnOnLED(2);    
            messageReceived = alarmSession.readPin();
            delay(250);

            if (messageReceived) {
              delay(2000);
            }
        }

        alarmSession.turnOnLED(0);  
        if (!messageReceived) {
            alarmSession.activateAllSensors(); 
        }
        else {
          alarmSession.openSettingsMenu();
        }          
      }   
      else {
        if (alarmSession.readSensors()) {
          alarmSession.triggerAlarmProcedure();
          delay(250);  // optional throttle
        }
        else {
        }  
      }
  }
}
