#include "daisy_field.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

// Declare classes and variables here

DaisyField hw;
Svf         svf;
NlFilt      nfilt;
Fold        fold;
Wavefolder  wavefolder;
Overdrive   overdrive;
DcBlock     dcblock;

bool  effectOn;
float wet;

float svfCutoff;
float svfLP;
float svfHP;

float nlFiltOut;

float wavefolderGain;
float wavefolderOffset;

float overdriveOut;
float overdriveDrive;

float filFreq;
float filRes;


float effectVolume;
float lastSample;
float finalOut;
char cstr[15];
char displayBuffer[15]; 

size_t page{0};
size_t keyboard_leds[] = {
	DaisyField::LED_KEY_B1,
	DaisyField::LED_KEY_B2,
	DaisyField::LED_KEY_B3,
	DaisyField::LED_KEY_B4,
	DaisyField::LED_KEY_B5,
	DaisyField::LED_KEY_B6,
	DaisyField::LED_KEY_B7,
	DaisyField::LED_KEY_B8,
	DaisyField::LED_KEY_A1,
	DaisyField::LED_KEY_A2,
	DaisyField::LED_KEY_A3,
	DaisyField::LED_KEY_A4,
	DaisyField::LED_KEY_A5,
	DaisyField::LED_KEY_A6,
	DaisyField::LED_KEY_A7,
	DaisyField::LED_KEY_A8,
};


void Controls()
{
    // This bit processes knob movement and translates it into float/init values
    hw.ProcessAllControls();

    /// Add pages to the knobs;
  switch (page){
    case 0:
    svfCutoff = hw.knob[0].Process();
    break;

    case 1:
    wavefolderGain = hw.knob[0].Process();
    break;
  }
    effectOn ^= hw.sw[0].RisingEdge();
}

//handle paging
void handleButton(){
     for (size_t i = 0; i < 16; i++){
            if(hw.KeyboardRisingEdge(i)){
                page = i;
            }
        }
        
}

void drawControl(char * buffer, const char* paramName, int paramValue, int position){
    switch (position){
        case 1:
        hw.display.SetCursor(0, 0);
        break;
        
        case 2:
        hw.display.SetCursor(0 ,10);
        break;

        case 3:
        hw.display.SetCursor(0, 20);
        break;

        case 4:
        hw.display.SetCursor(0, 30);
        break;

        case 5:
        hw.display.SetCursor(70, 0);
        break;

        case 6:
        hw.display.SetCursor(70, 10);
        break;

        case 7:
        hw.display.SetCursor(70, 20);
        break;

        case 8:
        hw.display.SetCursor(70, 30);
        break;
    }   
    sprintf(displayBuffer, "%s%d", paramName, paramValue);
    hw.display.WriteString(displayBuffer, Font_7x10, true);

}
void drawText(char * buffer, const char* paramName){
    hw.display.SetCursor(30, 50);
    sprintf(displayBuffer, "%s", paramName);
    hw.display.WriteString(displayBuffer, Font_7x10, true);
}
void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    Controls();
    handleButton();


    // DSP happenes here
    for(size_t i = 0; i < size; i++)
    {
        // Set Controls Here

        // Soap Controls
        svf.SetFreq(hw.AudioSampleRate() * pow(svfCutoff, 2) * 0.15);
        

        // Wavefolder Controls        the offset has -0.5 so it can ofset to negative too
        wavefolder.SetGain(wavefolderGain);
        wavefolder.SetOffset(wavefolderOffset - 0.5f);
        
        // Overdrive Controls
        overdrive.SetDrive(overdriveDrive);

        out[0][i] = in[0][i];
        out[1][i] = in[0][i];

        // Actual FX code below
        if(effectOn)
        {
            // SVF for split into Nfilt 

			// pha = phaser.Process(in[0][i]) * wet + in[0][i] * (1.f - wet);
           
            svf.Process(in[0][i]);
            svfLP = svf.Low();
            svfHP = svf.High();

            nfilt.ProcessBlock(&svfHP, &nlFiltOut, hw.AudioSampleRate());

			overdriveOut = overdrive.Process(wavefolder.Process(svf.High()));
        

            // Final output
            out[0][i] = out[1][i] = overdriveOut;

        }
    }
}

int main(void)
{
    
    hw.Init();
    float sample_rate = hw.AudioSampleRate();
    page = 0;

    // SVF Init
    svf.Init(sample_rate);
    filFreq = 0.2f;
    filRes = 0.0f;

    //NlFilt
    nfilt.Init();

    // Fold init
    fold.Init();

    // Wavefolder init
    wavefolder.Init();
    wavefolderGain = 0.5f;
    wavefolderOffset = 0.5f;

    // Overdrive init
    overdrive.Init();
    overdriveDrive = 0.1f;

    // DC block init
    dcblock.Init(sample_rate);


    filFreq = 0.5f;
    filRes = 0.5f;

    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    while(1)
    {
        // This bit is responsible for drawing on screen
        
        // example of the new function working
        //Handle Menu Pages draw
       switch (page){
        case 0 :
        drawControl(displayBuffer, "FREQ ",(int)(svfCutoff * 100), 1);

        drawText(displayBuffer, "FILTER      ");
        break;
        case 1:
        drawControl(displayBuffer, "FOLD ",(int)(wavefolderGain * 100), 1);
        drawText(displayBuffer, "WAVEFOLDER");
        break;
       } 

        hw.display.Update();
    }
}