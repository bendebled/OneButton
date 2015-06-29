// -----
// OneButton.cpp - Library for detecting button clicks, doubleclicks and long press pattern on a single button.
// This class is implemented for use with the Arduino environment.
// Copyright (c) by Matthias Hertel, http://www.mathertel.de
// This work is licensed under a BSD style license. See http://www.mathertel.de/License.aspx
// More information on: http://www.mathertel.de/Arduino
// -----
// Connect the button to pin A1 (MenuPin)
// 02.10.2010 created by Matthias Hertel
// 21.04.2011 support of active LOW and active HIGH button signal input.
// 01.12.2011 include file changed to work with the Arduino 1.0 environment
// 12.01.2014 some typos fixed.
// 01.03.2014 Enhanced long press functionalities by adding longPressStart and longPressStop callbacks
// -----

#include "OneButton.h"

// ----- Initialization and Default Values -----

OneButton::OneButton(int pin, int activeLow)
{
  pinMode(pin, INPUT);      // sets the MenuPin as input
  _pin = pin;

  _clickTicks = 250;        // number of millisec that have to pass by before a click is detected.
  _pressTicks = 600;       // number of millisec that have to pass by before a long button press is detected.
  
  _lastActivityTime = 0;
 
  _state = 0; // starting with state 0: waiting for button to be pressed
  _isLongPressed = false;  // Keep track of long press state

  if (activeLow) {
    // button connects ground to the pin when pressed.
    _buttonReleased = HIGH; // notPressed
    _buttonPressed = LOW;
    digitalWrite(pin, HIGH);   // turn on pullUp resistor

  } else {
    // button connects VCC to the pin when pressed.
    _buttonReleased = LOW;
    _buttonPressed = HIGH;
  } // if


  _doubleClickFunc = NULL;
  _pressFunc = NULL;
  _longPressStartFunc = NULL;
  _longPressStopFunc = NULL;
  _duringLongPressFunc = NULL;
} // OneButton


// explicitly set the number of millisec that have to pass by before a click is detected.
void OneButton::setClickTicks(int ticks) { 
  _clickTicks = ticks;
} // setClickTicks


// explicitly set the number of millisec that have to pass by before a long button press is detected.
void OneButton::setPressTicks(int ticks) {
  _pressTicks = ticks;
} // setPressTicks

// explicitly set the number of millisec that have to pass by before a long button press is detected.
unsigned long OneButton::getLastActivityTime() {
  return _lastActivityTime;
}

// save function for click event
void OneButton::attachClick(callbackFunction newFunction)
{
  _clickFunc = newFunction;
} // attachClick


// save function for doubleClick event
void OneButton::attachDoubleClick(callbackFunction newFunction)
{
  _doubleClickFunc = newFunction;
} // attachDoubleClick


// save function for press event
// DEPRECATED, is replaced by attachLongPressStart, attachLongPressStop, attachDuringLongPress, 
void OneButton::attachPress(callbackFunction newFunction)
{
  _pressFunc = newFunction;
} // attachPress

// save function for longPressStart event
void OneButton::attachLongPressStart(callbackFunction newFunction)
{
  _longPressStartFunc = newFunction;
} // attachLongPressStart

// save function for longPressStop event
void OneButton::attachLongPressStop(callbackFunction newFunction)
{
  _longPressStopFunc = newFunction;
} // attachLongPressStop

// save function for during longPress event
void OneButton::attachDuringLongPress(callbackFunction newFunction)
{
  _duringLongPressFunc = newFunction;
} // attachDuringLongPress

// function to get the current long pressed state
bool OneButton::isLongPressed(){
  return _isLongPressed;
}

// function to get the current pressed state
bool OneButton::isPressed(){
  int buttonLevel = digitalRead(_pin);
  return buttonLevel == _buttonPressed;
}

// function to get the current released state
bool OneButton::isReleased(){
  int buttonLevel = digitalRead(_pin);
  return buttonLevel == _buttonReleased;
}

int OneButton::tick(void)
{
  // Detect the input information 
  int buttonLevel = digitalRead(_pin); // current button signal.
  unsigned long now = millis(); // current (relative) time in msecs.
  int finalState = 0;

  // Implementation of the state machine
  if (_state == 0) { // waiting for menu pin being pressed.
    if (buttonLevel == _buttonPressed) {
      _state = 1; // step to state 1
      _startTime = now; // remember starting time
      finalState = PRESSED_STATE;
    } // if

  } else if (_state == 1) { // waiting for menu pin being released.
    if (buttonLevel == _buttonReleased) {
      _state = 2; // step to state 2

    } else if ((buttonLevel == _buttonPressed) && (now > _startTime + _pressTicks)) {
      _isLongPressed = true;  // Keep track of long press state
      if (_pressFunc) _pressFunc();
	  if (_longPressStartFunc) _longPressStartFunc();
	  if (_duringLongPressFunc) _duringLongPressFunc();
	  finalState = LONG_PRESS_START_STATE;
      _state = 6; // step to state 6
      
    } else {
      // wait. Stay in this state.
    } // if

  } else if (_state == 2) { // waiting for menu pin being pressed the second time or timeout.
    if (now > _startTime + _clickTicks) {
      // this was only a single short click
      if (_clickFunc) _clickFunc();
      finalState = CLICK_STATE;
      _state = 0; // restart.

    } else if (buttonLevel == _buttonPressed) {
      _state = 3; // step to state 3
    } // if

  } else if (_state == 3) { // waiting for menu pin being released finally.
    if (buttonLevel == _buttonReleased) {
      // this was a 2 click sequence.
      if (_doubleClickFunc) _doubleClickFunc();
      finalState = DOUBLE_CLICK_STATE;
      _state = 0; // restart.
    } // if

  } else if (_state == 6) { // waiting for menu pin being release after long press.
    if (buttonLevel == _buttonReleased) {
	  _isLongPressed = false;  // Keep track of long press state
	  if(_longPressStopFunc) _longPressStopFunc();
	  finalState = LONG_PRESS_STOP_STATE;
      _state = 0; // restart.
    } else {
	  // button is being long pressed
	  _isLongPressed = true; // Keep track of long press state
	  if (_duringLongPressFunc) _duringLongPressFunc();
	  finalState = DURING_LONG_PRESS_STATE;
    } // if  

  } // if  
  if(finalState != 0){
    _lastActivityTime = now;
  }
  return finalState;
} // OneButton.tick()


// end.

