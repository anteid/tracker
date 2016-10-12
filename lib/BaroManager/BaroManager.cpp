#include <Arduino.h>
#include <Wire.h>
#include <BaroManager.h>

BaroManager::BaroManager(){
    _pressureRaw = 0;
    _temperatureRaw = 0;
    _pressure = 0;
    _temperature = 0;
    _pressureZero = 0;
    _pressureFiltered = 0;
    _timestamp = 0;
    _timerBaro = 0;
    difft = 0;
    off = 0;
    sens = 0;
    convert_pression = true;
  }

  long BaroManager::process()
  {
    if(micros()-_timerBaro>15000) {
      acquireBaroData();
      _pressureFiltered = _pressureZero+filter.compute(_pressure-_pressureZero);
      setTimer(micros());
    }
    return _pressureFiltered;
  }


  void BaroManager::acquirePressureZero()
  {
    acquireBaroData(); // Première lecture : fausse car pas convertie
    delay(15);

    for(int i=0;i<NB_VALUES_PRESSURE_ZERO;i++)
    {
      acquireBaroData();
      _pressureZero+=_pressure;
      delay(15);
    }
    _pressureZero/=NB_VALUES_PRESSURE_ZERO;
    //LOG(pressureZero);
    //LOG("\n");
    _pressureFiltered = _pressureZero;

    /*on est obligé de faire ça au début car avant les 15 premières ms on aura
    fini de parser les données GPS et on sera prêt à envoyer mais on n'aura pas
    eu de nouvelles données baro
    */
  }

  void BaroManager::acquireBaroData()
  {
    if (convert_pression) {

      Wire.beginTransmission(PRESSION_ADDR);
      Wire.write(0x00);            //Reset
      Wire.endTransmission();      // stop transmitting
      Wire.requestFrom(PRESSION_ADDR,3);
      _pressureRaw = (long)Wire.read()<<16 | (long)Wire.read()<< 8 | Wire.read();

      // Conversion température
      Wire.beginTransmission(PRESSION_ADDR);
      Wire.write(0x58);            // 9ms de conversion
      Wire.endTransmission();
      setTimer(micros());


    } else {
      Wire.beginTransmission(PRESSION_ADDR);
      Wire.write(0x00);
      Wire.endTransmission();
      Wire.requestFrom(PRESSION_ADDR,3);
      _temperatureRaw = (long)Wire.read()<<16 | (long)Wire.read()<<8 | Wire.read();

      // Conversion pression
      Wire.beginTransmission(PRESSION_ADDR);
      Wire.write(0x48);            // 9ms de conversion
      Wire.endTransmission();
    }

    convert_pression ^= 1;

    difft = _temperatureRaw-((long)Calib_baro[4]<<8);
    _temperature = 2000 + ((difft*Calib_baro[5])>>23);
    off = ((int64_t)Calib_baro[1]<<17) + ((difft*Calib_baro[3])>>6);
    sens = ((int64_t)Calib_baro[0]<<16) + ((difft*Calib_baro[2])>>7);
    _pressure = (long)((((sens*_pressureRaw)>>21) - off)>>15);
  }

  void BaroManager::init()
  {

    Wire.beginTransmission(PRESSION_ADDR);
    Wire.write(0x1E); // reset
    Wire.endTransmission(); // stop transmitting

    delay(10);

    for (byte i=1;i<7;i++) {
       Wire.beginTransmission(PRESSION_ADDR);
       Wire.write(0xA0|(i<<1));            //Reset
       Wire.endTransmission();      // stop transmitting

       Wire.requestFrom(PRESSION_ADDR,2);
       Calib_baro[i-1] = Wire.read()<< 8 | Wire.read();
       //LOG(Calib_baro[i-1]);
       //LOG("\n");
    }
    Wire.beginTransmission(PRESSION_ADDR);
    Wire.write(0x48);
    Wire.endTransmission();
    delay(10);

    acquirePressureZero();
    setTimer(micros());
  }

  void BaroManager::setTimer(int timer){
    _timerBaro = timer;
  }

  BaroData_t BaroManager::getData(){
    BaroData_t data;
    data.pressureFiltered = _pressureFiltered;
    data.timestamp = _timestamp;
    return data;
  }

  void BaroManager::updateTimestamp()
  {
    _timestamp++;
  }
