/*
  ==============================================================================

  This is an automatically generated GUI class created by the Introjucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Introjucer version: 3.1.1

  ------------------------------------------------------------------------------

  The Introjucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-13 by Raw Material Software Ltd.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "PluginEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
AllpasserAudioProcessorEditor::AllpasserAudioProcessorEditor (AllpasserAudioProcessor& ownerFilter)
    : AudioProcessorEditor(ownerFilter)
{
    addAndMakeVisible (label = new Label ("new label",
                                          TRANS("Allpass Network")));
    label->setFont (Font ("james glover", 27.40f, Font::bold));
    label->setJustificationType (Justification::centredLeft);
    label->setEditable (false, false, false);
    label->setColour (Label::textColourId, Colours::black);
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (DriveSld = new Slider ("Drive Slider"));
    DriveSld->setRange (0, 16, 0.01);
    DriveSld->setSliderStyle (Slider::LinearHorizontal);
    DriveSld->setTextBoxStyle (Slider::TextBoxLeft, false, 80, 20);
    DriveSld->setColour (Slider::thumbColourId, Colours::red);
    DriveSld->setColour (Slider::trackColourId, Colour (0x7f000000));
    DriveSld->setColour (Slider::textBoxTextColourId, Colours::red);
    DriveSld->setColour (Slider::textBoxBackgroundColourId, Colours::black);
    DriveSld->setColour (Slider::textBoxHighlightColourId, Colour (0x40ee1111));
    DriveSld->addListener (this);

    addAndMakeVisible (BypassBtn = new TextButton ("Bypass Button"));
    BypassBtn->setButtonText (TRANS("give up?"));
    BypassBtn->setConnectedEdges (Button::ConnectedOnLeft);
    BypassBtn->addListener (this);
    BypassBtn->setColour (TextButton::buttonColourId, Colours::red);
    BypassBtn->setColour (TextButton::buttonOnColourId, Colour (0xff636363));
    BypassBtn->setColour (TextButton::textColourOnId, Colours::black);


    //[UserPreSize]
    //[/UserPreSize]

    setSize (600, 400);


    //[Constructor] You can add your own custom stuff here..
    getProcessor()->RequestUIUpdate();
    startTimer(200);
    BypassBtn->setClickingTogglesState(true);
    //[/Constructor]
}

AllpasserAudioProcessorEditor::~AllpasserAudioProcessorEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    label = nullptr;
    DriveSld = nullptr;
    BypassBtn = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void AllpasserAudioProcessorEditor::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::grey);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void AllpasserAudioProcessorEditor::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    label->setBounds (0, 0, proportionOfWidth (0.9495f), 28);
    DriveSld->setBounds (8, 32, proportionOfWidth (0.9495f), proportionOfHeight (0.0504f));
    BypassBtn->setBounds (8, 64, proportionOfWidth (0.9495f), proportionOfHeight (0.0504f));
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void AllpasserAudioProcessorEditor::sliderValueChanged (Slider* sliderThatWasMoved)
{
    //[UsersliderValueChanged_Pre]
    AllpasserAudioProcessor* ourProcessor = getProcessor();
    //[/UsersliderValueChanged_Pre]

    if (sliderThatWasMoved == DriveSld)
    {
        //[UserSliderCode_DriveSld] -- add your slider handling code here..
        ourProcessor->setParameter(AllpasserAudioProcessor::Drive, (float)DriveSld->getValue());
        //[/UserSliderCode_DriveSld]
    }

    //[UsersliderValueChanged_Post]
    //[/UsersliderValueChanged_Post]
}

void AllpasserAudioProcessorEditor::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    AllpasserAudioProcessor* ourProcessor = getProcessor();
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == BypassBtn)
    {
        //[UserButtonCode_BypassBtn] -- add your button handler code here..
        ourProcessor->setParameter(AllpasserAudioProcessor::MasterBypass, (float)BypassBtn->getToggleState());
        //[/UserButtonCode_BypassBtn]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void AllpasserAudioProcessorEditor::timerCallback()
{
    AllpasserAudioProcessor* ourProcessor = getProcessor();
    if(ourProcessor->NeedsUIUpdate())
    {
        BypassBtn->setToggleState(1.0f==ourProcessor->getParameter(AllpasserAudioProcessor::MasterBypass), dontSendNotification);
        DriveSld->setValue(ourProcessor->getParameter(AllpasserAudioProcessor::Drive), dontSendNotification);
        ourProcessor->ClearUIUpdateFlag();
    }
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="AllpasserAudioProcessorEditor"
                 componentName="" parentClasses="public AudioProcessorEditor, public Timer"
                 constructorParams="AllpasserAudioProcessor&amp; ownerFilter"
                 variableInitialisers="AudioProcessorEditor(ownerFilter)" snapPixels="8"
                 snapActive="1" snapShown="1" overlayOpacity="0.330" fixedSize="0"
                 initialWidth="600" initialHeight="400">
  <BACKGROUND backgroundColour="ff808080"/>
  <LABEL name="new label" id="205b2d5d7890e507" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="0 0 94.947% 28" posRelativeH="5e0c08a0b8108a7c"
         textCol="ff000000" edTextCol="ff000000" edBkgCol="0" labelText="Allpass Network"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="james glover" fontsize="27.399999999999998579" bold="1"
         italic="0" justification="33"/>
  <SLIDER name="Drive Slider" id="5e0c08a0b8108a7c" memberName="DriveSld"
          virtualName="" explicitFocusOrder="0" pos="8 32 94.947% 5.036%"
          thumbcol="ffff0000" trackcol="7f000000" textboxtext="ffff0000"
          textboxbkgd="ff000000" textboxhighlight="40ee1111" min="0" max="16"
          int="0.010000000000000000208" style="LinearHorizontal" textBoxPos="TextBoxLeft"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1"/>
  <TEXTBUTTON name="Bypass Button" id="5ae4b77fe351266c" memberName="BypassBtn"
              virtualName="" explicitFocusOrder="0" pos="8 64 94.947% 5.036%"
              bgColOff="ffff0000" bgColOn="ff636363" textCol="ff000000" buttonText="give up?"
              connectedEdges="1" needsCallback="1" radioGroupId="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
