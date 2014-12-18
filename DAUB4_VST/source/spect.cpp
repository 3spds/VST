//-------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
// Version 2.4		
//
// Category     : VST 2.x SDK Samples
// Filename     : spect.cpp
// Created by   : 3spds
// Description  : applies a discrete wavelet transform (variable basis)
//					multiband dynamics processing
//					multiband delay w/ feedback
//					inverse discrete wavelet transform
//					profit
//-------------------------------------------------------------------------------------------------------


#include "spect.h"
#include <math.h>
#include <stdio.h>
#include <time.h>
#include "AEffEditor.h"
#include "/Users/josephmariglio/Documents/c++/2012/vstsdk2.4/public.sdk/source/vst2.x/audioeffectx.h"
#include <stdlib.h>

//-----------------------------------------------------------------------------

typedef struct{
	int ncof, ioff, joff;
	float *cc, *cr;
} wavefilt;

wavefilt wfilt;

//-----------------------------------------------------------------------------


DaubProgram::DaubProgram()
{
	//Default program values
	fGateThresh = 0.0f;
	fDuckThresh = 1.0f;
	fProbThresh = 1.0f;
	fWindowSize = 1.0f;
	for(int i = 0; i<11; i++) 
	{
		feqs[i] = 1.0f;
		fsizes[i] = 1.0f;
		ffbcks[i] = 1.0f;
	}
	strcpy(name, "Init");
}

//-----------------------------------------------------------------------------


AudioEffect* createEffectInstance(audioMasterCallback audioMaster) {
	return new Spect(audioMaster, 8, kSpectParameters);
}


//-----------------------------------------------------------------------------
Spect::Spect(audioMasterCallback audioMaster, long pNumPrograms, long pNumParams)
: AudioEffectX(audioMaster, pNumPrograms, pNumParams)	// 1 program, as many parameters as are defined
{
	numOutputs = 2;
	numInputs = 2;
	
	
	outBufferL = outBufferR = inBufferL = inBufferR = 0;
	inDWTL = inDWTR = inShiftL = inShiftR = outShiftL = outShiftR = 0;
	inSpectraL = inSpectraR = outSpectraL = outSpectraR = 0;
	synthesisWindow = analysisWindow = 0;
	idleTimer = 0;
	bufferPosition = 0;
	inputTimeL = outputTimeL = 0;
	inputTimeR = outputTimeR = 0;
	
	paramB = paramC = paramD = 1.0f;
	paramA = 0.0f;
	windowSize = 4096;
	kMaxSizeDWT = 4096;
	kMaxDelSize = sampleRate;
	kSizeDWT = kMaxSizeDWT;
	sizeDWT = kSizeDWT;
	blockSize = sizeDWT >> 3;
	halfSizeDWT = sizeDWT >> 1;
	oneOverBlockSize = 1.0f/(float)blockSize;
	
	allocateMemory();
	//pwtset(20);
	pi = 4.0f * atanf(1.0f);
	twoPi = 8.0f * atanf(1.0f);
	
	// make the windows
	initHammingWindows();
	scaleWindows();
	setInitialDelay(sizeDWT);
	setNumInputs(2);		// stereo in
	setNumOutputs(2);		// stereo out
	setBypass(false);
	setUniqueID('daub');	// identify - only needed when not base class.
	//	canMono(); // allowing one input or one output
	canProcessReplacing();	// supports both accumulating and replacing output
	strcpy(programName, "Default");	// default program name
	suspend ();		// flush buffer
}

//-----------------------------------------------------------------------------------------
Spect::~Spect()
{
	freeMemory();
}

bool Spect::allocateMemory()
{
	inBufferL = 0;
	inBufferR = 0;
	inBufferL = (float *) malloc(kMaxSizeDWT*sizeof(float));
	inBufferR = (float *) malloc(kMaxSizeDWT*sizeof(float));
	inDWTL = 0;
	inDWTR = 0;
	inDWTL = (float *) malloc(kMaxSizeDWT*sizeof(float));
	inDWTR = (float *) malloc(kMaxSizeDWT*sizeof(float));
	inShiftL = 0;
	inShiftR = 0;
	inShiftL = (float *) malloc(kMaxSizeDWT*sizeof(float));
	inShiftR = (float *) malloc(kMaxSizeDWT*sizeof(float));
	inSpectraL = 0;
	inSpectraR = 0;
	inSpectraL = (float *) malloc(kMaxSizeDWT*sizeof(float));
	inSpectraR = (float *) malloc(kMaxSizeDWT*sizeof(float));
	outSpectraL = 0;
	outSpectraL = (float *) malloc(kMaxSizeDWT*sizeof(float));
	outSpectraR = 0;
	outSpectraR = (float *) malloc(kMaxSizeDWT*sizeof(float));
	outShiftL = 0;
	outShiftL = (float *) malloc(kMaxSizeDWT*sizeof(float));
	outShiftR = 0;
	outShiftR = (float *) malloc(kMaxSizeDWT*sizeof(float));
	outBufferL = 0;
	outBufferL = (float *) malloc(kMaxSizeDWT*sizeof(float));
	outBufferR = 0;
	outBufferR = (float *) malloc(kMaxSizeDWT*sizeof(float));
	synthesisWindow = 0;
	synthesisWindow = (float *) malloc(kMaxSizeDWT*sizeof(float));
	analysisWindow = 0;
	analysisWindow = (float *) malloc(kMaxSizeDWT*sizeof(float));
	wksp = 0;
	wksp = (float *) malloc(kMaxSizeDWT*sizeof(float));
	dwtInputL = new float[kMaxSizeDWT];
	dwtInputR = new float[kMaxSizeDWT];
	dwtOutputL = new float[kMaxSizeDWT];
	dwtOutputR = new float[kMaxSizeDWT];
	eqs = new float[10];
	dbufsL = new float*[11];
	dbufsR = new float*[11];
	for(int i = 0; i<11; i++)
	{
		dbufsL[i] = new float[kMaxDelSize];
		dbufsR[i] = new float[kMaxDelSize];
	}
	
	cursorsL = new long[11];
	cursorsR = new long[11];
	
	sizes = new long[11];
	
	fbcks = new float[11];
	
	programs = new DaubProgram[numPrograms];
	
	for(int i = 0; i<kMaxSizeDWT; i++)
	{
		inBufferL[i] = inBufferR[i] = outBufferL[i] = outBufferR[i] = 0.0f;
		inShiftL[i] = inShiftR[i] = outShiftL[i] = outShiftR[i] = 0.0f;
		inSpectraL[i] = inSpectraR[i] = outSpectraL[i] = outSpectraR[i] = 0.0f;
		wksp[i] = 0.0f;
		dwtInputL[i] = dwtInputR[i] = dwtOutputL[i] = dwtOutputR[i] = 0.0f;
	}
	for(int i = 0; i<11; i++) 
	{
		eqs[i] = 1.0f;
	}
	for(int i = 0; i<11; i++)
	{
		for(int j = 0; j<kMaxDelSize; j++)
		{
			dbufsL[i][j] = 0.0f; //initialize delay lines
			dbufsR[i][j] = 0.0f; //initialize delay lines
		}
		sizes[i] = sampleRate; //initialize delay times
		cursorsL[i] = 0; //initialize cursor positions
		cursorsR[i] = 0; //initialize cursor positions
		fbcks[i] = 1.0f; //initialize feedback amounts
	}
	
	return(true);
}

void Spect::freeMemory()
{
	if(inBufferL != 0) {free(inBufferL); inBufferL = 0; }
	if(inBufferR != 0 ) {free(inBufferR); inBufferR = 0;}
	if(inDWTL != 0 ) {free(inDWTL); inDWTL = 0;}
	if(inDWTR != 0 ) {free(inDWTR); inDWTR = 0;}
	if(outBufferL != 0) {free(outBufferL); outBufferL = 0;}
	if(outBufferR != 0 ) {free(outBufferR); outBufferR = 0;}
	if(inShiftL != 0 ) {free(inShiftL); inShiftL = 0;}
	if(inShiftR != 0 ) {free(inShiftR);inShiftR = 0;}
	if(outShiftL != 0 ) {free(outShiftL);outShiftL = 0;}
	if(outShiftR != 0 ) {free(outShiftR);outShiftR = 0;}
	if(inSpectraL != 0 ) {free(inSpectraL);inSpectraL = 0;}
	if(inSpectraR != 0 ) {free(inSpectraR);inSpectraR = 0;}
	if(outSpectraL != 0 ) {free(outSpectraL);outSpectraL = 0;}
	if(outSpectraR != 0 ) {free(outSpectraR);outSpectraR = 0;}
	if(analysisWindow != 0) {free(analysisWindow); analysisWindow = 0;}
	if(synthesisWindow != 0) {free(synthesisWindow); synthesisWindow = 0;}
	if(wksp != 0) {free(wksp); wksp = 0;}
	delete [] dwtInputL;
	delete [] dwtInputR;
	delete [] dwtOutputL;
	delete [] dwtOutputR;
	delete [] eqs;
	delete [] lens;
	for(int i=0; i<11; i++)
		delete [] dbufsL[i];
	delete [] dbufsL;
	for(int i=0; i<11; i++)
		delete [] dbufsR[i];
	delete [] dbufsR;
	delete [] sizes;
	delete [] cursorsL;
	delete [] cursorsR;
}



void Spect::suspend()
{
	long i;
	for(i = 0; i<kMaxSizeDWT; i++)
	{
		inBufferL[i] = inBufferR[i] = outBufferL[i] = outBufferR[i] = 0.0f;
		inShiftL[i] = inShiftR[i] = outShiftL[i] = outShiftR[i] = 0.0f;
		inSpectraL[i] = inSpectraR[i] = outSpectraL[i] = outSpectraR[i] = 0.0f;
		wksp[i] = 0.0f;
	}
	for(int i = 0; i<11; i++)
	{
		for(int j = 0; j<kMaxDelSize; j++)
		{
			dbufsL[i][j] = 0.0f;
			dbufsR[i][j] = 0.0f;
		}
	}
}

void Spect::resume()
{
	//	wantEvents();
}

//-----------------------------------------------------------------------------------------
bool Spect::getEffectName (char* name)
{
	strcpy (name, "DB4M");
	return true;
}

//-----------------------------------------------------------------------------------------
bool Spect::getProductString (char* text)
{
	strcpy (text, "DB4M");
	return true;
}

//-----------------------------------------------------------------------------------------
bool Spect::getVendorString (char* text)
{
	strcpy (text, "3SPDS");
	return true;
}

//-----------------------------------------------------------------------------------------
void Spect::setProgramName(char *name)
{
	strcpy(programs[curProgram].name, name);
}

//-----------------------------------------------------------------------------------------

void Spect::setProgram(VstInt32 program)
{
	DaubProgram* ap = &programs[program];
	curProgram = program;
	setParameter(kGateThresh, ap->fGateThresh);
	setParameter(kDuckThresh, ap->fDuckThresh);
	setParameter(kProbThresh, ap->fProbThresh);
	setParameter(kWindowSize, ap->fWindowSize);
	setParameter(kEQ1, ap->feqs[0]);
	setParameter(kEQ2, ap->feqs[1]);
	setParameter(kEQ3, ap->feqs[2]);
	setParameter(kEQ4, ap->feqs[3]);
	setParameter(kEQ5, ap->feqs[4]);
	setParameter(kEQ6, ap->feqs[5]);
	setParameter(kEQ7, ap->feqs[6]);
	setParameter(kEQ8, ap->feqs[7]);
	setParameter(kEQ9, ap->feqs[8]);
	setParameter(kEQ10, ap->feqs[9]);
	setParameter(kEQ11, ap->feqs[10]);
	setParameter(kDelay1, ap->fsizes[0]);
	setParameter(kDelay2, ap->fsizes[1]);
	setParameter(kDelay3, ap->fsizes[2]);
	setParameter(kDelay4, ap->fsizes[3]);
	setParameter(kDelay5, ap->fsizes[4]);
	setParameter(kDelay6, ap->fsizes[5]);
	setParameter(kDelay7, ap->fsizes[6]);
	setParameter(kDelay8, ap->fsizes[7]);
	setParameter(kDelay9, ap->fsizes[8]);
	setParameter(kDelay10, ap->fsizes[9]);
	setParameter(kDelay11, ap->fsizes[10]);
	setParameter(kFeedback1, ap->ffbcks[0]);
	setParameter(kFeedback2, ap->ffbcks[1]);
	setParameter(kFeedback3, ap->ffbcks[2]);
	setParameter(kFeedback4, ap->ffbcks[3]);
	setParameter(kFeedback5, ap->ffbcks[4]);
	setParameter(kFeedback6, ap->ffbcks[5]);
	setParameter(kFeedback7, ap->ffbcks[6]);
	setParameter(kFeedback8, ap->ffbcks[7]);
	setParameter(kFeedback9, ap->ffbcks[8]);
	setParameter(kFeedback10, ap->ffbcks[9]);
	setParameter(kFeedback11, ap->ffbcks[10]);
}

void Spect::getProgramName (char *name)
{
	if (!strcmp (programs[curProgram].name, "Init"))
		sprintf (name, "%s %d", programs[curProgram].name, curProgram + 1);
	else
		strcpy (name, programs[curProgram].name);
}

//-----------------------------------------------------------------------------------------
bool Spect::getProgramNameIndexed (VstInt32 category, VstInt32 index, char* text)
{
	if (index < kNumPrograms)
	{
		strcpy (text, programs[index].name);
		return true;
	}
	return false;
}
//-----------------------------------------------------------------------------------------
void Spect::setParameter(VstInt32 index, float value)
{
	
	DaubProgram* ap = &programs[curProgram];
	
	switch(index)
	{
		case kGateThresh:
			paramA = value;
			break;
		case kDuckThresh:
			paramB = value;
			break;
		case kProbThresh:
			paramC = value;
			break;
		case kWindowSize: //stepped!
			if(value < 0.25f) 
			{
				paramD = 0.0f;
				windowSize = 512;
			} else if(value < 0.5f) {
				paramD = 0.333f;
				windowSize = 1024;
			} else if(value < 0.75f) {
				paramD = 0.666f;
				windowSize = 2048;
			} else if(value < 1.0f) {
				paramD = 1.0f;
				windowSize = 4096;
			}
			setDWTSize(windowSize);
			break;
			//- EQs
		case kEQ1:
			eqs[0] = value;
			break;
		case kEQ2:
			eqs[1] = value;
			break;
		case kEQ3:
			eqs[2] = value;
			break;
		case kEQ4:
			eqs[3] = value;
			break;
		case kEQ5:
			eqs[4] = value;
			break;
		case kEQ6:
			eqs[5] = value;
			break;
		case kEQ7:
			eqs[6] = value;
			break;
		case kEQ8:
			eqs[7] = value;
			break;
		case kEQ9:
			eqs[8] = value;
			break;
		case kEQ10:
			eqs[9] = value;
			break;
		case kEQ11:
			eqs[10] = value;
			break;
			//- Delay Times
		case kDelay1:
			sizes[0] = value*sampleRate;
			break;
		case kDelay2:
			sizes[1] = value*sampleRate;
			break;
		case kDelay3:
			sizes[2] = value*sampleRate;
			break;
		case kDelay4:
			sizes[3] = value*sampleRate;
			break;
		case kDelay5:
			sizes[4] = value*sampleRate;
			break;
		case kDelay6:
			sizes[5] = value*sampleRate;
			break;
		case kDelay7:
			sizes[6] = value*sampleRate;
			break;
		case kDelay8:
			sizes[7] = value*sampleRate;
			break;
		case kDelay9:
			sizes[8] = value*sampleRate;
			break;
		case kDelay10:
			sizes[9] = value*sampleRate;
			break;
		case kDelay11:
			sizes[10] = value*sampleRate;
			break;
			//- Feedbacks
		case kFeedback1:
			fbcks[0] = value;
			break;
		case kFeedback2:
			fbcks[1] = value;
			break;
		case kFeedback3:
			fbcks[2] = value;
			break;
		case kFeedback4:
			fbcks[3] = value;
			break;
		case kFeedback5:
			fbcks[4] = value;
			break;
		case kFeedback6:
			fbcks[5] = value;
			break;
		case kFeedback7:
			fbcks[6] = value;
			break;
		case kFeedback8:
			fbcks[7] = value;
			break;
		case kFeedback9:
			fbcks[8] = value;
			break;
		case kFeedback10:
			fbcks[9] = value;
			break;
		case kFeedback11:
			fbcks[10] = value;
			break;	
	}
	
	
}

//-----------------------------------------------------------------------------------------
float Spect::getParameter(VstInt32 index)
{
	switch(index)
	{
		case kGateThresh:
			return(paramA);
			break;
		case kDuckThresh:
			return(paramB);
			break;
		case kProbThresh:
			return(paramC);
			break;
		case kWindowSize:
			return(paramD);
			break;
			//- EQs
		case kEQ1:
			return(eqs[0]);
			break;
		case kEQ2:
			return(eqs[1]);
			break;
		case kEQ3:
			return(eqs[2]);
			break;
		case kEQ4:
			return(eqs[3]);
			break;
		case kEQ5:
			return(eqs[4]);
			break;
		case kEQ6:
			return(eqs[5]);
			break;
		case kEQ7:
			return(eqs[6]);
			break;
		case kEQ8:
			return(eqs[7]);
			break;
		case kEQ9:
			return(eqs[8]);
			break;
		case kEQ10:
			return(eqs[9]);
			break;
		case kEQ11:
			return(eqs[10]);
			break;
			//- delay times
		case kDelay1:
			return(sizes[0] / sampleRate);
			break;
		case kDelay2:
			return(sizes[1] / sampleRate);
			break;
		case kDelay3:
			return(sizes[2] / sampleRate);
			break;
		case kDelay4:
			return(sizes[3] / sampleRate);
			break;
		case kDelay5:
			return(sizes[4] / sampleRate);
			break;
		case kDelay6:
			return(sizes[5] / sampleRate);
			break;
		case kDelay7:
			return(sizes[6] / sampleRate);
			break;
		case kDelay8:
			return(sizes[7] / sampleRate);
			break;
		case kDelay9:
			return(sizes[8] / sampleRate);
			break;
		case kDelay10:
			return(sizes[9] / sampleRate);
			break;
		case kDelay11:
			return(sizes[10] / sampleRate);
			break;
			//- Feedbacks
		case kFeedback1:
			return(fbcks[0]);
			break;
		case kFeedback2:
			return(fbcks[1]);
			break;
		case kFeedback3:
			return(fbcks[2]);
			break;
		case kFeedback4:
			return(fbcks[3]);
			break;
		case kFeedback5:
			return(fbcks[4]);
			break;
		case kFeedback6:
			return(fbcks[5]);
			break;
		case kFeedback7:
			return(fbcks[6]);
			break;
		case kFeedback8:
			return(fbcks[7]);
			break;
		case kFeedback9:
			return(fbcks[8]);
			break;
		case kFeedback10:
			return(fbcks[9]);
			break;
		case kFeedback11:
			return(fbcks[10]);
			break;
		default:
			return(0.0f);
	}
}

//-----------------------------------------------------------------------------------------
void Spect::getParameterName(VstInt32 index, char *label)
{
	switch(index)
	{
		case kGateThresh:
			strcpy(label, "Gate Thresh: ");
			break;
		case kDuckThresh:
			strcpy(label, "Duck Thresh: ");
			break;
		case kProbThresh:
			strcpy(label, "Probability: ");
			break;
		case kWindowSize:
			strcpy(label, "Window Size: ");
			break;
			//- EQs
		case kEQ1:
			strcpy(label, "Basis Level: ");
			break;
		case kEQ2:
			strcpy(label, "8va 2 Level: ");
			break;
		case kEQ3:
			strcpy(label, "8va 3 Level: ");
			break;
		case kEQ4:
			strcpy(label, "8va 4 Level: ");
			break;
		case kEQ5:
			strcpy(label, "8va 5 Level: ");
			break;
		case kEQ6:
			strcpy(label, "8va 6 Level: ");
			break;
		case kEQ7:
			strcpy(label, "8va 7 Level: ");
			break;
		case kEQ8:
			strcpy(label, "8va 8 Level: ");
			break;
		case kEQ9:
			strcpy(label, "8va 9 Level: ");
			break;
		case kEQ10:
			strcpy(label, "8va 10 Level: ");
			break;
		case kEQ11:
			strcpy(label, "8va 11 Level: ");
			break;
			//- delay times
		case kDelay1:
			strcpy(label, "Basis Delay Time: ");
			break;
		case kDelay2:
			strcpy(label, "8va 2 Delay Time: ");
			break;
		case kDelay3:
			strcpy(label, "8va 3 Delay Time: ");
			break;
		case kDelay4:
			strcpy(label, "8va 4 Delay Time: ");
			break;
		case kDelay5:
			strcpy(label, "8va 5 Delay Time: ");
			break;
		case kDelay6:
			strcpy(label, "8va 6 Delay Time: ");
			break;
		case kDelay7:
			strcpy(label, "8va 7 Delay Time: ");
			break;
		case kDelay8:
			strcpy(label, "8va 8 Delay Time: ");
			break;
		case kDelay9:
			strcpy(label, "8va 9 Delay Time: ");
			break;
		case kDelay10:
			strcpy(label, "8va 10 Delay Time: ");
			break;
		case kDelay11:
			strcpy(label, "8va 11 Delay Time: ");
			break;
			//- Feedbacks	
		case kFeedback1:
			strcpy(label, "Basis FB: ");
			break;
		case kFeedback2:
			strcpy(label, "8va 2 FB: ");
			break;
		case kFeedback3:
			strcpy(label, "8va 3 FB: ");
			break;
		case kFeedback4:
			strcpy(label, "8va 4 FB: ");
			break;
		case kFeedback5:
			strcpy(label, "8va 5 FB: ");
			break;
		case kFeedback6:
			strcpy(label, "8va 6 FB: ");
			break;
		case kFeedback7:
			strcpy(label, "8va 7 FB: ");
			break;
		case kFeedback8:
			strcpy(label, "8va 8 FB: ");
			break;
		case kFeedback9:
			strcpy(label, "8va 9 FB: ");
			break;
		case kFeedback10:
			strcpy(label, "8va 10 FB: ");
			break;
		case kFeedback11:
			strcpy(label, "8va 11 FB: ");
			break;
	}
}

//-----------------------------------------------------------------------------------------
void Spect::getParameterDisplay(VstInt32 index, char *text)
{
	switch(index)
	{
		case kGateThresh:
			dB2string(paramA, text, 255);
			break;
		case kDuckThresh:
			dB2string(paramB, text, 255);
			break;
		case kProbThresh:
			float2string(paramC*100.0f, text, 255);
			break;
		case kWindowSize:
			int2string(windowSize, text, 255);
			break;
		case kEQ1:
			dB2string(eqs[0], text, 255);
			break;
		case kEQ2:
			dB2string(eqs[1], text, 255);
			break;
		case kEQ3:
			dB2string(eqs[2], text, 255);
			break;
		case kEQ4:
			dB2string(eqs[3], text, 255);
			break;
		case kEQ5:
			dB2string(eqs[4], text, 255);
			break;
		case kEQ6:
			dB2string(eqs[5], text, 255);
			break;
		case kEQ7:
			dB2string(eqs[6], text, 255);
			break;
		case kEQ8:
			dB2string(eqs[7], text, 255);
			break;
		case kEQ9:
			dB2string(eqs[8], text, 255);
			break;
		case kEQ10:
			dB2string(eqs[9], text, 255);
			break;
		case kEQ11:
			dB2string(eqs[10], text, 255);
			break;
			//- Delay Times				
		case kDelay1:
			float2string(sizes[0] / sampleRate, text, 255);
			break;
		case kDelay2:
			float2string(sizes[1] / sampleRate, text, 255);
			break;
		case kDelay3:
			float2string(sizes[2] / sampleRate, text, 255);
			break;
		case kDelay4:
			float2string(sizes[3] / sampleRate, text, 255);
			break;
		case kDelay5:
			float2string(sizes[4] / sampleRate, text, 255);
			break;
		case kDelay6:
			float2string(sizes[5] / sampleRate, text, 255);
			break;
		case kDelay7:
			float2string(sizes[6] / sampleRate, text, 255);
			break;
		case kDelay8:
			float2string(sizes[7] / sampleRate, text, 255);
			break;
		case kDelay9:
			float2string(sizes[8] / sampleRate, text, 255);
			break;
		case kDelay10:
			float2string(sizes[9] / sampleRate, text, 255);
			break;
		case kDelay11:
			float2string(sizes[10] / sampleRate, text, 255);
			break;
			//- Feedbacks				
		case kFeedback1:
			float2string(fbcks[0]*100.f, text, 255);
			break;
		case kFeedback2:
			float2string(fbcks[1]*100.f, text, 255);
			break;
		case kFeedback3:
			float2string(fbcks[2]*100.f, text, 255);
			break;
		case kFeedback4:
			float2string(fbcks[3]*100.f, text, 255);
			break;
		case kFeedback5:
			float2string(fbcks[4]*100.f, text, 255);
			break;
		case kFeedback6:
			float2string(fbcks[5]*100.f, text, 255);
			break;
		case kFeedback7:
			float2string(fbcks[6]*100.f, text, 255);
			break;
		case kFeedback8:
			float2string(fbcks[7]*100.f, text, 255);
			break;
		case kFeedback9:
			float2string(fbcks[8]*100.f, text, 255);
			break;
		case kFeedback10:
			float2string(fbcks[9]*100.f, text, 255);
			break;
		case kFeedback11:
			float2string(fbcks[10]*100.f, text, 255);
			break;
	}
}

//-----------------------------------------------------------------------------------------
void Spect::getParameterLabel(VstInt32 index, char *label)
{
	switch(index)
	{
		case kGateThresh:
			strcpy(label, "dB");
			break;
		case kDuckThresh:
			strcpy(label, "dB");
			break;
		case kProbThresh:
			strcpy(label, "%");
			break;
		case kWindowSize:
			strcpy(label, "samples");
			break;
			//- EQs
		case kEQ1:
			strcpy(label, "dB");
			break;
		case kEQ2:
			strcpy(label, "dB");
			break;
		case kEQ3:
			strcpy(label, "dB");
			break;
		case kEQ4:
			strcpy(label, "dB");
			break;
		case kEQ5:
			strcpy(label, "dB");
			break;
		case kEQ6:
			strcpy(label, "dB");
			break;
		case kEQ7:
			strcpy(label, "dB");
			break;
		case kEQ8:
			strcpy(label, "dB");
			break;
		case kEQ9:
			strcpy(label, "dB");
			break;
		case kEQ10:
			strcpy(label, "dB");
			break;
		case kEQ11:
			strcpy(label, "dB");
			break;
			//- Delay Times
		case kDelay1:
			strcpy(label, "sec");
			break;
		case kDelay2:
			strcpy(label, "sec");
			break;
		case kDelay3:
			strcpy(label, "sec");
			break;
		case kDelay4:
			strcpy(label, "sec");
			break;
		case kDelay5:
			strcpy(label, "sec");
			break;
		case kDelay6:
			strcpy(label, "sec");
			break;
		case kDelay7:
			strcpy(label, "sec");
			break;
		case kDelay8:
			strcpy(label, "sec");
			break;
		case kDelay9:
			strcpy(label, "sec");
			break;
		case kDelay10:
			strcpy(label, "sec");
			break;
		case kDelay11:
			strcpy(label, "sec");
			break;
			//- Feedbacks		
		case kFeedback1:
			strcpy(label, "%");
			break;
		case kFeedback2:
			strcpy(label, "%");
			break;
		case kFeedback3:
			strcpy(label, "%");
			break;
		case kFeedback4:
			strcpy(label, "%");
			break;
		case kFeedback5:
			strcpy(label, "%");
			break;
		case kFeedback6:
			strcpy(label, "%");
			break;
		case kFeedback7:
			strcpy(label, "%");
			break;
		case kFeedback8:
			strcpy(label, "%");
			break;
		case kFeedback9:
			strcpy(label, "%");
			break;
		case kFeedback10:
			strcpy(label, "%");
			break;
		case kFeedback11:
			strcpy(label, "%");
			break;
			
	}
}
/*
 //-----------------------------------------------------------------------------------------
 //n.b. for wt1, daub, pwtset and pwt i adapted code from "numerical recipes in c" by press, teukolsky, vetterling & flannery. :)
 void Spect::daub(float input[], float output[], unsigned long n, int isign)
 {
 unsigned long nh, nh1, i, j;
 if(n<4) return;
 nh1=(nh=n>>1)+1;
 if(isign>=0){
 for(i=1, j=1; j<=n-3; j+=2, i++){
 //smoothing filter h0
 wksp[i] =		C0*input[j] + C1*input[j+1] + C2*input[j+2] + C3*input[j+3];
 //detail filter h1
 wksp[i+nh] =	C3*input[j] - C2*input[j+1] + C1*input[j+2] + C0*input[j+3];
 }
 //wrap at boundaries
 //h0
 wksp[i] =		C0*input[n-1] + C1*input[n] + C2*input[1] + C3*input[2];
 //h1
 wksp[i+nh] =	C3*input[n-1] - C2*input[n] + C1*input[1] + C0*input[2];
 } else {
 //inverse transform (transpose matrix)
 //wrap at boundaries
 //inverse h0
 wksp[1] = C2*input[nh] + C1*input[n] + C0*input[1] + C3*input[nh1];
 //inverse h1
 wksp[2] = C3*input[nh] - C0*input[n] + C1*input[1] - C2*input[nh1];
 for(i=1, j=3;i<nh;i++)
 {
 //inverse h0
 wksp[j++] = C2*input[i] + C1*input[i+nh] + C0*input[i+1] + C3*input[i+nh1];
 //inverse h1
 wksp[j++] = C3*input[i] - C0*input[i+nh] + C1*input[i+1] - C2*input[i+nh1];
 }
 }
 //copy wksp contents to output
 for(i=1;i<=n;i++) output[i] = wksp[i];
 }
 
 
 
 void Spect::pwtset(int n)
 {
 int k;
 float sig = -1.0;
 static float c4[5]={0.0,0.4829629131445341,0.8365163037378079,
 0.2241438680420134,-0.1294095225512604};
 static float c12[13]={0.0,0.111540743350, 0.494623890398, 0.751133908021,
 0.315250351709,-0.226264693965,-0.129766867567,
 0.097501605587, 0.027522865530,-0.031582039318,
 0.000553842201, 0.004777257511,-0.001077301085};
 static float c20[21]={0.0,0.026670057901, 0.188176800078, 0.527201188932,
 0.688459039454, 0.281172343661,-0.249846424327,
 -0.195946274377, 0.127369340336, 0.093057364604,
 -0.071394147166,-0.029457536822, 0.033212674059,
 0.003606553567,-0.010733175483, 0.001395351747,
 0.001992405295,-0.000685856695,-0.000116466855,
 0.000093588670,-0.000013264203};
 static float c4r[5],c12r[13],c20r[21];
 
 wfilt.ncof=n;
 if (n == 4) {
 wfilt.cc=c4;
 wfilt.cr=c4r;
 }
 else if (n == 12) {
 wfilt.cc=c12;
 wfilt.cr=c12r;
 }
 else if (n == 20) {
 wfilt.cc=c20;
 wfilt.cr=c20r;
 }
 else {
 wfilt.ncof=20;
 wfilt.cc=c20;
 wfilt.cr=c20r;
 }
 for (k=1;k<=n;k++) {
 wfilt.cr[wfilt.ncof+1-k]=sig*wfilt.cc[k];
 sig = -sig;
 }
 wfilt.ioff = wfilt.joff = -(n >> 1);
 }
 
 void Spect::pwt(float input[], float output[], unsigned long n) 
 {
 unsigned long iii, ii, j, jf, jr, k, n1, ni, nj, nh, nmod;
 if(n<4) return;
 nmod=wfilt.ncof*n;							//zero mod n
 n1=n-1;										//bitmask
 nh=n>>1;
 for(j=1;j<=n;j++) wksp[j-1]=0.0;				//zero out workspace
 for(ii=1, iii=1; iii<=n; iii+=2, ii++){
 ni=iii+nmod+wfilt.ioff;				//increment & wrap around pointers
 nj=iii+nmod+wfilt.joff;
 for(k=1;k<=wfilt.ncof;k++){
 jf=n1&(ni+k);					//bitwise & to wrap pointers
 jr=n1&(nj+k);
 wksp[ii-1] += wfilt.cc[k]*input[jf+1-1];
 wksp[ii+nh-1] += wfilt.cr[k]*input[jr+1-1];
 }
 }
 for(int j=1;j<=n;j++) output[j-1]=wksp[j-1];
 for(j=1;j<=n;j++) wksp[j-1]=0.0;	
 }
 
 void Spect::ipwt(float input[], float output[], unsigned long n)
 {	//transpose filter
 float ai, ai1;
 unsigned long iii, ii, j, jf, jr, k, n1, ni, nj, nh, nmod;
 if(n<4) return;
 nmod=wfilt.ncof*n;							//zero mod n
 n1=n-1;										//bitmask
 nh=n>>1;	
 for(j=1;j<=n;j++) wksp[j-1]=0.0;				//zero out workspace
 for(ii=1, iii=1; iii<=n;iii+=2, ii++){
 ai=input[ii-1];
 ai1=input[ii+nh-1];
 ni=iii+nmod+wfilt.ioff;
 nj=iii+nmod+wfilt.joff;
 for(k=1;k<=wfilt.ncof;k++)
 {
 jf=(n1&(ni+k))+1;
 jr=(n1&(nj+k))+1;
 wksp[jf-1] += wfilt.cc[k]*ai;
 wksp[jr-1] += wfilt.cr[k]*ai1;
 }
 }
 for(int j=1;j<=n;j++) output[j-1]=wksp[j-1];
 for(j=1;j<=n;j++) wksp[j-1]=0.0;	
 }
 
 void Spect::wt1(float input[], float output[], unsigned long n, void(Spect::*pwt)(float [], float [], unsigned long))
 {
 unsigned long nn;
 if (n<4) return;
 //forward transform:
 //start at highest resolution and work toward basis
 for (nn=n;nn>=4;nn>>=1)  (Spect::pwt)(input, output, nn);
 }
 void Spect::iwt1(float input[], float output[], unsigned long n, void(Spect::*pwt)(float [], float [], unsigned long))
 {
 unsigned long nn;
 if (n<4) return;
 for (nn=4;nn<=n;nn<<=1)  (Spect::pwt)(input, output, nn);
 }
 */
//-----------------------------------------------------------------------------------------
void Spect::processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames)
{
	long i, framesLeft, processframes;
	float *in1 = inputs[0];
	float *in2 = inputs[1];
	float *out1 = outputs[0];
	float *out2 = outputs[1];
	
	framesLeft = sampleFrames;
	while (framesLeft > 0)
	{
		// how many frames can we process now
		// with this we insure that we stop on the 
		// blockSize boundary
		if(framesLeft+bufferPosition < blockSize)
			processframes = framesLeft;
		else
			processframes = blockSize - bufferPosition;
		// flush out the previous output, copy in the new input...
		memcpy(inBufferL+bufferPosition, in1, processframes*sizeof(float));
		for(i=0; i<processframes; i++)
		{
			// copy the old output into the out buffers
			out1[i] = outBufferL[i+bufferPosition];
		}
		if(true)
		{
			memcpy(inBufferR+bufferPosition, in2, processframes*sizeof(float));
			for(i=0; i<processframes; i++)
				// copy the old output into the out buffers
				out2[i] = outBufferR[i+bufferPosition];
			
			in2 += processframes;
			out2 += processframes;
		}
		else if(channelMode == kMono2StereoMode)
		{
			for(i=0; i<processframes; i++)
			{
				// copy the old output into the out buffers
				out2[i] = outBufferL[i+bufferPosition];
			}
			
			out2 += processframes;
		}
		
		bufferPosition += processframes;
		// if filled a buffer, we process a new block
		if(bufferPosition >= blockSize)
		{
			bufferPosition = 0;
			if(bypass == true)
			{
				memcpy(outBufferL, inBufferL, (blockSize) * sizeof(float));
				if(true)
					memcpy(outBufferR, inBufferR, (blockSize) * sizeof(float));
			}
			else
				processBlock();
		}
		in1 += processframes;
		out1 += processframes;
		framesLeft -= processframes;
	}
    idleTimer+=sampleFrames;
}

void Spect::processDoubleReplacing(double **inputs, double **outputs, VstInt32 sampleFrames)
{
	long i, framesLeft, processframes;
	float *in1 = (float*)inputs[0];
	float *in2 = (float*)inputs[1];
	float *out1 = (float*)outputs[0];
	float *out2 = (float*)outputs[1];
	
	framesLeft = sampleFrames;
	while (framesLeft > 0)
	{
		// how many frames can we process now
		// with this we insure that we stop on the 
		// blockSize boundary
		if(framesLeft+bufferPosition < blockSize)
			processframes = framesLeft;
		else
			processframes = blockSize - bufferPosition;
		// flush out the previous output, copy in the new input...
		memcpy(inBufferL+bufferPosition, in1, processframes*sizeof(float));
		for(i=0; i<processframes; i++)
		{
			// copy the old output into the out buffers
			out1[i] = outBufferL[i+bufferPosition];
		}
		if(true)
		{
			memcpy(inBufferR+bufferPosition, in2, processframes*sizeof(float));
			for(i=0; i<processframes; i++)
				// copy the old output into the out buffers
				out2[i] = outBufferR[i+bufferPosition];
			
			in2 += processframes;
			out2 += processframes;
		}
		else if(channelMode == kMono2StereoMode)
		{
			for(i=0; i<processframes; i++)
			{
				// copy the old output into the out buffers
				out2[i] = outBufferL[i+bufferPosition];
			}
			
			out2 += processframes;
		}
		
		bufferPosition += processframes;
		// if filled a buffer, we process a new block
		if(bufferPosition >= blockSize)
		{
			bufferPosition = 0;
			if(bypass == true)
			{
				memcpy(outBufferL, inBufferL, (blockSize) * sizeof(float));
				if(true)
					memcpy(outBufferR, inBufferR, (blockSize) * sizeof(float));
			}
			else
				processBlock();
		}
		in1 += processframes;
		out1 += processframes;
		framesLeft -= processframes;
	}
    idleTimer+=sampleFrames;
}


bool Spect::setBypass(bool pBypass)
{
	bypass = pBypass;
	return(bypass);
}

void Spect::setDWTSize(long newSize)
{
	log2n = (long)log2f((float)newSize);
	newSize = (long)(powf(2.0f, log2n));
	sizeDWT = newSize;
	blockSize = sizeDWT >> 2;
	halfSizeDWT = sizeDWT >> 1;
	oneOverBlockSize = 1.0f/(float)blockSize;
	
	suspend();
	switch(windowSelected)
	{
		case kHamming:
			initHammingWindows();
			break;
		case kVonHann:
			initVonHannWindows();
			break;
		case kBlackman:
			initBlackmanWindows();
			break;
		case kKaiser:
			initKaiserWindows();
			break;
	}
	scaleWindows();
}

//------------------------------------------------------------------------

// this will generally be overridden to allow DWT size switching
void Spect::updateDWT()
{
	;
}

void Spect::processBlock()
{
	long	i;
	float	outTemp;
	long	maskDWT;
	long level = 0;
	
	updateDWT();
	maskDWT = sizeDWT - 1;
	//
	inputTimeL += blockSize;
	inputTimeR += blockSize;
	outputTimeL += blockSize;
	outputTimeR += blockSize;
	inputTimeL = inputTimeL & maskDWT;
	inputTimeR = inputTimeR & maskDWT;
	outputTimeL = outputTimeL & maskDWT;
	outputTimeR = outputTimeR & maskDWT;
	//
	// a - shift output buffer and zero out new location
	memcpy(outBufferL, outBufferL+blockSize, (sizeDWT - blockSize) * sizeof(float));
	memset(outBufferL+(sizeDWT - blockSize), 0, blockSize * sizeof(float));
	if(true)
	{
		// a - shift output buffer and zero out new location
		memcpy(outBufferR, outBufferR+blockSize, (sizeDWT - blockSize) * sizeof(float));
		memset(outBufferR+(sizeDWT - blockSize), 0, blockSize * sizeof(float));
	}
	// a - shift current samples in inShift
	memcpy(inShiftL, inShiftL+blockSize, (sizeDWT - blockSize) * sizeof(float));
	// a1 - copy the new stuff in
	memcpy(inShiftL+(sizeDWT - blockSize), inBufferL, blockSize * sizeof(float));
	// b - window the block in
	for(i = 0; i < sizeDWT; i++)
	{
		*(inDWTL + inputTimeL) = *(inShiftL + i) * *(analysisWindow + i);
		++inputTimeL;
		inputTimeL = inputTimeL & maskDWT;
	}
	if(true)
	{
		// a - shift current samples in inShift
		memcpy(inShiftR, inShiftR+blockSize, (sizeDWT - blockSize) * sizeof(float));
		// a1 - copy the new stuff in
		memcpy(inShiftR+(sizeDWT - blockSize), inBufferR, blockSize * sizeof(float));
		// b - window the block in
		for(i = 0; i < sizeDWT; i++)
		{
			*(inDWTR + inputTimeR) = *(inShiftR + i) * *(analysisWindow + i);
			++inputTimeR;
			inputTimeR = inputTimeR & maskDWT;
		}
	}
	
	//forward transform: (take DWT of inDWTL and place result into inSpectraL)
	
	//	wt1(inDWTL, inSpectraL, sizeDWT, &Spect::pwt);
	
	//for(int i=0; i<sizeDWT; i++) outSpectraL[i] = inSpectraL[i];
	
	unsigned long nn;
	for (nn=sizeDWT;nn>=4;nn>>=1)
	{
		//perform layer of forward transform
		unsigned long nh, nh1, j;
		//	if(nn<4) break;
		nh1=(nh=nn>>1)+1;
		for(i=0, j=0; j<nn-3; j+=2, i++){
			//smoothing filter h0
			wksp[i] =		C0*inDWTL[j] + C1*inDWTL[j+1] + C2*inDWTL[j+2] + C3*inDWTL[j+3];
			//detail filter h1
			wksp[i+nh] =	C3*inDWTL[j] - C2*inDWTL[j+1] + C1*inDWTL[j+2] - C0*inDWTL[j+3];
		}
		//wrap at boundaries
		//h0
		wksp[i] =		C0*inDWTL[nn-2] + C1*inDWTL[nn-1] + C2*inDWTL[0] + C3*inDWTL[1];
		//h1
		wksp[i+nh] =	C3*inDWTL[nn-2] - C2*inDWTL[nn-1] + C1*inDWTL[0] - C0*inDWTL[1];
		for(i=0;i<nn;i++) inDWTL[i] = inSpectraL[i] = wksp[i];
	}
	//copy wksp contents to output
	
	
	//DO SOMETHING TO inSpectraL & copy to outSpectraL... use processSignal();
	
	float modulus = 0.0f; 
	float prob = 0.0f;
	for(i=0;i<sizeDWT;i++)
	{ 
		//modulus gate(duck)ing!
		modulus = fabsf(inSpectraL[i]);
		if(paramB > modulus && modulus > paramA)
		{
			//stochastic gating!
			prob = (float)rand()/(float)RAND_MAX;
			if(paramC < prob) {
				outSpectraL[i] = 0.0f;
			} else {
				outSpectraL[i] = inSpectraL[i]; //here we just copy it straight
			}
		} else {
			outSpectraL[i] = 0.0f;
		}
	}
	
	//EQ- scale individual layers (lengths contained in array "lens") of outSpectraL by amounts stored in array "eqs"
	//smallest window case first, implement longer windows as test cases...
	int a = 0; //a stores begining of layer.
	int b = 0; //b sweeps from 0-length of layer.
	//a + b = position within layer. 
	for(i = 0; i<9; i++)
	{
		float scale = eqs[i];
		for(b = 0; b<lens[i]; b++) //b sweeps from 0-length of layer. a+b = position in layer
		{
			outSpectraL[a+b] *= scale; //scale values in layer
			
			//apply delay...
			float ffwd = outSpectraL[a+b];
			float fbck = dbufsL[i][cursorsL[i]];
			fbck = dbufsL[i][cursorsL[i]++] = ffwd + (fbck * fbcks[i]);
			if(cursorsL[i] >= sizes[i])
				cursorsL[i] = 0;
			outSpectraL[a+b] = fbck;
			//end apply delay...
			
		}
		a += lens[i]; //shift a to next begining point
	}
	//test cases for 3 longer possible window sizes
	if(sizeDWT>512) 
	{
		i++; //increment i
		float scale = eqs[i]; //find scale factor in eqs
		for(b = 0; b<lens[i]; b++) //b sweeps from 0-length of layer. a+b = position in layer
		{
			outSpectraL[a+b] *= scale; //scale values in layer
			
			//apply delay...
			float ffwd = outSpectraL[a+b];
			float fbck = dbufsL[i][cursorsL[i]];
			fbck = dbufsL[i][cursorsL[i]++] = ffwd + (fbck * fbcks[i]);
			if(cursorsL[i] >= sizes[i])
				cursorsL[i] = 0;
			outSpectraL[a+b] = fbck;
			//end apply delay...
			
		}
		a += lens[i]; //shift a to next beginning point
		if(sizeDWT>1024)
		{
			i++;
			scale = eqs[i];
			for(b = 0; b<lens[i]; b++) //b sweeps from 0-length of layer. a+b = position in layer
			{
				outSpectraL[a+b] *= scale; //scale values in layer
				
				//apply delay...
				float ffwd = outSpectraL[a+b];
				float fbck = dbufsL[i][cursorsL[i]];
				fbck = dbufsL[i][cursorsL[i]++] = ffwd + (fbck * fbcks[i]);
				if(cursorsL[i] >= sizes[i])
					cursorsL[i] = 0;
				outSpectraL[a+b] = fbck;
				//end apply delay...
				
			}
			a += lens[i]; //shift a to next beginning point
			if(sizeDWT>2048) //last case...
			{
				i++;
				scale = eqs[i];
				for(b = 0; b<lens[i]; b++)
				{
					outSpectraL[a+b] *= scale;
					
					//apply delay...
					float ffwd = outSpectraL[a+b];
					float fbck = dbufsL[i][cursorsL[i]];
					fbck = dbufsL[i][cursorsL[i]++] = ffwd + (fbck * fbcks[i]);
					if(cursorsL[i] >= sizes[i])
						cursorsL[i] = 0;
					outSpectraL[a+b] = fbck;
					//end apply delay...
					
				}
			}
		}
	}
	
	
	
	
	//inverse transform: (take IDWT of outSpectraL and place result into outShiftL)
	//iwt1(outSpectraL, outShiftL, sizeDWT, &Spect::ipwt);
	
	
	
	
	for (nn=4;nn<=sizeDWT;nn<<=1)
	{
		unsigned long nh, nh1, j;
		//if(nn<4) break;
		nh1=(nh=nn>>1)+1;
		//perform layer of inverse transform (transpose matrix)
		//wrap at boundaries
		//inverse h0
		wksp[0] = C2*outSpectraL[nh-1] + C1*outSpectraL[nn-1] + C0*outSpectraL[0] + C3*outSpectraL[nh1-1];
		//eq for basis layer:
		//wksp[0] *= eqs[level];
		//inverse h1
		wksp[1] = C3*outSpectraL[nh-1] - C0*outSpectraL[nn-1] + C1*outSpectraL[0] - C2*outSpectraL[nh1-1];
		//wksp[1] *= eqs[level];
		for(i=0, j=2;i<nh-1;i++)
		{
			//inverse h0
			wksp[j] = C2*outSpectraL[i] + C1*outSpectraL[i+nh] + C0*outSpectraL[i+1] + C3*outSpectraL[i+nh1];
			
			//	if(nn<sizeDWT) wksp[j] *= eqs[level];
			j++;
			//inverse h1
			wksp[j] = C3*outSpectraL[i] - C0*outSpectraL[i+nh] + C1*outSpectraL[i+1] - C2*outSpectraL[i+nh1];
			//	if(nn<sizeDWT) wksp[j] *= eqs[level];
			j++;
		}
		//level++;
		for(i=0;i<nn;i++) outSpectraL[i] = outShiftL[i] = wksp[i];
	}
	
	//copy wksp contents to output
	
	
	//for(i=0;i<sizeDWT; i++) outShiftL[i] = inDWTL[i];
	
	// e - overlap add
	for(i = 0; i < sizeDWT; i++)
	{
		outTemp = *(outShiftL + outputTimeL) * *(synthesisWindow + i);
		*(outBufferL+i) += outTemp;
		++outputTimeL;
		outputTimeL = outputTimeL & maskDWT;
	}
	
	if(true)
	{
		//level = 0;
		//forward transform: (take DWT of inDWTR and place result into inSpectraR)
		//clear wksp contents
		for(i=0;i<sizeDWT;i++) wksp[i] = 0.0f;
		
		unsigned long nn;
		for (nn=sizeDWT;nn>=4;nn>>=1)
		{
			//perform layer of forward transform
			unsigned long nh, nh1, j;
			//	if(nn<4) break;
			nh1=(nh=nn>>1)+1;
			for(i=0, j=0; j<nn-3; j+=2, i++){
				//smoothing filter h0
				wksp[i] =		C0*inDWTR[j] + C1*inDWTR[j+1] + C2*inDWTR[j+2] + C3*inDWTR[j+3];
				//detail filter h1
				wksp[i+nh] =	C3*inDWTR[j] - C2*inDWTR[j+1] + C1*inDWTR[j+2] - C0*inDWTR[j+3];
			}
			//wrap at boundaries
			//h0
			wksp[i] =		C0*inDWTR[nn-2] + C1*inDWTR[nn-1] + C2*inDWTR[0] + C3*inDWTR[1];
			//h1
			wksp[i+nh] =	C3*inDWTR[nn-2] - C2*inDWTR[nn-1] + C1*inDWTR[0] - C0*inDWTR[1];
			for(i=0;i<nn;i++) inDWTR[i] = inSpectraR[i] = wksp[i];
		}
		//copy wksp contents to output
		
		
		//DO SOMETHING TO inSpectraR & copy to outSpectraR... use processSignal();
		float modulus = 0.0f; 
		float prob = 0.0f;
		for(i=0;i<sizeDWT;i++)
		{ 
			//modulus gate(duck)ing!
			modulus = fabsf(inSpectraR[i]);
			if(paramB > modulus && modulus > paramA)
			{
				//stochastic gating!
				prob = (float)rand()/(float)RAND_MAX;
				if(paramC < prob) {
					outSpectraR[i] = 0.0f;
				} else {
					outSpectraR[i] = inSpectraR[i]; //here we just copy it straight
				}
			} else {
				outSpectraR[i] = 0.0f;
			}
		}
		
		//EQ- scale individual layers (lengths contained in array "lens") of outSpectraR by amounts stored in array "eqs"
		//smallest window case first, implement longer windows as test cases...
		int a = 0; //a stores begining of layer.
		int b = 0; //b sweeps from 0-length of layer.
		//a + b = position within layer. 
		for(i = 0; i<9; i++)
		{
			float scale = eqs[i];
			for(b = 0; b<lens[i]; b++) //b sweeps from 0-length of layer. a+b = position in layer
			{
				outSpectraR[a+b] *= scale; //scale values in layer
				
				//apply delay...
				float ffwd = outSpectraR[a+b];
				float fbck = dbufsR[i][cursorsR[i]];
				fbck = dbufsR[i][cursorsR[i]++] = ffwd + (fbck * fbcks[i]);
				if(cursorsR[i] >= sizes[i])
					cursorsR[i] = 0;
				outSpectraR[a+b] = fbck;
				//end apply delay...
				
			}
			a += lens[i]; //shift a to next begining point
		}
		//test cases for 3 longer possible window sizes
		if(sizeDWT>512) 
		{
			i++; //increment i
			float scale = eqs[i]; //find scale factor in eqs
			for(b = 0; b<lens[i]; b++) //b sweeps from 0-length of layer. a+b = position in layer
			{
				outSpectraR[a+b] *= scale; //scale values in layer
				
				//apply delay...
				float ffwd = outSpectraR[a+b];
				float fbck = dbufsR[i][cursorsR[i]];
				fbck = dbufsR[i][cursorsR[i]++] = ffwd + (fbck * fbcks[i]);
				if(cursorsR[i] >= sizes[i])
					cursorsR[i] = 0;
				outSpectraR[a+b] = fbck;
				//end apply delay...
				
			}
			a += lens[i]; //shift a to next beginning point
			if(sizeDWT>1024)
			{
				i++;
				scale = eqs[i];
				for(b = 0; b<lens[i]; b++) //b sweeps from 0-length of layer. a+b = position in layer
				{
					outSpectraR[a+b] *= scale; //scale values in layer
					
					//apply delay...
					float ffwd = outSpectraR[a+b];
					float fbck = dbufsR[i][cursorsR[i]];
					fbck = dbufsR[i][cursorsR[i]++] = ffwd + (fbck * fbcks[i]);
					if(cursorsR[i] >= sizes[i])
						cursorsR[i] = 0;
					outSpectraR[a+b] = fbck;
					//end apply delay...
					
				}
				a += lens[i]; //shift a to next beginning point
				if(sizeDWT>2048) //last case...
				{
					i++;
					scale = eqs[i];
					for(b = 0; b<lens[i]; b++)
					{
						outSpectraR[a+b] *= scale;
						
						//apply delay...
						float ffwd = outSpectraR[a+b];
						float fbck = dbufsR[i][cursorsR[i]];
						fbck = dbufsR[i][cursorsR[i]++] = ffwd + (fbck * fbcks[i]);
						if(cursorsR[i] >= sizes[i])
							cursorsR[i] = 0;
						outSpectraR[a+b] = fbck;
						//end apply delay...
						
					}
				}
			}
		}
		
		
		//inverse transform: (take IDWT of outSpectraR and place result into outShiftR)
		
		for (nn=4;nn<=sizeDWT;nn<<=1)
		{
			unsigned long nh, nh1, j;
			//if(nn<4) break;
			nh1=(nh=nn>>1)+1;
			//perform layer of inverse transform (transpose matrix)
			//wrap at boundaries
			//inverse h0
			wksp[0] = C2*outSpectraR[nh-1] + C1*outSpectraR[nn-1] + C0*outSpectraR[0] + C3*outSpectraR[nh1-1];
			//eq for basis layer:
			//wksp[0] *= eqs[level];
			//inverse h1
			wksp[1] = C3*outSpectraR[nh-1] - C0*outSpectraR[nn-1] + C1*outSpectraR[0] - C2*outSpectraR[nh1-1];
			//wksp[1] *= eqs[level];
			for(i=0, j=2;i<nh-1;i++)
			{
				//inverse h0
				wksp[j] = C2*outSpectraR[i] + C1*outSpectraR[i+nh] + C0*outSpectraR[i+1] + C3*outSpectraR[i+nh1];
				
				//	if(nn<sizeDWT) wksp[j] *= eqs[level];
				j++;
				//inverse h1
				wksp[j] = C3*outSpectraR[i] - C0*outSpectraR[i+nh] + C1*outSpectraR[i+1] - C2*outSpectraR[i+nh1];
				//	if(nn<sizeDWT) wksp[j] *= eqs[level];
				j++;
			}
			//level++;
			for(i=0;i<nn;i++) outSpectraR[i] = outShiftR[i] = wksp[i];
		}
		//copy wksp contents to output
		
		
		
		
		// e - overlap add
		for(i = 0; i < sizeDWT; i++)
		{
			outTemp = *(outShiftR + outputTimeR) * synthesisWindow[i];
			*(outBufferR+i) += outTemp;
			++outputTimeR;
			outputTimeR = outputTimeR & maskDWT;
		}
	}
}


void Spect::processDoubleBlock()
{
	long	i;
	float	outTemp;
	long	maskDWT;
	
	updateDWT();
	maskDWT = sizeDWT - 1;
	//
	inputTimeL += blockSize;
	inputTimeR += blockSize;
	outputTimeL += blockSize;
	outputTimeR += blockSize;
	inputTimeL = inputTimeL & maskDWT;
	inputTimeR = inputTimeR & maskDWT;
	outputTimeL = outputTimeL & maskDWT;
	outputTimeR = outputTimeR & maskDWT;
	//
	// a - shift output buffer and zero out new location
	memcpy(outBufferL, outBufferL+blockSize, (sizeDWT - blockSize) * sizeof(float));
	memset(outBufferL+(sizeDWT - blockSize), 0, blockSize * sizeof(float));
	if(true)
	{
		// a - shift output buffer and zero out new location
		memcpy(outBufferR, outBufferR+blockSize, (sizeDWT - blockSize) * sizeof(float));
		memset(outBufferR+(sizeDWT - blockSize), 0, blockSize * sizeof(float));
	}
	// a - shift current samples in inShift
	memcpy(inShiftL, inShiftL+blockSize, (sizeDWT - blockSize) * sizeof(float));
	// a1 - copy the new stuff in
	memcpy(inShiftL+(sizeDWT - blockSize), inBufferL, blockSize * sizeof(float));
	// b - window the block in
	for(i = 0; i < sizeDWT; i++)
	{
		*(inDWTL + inputTimeL) = *(inShiftL + i) * *(analysisWindow + i);
		++inputTimeL;
		inputTimeL = inputTimeL & maskDWT;
	}
	if(true)
	{
		// a - shift current samples in inShift
		memcpy(inShiftR, inShiftR+blockSize, (sizeDWT - blockSize) * sizeof(float));
		// a1 - copy the new stuff in
		memcpy(inShiftR+(sizeDWT - blockSize), inBufferR, blockSize * sizeof(float));
		// b - window the block in
		for(i = 0; i < sizeDWT; i++)
		{
			*(inDWTR + inputTimeR) = *(inShiftR + i) * *(analysisWindow + i);
			++inputTimeR;
			inputTimeR = inputTimeR & maskDWT;
		}
	}
	
	//forward transform: (take DWT of inDWTL and place result into inSpectraL)
	
	unsigned long nn;
	for (nn=sizeDWT;nn>=4;nn>>=1)
	{
		//perform layer of forward transform
		unsigned long nh, nh1, j;
		if(sizeDWT<4) break;
		nh1=(nh=sizeDWT>>1)+1;
		for(i=0, j=0; j<sizeDWT-3; j+=2, i++){
			//smoothing filter h0
			wksp[i] =		C0*inDWTL[j] + C1*inDWTL[j+1] + C2*inDWTL[j+2] + C3*inDWTL[j+3];
			//detail filter h1
			wksp[i+nh] =	C3*inDWTL[j] - C2*inDWTL[j+1] + C1*inDWTL[j+2] - C0*inDWTL[j+3];
		}
		//wrap at boundaries
		//h0
		wksp[i] =		C0*inDWTL[sizeDWT-2] + C1*inDWTL[sizeDWT-1] + C2*inDWTL[0] + C3*inDWTL[1];
		//h1
		wksp[i+nh] =	C3*inDWTL[sizeDWT-2] - C2*inDWTL[sizeDWT-1] + C1*inDWTL[0] - C0*inDWTL[1];
	}
	//copy wksp contents to output
	for(i=0;i<sizeDWT;i++) inSpectraL[i] = wksp[i];
	
	//DO SOMETHING TO inSpectraL & copy to outSpectraL... use processSignal();
	float modulus = 0.0f; 
	float prob = 0.0f;
	for(i=0;i<sizeDWT;i++)
	{ 
		//modulus gate(duck)ing!
		modulus = fabsf(inSpectraL[i]);
		if(paramB > modulus && modulus > paramA)
		{
			//stochastic gating!
			prob = (float)rand()/(float)RAND_MAX;
			if(paramC < prob) {
				outSpectraL[i] = 0.0f;
			} else {
				outSpectraL[i] = inSpectraL[i]; //here we just copy it straight
			}
		} else {
			outSpectraL[i] = 0.0f;
		}
	}
	
	//EQ- scale individual layers (lengths contained in array "lens") of outSpectraL by amounts stored in array "eqs"
	//smallest window case first, implement longer windows as test cases...
	int a = 0; //a stores begining of layer.
	int b = 0; //b sweeps from 0-length of layer.
	//a + b = position within layer. 
	for(i = 0; i<9; i++)
	{
		float scale = (eqs[i]*2)-1;
		for(b = 0; b<lens[i]; b++) //b sweeps from 0-length of layer. a+b = position in layer
		{
			outSpectraL[a+b] *= scale; //scale values in layer
		}
		a += lens[i]; //shift a to next begining point
	}
	//test cases for 3 longer possible window sizes
	if(sizeDWT>512) 
	{
		i++; //increment i
		float scale = (eqs[i]*2)-1; //find scale factor in eqs
		for(b = 0; b<lens[i]; b++) //b sweeps from 0-length of layer. a+b = position in layer
		{
			outSpectraL[a+b] *= scale; //scale values in layer
		}
		a += lens[i]; //shift a to next beginning point
		if(sizeDWT>1024)
		{
			i++;
			scale = (eqs[i]*2)-1;
			for(b = 0; b<lens[i]; b++) //b sweeps from 0-length of layer. a+b = position in layer
			{
				outSpectraL[a+b] *= scale; //scale values in layer
			}
			a += lens[i]; //shift a to next beginning point
			if(sizeDWT>2048) //last case...
			{
				i++;
				scale = (eqs[i]*2)-1;
				for(b = 0; b<lens[i]; b++)
				{
					outSpectraL[a+b] *= scale;
				}
			}
		}
	}
	
	//inverse transform: (take IDWT of outSpectraL and place result into outShiftL)
	
	for (nn=4;nn<=sizeDWT;nn<<=1)
	{
		unsigned long nh, nh1, j;
		if(sizeDWT<4) break;
		nh1=(nh=sizeDWT>>1)+1;
		//perform layer of inverse transform (transpose matrix)
		//wrap at boundaries
		//inverse h0
		wksp[0] = C2*outSpectraL[nh-1] + C1*outSpectraL[sizeDWT-1] + C0*outSpectraL[0] + C3*outSpectraL[nh1-1];
		//inverse h1
		wksp[1] = C3*outSpectraL[nh-1] - C0*outSpectraL[sizeDWT-1] + C1*outSpectraL[0] - C2*outSpectraL[nh1-1];
		for(i=0, j=2;i<nh;i++)
		{
			//inverse h0
			wksp[j++] = C2*outSpectraL[i] + C1*outSpectraL[i+nh] + C0*outSpectraL[i+1] + C3*outSpectraL[i+nh1];
			//inverse h1
			wksp[j++] = C3*outSpectraL[i] - C0*outSpectraL[i+nh] + C1*outSpectraL[i+1] - C2*outSpectraL[i+nh1];
		}
	}
	
	//copy wksp contents to output
	for(i=0;i<sizeDWT;i++) outShiftL[i] = wksp[i];
	
	// e - overlap add
	for(i = 0; i < sizeDWT; i++)
	{
		outTemp = *(outShiftL + outputTimeL) * *(synthesisWindow + i);
		*(outBufferL+i) += outTemp;
		++outputTimeL;
		outputTimeL = outputTimeL & maskDWT;
	}
	
	if(true)
	{
		//forward transform: (take DWT of inDWTL and place result into inSpectraL)
		//clear wksp contents
		for(i=0;i<sizeDWT;i++) wksp[i] = 0.0f;
		
		unsigned long nn;
		for (nn=sizeDWT;nn>=4;nn>>=1)
		{
			//perform layer of forward transform
			unsigned long nh, nh1, j;
			if(sizeDWT<4) break;
			nh1=(nh=sizeDWT>>1)+1;
			for(i=0, j=0; j<sizeDWT-3; j+=2, i++){
				//smoothing filter h0
				wksp[i] =		C0*inDWTR[j] + C1*inDWTR[j+1] + C2*inDWTR[j+2] + C3*inDWTR[j+3];
				//detail filter h1
				wksp[i+nh] =	C3*inDWTR[j] - C2*inDWTR[j+1] + C1*inDWTR[j+2] - C0*inDWTR[j+3];
			}
			//wrap at boundaries
			//h0
			wksp[i] =		C0*inDWTR[sizeDWT-2] + C1*inDWTR[sizeDWT-1] + C2*inDWTR[0] + C3*inDWTR[1];
			//h1
			wksp[i+nh] =	C3*inDWTR[sizeDWT-2] - C2*inDWTR[sizeDWT-1] + C1*inDWTR[0] - C0*inDWTR[1];
		}
		//copy wksp contents to output
		for(i=0;i<sizeDWT;i++) inSpectraR[i] = wksp[i];
		
		//DO SOMETHING TO inSpectraR & copy to outSpectraR... use processSignal();
		float modulus = 0.0f; 
		float prob = 0.0f;
		for(i=0;i<sizeDWT;i++)
		{ 
			//modulus gate(duck)ing!
			modulus = fabsf(inSpectraR[i]);
			if(paramB > modulus && modulus > paramA)
			{
				//stochastic gating!
				prob = (float)rand()/(float)RAND_MAX;
				if(paramC < prob) {
					outSpectraR[i] = 0.0f;
				} else {
					outSpectraR[i] = inSpectraR[i]; //here we just copy it straight
				}
			} else {
				outSpectraR[i] = 0.0f;
			}
		}
		
		//EQ- scale individual layers (lengths contained in array "lens") of outSpectraR by amounts stored in array "eqs"
		//smallest window case first, implement longer windows as test cases...
		a = 0; //a stores begining of layer.
		b = 0; //b sweeps from 0-length of layer.
		//a + b = position within layer. 
		for(i = 0; i<9; i++)
		{
			float scale = (eqs[i]*2)-1;
			for(b = 0; b<lens[i]; b++) //b sweeps from 0-length of layer. a+b = position in layer
			{
				outSpectraR[a+b] *= scale; //scale values in layer
			}
			a += lens[i]; //shift a to next begining point
		}
		//test cases for 3 longer possible window sizes
		if(sizeDWT>512) 
		{
			i++; //increment i
			float scale = (eqs[i]*2)-1; //find scale factor in eqs
			for(b = 0; b<lens[i]; b++) //b sweeps from 0-length of layer. a+b = position in layer
			{
				outSpectraR[a+b] *= scale; //scale values in layer
			}
			a += lens[i]; //shift a to next beginning point
			if(sizeDWT>1024)
			{
				i++;
				scale = (eqs[i]*2)-1;
				for(b = 0; b<lens[i]; b++) //b sweeps from 0-length of layer. a+b = position in layer
				{
					outSpectraR[a+b] *= scale; //scale values in layer
				}
				a += lens[i]; //shift a to next beginning point
				if(sizeDWT>2048) //last case...
				{
					i++;
					scale = (eqs[i]*2)-1;
					for(b = 0; b<lens[i]; b++)
					{
						outSpectraR[a+b] *= scale;
					}
				}
			}
		}
		
		//inverse transform: (take IDWT of outSpectraR and place result into outShiftR)
		
		for (nn=4;nn<=sizeDWT;nn<<=1)
		{
			unsigned long nh, nh1, j;
			if(sizeDWT<4) break;
			nh1=(nh=sizeDWT>>1)+1;
			//perform layer of inverse transform (transpose matrix)
			//wrap at boundaries
			//inverse h0
			wksp[0] = C2*outSpectraR[nh-1] + C1*outSpectraR[sizeDWT-1] + C0*outSpectraR[0] + C3*outSpectraR[nh1-1];
			//inverse h1
			wksp[1] = C3*outSpectraR[nh-1] - C0*outSpectraR[sizeDWT-1] + C1*outSpectraR[0] - C2*outSpectraR[nh1-1];
			for(i=0, j=2;i<nh;i++)
			{
				//inverse h0
				wksp[j++] = C2*outSpectraR[i] + C1*outSpectraR[i+nh] + C0*outSpectraR[i+1] + C3*outSpectraR[i+nh1];
				//inverse h1
				wksp[j++] = C3*outSpectraR[i] - C0*outSpectraR[i+nh] + C1*outSpectraR[i+1] - C2*outSpectraR[i+nh1];
			}
		}
		
		//copy wksp contents to output
		for(i=0;i<sizeDWT;i++) outShiftR[i] = wksp[i];
		
		
		
		// e - overlap add
		for(i = 0; i < sizeDWT; i++)
		{
			outTemp = *(outShiftR + outputTimeR) * synthesisWindow[i];
			*(outBufferR+i) += outTemp;
			++outputTimeR;
			outputTimeR = outputTimeR & maskDWT;
		}
	}
}

void	Spect::initHammingWindows(void)
{
	long	index;
	float	a = 0.54f, b	= 0.46f;
	
	windowSelected = kHamming;
	
	// a - make two hamming windows
	for (index = 0; index < sizeDWT; index++)
		synthesisWindow[index] = analysisWindow[index] = a - b*cosf(twoPi*index/(sizeDWT - 1));
}

void	Spect::initVonHannWindows(void)
{
	long	index;
	float	a = 0.50, b	= 0.40;
	
	windowSelected = kVonHann;
	
	// a - make two hamming windows
	for (index = 0; index < sizeDWT; index++)
		synthesisWindow[index] = analysisWindow[index] = a - b*cosf(twoPi*index/(sizeDWT - 1));
}

void	Spect::initBlackmanWindows(void)
{
	long	index;
	float	a = 0.42, b	= 0.50, c = 0.08;
	
	windowSelected = kBlackman;
	
	// a - make two hamming windows
	for (index = 0; index < sizeDWT; index++)
		synthesisWindow[index] = analysisWindow[index] = a - b*cosf(twoPi*index/(sizeDWT - 1)) + c*cosf(2.0f*twoPi*index/(sizeDWT - 1));
	
}

void	Spect::initKaiserWindows(void)
{
	
	double	bes, xind, floati;
	long	i;
	
	windowSelected = kKaiser;
	
	bes = kaiserIno(6.8);
	xind = (float)(sizeDWT-1)*(sizeDWT-1);
	
	for (i = 0; i < halfSizeDWT; i++)
	{
		floati = (double)i;
		floati = 4.0 * floati * floati;
		floati = sqrt(1. - floati / xind);
		synthesisWindow[i+halfSizeDWT] = kaiserIno(6.8 * floati);
		analysisWindow[halfSizeDWT - i] = synthesisWindow[halfSizeDWT - i] = analysisWindow[i+halfSizeDWT] = (synthesisWindow[i+halfSizeDWT] /= bes);
	}
	analysisWindow[sizeDWT - 1] = synthesisWindow[sizeDWT - 1] = 0.0;
	analysisWindow[0] = synthesisWindow[0] = 0.0;
}

float	Spect::kaiserIno(float x)
{
	float	y, t, e, de, sde, xi;
	long i;
	
	y = x / 2.;
	t = 1.e-08;
	e = 1.;
	de = 1.;
	for (i = 1; i <= 25; i++)
	{
		xi = i;
		de = de * y / xi;
		sde = de * de;
		e += sde;
		if (e * t > sde)
			break;
	}
	return(e);
}

void Spect::scaleWindows(void)
{
	long index;
	float max, rat;
	max = synthesisWindow[0];
	rat = 1.0f/max;
	//normalize window
	for(index = 0; index < sizeDWT; index++)
	{
		analysisWindow[index] *= rat;
		synthesisWindow[index] *= rat;
	}
	//rescale
	for(index = 0; index < sizeDWT; index++)
	{
		analysisWindow[index] *= 0.0445;
		synthesisWindow[index] *= 0.0445;
	}
}