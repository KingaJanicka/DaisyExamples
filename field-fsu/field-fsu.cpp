#include "daisy_field.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

// Declare classes and variables here

DaisyField hw;
Svf         svf;
Fold        fold;
Wavefolder  wavefolder1;
Wavefolder  wavefolder2;
Overdrive   overdrive1;
Overdrive   overdrive2;
Limiter     limiter;
DcBlock     dcblock;

bool  effectOn = 1;
float wet;

float svfCutoff;
float svfLP;
float svfHP;

float wavefolder1Gain;
float wavefolder1Offset;

float wavefolder2Gain;
float wavefolder2Offset;
float overdrive2Out;
float overdrive2Drive;
float wavefolder2BlockWet;


float filFreq;
float filRes;

float limiterPreGain;

float effectVolume;
float lastSample;
float finalOut;
size_t blockSize = 4;

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
void drawMenuLabel(char * buffer, const char* paramName){
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

        // SVF Controls
        svf.SetFreq(hw.AudioSampleRate() * pow(svfCutoff, 2) * 0.15);
        


        // Wavefolder Controls        the offset has -0.5 so it can ofset to negative too
        wavefolder2.SetGain(wavefolder2Gain*15.f);
        wavefolder2.SetOffset(wavefolder2Offset - .5f);
        // Overdrive Controls
        overdrive2.SetDrive(overdrive2Drive);

        out[0][i] = in[0][i];
        out[1][i] = in[0][i];

        // Actual FX code below
        if(effectOn)
        {

			// pha = phaser.Process(in[0][i]) * wet + in[0][i] * (1.f - wet);
           
            svf.Process(in[0][i]);
            svfLP = svf.Low();
            svfHP = svf.High();

			overdrive2Out = overdrive2.Process(wavefolder2.Process(svfHP));
        


            // Final output
            out[0][i] = out[1][i] = overdrive2Out * wavefolder2BlockWet + svfHP * (1.0f - wavefolder2BlockWet) + svfLP;

        }
    }
}

int main(void)
{
    
    hw.Init();
    float sample_rate = hw.AudioSampleRate();
    hw.SetAudioBlockSize(blockSize);
    page = 0;

    // SVF Init
    svf.Init(sample_rate);
    filFreq = 0.2f;
    filRes = 0.0f;

    // Wavefolder1 init
    wavefolder1.Init();
    wavefolder1Gain = 0.5f;
    wavefolder1Offset = 0.5f;

    // Fold init
    fold.Init();

    // Wavefolder2 init
    wavefolder2.Init();
    wavefolder2Gain = 0.1f;
    wavefolder2Offset = 0.1f;
    wavefolder2BlockWet = 1.0f;

    // Overdrive init
    overdrive2.Init();
    overdrive2Drive = 0.1f;

    // Limiter init
    limiter.Init();

    // DC block init
    dcblock.Init(sample_rate);


    filFreq = 0.5f;
    filRes = 0.5f;

    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    while(1)
    {
        // This bit is responsible for drawing on screen
        
        hw.display.Fill(false);
       
        hw.display.SetCursor(00, 50);
        sprintf(displayBuffer, "%s", effectOn ? "On" : "Off", effectOn);
        hw.display.WriteString(displayBuffer, Font_7x10, true);

        //Handle Menu Pages draw and knob assignment
       switch (page){
        case 0 :
        drawControl(displayBuffer, "SPLIT F",(int)(svfCutoff * 100), 3);
        svfCutoff = hw.knob[0].Process();
        drawMenuLabel(displayBuffer, "MIXER");
        break;

        case 1 :
        drawMenuLabel(displayBuffer, "WAVEFOLDER 1");
        break;
        
        case 2 :
   

        drawMenuLabel(displayBuffer, "FILTER");
        break;
        
        case 3:
        drawControl(displayBuffer, "FOLD ",(int)(wavefolder2Gain * 100), 1);
        wavefolder2Gain = hw.knob[0].Process();

        drawControl(displayBuffer, "OFFSET ",(int)(wavefolder2Offset * 100), 2);
        wavefolder2Offset = hw.knob[1].Process();
        
        drawControl(displayBuffer, "DRIVE ",(int)(overdrive2Drive * 100), 3);
        overdrive2Drive = hw.knob[2].Process();

        drawControl(displayBuffer, "WET ",(int)(wavefolder2BlockWet * 100), 4);
        wavefolder2BlockWet = hw.knob[3].Process();
        
        drawMenuLabel(displayBuffer, "WAVEFOLDER 2");
        break;

        case 4:
        drawMenuLabel(displayBuffer, "DECIMATE");
        break;

        case 5:
        drawMenuLabel(displayBuffer, " ");
        break;
       } 

        hw.display.Update();
    }
}