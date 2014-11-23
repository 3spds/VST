/*
ExpDist.h - waveshape with an exponential function.
Written by Joe Mariglio, 11/22/14
"drive" is the pre-shaper gain.
*/

class ExpDist
{
public:
ExpDist();
~ExpDist();

//Parameters
void SetDrive(float drive);
float GetDrive(void){return m_drive;};

//Use
void ClockProcess(float* LeftSample, float* RightSample);
void Average(float* input, float* level, float dec);
void SoftClip(float* input, float*output);
private:
float m_drive, gain, rms;
};

