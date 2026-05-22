Here is the English version of the README, ready to be copied and pasted directly into your GitHub repository!
📻 Arduino FM Radio Project
📝 General Description

This project consists of building a complete FM Radio using an Arduino Uno board and the TEA5767 radio module (operating in the 76 MHz - 108 MHz band).
The system features an LCD display to show the current station, a memory saving system for the last listened frequency, and an amplifier circuit to drive the volume on two speakers.
✨ Main Features

    Manual Tuning: Using two tactile buttons, you can increase or decrease the radio frequency to search for stations.

    I2C LCD Display: Shows the currently tuned frequency in real-time (e.g., 87.60 MHz).

    Persistent Memory (EEPROM): The radio remembers your favorite station! The last tuned frequency is saved in the Arduino's EEPROM memory and automatically reloaded upon the next power-up.

    Audio Control: The audio signal is amplified by a TDA2822M module to drive two 3W speakers in stereo.

    Power Management: An SPST switch allows you to easily turn the device on and off.

🛠️ Hardware Components

    1x Arduino Uno Rev3

    1x TEA5767 Radio Module

    1x 16x2 LCD Display with I2C module

    1x TDA2822M Amplifier

    2x 3W 8-ohm Speakers

    2x Tactile Buttons (Tactile Switch)

    1x SPST Switch

    1x Green LED (5 mm)

    5x 12-ohm Resistors

    1x 9V Battery & 1x 9V Adapter

    1x Generic Breadboard

    30x Jumper Wires

📚 Required Software Libraries

To successfully compile the source code in the Arduino IDE, make sure you have the following libraries installed:

    Wire.h (Included by default in Arduino)

    EEPROM.h (Included by default in Arduino)

    TEA5767N.h (For radio tuner management)

    LiquidCrystal_I2C.h (For display management via I2C protocol at address 0x27)

🚀 How to Run the Project

    Connect all components following your wiring diagram (remember that both the LCD display and the radio module communicate via the Arduino I2C pins A4 and A5).

    Open the code file using the Arduino IDE.

    Verify and upload the sketch to the board.

    Turn on the switch and enjoy your custom radio!
