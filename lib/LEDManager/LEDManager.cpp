#include <LEDManager.h>
#include <Arduino.h>

LEDManager::LEDManager(int pin) {
      _pin = pin;
      _timerLED = millis();
      _ledIsOn = false;
}

void LEDManager::blink(int thresholdOn, int thresholdOff) {
  if(thresholdOn==0 && thresholdOff==0)
  {
    digitalWrite(_pin,LOW);
  }
  else if(thresholdOn==1 && thresholdOff==1)
  {
    digitalWrite(_pin,LOW);
  }
  if ((millis()-_timerLED) >= thresholdOff && !_ledIsOn) {
    digitalWrite(_pin,HIGH);
    _ledIsOn = true;
    _timerLED = millis();
  }
  if ((millis()-_timerLED) >= thresholdOn && _ledIsOn) {
    digitalWrite(_pin,LOW);
    _ledIsOn = false;
    _timerLED = millis();
  }
}

void LEDManager::status(bool wifiOk, bool gpsOk) {
  if(wifiOk && !gpsOk)
  {
    blink(1e3,1e3);
  }

  else if(!wifiOk && gpsOk)
  {
    blink(1e2,1e2);
  }
  else if(wifiOk && gpsOk)
  {
    blink(1,0);
  }
  else
  {
    blink(0,0);
  }
}
