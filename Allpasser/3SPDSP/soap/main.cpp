// main.cpp - tester function for soap
#include "./soap.cpp"
#include <iostream>
#include <cstdlib>
#include <vector>
#include <time.h>

using namespace std;

void randArray(double* vec, int length){
    srand(time(NULL));
    for(int i=0; i<length; i++){
        vec[i] = (2*((double)rand()/(double)RAND_MAX))-1;
    }
}

void printArray(double* vec, int length){
    for(int i=0; i<length; i++){
        cout<<vec[i]<<" ";
    }
    cout<<endl;
}

int main()
{
    int length = 16;
    double* vec = new double[length]();
    randArray(vec, length);
    printArray(vec, length);
    cout<<"FU!"<<endl;
    Soap* filter0 = new Soap();
    for(int index = 0; index < length; index++)
    {
        filter0->apply(&vec[index]);
        //printArray(filter0->ffBuf, 3);
    }
    printArray(vec, length);
    return 0;
}
