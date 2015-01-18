/*
Allpasser.h - Apply cascades of allpass filters.
Written by Joe Mariglio, 1/16/15
*/
#include "../3SPDSP/soap/soap.h"
#include "Eigen/Dense"
#include "../JuceLibraryCode/JuceHeader.h"
#include "math.h"
#include "stdio.h"

#define ORDER 100

class Allpasser
{
public:
Allpasser();
~Allpasser();


//Parameters
void SetDrive(float drive);
float GetDrive(void){return m_drive;};

//Use
void ClockProcess(float* LeftSample, float* RightSample);
void Average(float* input, float* level, float dec);
void SoftClip(float* input, float* output);

private:
float m_drive, gain, rms;
Eigen::MatrixXd A, U, S, V;
Soap* filter0[ORDER];
};
