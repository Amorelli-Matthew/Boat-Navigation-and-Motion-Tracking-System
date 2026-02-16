# Boat Navigation & Motion Tracking System

This project is a custom-built navigation and motion tracking system designed for a boat. The system is intended to monitor the boat’s position, orientation, and motion in real time, providing accurate GPS data and inertial measurements for navigation, logging, or future control and analysis features.
The electronics are housed in a sealed plastic enclosure mounted inside the front sit-down seating area near the anchor cabinet, protecting the system from water exposure while keeping it centrally located on the vessel. At the core of the system is an ESP32-C3-MINI-1U microcontroller, chosen for its low power consumption, compact size, and built-in Bluetooth support. The ESP32 handles sensor data acquisition, processing, and wireless communication.For motion and orientation sensing, the system uses a 9-degree-of-freedom IMU (Inertial Measurement Unit) ie the WT901 high-accuracy 9-axis IMU. These sensors provide 3-axis acceleration, angular velocity, tilt angle, and magnetometer data, enabling accurate measurement of pitch, roll, heading, and overall boat motion. Position and speed data are obtained using a GPS module, such as the NEO-M7N.
The firmware integrates GPS and IMU data to create a unified navigation and motion dataset, which can be transmitted wirelessly or logged for later analysis. This project serves as a foundation for advanced features such as route tracking, motion analysis, stability monitoring, or integration with other onboard systems.


## Notice
BoatTrackingSystem is where the android app lives
BoatTrackingSystemEsp32 folder is where the esp32 tracking project lives

Obtain the "nimble_peripheral_utils" folder from path\to\esp32\example\directory\bluetooth\nimble\common\ and paste it into the BoatTrackingSystemEsp32 folder

## 3rd party licences


### For the esp32:

Apache Mynewt NimBLE
Copyright 2015-2025 The Apache Software Foundation

This product includes software developed at
The Apache Software Foundation (http://www.apache.org/).

Portions of this software were developed at
Runtime Inc, copyright 2015.
