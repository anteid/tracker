#ifndef LEDManager_h
#define LEDManager_h

class LEDManager {
public:
  LEDManager(int pin);
  void blink(int thresholdOn, int thresholdOff);
  void status(bool wifiOk, bool gpsOk);
private:
  int _pin;
  int _timerLED;
  bool _ledIsOn;
};

#endif
