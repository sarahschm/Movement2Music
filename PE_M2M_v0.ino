#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SoftwareSerial.h>
SoftwareSerial PortVirtuel(10, 11);

unsigned char Re_buf [ 11 ] , counter=0 ;

unsigned char sign=0;

float a[3] ,w[3] , angle [3] ,T;

boolean playing = false ;

const byte channel = 0;

const byte baseNote = 72;

int av2;

//---------------Setup-----------------//

void setup(){

  av2 = 0;
  Serial.begin(115200);
  PortVirtuel.begin(31250); //band rate du MIDI

  controlSend(0,0);
  controlSend(32,0);
  programChange(52);
  controlSend(101,0);
  controlSend(100,0);
  controlSend(6,72);
  controlSend(38,0);
  
}


//-----------Loop[Programme principal]---------------//

void loop(){

  //Traduction des données de l'accéléromètre en angle

  if(sign)
  {
    sign = 0;
    if (Re_buf[0]==0x55 && Re_buf[1]==0x53)
    {
      angle[0] = (short(Re_buf[3]<<8| Re_buf[2]))/32768.0*180; 
      //On s'intéresse ici à l'angle selon x
        //Serial.print("a:");
        //Serial.print(a[0]); Serial.print(" ");
        //Serial.print(a[1]); Serial.print(" ");
        //Serial.print(a[2]); Serial.print(" ");
        //Serial.print("w:");
        //Serial.print(w[0]); Serial.print(" ");
        //Serial.print(w[1]); Serial.print(" ");
        //Serial.print(w[2]); Serial.print(" ");
        //Serial.print("angle:");
        //Serial.print(angle[0]); Serial.print("        ");
        //Serial.print("note:"); Serial.print(" ");
        //av2=floor(map(floor(angle[0]),-90,90,0,869));
        //Serial.print(av2);
        //Serial.print(PortVirtuel.read());
        //Serial.print(angle[1]); Serial.print(" ");
        //Serial.print(angle[2]); Serial.print(" ");
        //Serial.print("T:");
        //Serial.println(T);

      //Conversion de l'angle en note

        int av2=floor(map(floor(angle[0]),-90,90,-8191,8192)); //Fréquence note
       //"map" renvoie l'image de l'angle [0] par l'application linéaire qui envoie [-90,90] sur [-8191,8192]. (C'est un changement d'échelle)
       //"floor" la fonction "partie entière". Son utilisation permet de stabiliser la fréquence de la note jouée

       int av1=20; //Volume note

      //Emission des données MIDI

        if(av1 < 30 && av2 < 8193 && av2>-8192){
          
          //Si l'angle mesuré est compris entre -90 et 90...
          if(!playing){
            noteOn(); 
            //... alors si on est pas en train de jouer, on émet une note
          }
          else{
            ModifierNote(av2,av1);
            //... si on est déjà en train de jouer une note, on modifie son Pitch Bend
          }
          
        }
        else {
          
          //Si l'angle mesuré n'est pas compris entre -90 et 90...
          if(playing){
            noteOff();
            //...on arrête de jouer
            
          }
        }
    }
  }
  
}

//S'execute à chaque fois qu'une nouvelle donnée arrive dans le port Serial//

void serialEvent(){

  //Réception des données de l'accéléromètre

  while(Serial.available()){

    Re_buf[counter]=(unsigned char)Serial.read();
    if(counter == 0 && Re_buf[0]!=0x55){
      return;
    }
    counter++;
    if(counter==11){
      counter=0;
      sign=1;
    }
    
  }
  
}

//---------Fonctions appelées-----------//

void noteOff(){

playing = false;
noteSend(0x80, baseNote, 127);
controlSend(64,0);
  
}

void noteOn(){

//note on + sustain on
noteSend(0x90, baseNote, 127);
controlSend(64,127);
playing = true;
  
}

int ModifierNote(int freq, int volume){

int pb = freq;
sendPB(pb);
int vel = volume>>3;
controlSend(7,vel);
  
}

void noteSend(byte cmd, byte data1, byte data2){

  cmd=cmd | channel;
  PortVirtuel.write(cmd);
  PortVirtuel.write(data1);
  PortVirtuel.write(data2);
  
}

void controlSend(byte CCnumber, byte CCdata){

  byte CCchannel = channel | 0xB0; //convert to Controller message
  PortVirtuel.write(CCchannel);
  PortVirtuel.write(CCnumber);
  PortVirtuel.write(CCdata);
  
}

void sendPB(int pb){

  PortVirtuel.write( (byte)0xE0 | channel);
  PortVirtuel.write( pb & (byte)0x7f);
  PortVirtuel.write( (pb>>7) & (byte)0x7f);
  
}

void programChange(byte voice){

  PortVirtuel.write((byte)0xC0 | channel);
  PortVirtuel.write(voice);
  
}

