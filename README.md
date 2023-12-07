# button-fixture-EEPROM
Arduino Uno code for button pushing fixture (4 solenoids - 4 buttons)


Basic premise of the testing fixture: actuate 4 individually powered and controlled solenoids to test the life cycle of mechanical switches, in a determined pattern (2X long presses (2 seconds), 2X short presses (0.5 seconds)
Each switch is evaluated individually to determine if the switch is not registering actuations (i.e. the switch is marked as "failed" or "F") or if the switch is stuck in the actuated position (i.e. the switch is marked as "stuck" or "S")

EEPROM is desired to allow the user to choose to either resume a previous test (where EEPROM would allow for retrieval of previous testing values) or start a new test
If there is a better way to do this without using EEPROM, or ways to optimize it, please let me know. I am by no means an Arduino IDE professional, so there are things that could definitely be optimized.
