#include <Arduino.h>
#include <GPSManager.h>

GPSManager::GPSManager()
{
  data.latitude = 500;
  data.longitude = 500;
  data.altitude = 500;
  data.northSpeed = 0;
  data.eastSpeed = 0;
  data.downSpeed = 0;
  data.horizontalAcc = 500e3;
  data.verticalAcc = 500e3;
  data.numberSV = 0;
  data.isReady = false;
  data.timestamp = 0;
}

void GPSManager::flash()
{
  writeUBX(UBX_CFG_NAV5,sizeof(UBX_CFG_NAV5)/sizeof(UBX_CFG_NAV5[0]));
  writeUBX(UBX_CFG_NAVX5,sizeof(UBX_CFG_NAVX5)/sizeof(UBX_CFG_NAVX5[0]));
  writeUBX(UBX_CFG_RATE,sizeof(UBX_CFG_RATE)/sizeof(UBX_CFG_RATE[0]));
  writeUBX(UBX_CFG_MSG,sizeof(UBX_CFG_MSG)/sizeof(UBX_CFG_MSG[0]));
  writeUBX(UBX_CFG_CFG,sizeof(UBX_CFG_CFG)/sizeof(UBX_CFG_CFG[0]));
  delay(1000);
}

void GPSManager::writeUBX(const byte msg[], int size)
{
  int payloadSize = size-UBX_CLASS_ID_OFFSET;
  byte buffer[UBX_HEADER_OFFSET+UBX_CLASS_ID_OFFSET+UBX_LENGTH_OFFSET+payloadSize+UBX_FOOTER_OFFSET];

  /* UBX Header */
  buffer[0] = 0xB5;
  buffer[1] = 0x62;

  /* Class and ID of message */
  buffer[UBX_HEADER_OFFSET] = msg[0];
  buffer[UBX_HEADER_OFFSET+1] = msg[1];

  /* Length */
  buffer[UBX_HEADER_OFFSET+UBX_CLASS_ID_OFFSET] = payloadSize & 0xFF;
  buffer[UBX_HEADER_OFFSET+UBX_CLASS_ID_OFFSET+1] = (payloadSize >> 8) & 0xFF;

  /* Payload */
  for (int i=UBX_LENGTH_OFFSET;i<size;i++) {
    buffer[i+UBX_HEADER_OFFSET+UBX_CLASS_ID_OFFSET] = msg[i];
  }

  /* Checksums */
  byte checksum[2] = {0,0};
  for(int i=UBX_HEADER_OFFSET;i<UBX_HEADER_OFFSET+UBX_CLASS_ID_OFFSET+UBX_LENGTH_OFFSET+payloadSize;i++)
  {
    checksum[0] = (checksum[0]+buffer[i]) & 0xFF;
    checksum[1] = (checksum[0]+checksum[1]) & 0xFF;
  }

  buffer[UBX_HEADER_OFFSET+UBX_CLASS_ID_OFFSET+UBX_LENGTH_OFFSET+payloadSize] = checksum[0]; /*ChkA*/
  buffer[UBX_HEADER_OFFSET+UBX_CLASS_ID_OFFSET+UBX_LENGTH_OFFSET+payloadSize+1] = checksum[1]; /*ChkB*/

  for(int i=0;i<UBX_HEADER_OFFSET+UBX_CLASS_ID_OFFSET+UBX_LENGTH_OFFSET+payloadSize+UBX_FOOTER_OFFSET;i++)
  {
    Serial1.print(buffer[i],HEX); // TODO : debuglog in hex
    Serial1.print(" ");
  }
  Serial1.println("");

  for(int i=0;i<size+UBX_HEADER_OFFSET+UBX_LENGTH_OFFSET+UBX_FOOTER_OFFSET;i++)
  {
    Serial.write(buffer[i]);
    Serial.flush();
  }
}

void GPSManager::process()
{
    while(Serial.available()>0)
    {
      parse(Serial.read());
    }
}

void GPSManager::prepareNextMeasure()
{
  updateTimestamp();
  data.isReady = false;
}

void GPSManager::updateTimestamp()
{
  data.timestamp++;
}

bool GPSManager::isReady()
{
  return data.isReady;
}

GPSData_t GPSManager::getData()
{
  return data;
}

void GPSManager::handle_NAV_PVT(byte fixType,
            byte numSV,
            long lon,
            long lat,
            long height,
            long hMSL,
            unsigned long hAcc,
            unsigned long vAcc,
            long velN,
            long velE,
            long velD,
            unsigned long sAcc) {
        data.longitude = lon;
        data.latitude = lat;
        data.altitude = height;
        data.northSpeed = velN;
        data.eastSpeed = velE;
        data.downSpeed = velD;
        data.horizontalAcc = hAcc;
        data.verticalAcc = vAcc;
        data.speedAcc = sAcc;
        data.numberSV = numSV;
        data.isReady = true;
}
