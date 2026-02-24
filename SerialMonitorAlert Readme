OVERVIEW

-This project is a standalone embedded wearable system designed to detect falls and monitor heart rate in real time. It is built using the ESP32 microcontroller and implements a Finite State Machine (FSM) for structured and reliable fall detection with reduced false positives.

PROBLEM STATEMENT

-Falls are a major cause of injury among elderly individuals.
-Basic threshold-based systems generate frequent false alarms.
-A reliable embedded system is required to accurately detect real falls, reduce false positives, and provide a cancellation window before triggering emergency alerts.

PROJECT OBJECTIVES

-Develop a standalone embedded wearable system.
-Detect falls using motion sensing.
-Implement impact and immobility confirmation logic.
-Provide a configurable cancellation window (currently 5 seconds).
-Implement on-demand heart rate monitoring.
-Use structured embedded system design with a Finite State Machine.

HARDWARE COMPONENTS

-ESP32 Microcontroller
240 MHz processor
I2C communication support
Multiple GPIO interfaces
Scalable for future IoT integration

MPU6050 Accelerometer
-3-axis acceleration measurement
-Used for detecting sudden impact
-Communicates via I2C

MAX30102 Pulse Sensor
-Uses Photoplethysmography (PPG)
-Detects heart rate from blood volume changes
-BPM calculated from time interval between pulse peaks

OLED Display
-Displays system states
-Shows heart rate readings
-Provides user feedback

Buzzer
-Provides audible emergency alerts

Buttons
-Cancel and mode button (supports triple-click detection)
-Dedicated panic button for immediate emergency trigger

SYSTEM ARCHITECTURE

-Sensor Layer
MPU6050 for motion detection
MAX30102 for heart rate monitoring

-Processing Layer
ESP32 reads sensor data
Computes acceleration magnitude
Implements FSM-based decision logic
Handles timing using millis()

-Output Layer
OLED display for visual feedback
Buzzer for audio alerts
Serial Monitor for debugging and demonstration

FALL DETECTION ALGORITHM

Step 1
Acceleration magnitude is calculated using
A = sqrt(Ax^2 + Ay^2 + Az^2)

Step 2
Impact detection occurs if magnitude exceeds 25 m/s^2.
System transitions from IDLE to IMPACT_DETECTED.

Step 3
Immobility confirmation occurs when magnitude stabilizes between 8 and 12 m/s^2 for at least 2 seconds.

Step 4
If both conditions are satisfied, the system transitions to FALL_CONFIRMED.

Step 5
A 5-second cancellation window is provided.
If not cancelled, the system enters EMERGENCY_ALERT state.

FINITE STATE MACHINE STATES

IDLE
IMPACT_DETECTED
FALL_CONFIRMED
EMERGENCY_ALERT

State transitions are deterministic and time-controlled.

EMBEDDED CONCEPTS USED

I2C communication
Sensor interfacing
Real-time monitoring
millis() based timing logic
Finite State Machine implementation
Threshold calibration through experimentation
