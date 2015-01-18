/*
Allpasser.h - Apply cascades of allpass filters.
Written by Joe Mariglio, 1/16/15
*/
#include "../3SPDSP/soap/soap.h"
#include "Eigen/Dense"
#include "../JuceLibraryCode/JuceHeader.h"
#include "math.h"
#include "stdio.h"

#define ORDER 200

class Allpasser
{
public:
Allpasser();
~Allpasser();


//Parameters
void SetFreq(float freqIn);
float GetFreq(void){return m_freq;};

//Use
void ClockProcess(float* LeftSample, float* RightSample);
void Average(float* input, float* level, float dec);
void SoftClip(float* input, float* output);

private:
float m_freq, gain, rms;
Eigen::MatrixXd A, U, S, V;
Soap* filter0[ORDER];
float fb0;
};
