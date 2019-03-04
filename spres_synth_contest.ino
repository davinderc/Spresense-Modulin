/*
 *  beep.ino - beep example application
 *  Copyright 2018 Sony Semiconductor Solutions Corporation
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <Audio.h>

#define ADC_PREC 1024;

AudioClass *theAudio;

static const uint8_t soft_2 = PIN_A4;
static const uint8_t soft_1 = PIN_A5;
static const uint8_t fsr_1 = PIN_A3;

int note_1 = 0;
int note_2 = 0;
int fsr_val = 0;
int note_1_on = 0;
int note_2_on = 0;
int vibrato_1 = 0;
float vibratoAmp = 1;
float envelopeAmp = 1;
float set_note = 94; // Minimum value accepted by setBeep
float curr_time = 0;
unsigned int curr_clock = 0;
float outputAmp = 0.0;

float note_freqs[12]{
  261.62,
  277.18,
  293.66,
  311.13,
  
  329.63,
  349.23,
  369.99,
  391.99,
  
  415.30,
  440.00,
  466.16,
  493.88
};

struct vibrato{
  float vibStartTime;
  float freq;
  bool noteStatus;

  vibrato(){
    vibStartTime = 0;
    freq = 0;
    noteStatus = false;
  }
  float vibAmp(float dTimeOn){
    if(noteStatus){
      return sin(62.8*dTimeOn);
    }
    else{
      return 1;
    }
  }
  
  void noteOn(float dTimeOn){
    vibStartTime = dTimeOn;
    noteStatus = true;
  }

  void noteOff(){
    freq = 0;
    noteStatus = false;
  }
  
  bool getStatus(void){
    return noteStatus;
  }
};

struct envelopeADSR{
  float attackTime;
  float decayTime;
  float attackAmp;
  float sustainAmp;
  float releaseTime;
  
  float triggerOnTime;
  float triggerOffTime;
  bool noteStatus;

  envelopeADSR(){
    attackTime = 0.250;
    decayTime = 0.5;
    attackAmp = -25;
    sustainAmp = -37;
    releaseTime = 0.500;
    triggerOnTime = 0.0;
    triggerOffTime = 0.0;
    noteStatus = false;
  }

  float getAmp(float dTime){
    float amp = 0.0;
    float lifeTime = dTime - triggerOnTime;

    if(noteStatus){
      // ADS

      // Attack
      if(lifeTime <= attackTime){
        amp = (lifeTime/attackTime)*attackAmp;
      }

      // Decay
      if(lifeTime > attackTime && lifeTime <= (attackTime + decayTime)){
        amp = ((lifeTime - attackTime)/decayTime)*(sustainAmp - attackAmp) + attackAmp;
      }

      // Sustain
      if(lifeTime > (attackTime + decayTime)){
        amp = sustainAmp;
      }
    }

    else{
      // Release
      amp = ((dTime - triggerOffTime)/releaseTime)*(0.0 - sustainAmp) + sustainAmp;
    }

    if(amp <= 0.0001 && amp >= -0.0001){
      amp = 0;
    }
    
    return amp;
  }
  
  void noteOn(float dTimeOn){
    triggerOnTime = dTimeOn;
    noteStatus = true;
  }

  void noteOff(float dTimeOff){
    triggerOffTime = dTimeOff;
    noteStatus = false;
  }
  bool getStatus(void){
    return noteStatus;
  }
};

unsigned int cycle = 25;
int timerInt = 0;

envelopeADSR envelope;
vibrato vibratoInst;

void setup()
{

  attachTimerInterrupt(isr,cycle);
  theAudio = AudioClass::getInstance();
  theAudio->begin();
  puts("initialization Audio Library");

  theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP);

  //theScore.init();
   
}

unsigned int isr(void) {
  timerInt = 1;
  curr_clock += 25;
  return cycle;
}

float get_freq(int analog_val){
  if(analog_val > 980){
    return note_freqs[11];
  }
  if(analog_val > 960){
    return note_freqs[10];
  }
  if(analog_val > 940){
    return note_freqs[9];
  }
  if(analog_val > 910){
    return note_freqs[8];
  }
  if(analog_val > 888){
    return note_freqs[7];
  }
  if(analog_val > 850){
    return note_freqs[6];
  }
  if(analog_val > 800){
    return note_freqs[5];
  }
  if(analog_val > 733){
    return note_freqs[4];
  }
  if(analog_val > 630){
    return note_freqs[3];
  }
  if(analog_val > 475){
    return note_freqs[2];
  }
  if(analog_val > 240){
    return note_freqs[1];
  }
  else{
    return note_freqs[0];
  }

}

void loop()
{
  puts("loop!!");
  printf("\t\t\t\tCurrent clock = %d, amp %d\n", curr_clock, 100*vibratoAmp);

  note_1 = analogRead(soft_1);
  note_2 = analogRead(soft_2);
  fsr_val = analogRead(fsr_1);
  printf("envelopeAmp = %d\n", 100*envelopeAmp);
  curr_time = 0.000001*curr_clock;
  if(note_2 > 1000 or note_2 < 1){
    note_2_on = 0;
    if(envelope.getStatus()){
      envelope.noteOff(curr_time);
      vibratoInst.noteOff();
    }
  }
  else{
    note_2_on = 1;
    if(!envelope.getStatus()){
      envelope.noteOn(curr_time);
      vibratoInst.noteOn(curr_time);
    }
    set_note = get_freq(note_2);
  }

  
  vibratoAmp = vibratoInst.vibAmp(curr_time);
  envelopeAmp = envelope.getAmp(curr_time);
  outputAmp = vibratoAmp*envelopeAmp;

  theAudio->setBeep(note_2_on,outputAmp,set_note);
}
