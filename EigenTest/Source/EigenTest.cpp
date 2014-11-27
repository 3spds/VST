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
    int m = (length/Fraction)-Order;
    float* pointer = buffer.getWritePointer(channel);
    VectorXd avg = VectorXd::Zero(Order);

    //now that we know the length of the buffer, we can resize A, U, S, & V
    A.resize(m, Order);
    U.resize(length, length);
    S.resize(length, 1);
    V.resize(length, length);

    //fill A with input vector
    for(int i=0;i<m;i++)
    {
        for(int j=0;j<Order;j++)
        {
            A(i, j) = pointer[i+j];
            //update average
            avg(j) += (pointer[i+j]/(length/Fraction));
        }
        for(int j=0;j<Order;j++)
        {
            A.col(j) -= (VectorXd::Ones(m) * avg(j) );
        }
    }

    //calculate svd:
    Eigen::JacobiSVD<Eigen::MatrixXd> svd(A, ComputeThinU | ComputeThinV);

    S = svd.singularValues();
    V = svd.matrixV();
    V.transposeInPlace();
    U = svd.matrixU();
    for(int i=0;i<Order;i++)
    {
       if(fabs(S(i))<0.0000001){S(i) = 0;};
    }

    //print for testing
    A = U * S.asDiagonal() * V;
//    cout<<length/2<<endl;
//    cout<<A.block(0, 0,(length/2)-Order,1)<<endl;
//    cout<<A.col(0)<<endl;
//    cout<<(U * S * V).array()<<endl;
//    cout<<endl;
//    cout<<endl;
    for(int i=0;i<length/Order;i++)
    {
        for(int j=0;j<Order;j++)
        {
            pointer[(i*Order)+j] =  A(i%m,j) + avg(j);
        }
//        pointer[i] = (A.col(0)).array()(i%m);
    }
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

