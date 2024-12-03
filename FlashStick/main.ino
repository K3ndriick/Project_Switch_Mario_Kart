// Commented out the homeButton functionality, the joystick works as intended with this version of code. 
#include "LUFAConfig.h"
#include <LUFA.h>
#include "Joystick.h"
#define BOUNCE_WITH_PROMPT_DETECTION
#include <Bounce2.h>

#define MILLIDEBOUNCE 1 // Debounce time in milliseconds
#define pinOBLED 21  // Onboard LED pin

bool buttonStartBefore;
bool buttonSelectBefore;
byte buttonStatus[14]; // Reduced to 14 since BUTTONSELECT is removed


/* 
The concept to understand the Bit Mask: 
What we trying to achive is to detect the discrypency of the button press through Bit Mask bitwise operation where in the  
buttonProcessing() function we are checking the buttonStatus[] array and then setting the ReportData.Button with the Bit Mask value 

Given we do the input pull up intially 

For the Bit Mask Concept here is the official Arduino Doc that explains with example 
https://docs.arduino.cc/learn/programming/bit-mask/ 
*/ 
#define DPAD_UP_MASK_ON 0x00
#define DPAD_UPRIGHT_MASK_ON 0x01
#define DPAD_RIGHT_MASK_ON 0x02
#define DPAD_DOWNRIGHT_MASK_ON 0x03
#define DPAD_DOWN_MASK_ON 0x04
#define DPAD_DOWNLEFT_MASK_ON 0x05
#define DPAD_LEFT_MASK_ON 0x06
#define DPAD_UPLEFT_MASK_ON 0x07
#define DPAD_NOTHING_MASK_ON 0x08
#define A_MASK_ON 0x04
#define B_MASK_ON 0x02
#define X_MASK_ON 0x08
#define Y_MASK_ON 0x01
#define LB_MASK_ON 0x10
#define RB_MASK_ON 0x20
#define ZL_MASK_ON 0x40
#define ZR_MASK_ON 0x80
#define START_MASK_ON 0x200
#define SELECT_MASK_ON 0x100
#define HOME_MASK_ON 0x1000

#define BUTTONUP 15
#define BUTTONDOWN 1
#define BUTTONLEFT 2
#define BUTTONRIGHT 3
#define BUTTONA 4
#define BUTTONB 5
#define BUTTONX 6
#define BUTTONY 15
// #define BUTTONLB 8
// #define BUTTONRB 9
#define BUTTONLT 7
#define BUTTONRT 8
#define BUTTONSTART 10
#define BUTTONHOME 9 // Moved BUTTONHOME to 13

int xAxis = 512; // Default centered value for X
int yAxis = 512; // Default centered value for Y


/* 
Please refer to the documentation of the Bounce2 Lib 
https://github.com/thomasfredericks/Bounce2?tab=readme-ov-file#bounce

From my understanding when we instantiate the Bounce class, we can perform actions such as input pull up in this case. 
Where the we chained it with the board pin number amd the input pull up. Which is use for the button press detection 
*/ 

Bounce joystickUP = Bounce();
Bounce joystickDOWN = Bounce();
Bounce joystickLEFT = Bounce();
Bounce joystickRIGHT = Bounce();
Bounce buttonA = Bounce();
Bounce buttonB = Bounce();
Bounce buttonX = Bounce();
Bounce buttonY = Bounce();
Bounce buttonLB = Bounce();
Bounce buttonRB = Bounce();
Bounce buttonLT = Bounce();
Bounce buttonRT = Bounce();
Bounce buttonSTART = Bounce();
Bounce buttonHOME = Bounce();

// Not using pin 16 
void setupPins() {
  joystickUP.attach(15, INPUT_PULLUP);
  joystickDOWN.attach(1, INPUT_PULLUP);
  joystickLEFT.attach(2, INPUT_PULLUP);
  joystickRIGHT.attach(3, INPUT_PULLUP);
  buttonA.attach(4, INPUT_PULLUP);
  buttonB.attach(5, INPUT_PULLUP);
  buttonX.attach(6, INPUT_PULLUP);
  // buttonY.attach(7, INPUT_PULLUP);
  // buttonLB.attach(8, INPUT_PULLUP);
  // buttonRB.attach(9, INPUT_PULLUP);
  buttonLT.attach(7, INPUT_PULLUP);
  buttonRT.attach(8, INPUT_PULLUP);
  buttonSTART.attach(10, INPUT_PULLUP);
  buttonHOME.attach(9, INPUT_PULLUP); // Change buttonHOME to pin 18

  joystickUP.interval(MILLIDEBOUNCE);
  joystickDOWN.interval(MILLIDEBOUNCE);
  joystickLEFT.interval(MILLIDEBOUNCE);
  joystickRIGHT.interval(MILLIDEBOUNCE);
  buttonA.interval(MILLIDEBOUNCE);
  buttonB.interval(MILLIDEBOUNCE);
  buttonX.interval(MILLIDEBOUNCE);
  // buttonY.interval(MILLIDEBOUNCE);
  buttonLB.interval(MILLIDEBOUNCE);
  buttonRB.interval(MILLIDEBOUNCE);
  buttonLT.interval(MILLIDEBOUNCE);
  buttonRT.interval(MILLIDEBOUNCE);
  buttonSTART.interval(MILLIDEBOUNCE);
  buttonHOME.interval(MILLIDEBOUNCE);

  pinMode(pinOBLED, OUTPUT);
  // Set the LED to low to make sure it is off
  digitalWrite(pinOBLED, HIGH);
}

void setup() {
  buttonStartBefore = false;
  buttonSelectBefore = false;
  setupPins();
  SetupHardware();
  GlobalInterruptEnable();
}

typedef enum {
  ANALOG_MODE,
  DIGITAL,
  SMASH
} State_t;
State_t state = DIGITAL;

void checkModeChange() {
  if (buttonStatus[BUTTONSTART] /*&& buttonStatus[BUTTONHOME]*/) {
    if (buttonStartBefore == 0 && buttonSelectBefore == 0) {
      switch (state) {
        case DIGITAL:
          state = ANALOG_MODE;
          break;
        case ANALOG_MODE:
          state = SMASH;
          break;
        case SMASH:
          state = DIGITAL;
          break;
      }
      buttonStartBefore = 1;
      buttonSelectBefore = 1;
    }
  } else {
    buttonSelectBefore = 0;
    buttonStartBefore = 0;
  }
}


void loop() {
  // Read the analog values from A0 and A1
  xAxis = analogRead(A0);
  yAxis = analogRead(A1);
  
  yAxis = 1023 - yAxis; 
   
  buttonRead();
  checkModeChange();
  processButtons();
  HID_Task();
  // We also need to run the main USB management task.
  USB_USBTask();
}

void buttonRead() {
  if (joystickUP.update()) {buttonStatus[BUTTONUP] = joystickUP.fell();}
  if (joystickDOWN.update()) {buttonStatus[BUTTONDOWN] = joystickDOWN.fell();}
  if (joystickLEFT.update()) {buttonStatus[BUTTONLEFT] = joystickLEFT.fell();}
  if (joystickRIGHT.update()) {buttonStatus[BUTTONRIGHT] = joystickRIGHT.fell();}
  if (buttonA.update()) {buttonStatus[BUTTONA] = buttonA.fell();}
  if (buttonB.update()) {buttonStatus[BUTTONB] = buttonB.fell();}
  if (buttonX.update()) {buttonStatus[BUTTONX] = buttonX.fell();}
  if (buttonY.update()) {buttonStatus[BUTTONY] = buttonY.fell();}
  // if (buttonLB.update()) {buttonStatus[BUTTONLB] = buttonLB.fell();}
  // if (buttonRB.update()) {buttonStatus[BUTTONRB] = buttonRB.fell();}
  if (buttonLT.update()) {buttonStatus[BUTTONLT] = buttonLT.fell();}
  if (buttonRT.update()) {buttonStatus[BUTTONRT] = buttonRT.fell();}
  if (buttonSTART.update()) {buttonStatus[BUTTONSTART] = buttonSTART.fell();}
  if (buttonHOME.update()) {buttonStatus[BUTTONHOME] = buttonHOME.fell();}
}

void processDPAD() {
  ReportData.LX = 128;
  ReportData.LY = 128;
  ReportData.RX = 128;
  ReportData.RY = 128;

  if ((yAxis < 200) && (xAxis > 800)) {ReportData.HAT = DPAD_UPRIGHT_MASK_ON;}
  else if ((yAxis > 800) && (xAxis > 800)) {ReportData.HAT = DPAD_DOWNRIGHT_MASK_ON;}
  else if ((yAxis > 800) && (xAxis < 200)) {ReportData.HAT = DPAD_DOWNLEFT_MASK_ON;}
  else if ((yAxis < 200) && (xAxis < 200)) {ReportData.HAT = DPAD_UPLEFT_MASK_ON;}
  else if (yAxis < 200) {ReportData.HAT = DPAD_UP_MASK_ON;}
  else if (yAxis > 800) {ReportData.HAT = DPAD_DOWN_MASK_ON;}
  else if (xAxis < 200) {ReportData.HAT = DPAD_LEFT_MASK_ON;}
  else if (xAxis > 800) {ReportData.HAT = DPAD_RIGHT_MASK_ON;}
  else {ReportData.HAT = DPAD_NOTHING_MASK_ON;}
}

void processLANALOG() {
  ReportData.HAT = DPAD_NOTHING_MASK_ON;
  ReportData.RX = 128;
  ReportData.RY = 128;

  ReportData.LX = map(xAxis, 0, 1023, 0, 255);
  ReportData.LY = map(yAxis, 0, 1023, 0, 255);
}

void processLANALOGSmash() {
  ReportData.HAT = DPAD_NOTHING_MASK_ON;
  ReportData.RX = 128;
  ReportData.RY = 128;

  ReportData.LX = map(xAxis, 0, 1023, 64, 192);
  ReportData.LY = map(yAxis, 0, 1023, 64, 192);
}

void processRANALOG() {
  ReportData.HAT = DPAD_NOTHING_MASK_ON;
  ReportData.LX = 128;
  ReportData.LY = 128;

  ReportData.RX = map(xAxis, 0, 1023, 0, 255);
  ReportData.RY = map(yAxis, 0, 1023, 0, 255);
   // Set the LED to low to make sure it is off
  digitalWrite(pinOBLED, HIGH);
}

void processButtons() {
  // state gets set with checkModeChange
  switch (state) {
    case DIGITAL:
      processDPAD();
      buttonProcessing();
      break;

     // Just follow the Digital case logic supposely else this could be another bug potentially 
    case ANALOG_MODE:
      if (buttonStatus[BUTTONLT]) {processRANALOG();}
      else {processLANALOG();}
      buttonProcessing();
      break;

     // The smash could potentially cause the issue with double triggering, hence, commenting out for testing: 
     // In this case, this could be triggering too much with the input buttons, where the analog up is triggered in the same time with plus button in switch

     //Seems its fully detecting the joystick up atm 

    case SMASH:
      if (buttonStatus[BUTTONB]) {processLANALOGSmash();}
      else {processLANALOG();}
      buttonProcessingSmash();
      break;
  }
}

/* 
The ReportData

*/
void buttonProcessing(){
  if (buttonStatus[BUTTONA]) {ReportData.Button |= A_MASK_ON;}
  if (buttonStatus[BUTTONB]) {ReportData.Button |= B_MASK_ON;}
  if (buttonStatus[BUTTONX]) {ReportData.Button |= X_MASK_ON;}
  if (buttonStatus[BUTTONY]) {ReportData.Button |= Y_MASK_ON;}
  // if (buttonStatus[BUTTONLB]) {ReportData.Button |= LB_MASK_ON;}
  // if (buttonStatus[BUTTONRB]) {ReportData.Button |= RB_MASK_ON;}
  if (buttonStatus[BUTTONLT]) {ReportData.Button |= ZL_MASK_ON;}
  if (buttonStatus[BUTTONRT]) {ReportData.Button |= ZR_MASK_ON;}
  if (buttonStatus[BUTTONSTART]){ReportData.Button |= START_MASK_ON;}
  // if (buttonStatus[BUTTONSELECT]){ReportData.Button |= SELECT_MASK_ON;}
  if (buttonStatus[BUTTONHOME]){ReportData.Button |= HOME_MASK_ON;}
}


// void buttonProcessingSmash(){
//   if (buttonStatus[BUTTONA]) {ReportData.Button |= A_MASK_ON;}
//   if (buttonStatus[BUTTONB]) {ReportData.Button |= B_MASK_ON;}
//   if (buttonStatus[BUTTONX]) {ReportData.Button |= X_MASK_ON;}
//   if (buttonStatus[BUTTONY]) {ReportData.Button |= Y_MASK_ON;}
//   // if (buttonStatus[BUTTONLB]) {ReportData.Button |= LB_MASK_ON;}
//   // if (buttonStatus[BUTTONRB]) {ReportData.Button |= RB_MASK_ON;}
//   if (buttonStatus[BUTTONLT]) {ReportData.Button |= ZL_MASK_ON;}
//   if (buttonStatus[BUTTONRT]) {ReportData.Button |= ZR_MASK_ON;}
//   if (buttonStatus[BUTTONSTART]){ReportData.Button |= START_MASK_ON;}
//   // if (buttonStatus[BUTTONSELECT]){ReportData.Button |= SELECT_MASK_ON;}
//   if (buttonStatus[BUTTONHOME]){ReportData.Button |= HOME_MASK_ON;}
// }