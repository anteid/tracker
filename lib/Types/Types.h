#ifndef Types_h
#define Types_h

typedef struct {
  double longitude;
  double latitude;
  double timestampGPS;
  float altitude;
  float northSpeed;
  float eastSpeed;
  float downSpeed;
  float horizontalAcc;
  float verticalAcc;
  float speedAcc;
  int numberSV;
  bool isReady;
  int timestamp;
} GPSData_t;

typedef struct {
  long pressureFiltered;
  long pressureZero;
  int timestamp;
} BaroData_t;

enum
{
    MSG_GPS = 1,
    MSG_BARO,
};

#endif
