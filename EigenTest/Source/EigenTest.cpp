/*
EigenTest.c - waveshape with an exponential function.
Will eventually include some nifty linear algebra stuff, courtesy Eigen...
Written by Joe Mariglio, 11/25/14
*/

#include "EigenTest.h"

using namespace std;
using namespace Eigen;

EigenTest::EigenTest(){SetDrive(1.0f);}
EigenTest::~EigenTest(){}

void EigenTest::SetDrive(float drive)
{
    m_drive=drive;
    gain=1/exp(drive*0.3);
}

void EigenTest::Average(float* input, float* level, float dec)
{
    *level = dec*(*level) + (1-dec)*(*input);
}

void EigenTest::SoftClip(float* input, float* output)
{
    *output = tanh(*input);
}

void EigenTest::AudioSVD(AudioSampleBuffer& buffer, int channel)
{
    int length = buffer.getNumSamples();
    float* pointer = buffer.getWritePointer(channel);
    VectorXd avg = VectorXd::Zero(Order);

    //now that we know the length of the buffer, we can resize A, U, S, & V
    A.resize((length/2)-Order, Order);
    U.resize(length-1, length-1);
    S.resize(length-1, length-1);
    V.resize(length-1, length-1);

    //fill A with input vector
    for(int i=0;i<(length/2)-Order;i++)
    {
        for(int j=0;j<Order;j++)
        {
            A(i, j) = pointer[i+j];
            //update average
            avg(j) += (pointer[i+j]/(length/2));
        }
    }
    //calculate svd:
    Eigen::JacobiSVD<Eigen::MatrixXd> svd(A, ComputeThinU | ComputeThinV);
    //print for testing
//    cout<<length/2<<endl;
    cout<<A.block(0, 0,(length/2)-Order,1)<<endl;
//    cout<<endl;
}

void EigenTest::ClockProcess(float* LeftSample, float* RightSample)
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

