#include <EEPROM.h>

//set up solenoid pins
const int S1 = 2; //solenoid 1 at pin 2
const int S2 = 5; //solenoid 2 at pin 5
const int S3 = 6; //solenoid 3 at pin 6
const int S4 = 8; //solenoid 4 at pin 8

//set up switch pins
const int sw1 = 10; //switch 1 at pin 10
const int sw2 = 11; //switch 2 at pin 11
const int sw3 = 12; //switch 3 at pin 12
const int sw4 = 13; //switch 4 at pin 13

//set up variables for switch states
int sw1_state = 0; //used to read state of switch 1
int sw2_state = 0; //used to read state of switch 2
int sw3_state = 0; //used to read state of switch 3
int sw4_state = 0; //used to read state of switch 4

//set up the failure variable (change these to change values throughout code)
unsigned long int life = 328000; //life actuations for switches
int fail_count = 10; //max count of failures
int stuck_count = 10; //max count of stuck switch

//set up counter for total test iterations (to compare with switch functionality)
unsigned long int total_act_S1; //total number of solenoid/switch actuations for sw 1
unsigned long int total_act_S2; //total number of solenoid/switch actuations for sw 2
unsigned long int total_act_S3; //total number of solenoid/switch actuations for sw 3
unsigned long int total_act_S4; //total number of solenoid/switch actuations for sw 4

//set up counters to track test iterations
//using variable type 'unsigned long' as that is 0 to 4,294,967,296 (int only goes to 32,768)
unsigned long S1_num; //initialize counter for S1
unsigned long S2_num; //initialize counter for S2
unsigned long S3_num; //initialize counter for S3
unsigned long S4_num; //initialize counter for S4

//set up counters to track switch failures
int sw1_fail = 0; //counter for sw1 failures
int sw2_fail = 0; //counter for sw2 failures
int sw3_fail = 0; //counter for sw3 failures
int sw4_fail = 0; //counter for sw4 failures

//set up counters to track switches sticking (remaining in "On" position without force applied)
int sw1_stuck = 0; //counter for sw1 sticking
int sw2_stuck = 0; //counter for sw2 sticking
int sw3_stuck = 0; //counter for sw3 sticking
int sw4_stuck = 0; //counter for sw4 sticking

void setup() {  
  //EEPROM addresses for the total test iteration counters
  total_act_S1 = EEPROM.read(0);
  total_act_S2 = EEPROM.read(1);
  total_act_S3 = EEPROM.read(2);
  total_act_S4 = EEPROM.read(3);

  //EEPROM addresses for the test iteration counters
  S1_num = EEPROM.read(4);
  S2_num = EEPROM.read(5);
  S3_num = EEPROM.read(6);
  S4_num = EEPROM.read(7);

  //setup solenoid pins as outputs
  pinMode(S1, OUTPUT); //sets S1 pin as output
  pinMode(S2, OUTPUT); //sets S2 pin as output
  pinMode(S3, OUTPUT); //sets S3 pin as output
  pinMode(S4, OUTPUT); //sets S4 pin as output

  //ensure that all solenoids are inactive
  digitalWrite(S1, LOW); //turn off solenoid 1
  digitalWrite(S2, LOW); //turn off solenoid 2
  digitalWrite(S3, LOW); //turn off solenoid 3
  digitalWrite(S4, LOW); //turn off solenoid 4

  //setup switch pins as inputs
  pinMode(sw1, INPUT); //sets sw1 pin as input
  pinMode(sw2, INPUT); //sets sw2 pin as input
  pinMode(sw3, INPUT); //sets sw3 pin as input
  pinMode(sw4, INPUT); //sets sw4 pin as input

  //setup serial communication
  Serial.begin(9600);

  //print options for user to input integer regarding continuing or starting test
  Serial.println("Enter either 1 or 2 to indicate your decision");
  Serial.println("1. Continue previous test");
  Serial.println("2. Start new test");

  //holds code until user enters their choice
  while (Serial.available() == 0) {
  }

  int menuChoice = Serial.parseInt(); //sets a variable equal to the user input

  switch (menuChoice) {
    case 1:
      //read total actuation counters from EEPROM memory to continue with test
      EEPROM.get(0, total_act_S1);
      EEPROM.get(1, total_act_S2);
      EEPROM.get(2, total_act_S3);
      EEPROM.get(3, total_act_S4);
      //read test counters from EEPROM memory to continue with test
      EEPROM.get(4, S1_num);
      EEPROM.get(5, S2_num);
      EEPROM.get(6, S3_num);
      EEPROM.get(7, S4_num);
      break;

    case 2:
      //reset test counters to 0, as it is starting a new test
      total_act_S1 = total_act_S2 = total_act_S3 = total_act_S4 = 0; //initialize at 0
      //write values to EEPROM memory
      EEPROM.write(0, 0);
      EEPROM.write(1, 0);
      EEPROM.write(2, 0);
      EEPROM.write(3, 0);
      //initialize test iteration counters
      S1_num = S4_num = 0; //initialize test iteration counter at 0
      S2_num = S3_num = 1; //initialize counters at 1 to account for pattern
      EEPROM.write(4, 0); EEPROM.write(5, 0);
      EEPROM.write(6, 0); EEPROM.write(7, 0);
      
      break;

    default:
      Serial.println("Please enter only an integer value");
  }

  Serial.println("Plug in power supply");
  delay(2500);

  //delay to allow time to turn power supply on and print "Test Starting" message to serial monitor
  for (int i = 5; i >= 1; i--) {
    Serial.print("Test starting in "); Serial.print(i); Serial.println("...");
    delay(1000);
  }


  //iterate loop to start the pattern correctly (not having short presses for S3 and S4 at start)
  //0 s ---------------------------------------------------------------------------------------------------
  
  //switch 1 counter -----------------------------------------------------------------------
  sw1_state = digitalRead(sw1); //read switch 1 state

  //if switch is still reading "On" without actuation force, increment switch stuck counter
  if (sw1_state == HIGH) {
    sw1_stuck = sw1_stuck + 1; //increment counter by 1
  }

  //if switch is reading "Off" without actuation force, reset switch stuck counter to 0
  if (sw1_state == LOW) {
    sw1_stuck = 0; //reset counter to 0 as the switch is not stuck
  }
  //---------------------------------------------------------------------------------------- 
  
  //long press 1 for switch 1
  if (sw1_fail < fail_count && sw1_stuck < stuck_count) {
    if (total_act_S1 < life) {
      digitalWrite(S1, HIGH); //long press 1 for S1
      total_act_S1 = total_act_S1 + 1; //iterate total actuation counter by 1
    }
  }

  delay(0250);

  //0.25 s ------------------------------------------------------------------------------------------------
  delay(0250);

  //0.5 s -------------------------------------------------------------------------------------------------

  //switch 2 counter -----------------------------------------------------------------------
  sw2_state = digitalRead(sw2); //read switch 2 state

  //if switch is still reading "On" without actuation force, increment switch stuck counter
  if (sw2_state == HIGH) {
    sw2_stuck = sw2_stuck + 1; //increment counter by 1
  }

  //if switch is reading "Off" without actuation force, reset switch stuck counter to 0
  if (sw2_state == LOW) {
    sw2_stuck = 0; //reset counter to 0 as the switch is not stuck
  }
  //----------------------------------------------------------------------------------------
  
  //long press 1 for switch 2
  if (sw2_fail < fail_count && sw2_stuck < stuck_count) {
    if (total_act_S1 < life) {
      digitalWrite(S2, HIGH); //long press 1 for S2
      total_act_S2 = total_act_S2 + 1; //iterate total actuation counter by 1
    }
  }

  delay(0250);

  //0.75 s ------------------------------------------------------------------------------------------------
  delay(0250);

  //1.0 s -------------------------------------------------------------------------------------------------

  //switch 3 counter -----------------------------------------------------------------------
  sw3_state = digitalRead(sw3); //read switch 3 state

  //if switch is still reading "On" without actuation force, increment switch stuck counter
  if (sw3_state == HIGH) {
    sw3_stuck = sw3_stuck + 1; //increment counter by 1
  }

  //if switch is reading "Off" without actuation force, reset switch stuck counter to 0
  if (sw3_state == LOW) {
    sw3_stuck = 0; //reset counter to 0 as the switch is not stuck
  }
  //----------------------------------------------------------------------------------------

  
  //long press 1 for switch 3
  if (sw3_fail < fail_count && sw3_stuck < stuck_count) {
    if (total_act_S1 < life) {
      digitalWrite(S3, HIGH); //long press 1 for S3
      total_act_S3 = total_act_S3 + 1; //iterate total actuation counter by 1
    }
  }

  delay(0250);

  //1.25 s ------------------------------------------------------------------------------------------------
  delay(0250);

  //1.5 s -------------------------------------------------------------------------------------------------

  //switch 4 counter -----------------------------------------------------------------------
  sw4_state = digitalRead(sw4); //read switch 4 state

  //if switch is still reading "On" without actuation force, increment switch stuck counter
  if (sw4_state == HIGH) {
    sw4_stuck = sw4_stuck + 1; //increment counter by 1
  }

  //if switch is reading "Off" without actuation force, reset switch stuck counter to 0
  if (sw4_state == LOW) {
    sw4_stuck = 0; //reset counter to 0 as the switch is not stuck
  }
  //----------------------------------------------------------------------------------------
  
  //long press 1 for switch 4
  if (sw4_fail < fail_count && sw4_stuck < stuck_count) {
    if (total_act_S4 < life) {
      digitalWrite(S4, HIGH); //long press 1 for S4
      total_act_S4 = total_act_S4 + 1; //iterate total actuation counter by 1
    }
  }

  delay(0250);

  //1.75 s ------------------------------------------------------------------------------------------------
  delay(0250);

  //2.0 s -------------------------------------------------------------------------------------------------

  //switch 1 counter -----------------------------------------------------------------------
  sw1_state = digitalRead(sw1); //read switch 1 state

  //if switch is functional, increment counter
  if (sw1_state == HIGH) {
    S1_num = S1_num + 1; //increment counter by 1
    sw1_fail = 0; //reset switch failure counter to 0 since not consecutive failures
  }

  //if switch is non-functional, increment failure counter
  else if (sw1_state == LOW) {
    sw1_fail = sw1_fail + 1; //increment consecutive failure counter by 1
  }
  //----------------------------------------------------------------------------------------
  digitalWrite(S1, LOW); //end of long press 1 for S1
  sw1_state = 0; //sw1

  delay(0250);

  //2.25 s ------------------------------------------------------------------------------------------------
  delay(0250);

  //2.5 s -------------------------------------------------------------------------------------------------

  //switch 1 counter -----------------------------------------------------------------------
  sw1_state = digitalRead(sw1); //read switch 1 state

  //if switch is still reading "On" without actuation force, increment switch stuck counter
  if (sw1_state == HIGH) {
    sw1_stuck = sw1_stuck + 1; //increment counter by 1
  }

  //if switch is reading "Off" without actuation force, reset switch stuck counter to 0
  if (sw1_state == LOW) {
    sw1_stuck = 0; //reset counter to 0 as the switch is not stuck
  }
  //----------------------------------------------------------------------------------------
  
  //long press 2 for switch 1
  if (sw1_fail < fail_count && sw1_stuck < stuck_count) {
    if (total_act_S1 < life) {
      digitalWrite(S1, HIGH); //long press 2 for S1
      total_act_S1 = total_act_S1 + 1; //iterate total actuation counter by 1
    }
  }

  //switch 2 counter -----------------------------------------------------------------------
  sw2_state = digitalRead(sw2); //read switch 2 state
  
  //if switch is functional, increment counter
  if (sw2_state == HIGH) {
    S2_num = S2_num + 1; //increment counter by 1
    sw2_fail = 0; //reset switch failure counter to 0 since not consecutive failures
  }
  
  //if switch is non-functional, incremenet failure counter
  else if (sw2_state == LOW) {
    sw2_fail = sw2_fail + 1; //incremement consecutive failure counter by 1
  }
  //----------------------------------------------------------------------------------------
  digitalWrite(S2, LOW); //end of long press 1 for S2
  sw2_state = 0; //sw2

  delay(0250);

  //2.75 s ------------------------------------------------------------------------------------------------
  delay(0250);

  //3.0 s -------------------------------------------------------------------------------------------------

  //switch 2 counter -----------------------------------------------------------------------
  sw2_state = digitalRead(sw2); //read switch 2 state

  //if switch is still reading "On" without actuation force, increment switch stuck counter
  if (sw2_state == HIGH) {
    sw2_stuck = sw2_stuck + 1; //increment counter by 1
  }

  //if switch is reading "Off" without actuation force, reset switch stuck counter to 0
  if (sw2_state == LOW) {
    sw2_stuck = 0; //reset counter to 0 as the switch is not stuck
  }
  //----------------------------------------------------------------------------------------
  
  //long press 2 for switch 2
  if (sw2_fail < fail_count && sw2_stuck < stuck_count) {
    if (total_act_S2 < life) {
      digitalWrite(S2, HIGH); //long press 2 for S2
      total_act_S2 = total_act_S2 + 1; //iterate total actuation counter by 1
    }
  }

  //switch 3 counter -----------------------------------------------------------------------
  sw3_state = digitalRead(sw3); //read switch 3 state

  //if switch is functional, increment counter
  if (sw3_state == HIGH) {
    S3_num = S3_num + 1; //increment counter by 1
    sw3_fail = 0; //reset switch failure counter to 0 since not consecutive failures
  }

  //if switch is non-functional, increment failure counter
  else if (sw3_state == LOW) {
    sw3_fail = sw3_fail + 1; //incremement consecutive failure counter by 1
  }
  //----------------------------------------------------------------------------------------
  digitalWrite(S3, LOW); //end of long press 1 for S3
  sw3_state = 0; //sw3

  delay(0250);

  //3.25 s ------------------------------------------------------------------------------------------------
  delay(0250);

  //3.5 s -------------------------------------------------------------------------------------------------

  //switch 3 counter -----------------------------------------------------------------------
  sw3_state = digitalRead(sw3); //read switch 3 state

  //if switch is still reading "On" without actuation force, increment switch stuck counter
  if (sw3_state == HIGH) {
    sw3_stuck = sw3_stuck + 1; //increment counter by 1
  }

  //if switch is reading "Off" without actuation force, reset switch stuck counter to 0
  if (sw3_state == LOW) {
    sw3_stuck = 0; //reset counter to 0 as the switch is not stuck
  }
  //----------------------------------------------------------------------------------------

  //long press 2 for switch 3
  if (sw3_fail < fail_count && sw3_stuck < stuck_count) {
    if (total_act_S3 < life) {
      digitalWrite(S3, HIGH); //long press 2 for S3
      total_act_S3 = total_act_S3 + 1; //iterate total actuation counter by 1
    }
  }

  //switch 4 counter -----------------------------------------------------------------------
  sw4_state = digitalRead(sw4); //read switch 4 state

  //if switch is functional, increment counter
  if (sw4_state == HIGH) {
    S4_num = S4_num + 1; //increment counter by 1
    sw4_fail = 0; //reset switch failure counter to 0 since not consecutive failures
  }

  //if switch is non-functional, increment failure counter
  else if (sw4_state == LOW) {
    sw4_fail = sw4_fail + 1; //increment consecutive failure counter by 1
  } 
  //----------------------------------------------------------------------------------------
  digitalWrite(S4, LOW); //end of long press 1 for S4
  sw4_state = 0; //sw4

  delay(0250);

  //3.75 s ------------------------------------------------------------------------------------------------
  delay(0250);

  //4.0 s -------------------------------------------------------------------------------------------------

  //switch 4 counter -----------------------------------------------------------------------
  sw4_state = digitalRead(sw4); //read switch 4 state

  //if switch is still reading "On" without actuation force, increment switch stuck counter
  if (sw4_state == HIGH) {
    sw4_stuck = sw4_stuck + 1; //increment counter by 1
  }

  //if switch is reading "Off" without actuation force, reset switch stuck counter to 0
  if (sw4_state == LOW) {
    sw4_stuck = 0; //reset counter to 0 as the switch is not stuck
  }
  //----------------------------------------------------------------------------------------
  
  //long press 2 for switch 4
  if (sw4_fail < fail_count && sw4_stuck < stuck_count) {
    if (total_act_S4 < life) {
      digitalWrite(S4, HIGH); //long press 2 for S4
      total_act_S4 = total_act_S4 + 1; //iterate total actuation counter by 1
    }
  }

  delay(0250);

  //4.25 s ------------------------------------------------------------------------------------------------
  delay(0250);

  //4.5 s -------------------------------------------------------------------------------------------------

  //switch 1 counter -----------------------------------------------------------------------
  sw1_state = digitalRead(sw1); //read switch 1 state

  //if switch is functional, increment counter
  if (sw1_state == HIGH) {
    S1_num = S1_num + 1; //increment counter by 1
    sw1_fail = 0; //reset switch failure counter to 0 since not consecutive failures
  }

  //if switch is non-functional, increment failure counter
  else if (sw1_state == LOW) {
    sw1_fail = sw1_fail + 1; //increment consecutive failure counter by 1
  }
  //----------------------------------------------------------------------------------------
  digitalWrite(S1, LOW); //end of long press 1 for S1
  sw1_state = 0; //sw1

  delay(0250);

  //4.75 s ------------------------------------------------------------------------------------------------
  delay(0250);

  //5.0 s -------------------------------------------------------------------------------------------------

  //switch 1 counter -----------------------------------------------------------------------
  sw1_state = digitalRead(sw1); //read switch 1 state

  //if switch is still reading "On" without actuation force, increment switch stuck counter
  if (sw1_state == HIGH) {
    sw1_stuck = sw1_stuck + 1; //increment counter by 1
  }

  //if switch is reading "Off" without actuation force, reset switch stuck counter to 0
  if (sw1_state == LOW) {
    sw1_stuck = 0; //reset counter to 0 as the switch is not stuck
  }
  //----------------------------------------------------------------------------------------
  
  //short press 1 for switch 1
  if (sw1_fail < fail_count && sw1_stuck < stuck_count) {
    digitalWrite(S1, HIGH); //short press 1 for S1
    total_act_S1 = total_act_S1 + 1; //iterate total actuation counter by 1
  }

  //switch 2 counter -----------------------------------------------------------------------
  sw2_state = digitalRead(sw2); //read switch 2 state
  
  //if switch is functional, increment counter
  if (sw2_state == HIGH) {
    S2_num = S2_num + 1; //increment counter by 1
    sw2_fail = 0; //reset switch failure counter to 0 since not consecutive failures
  }
  
  //if switch is non-functional, incremenet failure counter
  else if (sw2_state == LOW) {
    sw2_fail = sw2_fail + 1; //incremement consecutive failure counter by 1
  }
  //----------------------------------------------------------------------------------------
  digitalWrite(S2, LOW); //end of long press 1 for S2
  sw2_state = 0; //sw2

  delay(0250);

  //5.25 s ------------------------------------------------------------------------------------------------
  delay(0250);

  //5.5 s -------------------------------------------------------------------------------------------------

  //switch 1 counter -----------------------------------------------------------------------
  sw1_state = digitalRead(sw1); //read switch 1 state

  //if switch is functional, increment counter
  if (sw1_state == HIGH) {
    S1_num = S1_num + 1; //increment counter by 1
    sw1_fail = 0; //reset switch failure counter to 0 since not consecutive failures
  }

  //if switch is non-functional, increment failure counter
  else if (sw1_state == LOW) {
    sw1_fail = sw1_fail + 1; //increment consecutive failure counter by 1
  }
  //----------------------------------------------------------------------------------------
  digitalWrite(S1, LOW); //end of long press 1 for S1
  sw1_state = 0; //sw1

  //switch 2 counter -----------------------------------------------------------------------
  sw2_state = digitalRead(sw2); //read switch 2 state

  //if switch is still reading "On" without actuation force, increment switch stuck counter
  if (sw2_state == HIGH) {
    sw2_stuck = sw2_stuck + 1; //increment counter by 1
  }

  //if switch is reading "Off" without actuation force, reset switch stuck counter to 0
  if (sw2_state == LOW) {
    sw2_stuck = 0; //reset counter to 0 as the switch is not stuck
  }
  //----------------------------------------------------------------------------------------

  //short press 1 for switch 2
  if (sw2_fail < fail_count && sw2_stuck < stuck_count) {
    if (total_act_S2 < life) {
      digitalWrite(S2, HIGH); //short press 1 for S2
      total_act_S2 = total_act_S2 + 1; //iterate total actuation counter by 1
    }
  }

  //switch 3 counter -----------------------------------------------------------------------
  sw3_state = digitalRead(sw3); //read switch 3 state

  //if switch is functional, increment counter
  if (sw3_state == HIGH) {
    S3_num = S3_num + 1; //increment counter by 1
    sw3_fail = 0; //reset switch failure counter to 0 since not consecutive failures
  }

  //if switch is non-functional, increment failure counter
  else if (sw3_state == LOW) {
    sw3_fail = sw3_fail + 1; //incremement consecutive failure counter by 1
  }
  //----------------------------------------------------------------------------------------
  digitalWrite(S3, LOW); //end of long press 1 for S3
  sw3_state = 0; //sw3

  delay(0250);

  //5.75 s ------------------------------------------------------------------------------------------------

  //switch 1 counter -----------------------------------------------------------------------
  sw1_state = digitalRead(sw1); //read switch 1 state

  //if switch is still reading "On" without actuation force, increment switch stuck counter
  if (sw1_state == HIGH) {
    sw1_stuck = sw1_stuck + 1; //increment counter by 1
  }

  //if switch is reading "Off" without actuation force, reset switch stuck counter to 0
  if (sw1_state == LOW) {
    sw1_stuck = 0; //reset counter to 0 as the switch is not stuck
  }
  //----------------------------------------------------------------------------------------
  
  //short press 2 for switch 1
  if (sw1_fail < fail_count && sw1_stuck < stuck_count) {
    if (total_act_S1 < life) {
      digitalWrite(S1, HIGH); //short press 2 for S1
      total_act_S1 = total_act_S1 + 1; //iterate total actuation counter by 1
    }
  }

  delay(0250);

  //6.0 s -------------------------------------------------------------------------------------------------

  //switch 2 counter -----------------------------------------------------------------------
  sw2_state = digitalRead(sw2); //read switch 2 state
  
  //if switch is functional, increment counter
  if (sw2_state == HIGH) {
    S2_num = S2_num + 1; //increment counter by 1
    sw2_fail = 0; //reset switch failure counter to 0 since not consecutive failures
  }
  
  //if switch is non-functional, incremenet failure counter
  else if (sw2_state == LOW) {
    sw2_fail = sw2_fail + 1; //incremement consecutive failure counter by 1
  }
  //----------------------------------------------------------------------------------------
  digitalWrite(S2, LOW); //end of long press 1 for S2
  sw2_state = 0; //sw2

  //switch 3 counter -----------------------------------------------------------------------
  sw3_state = digitalRead(sw3); //read switch 3 state

  //if switch is still reading "On" without actuation force, increment switch stuck counter
  if (sw3_state == HIGH) {
    sw3_stuck = sw3_stuck + 1; //increment counter by 1
  }

  //if switch is reading "Off" without actuation force, reset switch stuck counter to 0
  if (sw3_state == LOW) {
    sw3_stuck = 0; //reset counter to 0 as the switch is not stuck
  }
  //----------------------------------------------------------------------------------------

  //short press 1 for switch 3
  if (sw3_fail < fail_count && sw3_stuck < stuck_count) {
    if (total_act_S3 < life) {
      digitalWrite(S3, HIGH); //short press 1 for S3
      total_act_S3 = total_act_S3 + 1; //iterate total actuation counter by 1
    }
  }

  //switch 4 counter -----------------------------------------------------------------------
  sw4_state = digitalRead(sw4); //read switch 4 state

  //if switch is functional, increment counter
  if (sw4_state == HIGH) {
    S4_num = S4_num + 1; //increment counter by 1
    sw4_fail = 0; //reset switch failure counter to 0 since not consecutive failures
  }

  //if switch is non-functional, increment failure counter
  else if (sw4_state == LOW) {
    sw4_fail = sw4_fail + 1; //increment consecutive failure counter by 1
  } 
  //----------------------------------------------------------------------------------------
  digitalWrite(S4, LOW); //end of long press 1 for S4
  sw4_state = 0; //sw4

  delay(0250);

  //6.25 s ------------------------------------------------------------------------------------------------

  //switch 1 counter -----------------------------------------------------------------------
  sw1_state = digitalRead(sw1); //read switch 1 state

  //if switch is functional, increment counter
  if (sw1_state == HIGH) {
    S1_num = S1_num + 1; //increment counter by 1
    sw1_fail = 0; //reset switch failure counter to 0 since not consecutive failures
  }

  //if switch is non-functional, increment failure counter
  else if (sw1_state == LOW) {
    sw1_fail = sw1_fail + 1; //increment consecutive failure counter by 1
  }
  //----------------------------------------------------------------------------------------
  digitalWrite(S1, LOW); //end of long press 1 for S1
  sw1_state = 0; //sw1

  //switch 2 counter -----------------------------------------------------------------------
  sw2_state = digitalRead(sw2); //read switch 2 state

  //if switch is still reading "On" without actuation force, increment switch stuck counter
  if (sw2_state == HIGH) {
    sw2_stuck = sw2_stuck + 1; //increment counter by 1
  }

  //if switch is reading "Off" without actuation force, reset switch stuck counter to 0
  if (sw2_state == LOW) {
    sw2_stuck = 0; //reset counter to 0 as the switch is not stuck
  }
  //----------------------------------------------------------------------------------------

  //short press 2 for switch 2
  if (sw2_fail < fail_count && sw2_stuck < stuck_count) {
    if (total_act_S2 < life) {
      digitalWrite(S2, HIGH); //short press 2 for S2
      total_act_S2 = total_act_S2 + 1; //iterate total actuation counter by 1
    }
  }

  delay(0100);

  //6.5 s -------------------------------------------------------------------------------------------------

  //print the test iteration counters after cycle, before wait time
  if ((sw1_fail < fail_count) || (sw2_fail < fail_count) || (sw3_fail < fail_count) || (sw4_fail < fail_count)) {
  //print to PuTTY serial monitor
  //column headers
    Serial.print("Solenoid_1_Actuations,Switch_1_Actuations,Solenoid_2_Actuations,Switch_2_Actuations,");
    Serial.print("Solenoid_3_Actuations,Switch_3_Actuations,Solenoid_4_Actuations,Switch_4_Actuations");
    Serial.print('\n'); //next line for data
  //solenoid and switch actuation counts  
    Serial.print(total_act_S1); Serial.print(","); Serial.print(S1_num); Serial.print(","); //S1 and sw1 data
    Serial.print(total_act_S2); Serial.print(","); Serial.print(S2_num); Serial.print(","); //S2 and sw2 data
    Serial.print(total_act_S3); Serial.print(","); Serial.print(S3_num); Serial.print(","); //S3 and sw3 data
    Serial.print(total_act_S4); Serial.print(","); Serial.print(S4_num); //S4 and sw4 data
    Serial.print('\n'); //next line for iterative data
  }

  //update EEPROM variables in case user wants to continue test after stopping
  EEPROM.write(0, total_act_S1); EEPROM.write(1, total_act_S2);
  EEPROM.write(2, total_act_S3); EEPROM.write(3, total_act_S4);
  EEPROM.write(4, S1_num); EEPROM.write(5, S2_num);
  EEPROM.write(6, S3_num); EEPROM.write(7, S4_num);
  
}


void loop() {
//0 s ---------------------------------------------------------------------------------------------------

//switch 1 counter -----------------------------------------------------------------------
sw1_state = digitalRead(sw1); //read switch 1 state

//if switch is still reading "On" without actuation force, increment switch stuck counter
if (sw1_state == HIGH) {
  sw1_stuck = sw1_stuck + 1; //increment counter by 1
}

//if switch is reading "Off" without actuation force, reset switch stuck counter to 0
if (sw1_state == LOW) {
  sw1_stuck = 0; //reset counter to 0 as the switch is not stuck
}
//----------------------------------------------------------------------------------------

//long press 1 for switch 1
if (sw1_fail < fail_count && sw1_stuck < stuck_count) {
  if (total_act_S1 < life) {
    digitalWrite(S1, HIGH); //long press 1 for S1
    total_act_S1 = total_act_S1 + 1; //iterate total actuation counter by 1
  }
}

//switch 3 counter -----------------------------------------------------------------------
sw3_state = digitalRead(sw3); //read switch 3 state

//if switch is functional, increment counter
if (sw3_state == HIGH) {
  S3_num = S3_num + 1; //increment counter by 1
  sw3_fail = 0; //reset switch failure counter to 0 since not consecutive failures
}

//if switch is non-functional, increment failure counter
else if (sw3_state == LOW) {
  sw3_fail = sw3_fail + 1; //incremement consecutive failure counter by 1
}
//----------------------------------------------------------------------------------------
digitalWrite(S3, LOW); //end of long press 1 for S3
sw3_state = 0; //sw3

//switch 4 counter -----------------------------------------------------------------------
sw4_state = digitalRead(sw4); //read switch 4 state

//if switch is still reading "On" without actuation force, increment switch stuck counter
if (sw4_state == HIGH) {
  sw4_stuck = sw4_stuck + 1; //increment counter by 1
}

//if switch is reading "Off" without actuation force, reset switch stuck counter to 0
if (sw4_state == LOW) {
  sw4_stuck = 0; //reset counter to 0 as the switch is not stuck
}
//----------------------------------------------------------------------------------------

//short press 1 for switch 4
if (sw4_fail < fail_count && sw4_stuck < stuck_count) {
  if (total_act_S4 < life) {
    digitalWrite(S4, HIGH); //long press 1 for S4
    total_act_S4 = total_act_S4 + 1; //iterate total actuation counter by 1
  }
}

delay(0250);

//0.25 s ------------------------------------------------------------------------------------------------

//switch 2 counter -----------------------------------------------------------------------
sw2_state = digitalRead(sw2); //read switch 2 state
  
//if switch is functional, increment counter
if (sw2_state == HIGH) {
  S2_num = S2_num + 1; //increment counter by 1
  sw2_fail = 0; //reset switch failure counter to 0 since not consecutive failures
}
  
//if switch is non-functional, incremenet failure counter
else if (sw2_state == LOW) {
  sw2_fail = sw2_fail + 1; //incremement consecutive failure counter by 1
}
//----------------------------------------------------------------------------------------
digitalWrite(S2, LOW); //end of long press 1 for S2
sw2_state = 0; //sw2

//switch 3 counter -----------------------------------------------------------------------
sw3_state = digitalRead(sw3); //read switch 3 state

//if switch is still reading "On" without actuation force, increment switch stuck counter
if (sw3_state == HIGH) {
  sw3_stuck = sw3_stuck + 1; //increment counter by 1
}

//if switch is reading "Off" without actuation force, reset switch stuck counter to 0
if (sw3_state == LOW) {
  sw3_stuck = 0; //reset counter to 0 as the switch is not stuck
}
//----------------------------------------------------------------------------------------

//short press 2 for switch 3
if (sw3_fail < fail_count && sw3_stuck < stuck_count) {
  if (total_act_S3 < life) {
    digitalWrite(S3, HIGH); //long press 1 for S3
    total_act_S3 = total_act_S3 + 1; //iterate total actuation counter by 1
  }
}

delay(0250);

//0.5 s -------------------------------------------------------------------------------------------------

//switch 2 counter -----------------------------------------------------------------------
sw2_state = digitalRead(sw2); //read switch 2 state

//if switch is still reading "On" without actuation force, increment switch stuck counter
if (sw2_state == HIGH) {
  sw2_stuck = sw2_stuck + 1; //increment counter by 1
}

//if switch is reading "Off" without actuation force, reset switch stuck counter to 0
if (sw2_state == LOW) {
  sw2_stuck = 0; //reset counter to 0 as the switch is not stuck
}
//----------------------------------------------------------------------------------------

//long press 1 for switch 2
if (sw2_fail < fail_count && sw2_stuck < stuck_count) {
  if (total_act_S2 < life) {
    digitalWrite(S2, HIGH); //long press 1 for S2
    total_act_S2 = total_act_S2 + 1; //iterate total actuation counter by 1
  }
}

//switch 4 counter -----------------------------------------------------------------------
sw4_state = digitalRead(sw4); //read switch 4 state

//if switch is functional, increment counter
if (sw4_state == HIGH) {
  S4_num = S4_num + 1; //increment counter by 1
  sw4_fail = 0; //reset switch failure counter to 0 since not consecutive failures
}

//if switch is non-functional, increment failure counter
else if (sw4_state == LOW) {
  sw4_fail = sw4_fail + 1; //increment consecutive failure counter by 1
} 
//----------------------------------------------------------------------------------------
digitalWrite(S4, LOW); //end of long press 1 for S4
sw4_state = 0; //sw4

delay(0250);

//0.75 s ------------------------------------------------------------------------------------------------

//switch 3 counter -----------------------------------------------------------------------
sw3_state = digitalRead(sw3); //read switch 3 state

//if switch is functional, increment counter
if (sw3_state == HIGH) {
  S3_num = S3_num + 1; //increment counter by 1
  sw3_fail = 0; //reset switch failure counter to 0 since not consecutive failures
}

//if switch is non-functional, increment failure counter
else if (sw3_state == LOW) {
  sw3_fail = sw3_fail + 1; //incremement consecutive failure counter by 1
}
//----------------------------------------------------------------------------------------
digitalWrite(S3, LOW); //end of long press 1 for S3
sw3_state = 0; //sw3

//switch 4 counter -----------------------------------------------------------------------
sw4_state = digitalRead(sw4); //read switch 4 state

//if switch is still reading "On" without actuation force, increment switch stuck counter
if (sw4_state == HIGH) {
  sw4_stuck = sw4_stuck + 1; //increment counter by 1
}

//if switch is reading "Off" without actuation force, reset switch stuck counter to 0
if (sw4_state == LOW) {
  sw4_stuck = 0; //reset counter to 0 as the switch is not stuck
}
//----------------------------------------------------------------------------------------

//short press 2 for switch 4
if (sw4_fail < fail_count && sw4_stuck < stuck_count) {
  if (total_act_S4 < life) {
    digitalWrite(S4, HIGH); //long press 1 for S4
    total_act_S4 = total_act_S4 + 1; //iterate total actuation counter by 1
  }
}

delay(0250);

//1.0 s -------------------------------------------------------------------------------------------------

//switch 3 counter -----------------------------------------------------------------------
sw3_state = digitalRead(sw3); //read switch 3 state

//if switch is still reading "On" without actuation force, increment switch stuck counter
if (sw3_state == HIGH) {
  sw3_stuck = sw3_stuck + 1; //increment counter by 1
}

//if switch is reading "Off" without actuation force, reset switch stuck counter to 0
if (sw3_state == LOW) {
  sw3_stuck = 0; //reset counter to 0 as the switch is not stuck
}
//----------------------------------------------------------------------------------------

//long press 1 for switch 3
if (sw3_fail < fail_count && sw3_stuck < stuck_count) {
  if (total_act_S3 < life) {
    digitalWrite(S3, HIGH); //long press 1 for S3
    total_act_S3 = total_act_S3 + 1; //iterate total actuation counter by 1
  }
}

delay(0250);

//1.25 s ------------------------------------------------------------------------------------------------

//switch 4 counter -----------------------------------------------------------------------
sw4_state = digitalRead(sw4); //read switch 4 state

//if switch is functional, increment counter
if (sw4_state == HIGH) {
  S4_num = S4_num + 1; //increment counter by 1
  sw4_fail = 0; //reset switch failure counter to 0 since not consecutive failures
}

//if switch is non-functional, increment failure counter
else if (sw4_state == LOW) {
  sw4_fail = sw4_fail + 1; //increment consecutive failure counter by 1
} 
//----------------------------------------------------------------------------------------
digitalWrite(S4, LOW); //end of long press 1 for S4
sw4_state = 0; //sw4

delay(0250);

//1.5 s -------------------------------------------------------------------------------------------------

//switch 4 counter -----------------------------------------------------------------------
sw4_state = digitalRead(sw4); //read switch 4 state

//if switch is still reading "On" without actuation force, increment switch stuck counter
if (sw4_state == HIGH) {
  sw4_stuck = sw4_stuck + 1; //increment counter by 1
}

//if switch is reading "Off" without actuation force, reset switch stuck counter to 0
if (sw4_state == LOW) {
  sw4_stuck = 0; //reset counter to 0 as the switch is not stuck
}
//----------------------------------------------------------------------------------------

//long press 1 for switch 4
if (sw4_fail < fail_count && sw4_stuck < stuck_count) {
  if (total_act_S4 < life) {
    digitalWrite(S4, HIGH); //long press 1 for S4
    total_act_S4 = total_act_S4 + 1; //iterate total actuation counter by 1
  }
}

delay(0250);

//1.75 s ------------------------------------------------------------------------------------------------
delay(0250);

//2.0 s -------------------------------------------------------------------------------------------------

//switch 1 counter -----------------------------------------------------------------------
sw1_state = digitalRead(sw1); //read switch 1 state

//if switch is functional, increment counter
if (sw1_state == HIGH) {
  S1_num = S1_num + 1; //increment counter by 1
  sw1_fail = 0; //reset switch failure counter to 0 since not consecutive failures
}

//if switch is non-functional, increment failure counter
else if (sw1_state == LOW) {
  sw1_fail = sw1_fail + 1; //increment consecutive failure counter by 1
}
//----------------------------------------------------------------------------------------
digitalWrite(S1, LOW); //end of long press 1 for S1
sw1_state = 0; //sw1

delay(0250);

//2.25 s ------------------------------------------------------------------------------------------------
delay(0250);

//2.5 s -------------------------------------------------------------------------------------------------

//switch 1 counter -----------------------------------------------------------------------
sw1_state = digitalRead(sw1); //read switch 1 state

//if switch is still reading "On" without actuation force, increment switch stuck counter
if (sw1_state == HIGH) {
  sw1_stuck = sw1_stuck + 1; //increment counter by 1
}

//if switch is reading "Off" without actuation force, reset switch stuck counter to 0
if (sw1_state == LOW) {
  sw1_stuck = 0; //reset counter to 0 as the switch is not stuck
}
//----------------------------------------------------------------------------------------

//long press 2 for switch 1
if (sw1_fail < fail_count && sw1_stuck < stuck_count) {
  if (total_act_S1 < life) {
    digitalWrite(S1, HIGH); //long press 1 for S1
    total_act_S1 = total_act_S1 + 1; //iterate total actuation counter by 1
  }
}

//switch 2 counter -----------------------------------------------------------------------
sw2_state = digitalRead(sw2); //read switch 2 state
  
//if switch is functional, increment counter
if (sw2_state == HIGH) {
  S2_num = S2_num + 1; //increment counter by 1
  sw2_fail = 0; //reset switch failure counter to 0 since not consecutive failures
}
  
//if switch is non-functional, incremenet failure counter
else if (sw2_state == LOW) {
  sw2_fail = sw2_fail + 1; //incremement consecutive failure counter by 1
}
//----------------------------------------------------------------------------------------
digitalWrite(S2, LOW); //end of long press 1 for S2
sw2_state = 0; //sw2

delay(0250);

//2.75 s ------------------------------------------------------------------------------------------------
delay(0250);

//3.0 s -------------------------------------------------------------------------------------------------

//switch 2 counter -----------------------------------------------------------------------
sw2_state = digitalRead(sw2); //read switch 2 state

//if switch is still reading "On" without actuation force, increment switch stuck counter
if (sw2_state == HIGH) {
  sw2_stuck = sw2_stuck + 1; //increment counter by 1
}

//if switch is reading "Off" without actuation force, reset switch stuck counter to 0
if (sw2_state == LOW) {
  sw2_stuck = 0; //reset counter to 0 as the switch is not stuck
}
//----------------------------------------------------------------------------------------

//long press 2 for switch 2
if (sw2_fail < fail_count && sw2_stuck < stuck_count) {
  if (total_act_S2 < life) {
    digitalWrite(S2, HIGH); //long press 1 for S2
    total_act_S2 = total_act_S2 + 1; //iterate total actuation counter by 1
  }
}

//switch 3 counter -----------------------------------------------------------------------
sw3_state = digitalRead(sw3); //read switch 3 state

//if switch is functional, increment counter
if (sw3_state == HIGH) {
  S3_num = S3_num + 1; //increment counter by 1
  sw3_fail = 0; //reset switch failure counter to 0 since not consecutive failures
}

//if switch is non-functional, increment failure counter
else if (sw3_state == LOW) {
  sw3_fail = sw3_fail + 1; //incremement consecutive failure counter by 1
}
//----------------------------------------------------------------------------------------
digitalWrite(S3, LOW); //end of long press 1 for S3
sw3_state = 0; //sw3

delay(0250);

//3.25 s ------------------------------------------------------------------------------------------------
delay(0250);

//3.5 s -------------------------------------------------------------------------------------------------

//switch 3 counter -----------------------------------------------------------------------
sw3_state = digitalRead(sw3); //read switch 3 state

//if switch is still reading "On" without actuation force, increment switch stuck counter
if (sw3_state == HIGH) {
  sw3_stuck = sw3_stuck + 1; //increment counter by 1
}

//if switch is reading "Off" without actuation force, reset switch stuck counter to 0
if (sw3_state == LOW) {
  sw3_stuck = 0; //reset counter to 0 as the switch is not stuck
}
//----------------------------------------------------------------------------------------

//long press 2 for switch 3
if (sw3_fail < fail_count && sw3_stuck < stuck_count) {
  if (total_act_S3 < life) {
    digitalWrite(S3, HIGH); //long press 1 for S3
    total_act_S3 = total_act_S3 + 1; //iterate total actuation counter by 1
  }
}

//switch 4 counter -----------------------------------------------------------------------
sw4_state = digitalRead(sw4); //read switch 4 state

//if switch is functional, increment counter
if (sw4_state == HIGH) {
  S4_num = S4_num + 1; //increment counter by 1
  sw4_fail = 0; //reset switch failure counter to 0 since not consecutive failures
}

//if switch is non-functional, increment failure counter
else if (sw4_state == LOW) {
  sw4_fail = sw4_fail + 1; //increment consecutive failure counter by 1
} 
//----------------------------------------------------------------------------------------
digitalWrite(S4, LOW); //end of long press 1 for S4
sw4_state = 0; //sw4

delay(0250);

//3.75 s ------------------------------------------------------------------------------------------------
delay(0250);

//4.0 s -------------------------------------------------------------------------------------------------

//switch 4 counter -----------------------------------------------------------------------
sw4_state = digitalRead(sw4); //read switch 4 state

//if switch is still reading "On" without actuation force, increment switch stuck counter
if (sw4_state == HIGH) {
  sw4_stuck = sw4_stuck + 1; //increment counter by 1
}

//if switch is reading "Off" without actuation force, reset switch stuck counter to 0
if (sw4_state == LOW) {
  sw4_stuck = 0; //reset counter to 0 as the switch is not stuck
}
//----------------------------------------------------------------------------------------

//long press 2 for switch 4
if (sw4_fail < fail_count && sw4_stuck < stuck_count) {
  if (total_act_S4 < life) {
    digitalWrite(S4, HIGH); //long press 1 for S4
    total_act_S4 = total_act_S4 + 1; //iterate total actuation counter by 1
  }
}

delay(0250);

//4.25 s ------------------------------------------------------------------------------------------------
delay(0250);

//4.5 s -------------------------------------------------------------------------------------------------

//switch 1 counter -----------------------------------------------------------------------
sw1_state = digitalRead(sw1); //read switch 1 state

//if switch is functional, increment counter
if (sw1_state == HIGH) {
  S1_num = S1_num + 1; //increment counter by 1
  sw1_fail = 0; //reset switch failure counter to 0 since not consecutive failures
}

//if switch is non-functional, increment failure counter
else if (sw1_state == LOW) {
  sw1_fail = sw1_fail + 1; //increment consecutive failure counter by 1
}
//----------------------------------------------------------------------------------------
digitalWrite(S1, LOW); //end of long press 1 for S1
sw1_state = 0; //sw1

delay(0250);

//4.75 s ------------------------------------------------------------------------------------------------
delay(0250);

//5.0 s -------------------------------------------------------------------------------------------------

//switch 1 counter -----------------------------------------------------------------------
sw1_state = digitalRead(sw1); //read switch 1 state

//if switch is still reading "On" without actuation force, increment switch stuck counter
if (sw1_state == HIGH) {
  sw1_stuck = sw1_stuck + 1; //increment counter by 1
}

//if switch is reading "Off" without actuation force, reset switch stuck counter to 0
if (sw1_state == LOW) {
  sw1_stuck = 0; //reset counter to 0 as the switch is not stuck
}
//----------------------------------------------------------------------------------------

//short press 1 for switch 1
if (sw1_fail < fail_count && sw1_stuck < stuck_count) {
  if (total_act_S1 < life) {
    digitalWrite(S1, HIGH); //long press 1 for S1
    total_act_S1 = total_act_S1 + 1; //iterate total actuation counter by 1
  }
}

//switch 2 counter -----------------------------------------------------------------------
sw2_state = digitalRead(sw2); //read switch 2 state
  
//if switch is functional, increment counter
if (sw2_state == HIGH) {
  S2_num = S2_num + 1; //increment counter by 1
  sw2_fail = 0; //reset switch failure counter to 0 since not consecutive failures
}
  
//if switch is non-functional, incremenet failure counter
else if (sw2_state == LOW) {
  sw2_fail = sw2_fail + 1; //incremement consecutive failure counter by 1
}
//----------------------------------------------------------------------------------------
digitalWrite(S2, LOW); //end of long press 1 for S2
sw2_state = 0; //sw2

delay(0250);

//5.25 s ------------------------------------------------------------------------------------------------
delay(0250);

//5.5 s -------------------------------------------------------------------------------------------------

//switch 1 counter -----------------------------------------------------------------------
sw1_state = digitalRead(sw1); //read switch 1 state

//if switch is functional, increment counter
if (sw1_state == HIGH) {
  S1_num = S1_num + 1; //increment counter by 1
  sw1_fail = 0; //reset switch failure counter to 0 since not consecutive failures
}

//if switch is non-functional, increment failure counter
else if (sw1_state == LOW) {
  sw1_fail = sw1_fail + 1; //increment consecutive failure counter by 1
}
//----------------------------------------------------------------------------------------
digitalWrite(S1, LOW); //end of long press 1 for S1
sw1_state = 0; //sw1

//switch 2 counter -----------------------------------------------------------------------
sw2_state = digitalRead(sw2); //read switch 2 state

//if switch is still reading "On" without actuation force, increment switch stuck counter
if (sw2_state == HIGH) {
  sw2_stuck = sw2_stuck + 1; //increment counter by 1
}

//if switch is reading "Off" without actuation force, reset switch stuck counter to 0
if (sw2_state == LOW) {
  sw2_stuck = 0; //reset counter to 0 as the switch is not stuck
}
//----------------------------------------------------------------------------------------

//short press 1 for switch 2
if (sw2_fail < fail_count && sw2_stuck < stuck_count) {
  if (total_act_S2 < life) {
    digitalWrite(S2, HIGH); //long press 1 for S2
    total_act_S2 = total_act_S2 + 1; //iterate total actuation counter by 1
  }
}

//switch 3 counter -----------------------------------------------------------------------
sw3_state = digitalRead(sw3); //read switch 3 state

//if switch is functional, increment counter
if (sw3_state == HIGH) {
  S3_num = S3_num + 1; //increment counter by 1
  sw3_fail = 0; //reset switch failure counter to 0 since not consecutive failures
}

//if switch is non-functional, increment failure counter
else if (sw3_state == LOW) {
  sw3_fail = sw3_fail + 1; //incremement consecutive failure counter by 1
}
//----------------------------------------------------------------------------------------
digitalWrite(S3, LOW); //end of long press 1 for S3
sw3_state = 0; //sw3

delay(0250);

//5.75 s ------------------------------------------------------------------------------------------------

//switch 1 counter -----------------------------------------------------------------------
sw1_state = digitalRead(sw1); //read switch 1 state

//if switch is still reading "On" without actuation force, increment switch stuck counter
if (sw1_state == HIGH) {
  sw1_stuck = sw1_stuck + 1; //increment counter by 1
}

//if switch is reading "Off" without actuation force, reset switch stuck counter to 0
if (sw1_state == LOW) {
  sw1_stuck = 0; //reset counter to 0 as the switch is not stuck
}
//----------------------------------------------------------------------------------------

//short press 2 for switch 1
if (sw1_fail < fail_count && sw1_stuck < stuck_count) {
  if (total_act_S1 < life) {
    digitalWrite(S1, HIGH); //long press 1 for S1
    total_act_S1 = total_act_S1 + 1; //iterate total actuation counter by 1
  }
}

delay(0250);

//6.0 s -------------------------------------------------------------------------------------------------

//switch 2 counter -----------------------------------------------------------------------
sw2_state = digitalRead(sw2); //read switch 2 state
  
//if switch is functional, increment counter
if (sw2_state == HIGH) {
  S2_num = S2_num + 1; //increment counter by 1
  sw2_fail = 0; //reset switch failure counter to 0 since not consecutive failures
}
  
//if switch is non-functional, incremenet failure counter
else if (sw2_state == LOW) {
  sw2_fail = sw2_fail + 1; //incremement consecutive failure counter by 1
}
//----------------------------------------------------------------------------------------
digitalWrite(S2, LOW); //end of long press 1 for S2
sw2_state = 0; //sw2

//switch 3 counter -----------------------------------------------------------------------
sw3_state = digitalRead(sw3); //read switch 3 state

//if switch is still reading "On" without actuation force, increment switch stuck counter
if (sw3_state == HIGH) {
  sw3_stuck = sw3_stuck + 1; //increment counter by 1
}

//if switch is reading "Off" without actuation force, reset switch stuck counter to 0
if (sw3_state == LOW) {
  sw3_stuck = 0; //reset counter to 0 as the switch is not stuck
}
//----------------------------------------------------------------------------------------

//short press 1 for switch 3
if (sw3_fail < fail_count && sw3_stuck < stuck_count) {
  if (total_act_S3 < life) {
    digitalWrite(S3, HIGH); //long press 1 for S3
    total_act_S3 = total_act_S3 + 1; //iterate total actuation counter by 1
  }
}

//switch 4 counter -----------------------------------------------------------------------
sw4_state = digitalRead(sw4); //read switch 4 state

//if switch is functional, increment counter
if (sw4_state == HIGH) {
  S4_num = S4_num + 1; //increment counter by 1
  sw4_fail = 0; //reset switch failure counter to 0 since not consecutive failures
}

//if switch is non-functional, increment failure counter
else if (sw4_state == LOW) {
  sw4_fail = sw4_fail + 1; //increment consecutive failure counter by 1
} 
//----------------------------------------------------------------------------------------
digitalWrite(S4, LOW); //end of long press 1 for S4
sw4_state = 0; //sw4

delay(0250);

//6.25 s ------------------------------------------------------------------------------------------------

//switch 1 counter -----------------------------------------------------------------------
sw1_state = digitalRead(sw1); //read switch 1 state

//if switch is functional, increment counter
if (sw1_state == HIGH) {
  S1_num = S1_num + 1; //increment counter by 1
  sw1_fail = 0; //reset switch failure counter to 0 since not consecutive failures
}

//if switch is non-functional, increment failure counter
else if (sw1_state == LOW) {
  sw1_fail = sw1_fail + 1; //increment consecutive failure counter by 1
}
//----------------------------------------------------------------------------------------
digitalWrite(S1, LOW); //end of long press 1 for S1
sw1_state = 0; //sw1

//switch 2 counter -----------------------------------------------------------------------
sw2_state = digitalRead(sw2); //read switch 2 state

//if switch is still reading "On" without actuation force, increment switch stuck counter
if (sw2_state == HIGH) {
  sw2_stuck = sw2_stuck + 1; //increment counter by 1
}

//if switch is reading "Off" without actuation force, reset switch stuck counter to 0
if (sw2_state == LOW) {
  sw2_stuck = 0; //reset counter to 0 as the switch is not stuck
}
//----------------------------------------------------------------------------------------

//short press 2 for switch 2
if (sw2_fail < fail_count && sw2_stuck < stuck_count) {
  if (total_act_S2 < life) {
    digitalWrite(S2, HIGH); //long press 1 for S2
    total_act_S2 = total_act_S2 + 1; //iterate total actuation counter by 1
  }
}

delay(0250);

//6.5 s -------------------------------------------------------------------------------------------------

//print the test iteration counters after cycle, before wait time
if ((sw1_fail < fail_count) || (sw2_fail < fail_count) || (sw3_fail < fail_count) || (sw4_fail < fail_count)) {
//print to PuTTY serial monitor
//S1 and sw1 data
  Serial.print(total_act_S1); Serial.print(","); Serial.print(S1_num); 
  if (sw1_fail >= fail_count) {
    Serial.print("F"); //display F to represent the switch failing
  }
  if (sw1_stuck >= stuck_count) {
    Serial.print("S"); //display S to represent the switch sticking
  }  
  Serial.print(","); 
//S2 and sw2 data
  Serial.print(total_act_S2); Serial.print(","); Serial.print(S2_num);
  if (sw2_fail >= fail_count) {
    Serial.print("F"); //display F to represent the switch failing
  }
  if (sw2_stuck >= stuck_count) {
    Serial.print("S"); //display S to represent the switch sticking
  }  
  Serial.print(",");
//S3 and sw3 data
  Serial.print(total_act_S3); Serial.print(","); Serial.print(S3_num);
  if (sw3_fail >= fail_count) {
    Serial.print("F"); //display F to represent the switch failing
  }
  if (sw3_stuck >= stuck_count) {
    Serial.print("S"); //display S to represent the switch sticking
  }  
  Serial.print(",");
//S4 and sw4 data
  Serial.print(total_act_S4); Serial.print(","); Serial.print(S4_num);
  if (sw3_fail >= fail_count) {
    Serial.print("F"); //display F to represent the switch failing
  }
  if (sw4_stuck >= stuck_count) {
    Serial.print("S"); //display S to represent the switch sticking
  }  
  Serial.print('\n'); //next line for iterative data
}

//update EEPROM variables in case user wants to continue test after stopping
EEPROM.write(0, total_act_S1); EEPROM.write(1, total_act_S2);
EEPROM.write(2, total_act_S3); EEPROM.write(3, total_act_S4);
EEPROM.write(4, S1_num); EEPROM.write(5, S2_num);
EEPROM.write(6, S3_num); EEPROM.write(7, S4_num);

}
