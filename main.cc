#include <iostream>
#include <fstream>
#include <math.h>

using namespace std;

#define SAMP_RATE 44100UL
#define SAMP_RATE_F 44100.0f

#define VVVF_STAGE1 4.2f
#define VVVF_STAGE2 (2.4f + VVVF_STAGE1)
#define VVVF_STAGE3 (3.0f + VVVF_STAGE2)
#define VVVF_STAGE4 (4.0f + VVVF_STAGE3)

#define VVVF_FREQ1	600.0
#define VVVF_FREQ2	800.0
#define PI			3.1415926f

#define FREQ_RATIO ((0x0FFFFFFF) / SAMP_RATE_F)
short int generatesample(float carrierfreq, float basefreq, float modulateratio)
{
	float sine = 0;
	static unsigned int phase_counter;
	static unsigned short int saw;
	int compare;

	sine = sin(phase_counter * 2.0f * PI / 0x0FFFFFFF) * (modulateratio * 65535.0);
	phase_counter += FREQ_RATIO * basefreq;

	saw += (0xffff) / SAMP_RATE_F * carrierfreq;

	compare = saw;

	if(sine > 0)
	{
		if(compare <(int)sine)
			return +3276;
		else
			return 0;
	}
	else
	{
		if(compare < (int)(-sine))
			return -3276;
		else
			return 0;
	}

}
int main()
{ /* 4.2s carrier: 600Hz, 2.4s carrier: 800hz, samplerate:48kHz */
	unsigned int phase_counter = 0;
	int t = 0;
	short int sample;
	unsigned short int compare;
	ofstream wavefile("vvvf.raw", ios::binary);
	for(t; t <= SAMP_RATE * 20; t++)
	{/* MAX while motor reachs 100Hz at 20s */

		if(t < SAMP_RATE * VVVF_STAGE1)
			sample = generatesample(VVVF_FREQ1, (t / SAMP_RATE_F) * 5, (t / SAMP_RATE_F) / 13.6);
		if(t < SAMP_RATE * VVVF_STAGE2 && t >= SAMP_RATE * VVVF_STAGE1)
			sample = generatesample(VVVF_FREQ2, (t / SAMP_RATE_F) * 5, (t / SAMP_RATE_F) / 13.6);
		if(t < SAMP_RATE * VVVF_STAGE3 && t >= SAMP_RATE * VVVF_STAGE2) /* 33Hz */
			sample = generatesample((t * 5.0 * 36.0 / SAMP_RATE_F), (t / SAMP_RATE_F) * 5, (t / SAMP_RATE_F) / 13.6);
		if(t < SAMP_RATE * VVVF_STAGE4 && t >= SAMP_RATE * VVVF_STAGE3)//(t < SAMP_RATE * VVVF_STAGE4) /* 48Hz, repeat carrier to 1.2kHz */
			sample = generatesample((t * 5.0 * 25.0 / SAMP_RATE_F), (t / SAMP_RATE_F) * 5, (t / SAMP_RATE_F) / 13.6);
		if(t >= SAMP_RATE * VVVF_STAGE4)//(t < SAMP_RATE * VVVF_STAGE4) /* 2kHz/68Hz approx normal operation */
			sample = generatesample(1700, 68, 1);

		wavefile.write((char *)&sample, 2);
	}
	wavefile.close();
	return 0;
}

