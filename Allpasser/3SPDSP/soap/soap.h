// soap.h - implements a second-order allpass filter

class Soap
{
public:
    double* fbBuf;
    double* ffBuf;
    double* fbCoef; //b's
    double* ffCoef; //a's
    int bufferSize;
    int bufferIndex;
    double w0;
    double rq;
    Soap();
    ~Soap();
    void calcCoefs();
    void calcCoefsAP();
    void init();
    void freeBuffers();
    void clearBuffers();
    void apply(float* signal);
};
