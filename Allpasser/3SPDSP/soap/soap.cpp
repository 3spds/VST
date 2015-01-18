// soap.cpp - implements a second order allpass filter
#include "./soap.h"
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <vector>

using namespace std;

Soap::Soap()
{
    bufferSize = 3;
    bufferIndex = 0;
    fbBuf = new double[bufferSize]();
    ffBuf = new double[bufferSize]();
    fbCoef = new double[bufferSize]();
    ffCoef = new double[bufferSize]();
    Soap::init();
}

Soap::~Soap()
{
    delete[] fbBuf;
    delete[] ffBuf;
    delete[] fbCoef;
    delete[] fbCoef;
}

void Soap::init()
{
    clearBuffers();
    this->w0 = M_PI*0.5;
    this->rq = 0.5;
    calcCoefs();
}

void Soap::calcCoefs()
{
    double cosw0, sinw0, alpha, b0, b1, b2, a0, a1, a2, ia0;

    cosw0 = cos(w0);
    sinw0 = sin(w0);
    alpha = sinw0*0.5*rq;

    b0 = rq*alpha;
    b1 = 0;
    b2 = -1*rq*alpha;

    a0 = 1.0+alpha;
    a1 = -2.0*cosw0;
    a2 = 1.0-alpha;
    ia0 = 1.0/a0;

    ffCoef[0] = b0 * ia0;
    ffCoef[1] = b1 * ia0;
    ffCoef[2] = b2 * ia0;
    fbCoef[0] = 1.0;
    fbCoef[1] = a1 * ia0;
    fbCoef[2] = a2 * ia0;
}

void Soap::calcCoefsAP()
{
    double cosw0, sinw0, alpha, b0, b1, b2, a0, a1, a2, ia0;

    cosw0 = cos(w0);
    sinw0 = sin(w0);
    alpha = sinw0*0.5*rq;

    b0 = 1.0-alpha;
    b1 = -2.0*cosw0;
    b2 = 1.0+alpha;

    a0 = 1.0+alpha;
    a1 = -2.0*cosw0;
    a2 = 1.0-alpha;
    ia0 = 1.0/a0;

    ffCoef[0] = b0 * ia0;
    ffCoef[1] = b1 * ia0;
    ffCoef[2] = b2 * ia0;
    fbCoef[0] = 1.0;
    fbCoef[1] = a1 * ia0;
    fbCoef[2] = a2 * ia0;
}

void Soap::clearBuffers()
{
    for(int i = 0; i<bufferSize; i++)
    {
        fbBuf[i] = ffBuf[i] = fbCoef[i] = ffCoef[i] = 0.0;
    }
}

void Soap::apply(float* signal)
{
    double dsignal = *signal;
    ffBuf[bufferIndex] = dsignal;
    fbBuf[bufferIndex] = ffCoef[0] * ffBuf[bufferIndex]
        + ffCoef[1] * ffBuf[(bufferSize + bufferIndex - 1)%bufferSize]
        + ffCoef[2] * ffBuf[(bufferSize + bufferIndex - 2)%bufferSize]
        - fbCoef[1] * fbBuf[(bufferSize + bufferIndex - 1)%bufferSize]
        - fbCoef[2] * fbBuf[(bufferSize + bufferIndex - 2)%bufferSize];
    dsignal = fbBuf[bufferIndex];
    *signal = (float)dsignal;
    bufferIndex ++;
    bufferIndex %= bufferSize;
}
