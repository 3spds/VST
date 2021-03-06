/*
  ==============================================================================

    This file was auto-generated by the Introjucer!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
EigenTestAudioProcessor::EigenTestAudioProcessor()
{
    UserParams[MasterBypass]=0.0f;
    UserParams[Drive]=1.0f;
    mDist.SetDrive(UserParams[Drive]);
    UIUpdateFlag=true;
}

EigenTestAudioProcessor::~EigenTestAudioProcessor()
{
}

//==============================================================================
const String EigenTestAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

int EigenTestAudioProcessor::getNumParameters()
{
    return totalNumParam;
}

float EigenTestAudioProcessor::getParameter (int index)
{
    switch(index)
    {
    case MasterBypass:
        return UserParams[MasterBypass];
    case Drive:
        UserParams[Drive]=mDist.GetDrive();
        return UserParams[Drive];
    default: return 0.0f;
    }
}

void EigenTestAudioProcessor::setParameter (int index, float newValue)
{
    switch(index)
    {
    case MasterBypass:
        UserParams[MasterBypass]=newValue;
        break;
    case Drive:
        UserParams[Drive]=newValue;
        mDist.SetDrive(UserParams[Drive]);
        break;
    default: return;
    }
    UIUpdateFlag=true;
}

const String EigenTestAudioProcessor::getParameterName (int index)
{
    switch(index)
    {
    case MasterBypass: return "Master Bypass";
    case Drive: return "Drive";
    default: return String::empty;
    }
}

const String EigenTestAudioProcessor::getParameterText (int index)
{
    if(index>=0 && index<totalNumParam)
        return String(UserParams[index]);
    else return String::empty;
}

const String EigenTestAudioProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String EigenTestAudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool EigenTestAudioProcessor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool EigenTestAudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool EigenTestAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EigenTestAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EigenTestAudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

double EigenTestAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EigenTestAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int EigenTestAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EigenTestAudioProcessor::setCurrentProgram (int index)
{
}

const String EigenTestAudioProcessor::getProgramName (int index)
{
    return String();
}

void EigenTestAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void EigenTestAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void EigenTestAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void EigenTestAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // I've added this to avoid people getting screaming feedback
    // when they first compile the plugin, but obviously you don't need to
    // this code if your algorithm already fills all the output channels.
    if(getNumInputChannels()<2 || UserParams[MasterBypass])
    {}
    else
    {
        for(int i=0;i<Fraction;i++)
        {
            mDist.AudioSVD(buffer, 0, i*(int)(buffer.getNumSamples()/Fraction));
        }
        //mDist.AudioSVD(buffer, 1);
        /*
        for(long i=0; i<buffer.getNumSamples();i++)
            mDist.ClockProcess(&leftData[i], &rightData[i]);
            */
    }
}

//==============================================================================
bool EigenTestAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* EigenTestAudioProcessor::createEditor()
{
    return new EigenTestAudioProcessorEditor (*this);
}

//==============================================================================
void EigenTestAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    XmlElement root("Root");
    XmlElement *el;
    el = root.createNewChildElement("Bypass");
    el->addTextElement(String(UserParams[MasterBypass]));
    el = root.createNewChildElement("Drive");
    el->addTextElement(String(UserParams[Drive]));
    copyXmlToBinary(root,destData);
}

void EigenTestAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    XmlElement* pRoot = getXmlFromBinary(data,sizeInBytes);
    if(pRoot!=NULL)
    {
        forEachXmlChildElement((*pRoot),pChild)
        {
        if(pChild->hasTagName("Bypass"))
            {
            String text = pChild->getAllSubText();
            setParameter(MasterBypass,text.getFloatValue());
            }
        else if(pChild->hasTagName("Drive"))
            {
            String text = pChild->getAllSubText();
            setParameter(Drive,text.getFloatValue());
            }
        }
        delete pRoot;
        UIUpdateFlag=true;
    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EigenTestAudioProcessor();
}
