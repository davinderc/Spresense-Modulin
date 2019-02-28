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

int note_1 = 0;
int note_2 = 0;
int note_1_on = 0;
int note_2_on = 0;
int vibrato_1 = 0;
float set_note = 94;
float curr_time = 0;
unsigned int curr_clock = 0;

struct envelopeADSR{
  float attackTime;
  float decayTime;
  float sustainAmp;
  float releaseTime;
  float attackAmp;
  
  float triggerOnTime;
  float triggerOffTime;
  bool noteStatus;

  envelopeADSR(){
    attackTime = 0.100;
    decayTime = 0.01;
    attackAmp = 1.0;
    sustainAmp = 0.8;
    releaseTime = 0.200;
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

    if(amp <= 0.0001){
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
};

unsigned int cycle = 25;
int timerInt = 0;

class Score
{
public:

  typedef struct {
    int fs;  
    int time;
  } Note;

  void init(){
    pos = 0;
  }
  
  Note get(){
    if(pos == 20){
      pos = 0;
    }
    return data[pos++];
  }
  
private:

  int pos;
 
  Note data[5] =
  {
    {100,1000},
    {120,1000},
    {150,1000},
    {180,1000},
    {2500,1000}
  };
  
};

Score theScore;
envelopeADSR envelope;

void setup()
{

  attachTimerInterrupt(isr,cycle);
  theAudio = AudioClass::getInstance();
  theAudio->begin();
  puts("initialization Audio Library");

  theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP);

  theScore.init();
   
}

unsigned int isr(void) {
  timerInt = 1;
  curr_clock += 25;
  return cycle;
}

void loop()
{
  puts("loop!!");
  printf("Current clock = %d", curr_clock);

  note_1 = analogRead(soft_1);
  note_2 = analogRead(soft_2);
  printf("softpot = %d, fsr = %d\n", note_1, vibrato_1);
  //Score::Note theNote = theScore.get();
  //if (theNote.fs == 0) {
    //puts("End,");
    //exit(1);
  //}
  if(note_1 > 1000 or note_1 < 20){
    note_1_on = 0;
    //duration = 0;
  }
  else{
    note_1_on = 1;
    set_note = 150 + (450*note_1)/ADC_PREC;
    //duration += 1;
  }

  curr_time = 0.01;
  envelope.noteOn(curr_time);
  float outputAmp = -40*envelope.getAmp(curr_time);
  theAudio->setBeep(note_1_on,outputAmp,set_note);
  //usleep(theNote.time * 1000);
  //theAudio->setBeep(0,0,0);
  //usleep(1000);

}
