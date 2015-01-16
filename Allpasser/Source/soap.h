// soap.h - implements a second-order allpass filter

class Soap
{
public:
    double* inBuf;
    double* outBuf;
    Soap();
    ~Soap();
    void calcCoefs();
    void init();
    void freeBuffers();
    void clearBuffers();
    void apply();
}
