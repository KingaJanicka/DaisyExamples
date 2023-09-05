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

float wavefolderGain;
float wavefolderOffset;

float overdriveOut;
float overdriveDrive;

float filFreq;
float filRes;


float effectVolume;
float lastSample;
float finalOut;


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

    // wet = hw.knob[0].Process();
    // numstages = 1 + (hw.knob[1].Process() * 8);
    // phaser.SetPoles(numstages);
    // float k = hw.knob[2].Process();
    // phaser.SetLfoFreq(k * k * 20.f);
    // lfo  = hw.knob[3].Process();
    // k    = hw.knob[4].Process();
    // freq = k * k * 7000; //0 - 10 kHz, square curve
    // phaser.SetFeedback(hw.knob[5].Process());

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

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    Controls();
    handleButton();


   
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
        
        hw.display.Fill(false);

        char cstr[15];
        sprintf(cstr, "Page: %d", page);
        hw.display.SetCursor(0, 0);
        hw.display.WriteString(cstr, Font_7x10, true);
        
        
        sprintf(cstr, "Freq: %d", (int)(svfCutoff * 101));
        hw.display.SetCursor(0, 30);
        hw.display.WriteString(cstr, Font_7x10, true);

        //Handle Menu Pages draw
       switch (page){
        case 0 :
        sprintf(cstr, "Freq: %d", (int)(svfCutoff * 101));
        hw.display.SetCursor(0, 10);
        hw.display.WriteString(cstr, Font_7x10, true);
        break;
        case 1:
        sprintf(cstr, "Fld: %d", (int)(wavefolderGain * 101));
        hw.display.SetCursor(0, 20);
        hw.display.WriteString(cstr, Font_7x10, true);
        break;
       } 

        
        // if (page = 0){
        //     sprintf(cstr, "Freq: %d", svfCutoff);
        //     hw.display.SetCursor(0, 10);
        //     hw.display.WriteString(cstr, Font_6x8, true);

        // } else if (page = 1){
        //     sprintf(cstr, "Fld: %d", wavefolderGain);
        //     hw.display.SetCursor(0, 10);
        //     hw.display.WriteString(cstr, Font_6x8, true);
        // }
        
        // sprintf(cstr, "Effect: %s", effectOn ? "On" : "Off");
        // hw.display.SetCursor(0, 0);
        // hw.display.WriteString(cstr, Font_7x10, true);

        // sprintf(cstr, "Dry/Wet: %d", (int)(wet * 101));
        // hw.display.SetCursor(0, 20);
        // hw.display.WriteString(cstr, Font_7x10, true);

        // sprintf(cstr, "Num Poles: %d", numstages);
        // hw.display.SetCursor(0, 10);
        // hw.display.WriteString(cstr, Font_7x10, true);

        // sprintf(cstr, "Freq: %d", (int)(filFreq * 101));
        // hw.display.SetCursor(0, 30);
        // hw.display.WriteString(cstr, Font_7x10, true);
        
        // sprintf(cstr, "Res: %d", (int)(filRes * 101));
        // hw.display.SetCursor(0, 40);
        // hw.display.WriteString(cstr, Font_7x10, true);
        
        hw.display.Update();
    }
}