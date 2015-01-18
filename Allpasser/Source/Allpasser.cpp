/*
Allpassser.cpp - Apply dynamics processing to singular values of transformation matrix.
Written by Joe Mariglio, 1/16/15
*/

#include "Allpasser.h"

using namespace std;
using namespace Eigen;

Allpasser::Allpasser()
{
    for(int i=0; i<ORDER; i++)
    {
        filter0[i] = new Soap();
        filter0[i]->rq = 0.6;
    }
    SetDrive(1.0f);

}

Allpasser::~Allpasser()
{
}

void Allpasser::SetDrive(float drive)
{
    m_drive=drive+1e-4;
    gain=1/exp(drive*0.3);
    filter0[0]->w0 = (double)(m_drive*M_PI*0.125*0.25);
    filter0[0]->calcCoefsAP();
    cout<<filter0[0]->w0<<endl;
}

void Allpasser::Average(float* input, float* level, float dec)
{
    *level = dec*(*level) + (1-dec)*(*input);
}

void Allpasser::SoftClip(float* input, float* output)
{
    *output = tanh(*input);
}

void Allpasser::ClockProcess(float* LeftSample, float* RightSample)
{
    float inputL = *LeftSample;
    filter0[0]->apply(&inputL);
    inputL += *LeftSample;
    SoftClip(&inputL, LeftSample);
    SoftClip(&inputL, RightSample);
}

