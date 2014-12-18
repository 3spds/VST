//-------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
// Version 2.4		
//
// Category     : VST 2.x SDK Samples
// Filename     : spect.h
// Created by   : 3spds
// Description  : applies a discrete wavelet transform
//					multiband dynamics processing
//					multiband delay w/ feedback
//					inverse discrete wavelet transform
//					profit
//-------------------------------------------------------------------------------------------------------

#ifndef __spect__
#define __spect__

#include "/Users/josephmariglio/Documents/c++/2012/vstsdk2.4/public.sdk/source/vst2.x/audioeffectx.h"

float lens[11] = { 4, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048 };

// db4 coeffs:
#define C0 0.4829629131445341
#define C1 0.8365163037378079
#define C2 0.2241438680420134
#define C3 -0.1294095225512604

enum
{

	kNumPrograms = 16,
	kGateThresh = 0,
	kDuckThresh,
	kProbThresh,
	kWindowSize,
	kEQ1,
	kEQ2,
	kEQ3,
	kEQ4,
	kEQ5,
	kEQ6,
	kEQ7,
	kEQ8,
	kEQ9,
	kEQ10,
	kEQ11,
	kDelay1,
	kDelay2,
	kDelay3,
	kDelay4,
	kDelay5,
	kDelay6,
	kDelay7,
	kDelay8,
	kDelay9,
	kDelay10,
	kDelay11,
	kFeedback1,
	kFeedback2,
	kFeedback3,
	kFeedback4,
	kFeedback5,
	kFeedback6,
	kFeedback7,
	kFeedback8,
	kFeedback9,
	kFeedback10,
	kFeedback11,
	kSpectParameters	
};

enum
{
	kHamming,
	kVonHann,
	kBlackman,
	kKaiser
};

int windowSelected = 0;

enum
{
	kMonoMode,
	kMono2StereoMode,
	kStereoMode
};

int channelMode = kStereoMode;
class DaubProgram
{
friend class Spect;
public:
	DaubProgram();
	~DaubProgram();
private:
	float fGateThresh;
	float fDuckThresh;
	float fProbThresh;
	float fWindowSize;
	float feqs[11];
	float fsizes[11];
	float ffbcks[11];
	char name[24];
};
//-------------------------------------------------------------------------------------------------------
class Spect : public AudioEffectX
{
public:
	Spect (audioMasterCallback audioMaster, long pNumPrograms, long pNumParams);
	~Spect ();

	// Processing
//	virtual void processActual(float **inputs, float **outputs, long sampleframes, bool replacing);
	virtual void processBlock();
	virtual void processDoubleBlock();
	virtual void processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames);
	virtual void processDoubleReplacing (double** inputs, double** outputs, VstInt32 sampleFrames);
	virtual void suspend();
	virtual void resume();
	virtual bool setBypass(bool pBypass);
//	virtual void setNumInputs(int inputs);

	// Program
	virtual void setProgramName (char* name);
	virtual void getProgramName (char* name);
	virtual void setProgram (VstInt32 program);
	virtual bool getProgramNameIndexed (VstInt32 category, VstInt32 index, char* text);

	// Parameters
	virtual void setParameter (VstInt32 index, float value);
	virtual float getParameter (VstInt32 index);
	virtual void getParameterLabel (VstInt32 index, char* label);
	virtual void getParameterDisplay (VstInt32 index, char* text);
	virtual void getParameterName (VstInt32 index, char* text);

	virtual bool getEffectName (char* name);
	virtual bool getVendorString (char* text);
	virtual bool getProductString (char* text);

//	virtual VstInt32 getVendorVersion ();
	
	//methods
//	void processActual(float **inputs, float **outputs, long sampleframes, bool replacing);
	bool allocateMemory();
	void processSpect();
//	void processSignal();
	void daub(float input[], float output[], unsigned long n, int isign);
	
	void pwtset(int n);		//initializing routine. must be called prior to pwt
	void pwt(float input[], float output[], unsigned long n);		//applies single wavelet filter
	void wt1(float input[], float output[], unsigned long n, void(Spect::*pwt)(float[], float[], unsigned long));		//implement the "pyramid algorithm" with recursive calls to pwt
	void ipwt(float input[], float output[], unsigned long n);		//applies single wavelet filter
	void iwt1(float input[], float output[], unsigned long n, void(Spect::*ipwt)(float[], float[], unsigned long));		//implement the "pyramid algorithm" with recursive calls to pwt
	
	void initHammingWindows();
	void initVonHannWindows();
	void initBlackmanWindows();
	void initKaiserWindows();
	float kaiserIno(float x);
	void scaleWindows();
//	void setInitialDelay(int sizeDWT);
	void freeMemory();
	void setDWTSize(long int);
	void updateDWT();
	

protected:
	int numOutputs;
	int numInputs;
	bool bypass;
	
	//inputs...
	float *inBufferL;
	float *inBufferR;
	
	//outputs...
	float *outBufferL;
	float *outBufferR;
	
	//inDWT...
	float *inDWTL;
	float *inDWTR;
	
	//outDWT...
	float *outDWTL;
	float *outDWTR;
	
	//inShift...
	float *inShiftL;
	float *inShiftR;
	
	float * dwtInputL;
	float * dwtInputR;
	float * dwtOutputL;
	float * dwtOutputR;

	
	//outShift...
	float *outShiftL;
	float *outShiftR;
	
	//shared workspace for calculating DWT...
	float *wksp;
	//float **wksp;
	
	//inSpectra... (yes i know it's not actually a *spectrum* unless it's fourier. close enough though...)
	float *inSpectraL;
	float *inSpectraR;
	
	//outSpectra... (ditto. functionally identical to fourier.)
	float *outSpectraL;
	float *outSpectraR;
	
	//delay buffers
	int kMaxDelSize;
	float **dbufsL;
	float **dbufsR;
	
	//cursors...
	long *cursorsL;
	long *cursorsR;
	
	//sizes...
	long *sizes;
	
	//feedbacks...
	float *fbcks;	
	
	//windows...
	float *synthesisWindow;
	float *analysisWindow;
	
	//timers...
	long idleTimer;
	int bufferPosition;
	int inputTimeL;
	int inputTimeR;
	int outputTimeL;
	int outputTimeR;
	
	//params
	float paramA;
	float paramB;
	float paramC;
	float paramD;
	
	float *eqs;

	long windowSize;
	
	//blocking
	int kMaxSizeDWT;
	int sizeDWT;
	int kSizeDWT;
	int blockSize;
	int halfSizeDWT;
	float oneOverBlockSize;
	float log2n;
	
	//pi
	float pi;
	float twoPi;
	
	//program
	char programName[kVstMaxProgNameLen + 1];
	
	DaubProgram *programs;
	
};

#endif
