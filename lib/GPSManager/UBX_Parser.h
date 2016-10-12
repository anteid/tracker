/**
  *
  * UBX_Parser.h - A header-only C++ class for parsing UBX messages from Ublox GPS
  *
  * Copyright (C) 2015 Simon D. Levy
  *
  * This code is free software: you can redistribute it and/or modify
  * it under the terms of the GNU Lesser General Public License as
  * published by the Free Software Foundation, either version 3 of the
  * License, or (at your option) any later version.
  *
  * This code is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU Lesser General Public License
  * along with this code.  If not, see <http://www.gnu.org/licenses/>.
  */

/**
  * A class for parsing UBX messages.
  */

#include <Arduino.h>

class UBX_Parser {

    private:

        typedef enum {

            GOT_NONE,
            GOT_SYNC1,
            GOT_SYNC2,
            GOT_CLASS,
            GOT_ID,
            GOT_LENGTH1,
            GOT_LENGTH2,
            GOT_PAYLOAD,
            GOT_CHKA

        } state_t;

        state_t state;
        int msgclass;
        int msgid;
        int msglen;
        char chka;
        char chkb;
        int count;
        char payload[1000];
        bool hdseen;
        int timer;

        void addchk(int b) {

            this->chka = (this->chka + b) & 0xFF;
            this->chkb = (this->chkb + this->chka) & 0xFF;
        }


        void dispatchMessage() {
            switch (this->msgid) {
                case 0x02:
                    {
                    unsigned long iTOW = (unsigned long)this->unpack_int32(0);
                    long lon = this->unpack_int32(4);
                    long lat = this->unpack_int32(8);
                    long height = this->unpack_int32(12);
                    long hMSL = this->unpack_int32(16);
                    unsigned long hAcc = (unsigned long)this->unpack_int32(20);
                    unsigned long vAcc = (unsigned long)this->unpack_int32(24);
                    this->handle_NAV_POSLLH(iTOW, lon, lat, height, hMSL, hAcc, vAcc);
                    }
                    break;
                case 0x04:
                    {
                    unsigned long iTOW = (unsigned long)this->unpack_int32(0);
                    unsigned short gDOP = (unsigned short)this->unpack_int16(4);
                    unsigned short pDOP = (unsigned short)this->unpack_int16(6);
                    unsigned short tDOP = (unsigned short)this->unpack_int16(8);
                    unsigned short vDOP = (unsigned short)this->unpack_int16(10);
                    unsigned short hDOP = (unsigned short)this->unpack_int16(12);
                    unsigned short nDOP = (unsigned short)this->unpack_int16(14);
                    unsigned short eDOP = (unsigned short)this->unpack_int16(16);
                    this->handle_NAV_DOP(iTOW, gDOP, pDOP, tDOP, vDOP, hDOP, nDOP, eDOP);
                    }
                    break;
                 case 0x07:
                    {
                    //unsigned long iTOW = (unsigned long)this->unpack_int32(0);
                    //unsigned short year = (unsigned short)this->unpack_int16(4);
                    //unsigned int month = (unsigned int)this->unpack_int(6);
                    //unsigned int day = (unsigned int)this->unpack_int(7);
                    //unsigned int hour = (unsigned int)this->unpack_int(8);
                    //unsigned int min = (unsigned int)this->unpack_int(9);
                    //unsigned int sec = (unsigned int)this->unpack_int(10);
                    //unsigned int valid = (unsigned int)this->unpack_int(11);
                    //unsigned long tacc = (unsigned long)this->unpack_int32(12);
                    //long nano = this->unpack_int32(16);
                    unsigned char fixType = this->payload[20];
                    //unsigned int flags = (unsigned int)this->unpack_int(21);
                    //unsigned int flags2 = (unsigned int)this->unpack_int(22);
                    unsigned char numSV = this->payload[23];
                    long lon = this->unpack_int32(24);
                    long lat = this->unpack_int32(28);
                    long height = this->unpack_int32(32);
                    long hMSL = this->unpack_int32(36);
                    unsigned long hAcc = (unsigned long)this->unpack_int32(40);
                    unsigned long vAcc = (unsigned long)this->unpack_int32(44);
					          long velN = this->unpack_int32(48);
                    long velE = this->unpack_int32(52);
                    long velD = this->unpack_int32(56);
                    //long gSpeed = this->unpack_int32(60);
                    //long headMot = this->unpack_int32(64);
                    unsigned long sAcc = (unsigned long)this->unpack_int32(68);
                    this->handle_NAV_PVT(fixType, numSV, lon, lat, height, hMSL, hAcc, vAcc, velN, velE, velD,sAcc);
                    }
                    break;
                case 0x12:
                    {
                    unsigned long iTOW = (unsigned long)this->unpack_int32(0);
                    long velN = this->unpack_int32(4);
                    long velE = this->unpack_int32(8);
                    long velD = this->unpack_int32(12);
                    unsigned long speed = (unsigned long)this->unpack_int32(16);
                    unsigned long gSpeed = (unsigned long)this->unpack_int32(20);
                    long heading = this->unpack_int32(24);
                    unsigned long sAcc = (unsigned long)this->unpack_int32(28);
                    unsigned long cAcc = (unsigned long)this->unpack_int32(32);
                    this->handle_NAV_VELNED(iTOW, velN, velE, velD, speed, gSpeed, heading, sAcc, cAcc);
                    }
                    /*
                case 0x06:
                    {
                    unsigned long iTOW = (unsigned long)this->unpack_int32(0);
                    long fTOW = this->unpack_int32(4);
                    short week = (short)this->unpack_int16(8);
                    char gpsFix = this->payload[10];
                    this->handle_NAV_SOL(iTOW, fTOW, week, gpsFix);
                    }
                    break;
                    */
                  default:
                    this->reportUnhandled(this->msgid);
                    break;
            }
        }

        long unpack_int32(int offset) {

            return this->unpack(offset, 4);
        }

        long unpack_int16(int offset) {

            return this->unpack(offset, 2);
        }


        long unpack(int offset, int size) {

            long value = 0; // four bytes on most Arduinos

            for (int k=0; k<size; ++k) {
                value <<= 8;
                value |= (0xFF & this->payload[offset+4-k-1]);
            }

            return value;
         }

     protected:

        /**
          Override this method to handle NAV-POSLLH messages.
          @param iTOW GPS Millisecond Time of Week
          @param lon Longitude in degrees * 10<sup>7</sup>
          @param lat Latitude in degrees * 10<sup>7</sup>
          @param height Height above Ellipsoid in millimeters
          @param hMSL Height above mean sea level in millimeters
          @param hAcc Horizontal Accuracy Estimate in millimeters
          @param vAcc Vertical Accuracy Estimate in millimeters
          */
        virtual void handle_NAV_POSLLH(unsigned long iTOW,
                long lon,
                long lat,
                long height,
                long hMSL,
                unsigned long hAcc,
                unsigned long vAcc) { }

        /**
          Override this method to handle NAV-DOP messages.
          @param iTOW GPS Millisecond Time of Week
          @param gDOP Geometric DOP
          @param pDOP Posiition DOP
          @param tDOP Time DOP
          @param vDOP Vertical DOP
          @param hDOP Horizontal DOP
          @param nDOP Northing DOP
          @param eDOP Easting DOP
          */
         virtual void handle_NAV_DOP(unsigned long iTOW,
                unsigned short gDOP,
                unsigned short pDOP,
                unsigned short tDOP,
                unsigned short vDOP,
                unsigned short hDOP,
                unsigned short nDOP,
                unsigned short eDOP) { }

        /**
          Override this method to handle NAV-VELPVT messages.
          @param gpsType GPS fix type
		  @param numSV number of satellites used
          @param lon Longitude in degrees * 10<sup>7</sup>
          @param lat Latitude in degrees * 10<sup>7</sup>
          @param height Height above Ellipsoid in millimeters
          @param hMSL Height above mean sea level in millimeters
          @param hAcc Horizontal Accuracy Estimate in millimeters
          @param vAcc Vertical Accuracy Estimate in millimeters
          @param velN NED north velocity in cm/sec
          @param velE NED east velocity in cm/sec
          @param velD NED down velocity in cm/sec
          @param sAcc Speed Accuracy Estimate in cm/sec
          */
          virtual void handle_NAV_PVT(unsigned char fixType,
        		unsigned char numSV,
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
            }

        /**
          Override this method to handle NAV-VELNED messages.
          @param iTOW GPS Millisecond Time of Week
          @param velN NED north velocity in cm/sec
          @param velE NED east velocity in cm/sec
          @param velD NED down velocity in cm/sec
          @param speed Speed (3-D)in cm/sec
          @param gSpeed Ground Speed (3-D)in cm/sec
          @param heading Heading of motion 2-D in degrees * 10<sup>5</sup>
          @param sAcc Speed Accuracy Estimate in cm/sec
          @param cAcc Course / Heading Accuracy Estimate in degrees
          */
         virtual void handle_NAV_VELNED(unsigned long iTOW,
                long velN,
                long velE,
                long velD,
                unsigned long speed,
                unsigned long gSpeed,
                long heading,
                unsigned long sAcc,
                unsigned long cAcc) { }

        /**
          Override this method to handle NAV-SOL messages.
          @param iTOW GPS Millisecond Time of Week
          @param fTOW fractional time of week in nanoseconds
          @param week GPS week number of the navigation epoch
          @param gpsFix GPS fix type
          */
         /*
         virtual void handle_NAV_SOL(unsigned long iTOW,
                 long fTOW,
                 short week,
                 char gpsFix) { }
                 */

         /**
           * Override this method to report receipt of messages not
           * handled by current code.
           * @param msgid ID of current message
           */
        virtual void reportUnhandled(char msgid) { }

    public:

        /**
          * Constructs a UBX parser.
          */
        UBX_Parser()
        {
            this->state    = GOT_NONE;
            this->msgclass = -1;
            this->msgid    = -1;
            this->msglen   = -1;
            this->chka     = -1;
            this->chkb     = -1;
            this->count    = 0;
            this->hdseen = false;
            this->timer = 0;
        }

        /**
          * Parses a new byte from the GPS. Automatically calls handle_ methods when a new
          * message is successfully parsed.
          * @param b the byte
          */
        void parse(int b)
        {
            if (b == 0xB5 && !(this->hdseen)) {
                this->state = GOT_SYNC1;
                this->hdseen = true;
                // Serial1.print("got_sync1 ");
                // Serial1.print((micros()-this->timer)/1000.0,2);
                // Serial1.println(" ms");
                // this->timer = micros();
            }

            else if (b == 0x62 && this->state == GOT_SYNC1) {
                this->state = GOT_SYNC2;
                this->chka = 0;
                this->chkb = 0;
                // Serial1.print("got_sync2 ");
            }

            else if (this->state == GOT_SYNC2) {

                this->state = GOT_CLASS;
                this->msgclass = b;
                this->addchk(b);
                // Serial1.print("got_class ");
            }

            else if (this->state == GOT_CLASS) {

                this->state = GOT_ID;
                this->msgid = b;
                this->addchk(b);
                // Serial1.print("got_id ");
            }

            else if (this->state == GOT_ID) {

                this->state = GOT_LENGTH1;
                this->msglen = b;
                this->addchk(b);
                // Serial1.print("got_length ");
            }

            else if (this->state == GOT_LENGTH1) {

                this->state = GOT_LENGTH2;
                this->msglen += (b << 8);
                this->count = 0;
                this->addchk(b);
                // Serial1.print("got_length2 ");
            }

            else if (this->state == GOT_LENGTH2) {

                this->addchk(b);
                this->payload[this->count] = b;
                this->count += 1;

                if (this->count == this->msglen) {

                    this->state = GOT_PAYLOAD;
                      // Serial1.print("got_payload ");
                }
            }

            else if (this->state == GOT_PAYLOAD) {

                this->state = (b == this->chka) ? GOT_CHKA : GOT_NONE;
                  // Serial1.print("got_chka ");
            }

            else if (this->state == GOT_CHKA) {

                if (b == this->chkb) {
                    // Serial1.println("got_checkb");
                    this->hdseen = false;
                    this->dispatchMessage();
                }

                else {
                    this->state = GOT_NONE;
                }
            }
            else {
                this->hdseen = false;
                    //Serial1.println(">> frame is KO");
            }
    }
};
