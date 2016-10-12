// From http://www.schwietering.com/jayduino/filtuino/index.php

//Low pass butterworth filter order=2 alpha1=0.0083333333333333  (Fe = 60Hz, fc = 0.5Hz)
#include <Butterworth.h>

Butterworth::Butterworth()
{
	v[0]=0.0;
	v[1]=0.0;
}

float Butterworth::compute(float x) //class II
{
	v[0] = v[1];
	v[1] = v[2];
	v[2] = (6.607790982303962668e-4 * x)
		 + (-0.92862708612480726611 * v[0])
		 + (1.92598396973188568104 * v[1]);
	return (v[0] + v[2]) + 2 * v[1];
}
