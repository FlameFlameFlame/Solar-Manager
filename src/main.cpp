/* Encoder Library - Basic Example
 * http://www.pjrc.com/teensy/td_libs_Encoder.html
 *
 * This example code is in the public domain.
 */

#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>
#include <Arduino.h>
#include <LiquidCrystal.h>

#include <time.h>

#define ENCODER_POS_PIN 11
#define ENCODER_NEG_PIN 3
#define BUTTON_PIN 2
#define BUTTON_PRESSED_STATE LOW
#define RELAY_PIN 13

#define LONG_PRESS_DURATION_MS 2000

#define PASSCODE 451
#define PASSCODE_LENGTH 3

// Change these two numbers to the pins connected to your encoder.
//   Best Performance: both pins have interrupt capability
//   Good Performance: only the first pin has interrupt capability
//   Low Performance:  neither pin has interrupt capability
Encoder myEnc(ENCODER_POS_PIN, ENCODER_NEG_PIN);

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
// struct for storing cursor coordinates
struct Cursor
{
  int line;
  int row;
};
// class for a relay control
class RelayManager
{
private:
  int pin;
  bool state;
public:
  RelayManager (int _pin)
  {
    pin = _pin;
    state = LOW;

    pinMode(pin, OUTPUT);
    digitalWrite(pin, state);
  }
  bool GetState () const
  {
    return state;
  }
  void TurnOn ()
  {
    if (!state) 
    {
      state = HIGH;
      digitalWrite(pin, state);
    }
  }
  void TurnOff ()
  {
    if (state)
    {
      state = LOW;
      digitalWrite(pin, state);
    }
  }
};
// device state typedf
typedef enum
{
  SETTING_TIME,
  WARMING,
  RADIATING
} State;

State state = SETTING_TIME;

Cursor cur{0, 0};

bool IsButtonPressed(const int &button_pin)
{
  return digitalRead(button_pin) == BUTTON_PRESSED_STATE;
}

void SetCursor(LiquidCrystal &lcd, const Cursor &newPos)
{
  lcd.setCursor(newPos.row, newPos.line);
}

void ClearLine(const int &line, LiquidCrystal &display)
{
  display.setCursor(0, line);
  for (int i = 0; i < 16; i++)
  {
    display.print(" ");
  }
}
// enter single number in range from 0 to 9
int EnterNumberRotary(LiquidCrystal &display, const Cursor &cursor, Encoder &enc, const int &button)
{
  int result = 0;
  bool doExit = false, isUpdated = false;
  int encPrevState = enc.read();
  SetCursor(display, cursor);
  display.cursor();

  while (!doExit)
  {
    int encNewState = enc.read();
    // rotary encoder turn to "negative" side
    if (encNewState - encPrevState == 4)
    {
      result < 9 ? result++ : result = 0;
      encPrevState = encNewState;
      Serial.println(result);
      isUpdated = true;
    }
    // rotary encoder turn to "positive" side
    else if (encNewState - encPrevState == -4)
    {
      result > 0 ? result-- : result = 9;
      encPrevState = encNewState;
      Serial.println(result);
      isUpdated = true;
    }
    // avoid false positive
    if (IsButtonPressed(button))
    {
      while (IsButtonPressed(button))
        ;
      doExit = true;
    }
    // user has entered new number, update LCD
    if (isUpdated)
    {
      display.print(result);
      SetCursor(display, cursor);
      isUpdated = false;
    }
  }
  display.noCursor();
  return result;
}
// enter time in format mm:ss using rotary encoder
// short button press for change from minutes to seconds and back
// long button press for setting time
Time EnterTimeRotary(LiquidCrystal &display, Cursor cursor, Encoder &enc, const int &button, const Time &initialTime)
{
  bool doExit = false, isUpdated = false;
  int encPrevState = enc.read();
  SetCursor(display, cursor);
  Cursor loc_cursor = cursor;
  display.cursor();

  int minutes = initialTime.GetMinutes(), seconds = initialTime.GetSeconds();

  bool isMinutesSet = false, isSecondsSet = true;

  while (!doExit)
  {
    int encNewState = enc.read();
  
    if (encNewState - encPrevState == 4)
    {
      if (!isMinutesSet)
        minutes < 59 ? minutes++ : minutes = 0;
      else if (!isSecondsSet)
        seconds < 59 ? seconds++ : seconds = 0;

      encPrevState = encNewState;
      if (!isMinutesSet)
        Serial.println(minutes);
      if (!isSecondsSet)
        Serial.println(seconds);
      isUpdated = true;
    }

    if (encNewState - encPrevState == -4)
    {
      if (!isMinutesSet)
        minutes > 0 ? minutes-- : minutes = 59;
      else if (!isSecondsSet)
        seconds > 0 ? seconds-- : seconds = 59;

      encPrevState = encNewState;
      if (!isMinutesSet)
        Serial.println(minutes);
      if (!isSecondsSet)
        Serial.println(seconds);
      isUpdated = true;
    }

    if (IsButtonPressed(button))
    {
      unsigned long long startTime = millis();
      while (IsButtonPressed(button) && millis() - startTime < LONG_PRESS_DURATION_MS)
        ;
      // check if button press registers as "long"
      if (millis() - startTime >= LONG_PRESS_DURATION_MS)
      {
        doExit = true;
      }
      // if button press is not register as "long", change from minutes to seconds or back
      else
      {
        if (!isMinutesSet)
        {
          isMinutesSet = true;
          isSecondsSet = false;
          loc_cursor.row += 3;
          Serial.println("set minutes");
        }
        else if (!isSecondsSet)
        {
          isSecondsSet = true;
          isMinutesSet = false;
          loc_cursor.row -= 3;
          Serial.println("set seconds");
        }
      }
      SetCursor(lcd, loc_cursor);
    }
    // update LCD
    if (isUpdated)
    {
      display.noCursor();
      SetCursor(lcd, cursor);
      if (minutes < 10)
      {
        display.print('0');
        display.print(minutes);
      }
      else
        display.print(minutes);

      display.print(':');
      if (seconds < 10)
      {
        display.print('0');
        display.print(seconds);
      }
      else
        display.print(seconds);

      SetCursor(display, loc_cursor);
      display.cursor();
      isUpdated = false;
    }
  }
  display.noCursor();
  return (Time){0, minutes, seconds};
}

int EnterPasscode(LiquidCrystal &display, Cursor cursor, Encoder &enc, const int &button, const int &passcode_length)
{
  int result = 0;
  for (int i = 0; i < passcode_length; i++)
  {
    result += EnterNumberRotary(display, cursor, enc, BUTTON_PIN) * round(pow(10, passcode_length - i - 1));
    cursor.row++;
  }
  return result;
}

int pwd = 0;
// RalayManager takes care of initializing the relay pin
RelayManager relay(RELAY_PIN);

void setup()
{
  //initialize button
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  // initialize lcd
  lcd.begin(16, 2);
  SetCursor(lcd, cur);

  lcd.println("Enter a passcode");
  // wait for a correct password input
  bool isPwdCorrect = false;
  do
  {
    ClearLine(1, lcd);
    cur.line = 1;
    cur.row = 0;
    isPwdCorrect = EnterPasscode(lcd, cur, myEnc, BUTTON_PIN, PASSCODE_LENGTH) == PASSCODE;
    if (!isPwdCorrect)
    {
      SetCursor(lcd, cur);
      lcd.print("Passcode incorrect");
      delay(1000);
    }
    else
    {
      SetCursor(lcd, cur);
      lcd.print("Passcode correct");
      delay(1000);
    }
  } while (!isPwdCorrect);

  cur.row = 0;
  cur.line = 0;
  SetCursor(lcd, cur);
}

unsigned long long prevMillis = 0;
bool secondPassed = false;
Time t(0, 0, 0);
Time setTime(0, 0, 0);

void loop()
{
  // "timer"
  if (millis() - prevMillis > 1000)
  {
    secondPassed = true;
    prevMillis = millis();
  }

  // stop
  if (IsButtonPressed(BUTTON_PIN) && (state == WARMING || state == RADIATING))
  {
    state = SETTING_TIME;
    relay.TurnOff();
    cur.line = 0;
    cur.row = 0;
    SetCursor(lcd, cur);
    lcd.println("Abort");
    delay(1000);
  }

  // check state of a device, decide what to do
  switch (state)
  {
  case SETTING_TIME:
    while (state == SETTING_TIME)
    {
      // write header
      cur.line = 0;
      cur.row = 0;
      SetCursor(lcd, cur);
      lcd.println("Set time        ");
      cur.line = 1;
      ClearLine(1, lcd);
      SetCursor(lcd, cur);
      lcd.print(setTime.AsString());
      cur.row = 0;
      // enter time
      t = EnterTimeRotary(lcd, cur, myEnc, BUTTON_PIN, setTime);
      Serial.print(t.AsString());
      // remember time user has set to future use
      setTime = t;
      // when time is set, write "Time set!"
      SetCursor(lcd, cur);
      lcd.print("Time set!");
      delay(1000);
      ClearLine(1, lcd);
      state = WARMING;
    }
    break;
  // reserved for a future use
  case WARMING:
    state = RADIATING;
    cur.line = 0;
    cur.row = 0;
    SetCursor(lcd, cur);
    lcd.print("Time remaining");
    cur.line = 1;
    SetCursor(lcd, cur);
    break;
  // radiate for a said period of time
  case RADIATING:
    relay.TurnOn();
    // update time on screen, update radiation time
    if (secondPassed)
    {
      secondPassed = false;

      // stop radiating
      if (t.GetMinutes() == 0 && t.GetSeconds() == 0)
      {
        relay.TurnOff();
        SetCursor(lcd, cur);
        lcd.print("Done!");
        delay(1000);
        state = SETTING_TIME;
      }

      t--;
      SetCursor(lcd, cur);
      lcd.print(t.AsString());
      Serial.println(t.AsString());
    }
    break;
  }
}