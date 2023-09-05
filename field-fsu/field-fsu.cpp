#include "daisy_field.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

// Declare classes and variables here

DaisyField hw;
Soap        soap;
Fold        fold;
Wavefolder  wavefolder;
Overdrive   overdrive;
DcBlock     dcblock;

bool  effectOn;
float wet;

float bandBase;
float bandWidth;

float wavefolderGain;
float wavefolderOffset;

float overdriveDrive;

float filFreq;
float filRes;

float bandSplit;
float effectVolume;
float lastSample;
float finalOut;


int page{0};
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

    bandBase = hw.knob[0].Process();
    bandWidth = hw.knob[1].Process();
    wavefolderGain = hw.knob[2].Process();

    effectOn ^= hw.sw[0].RisingEdge();
}

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    Controls();



   
    for(size_t i = 0; i < size; i++)
    {
        // Set Controls Here

        // Soap Controls
        soap.SetCenterFreq(hw.AudioSampleRate() * pow(bandBase, 2) * 0.15);
        soap.SetFilterBandwidth(hw.AudioSampleRate() * pow(bandWidth, 2) * 0.15);

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
			// pha = phaser.Process(in[0][i]) * wet + in[0][i] * (1.f - wet);
            bandSplit = soap.Process(in[0][i])
			overdrive = overdrive.Process(wavefolder.Process(bandSplit));
        

            // Final output
            out[0][i] = out[1][i] = svf.Peak();

        }
    }
}

int main(void)
{
    hw.Init();
    float sample_rate = hw.AudioSampleRate();


    // Soap Bandpass filer init
    soap.Init(sample_rate);
    bandBase = 0.25f;
    bandWidth = 0.5f;

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

        for (size_t i = 0; i < 16; i++){
            if(hw.KeyboardRisingEdge(i)){
                sprintf(cstr, "key %d has been pressed", hw.KeyboardRisingEdge(i));
                hw.display.SetCursor(0, 10);
                hw.display.WriteString(cstr, Font_7x10, true);
            }
        }
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