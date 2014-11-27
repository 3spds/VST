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

void EigenTest::AudioSVD(AudioSampleBuffer& buffer, int channel, int offset)
{
    int length = buffer.getNumSamples();
    int m = (length/Fraction)-Order;
    float* pointer = buffer.getWritePointer(channel);
    VectorXd avg = VectorXd::Zero(Order);

    //now that we know the length of the buffer, we can resize A, U, S, & V
    A.resize(m, Order);
    U.resize(m, Order);
    S.resize(Order, 1);
    V.resize(Order, length);

    //fill A with input vector

    for(int j=0;j<Order;j++)
    {
        for(int i=0;i<m;i++)
        {
            A(i, j) = pointer[i+j+offset];
            //update average
            avg(j) += (pointer[i+j+offset]/(double)m);
        }
        A.col(j) -= (VectorXd::Ones(m) * avg(j) );
    }

    //calculate svd:
    Eigen::JacobiSVD<Eigen::MatrixXd> svd(A, ComputeThinU | ComputeThinV);

    S = svd.singularValues();
    V = svd.matrixV();
    V.transposeInPlace();
    U = svd.matrixU();
    for(int i=0;i<Order;i++)
    {
       if(fabs(S(i))<m_drive){S(i) = 0;};
    }

    //print for testing
    A = U * S.asDiagonal() * V;
//    cout<<length/2<<endl;
//    cout<<A.block(0, 0,(length/2)-Order,1)<<endl;
//    cout<<A.col(0)<<endl;
//    cout<<(U * S * V).array()<<endl;
//    cout<<endl;
//    cout<<endl;

    A.rowwise() += avg.transpose();

    //copy the first (length/Order) - Order samples
    for(int i=0;i<m;i++)
    {
        pointer[i+offset] = (A.col(0)).array()(i);
//        cout<<i+offset<<endl;
    }
    //then copy the last Order samples
    for(int i=0;i<Order;i++)
    {
        pointer[m+i+offset] = A.array()(m-Order+i, Order-1);
//        cout<<m+i+offset<<endl;
    }
}

void EigenTest::ClockProcess(float* LeftSample, float* RightSample)
{

}

