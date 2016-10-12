#ifndef BaroManager_h
#define BaroManager_h

#include <Arduino.h>
#include <Types.h>
#include <Butterworth.h>

class BaroManager
{

public:
  BaroManager();
  void init();
  long process(); /* Acquire and filter baro data */
  void updateTimestamp();
  BaroData_t getData();

private:
  static const int NB_VALUES_PRESSURE_ZERO = 50;
  static const int PRESSION_ADDR = 119;
  volatile unsigned int Calib_baro[6];
  volatile bool convert_pression;
  volatile long _temperatureRaw,_temperature,_pressureRaw,_pressure,_pressureZero,_pressureFiltered;
  int64_t difft,off,sens;
  int _timestamp;
  int _timerBaro;
  Butterworth filter;
  void setTimer(int timer);
  void acquirePressureZero();
  void acquireBaroData();
};

#endif
