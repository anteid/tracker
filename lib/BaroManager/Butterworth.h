#ifndef Butterworth_h
#define Butterworth_h

class Butterworth
{
  public:
    Butterworth();
    float compute(float x);

  private:
    float v[3];
};

#endif
