#include "ExpDist.h"
#include "math.h"

ExpDist::ExpDist(){SetDrive(1.0f);}
ExpDist::~ExpDist(){}

void ExpDist::SetDrive(float drive)
{
    m_drive=drive;
    gain=1/exp(drive*0.3);
}

void ExpDist::Average(float* input, float* level, float dec)
{
    *level = dec*(*level) + (1-dec)*(*input);
}

void ExpDist::SoftClip(float* input, float* output)
{
    *output = tanh(*input);
}

void ExpDist::ClockProcess(float* LeftSample, float* RightSample)
{
    float left = *LeftSample;
    float right = *RightSample;
    float lr = 0.0f;
    left = gain*(exp(m_drive*left)-1.0f);
    right = gain*(exp(m_drive*right)-1.0f);
    lr = (left+right)*0.5;
    lr = sqrt(pow(lr, 2));
    Average(&lr, &rms, 0.3);
    left = left*(1.0f/(rms+0.2));
    right = right*(1.0f/(rms+0.2));
    SoftClip(&left, &left);
    SoftClip(&right, &right);
    *LeftSample=left;
    *RightSample=right;
}

