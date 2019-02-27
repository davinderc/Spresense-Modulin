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

static const uint8_t soft_1 = PIN_A4;
static const uint8_t fsr_1 = PIN_A5;

int note_1 = 0;
int note_1_on = 0;
int vibrato_1 = 0;
float set_note = 94;

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
/*
  Note data[17] =
  {
    {262,500},
    {294,500},
    {330,500},
    {349,500},
    {392,500},
    {440,500},
    {494,500},
    {523,1000},

    {523,500},
    {494,500},
    {440,500},
    {392,500},
    {349,500},
    {330,500},
    {294,500},
    {262,1000},

    {0,0}
  };*/
 
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

void setup()
{
  
  theAudio = AudioClass::getInstance();
  theAudio->begin();
  puts("initialization Audio Library");

  theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP);

  theScore.init();
   
}

void loop()
{
  puts("loop!!");

  note_1 = analogRead(soft_1);
  vibrato_1 = analogRead(fsr_1);
  printf("softpot = %d, fsr = %d\n", note_1, vibrato_1);
  //Score::Note theNote = theScore.get();
  //if (theNote.fs == 0) {
    //puts("End,");
    //exit(1);
  //}
  if(note_1 > 1000 or note_1 < 20){
    note_1_on = 0;
  }
  else{
    note_1_on = 1;
    set_note = 94 + (3991*note_1)/ADC_PREC;
  }
  
  theAudio->setBeep(note_1_on,-40,set_note);
  //usleep(theNote.time * 1000);
  //theAudio->setBeep(0,0,0);
  //usleep(1000);

}
