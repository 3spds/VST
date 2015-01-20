/*
Allpassser.cpp - Apply 400th order allpass filter cascade, with feedback.
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
    filterFB = new Soap();
    fb0 = 0.0;
    SetDrive(1.0f);
}

Allpasser::~Allpasser()
{
}

void Allpasser::SetDrive(float drive)
{
    double freq = (double)(m_drive*M_PI*0.125*0.25);

    m_drive=drive+1e-4;
    gain=1/exp(drive*0.3);
    for(int i=0; i<ORDER; i++)
    {
        filter0[i]->w0 = freq;
        filter0[i]->calcCoefsAP();
    }
    //cout<<filter0[0]->w0<<endl;
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
    filterFB->apply(&fb0);
    SoftClip(&fb0, &fb0);
// still tuning the gain params here. eventually there could
//    be presets or something... jm
//    inputL *= 1;
    inputL += (fb0*1.01)*((inputL + 1 )*1);
    for(int i=0; i<ORDER; i++)
    {
        filter0[i]->apply(&inputL);
    }
//    inputL += *LeftSample;
    SoftClip(&inputL, LeftSample);
    SoftClip(&inputL, RightSample);
    fb0 = *LeftSample;
}

