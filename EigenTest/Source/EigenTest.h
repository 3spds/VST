/*
EigenTest.h - waveshape with an exponential function.
Will eventually include some nifty linear algebra stuff, courtesy Eigen...
Written by Joe Mariglio, 11/25/14
*/
#include "Eigen/Dense"
#include "../JuceLibraryCode/JuceHeader.h"
#include "math.h"
#include "stdio.h"

#define Order 8
#define Fraction 4

class EigenTest
{
public:
EigenTest();
~EigenTest();

//Parameters
void SetDrive(float drive);
float GetDrive(void){return m_drive;};

//Use
void ClockProcess(float* LeftSample, float* RightSample);
void Average(float* input, float* level, float dec);
void SoftClip(float* input, float* output);
void AudioSVD(AudioSampleBuffer& buffer, int channel);
private:
float m_drive, gain, rms;
Eigen::MatrixXd A, U, S, V;
};

