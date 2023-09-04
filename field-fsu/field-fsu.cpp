#include "daisy_field.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisyField hw;
Phaser     phaser;
Svf       svf;

bool  effectOn;
float wet;

float freqtarget, freq;
float lfotarget, lfo;
int   numstages;

float filFreq;
float filRes;

float pha;
float fil;

void Controls()
{
    hw.ProcessAllControls();

    wet = hw.knob[0].Process();

    numstages = 1 + (hw.knob[1].Process() * 8);
    phaser.SetPoles(numstages);

    float k = hw.knob[2].Process();
    phaser.SetLfoFreq(k * k * 20.f);
    lfo  = hw.knob[3].Process();
    k    = hw.knob[4].Process();
    freq = k * k * 7000; //0 - 10 kHz, square curve
    phaser.SetFeedback(hw.knob[5].Process());
	filFreq = hw.knob[6].Process();
	filRes = hw.knob[7].Process();

    //footswitch
    effectOn ^= hw.sw[0].RisingEdge();
}

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    Controls();



   
    for(size_t i = 0; i < size; i++)
    {
        fonepole(freq, freqtarget, .0001f); //smooth at audio rate
        phaser.SetFreq(freq);

        fonepole(lfo, lfotarget, .0001f); //smooth at audio rate
        phaser.SetLfoDepth(lfo);

		svf.SetFreq(hw.AudioSampleRate() * pow(filFreq, 2) * 0.15);
		svf.SetRes(filRes);

        out[0][i] = in[0][i];
        out[1][i] = in[0][i];

        if(effectOn)
        {
			pha = phaser.Process(in[0][i]) * wet + in[0][i] * (1.f - wet);

            
			svf.Process(pha);
        

            out[0][i] = out[1][i] = svf.Peak();

        }
    }
}

int main(void)
{
    hw.Init();
    float sample_rate = hw.AudioSampleRate();

    phaser.Init(sample_rate);

    effectOn   = true;
    wet        = .9f;
    freqtarget = freq = 0.f;
    lfotarget = lfo = 0.f;
    numstages       = 4;
 
    svf.Init(sample_rate);

    filFreq = 0.5f;
    filRes = 0.5f;

    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    while(1)
    {
        hw.display.Fill(false);

        char cstr[15];
        sprintf(cstr, "Effect: %s", effectOn ? "On" : "Off");
        hw.display.SetCursor(0, 0);
        hw.display.WriteString(cstr, Font_7x10, true);

        sprintf(cstr, "Dry/Wet: %d", (int)(wet * 101));
        hw.display.SetCursor(0, 20);
        hw.display.WriteString(cstr, Font_7x10, true);

        sprintf(cstr, "Num Poles: %d", numstages);
        hw.display.SetCursor(0, 10);
        hw.display.WriteString(cstr, Font_7x10, true);

        sprintf(cstr, "Freq: %d", (int)(filFreq * 101));
        hw.display.SetCursor(0, 30);
        hw.display.WriteString(cstr, Font_7x10, true);
        
        sprintf(cstr, "Res: %d", (int)(filRes * 101));
        hw.display.SetCursor(0, 40);
        hw.display.WriteString(cstr, Font_7x10, true);
        
        hw.display.Update();
    }
}