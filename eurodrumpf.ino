/*
  Analog input, analog output, serial output

A1  POT
A2  POT 
A4  POT
A4  POT
A5  POT

D2  input jack (via 10K resistor)
D3  input jack (via 10K resistor)
D4  output (via diode)
D5  output (via diode)
D6  output (via diode)

 */

#define PRINT_DEBUG false


#define CLK_PIN 2
#define RST_PIN 3
#define BD_PIN 4
#define MD_PIN 5
#define HD_PIN 6

#define BD_MASK (1<<BD_PIN)
#define MD_MASK (1<<MD_PIN)
#define HD_MASK (1<<HD_PIN)

#define BD_DENSITY_PIN A5
#define MD_DENSITY_PIN A4
#define MD_OFFSET_PIN A3

#define HD_DENSITY_PIN A2
#define HD_OFFSET_PIN A1

int analogInPin = A5;  // Analog input pin that the potentiometer is attached to


volatile byte clock_count=0;
volatile unsigned long clock_start=0;
volatile unsigned long clock_stop=0;

#define SEQ_LENGTH 16
volatile unsigned char beats_buffer1[SEQ_LENGTH] = 
{
 
 B00000000 ,
 B00000000 ,
 B00000000 , 
 B00000000 ,
 B00000000 , 
 B00000000 ,
 B00000000 , 
 B00000000 ,
 B00000000 , 
 B00000000 ,
 B00000000 , 
 B00000000 ,
 B00000000 ,
 B00000000 ,
 B00000000 , 
 B00000000
};

volatile unsigned char beats_buffer2[SEQ_LENGTH] = 
{
 
 B00000000 ,
 B00000000 ,
 B00000000 , 
 B00000000 ,
 B00000000 , 
 B00000000 ,
 B00000000 , 
 B00000000 ,
 B00000000 , 
 B00000000 ,
 B00000000 , 
 B00000000 ,
 B00000000 ,
 B00000000 ,
 B00000000 , 
 B00000000 
};

volatile unsigned char* play_buffer=beats_buffer1;
volatile unsigned char* edit_buffer=beats_buffer2;
volatile unsigned char * tmp_ptr;

 const byte BASS_BEAT_ORDER[8]={0,8,12,4,14,6,2,10};

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
  pinMode(CLK_PIN,INPUT); // can't be INPUT_PULLUP because of the 10k resistor
  pinMode(RST_PIN,INPUT); // can't be INPUT_PULLUP because of the 10k resistor
  
  attachInterrupt(digitalPinToInterrupt(CLK_PIN), clock_change_isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RST_PIN), reset_isr, RISING);
  
  pinMode(BD_PIN, OUTPUT); 
  pinMode(MD_PIN, OUTPUT); 
  pinMode(HD_PIN, OUTPUT); 
  
    
}


void distribute_beats(int number_of_beats,  int offset,byte note_mask) {
 
   if (offset<4) {
      distribute_euclid(number_of_beats,offset,note_mask);
   } else {
      distribute_cyclic(number_of_beats,offset,note_mask);
   }
}
void distribute_euclid(int number_of_beats,  int offset,byte note_mask) {
  int i  = offset;

  int pauses  = SEQ_LENGTH - number_of_beats;
  if (number_of_beats >= 0) {
    int per_pulse = (pauses / number_of_beats);
    int remainder = pauses % number_of_beats;

    for (int pulse = 0; pulse < number_of_beats; pulse++) {
      edit_buffer[i % SEQ_LENGTH] |= note_mask;
      i += 1;
      i += per_pulse;
      if (pulse < remainder) {
        i++;
      }
    }
  }
}

void distribute_cyclic(int number_of_beats,  int offset,byte note_mask) {
  if (number_of_beats >= 0) {
    int i=offset;
    for (int j = 0; j < number_of_beats; j++) {
      while ((edit_buffer[i % SEQ_LENGTH] & note_mask)>0) {
        i++;
    }
      edit_buffer[i % SEQ_LENGTH] |= note_mask;
      i = ((i+offset) % SEQ_LENGTH);
    }
  }
}


void clock_change_isr() {

  
   if (digitalRead (CLK_PIN) == LOW) {
//  if ((PORTD & (1<<CLK_PIN))==0) {
    //falling clock
     PORTD = PORTD & B10001111;  //turn off pins 4,5,6 while leaving rest alone
    clock_stop=millis();
  } else {
    //rising clock
    unsigned char beat=play_buffer[clock_count];  //turn on pins 4,5 and/or 6 while leaving rest alone
    PORTD = PORTD | beat;
    clock_count++;
    if (clock_count>=SEQ_LENGTH) {
        clock_count=0;
    }      
      
    clock_start=millis();

  }
}

void reset_isr() {
  clock_count=0;
}


void loop() {
int i;
  //clear the edit buffer;
   for (i=0;i<SEQ_LENGTH;i++) {
       edit_buffer[i]=0;
   } 

   int bd_density=  map(analogRead(BD_DENSITY_PIN),0,1023,7,0);  
  
   for (i=0;i<=bd_density;i++) {
       edit_buffer[BASS_BEAT_ORDER[i]]+=BD_MASK;
   } 


   int md_density=  map(analogRead(MD_DENSITY_PIN),0,1023,16,0);  

   int md_offset=  map(analogRead(MD_OFFSET_PIN),0,1023,8,0);  

    
    distribute_beats(md_density, md_offset,MD_MASK);


   int hd_density=  map(analogRead(HD_DENSITY_PIN),0,1023,16,0);  

   int hd_offset=  map(analogRead(HD_OFFSET_PIN),0,1023,8,0);  
    
    distribute_beats(hd_density, hd_offset,HD_MASK);

    //make the 'edit' buffer live 

    tmp_ptr=play_buffer;
    play_buffer=edit_buffer;
    edit_buffer=tmp_ptr;



//not printing to serial port makes the arduino respond faster to changes in the knobs
if (PRINT_DEBUG) {
//dump the beat out

    Serial.print("BD DENSITY: ");
    Serial.print(bd_density);   
    Serial.print(" MD DENSITY: ");
    Serial.print(md_density);
    Serial.print(" MD OFFSET: ");
    Serial.print(md_offset);   
    Serial.print(" HD DENSITY: ");
    Serial.print(hd_density);   
    Serial.print(" HD OFFSET: ");
    Serial.print(hd_offset);

      Serial.print("\n");        
   for (i=0;i<SEQ_LENGTH;i++) {
       Serial.print("BEAT #: ");
       Serial.print(i);
       Serial.print("\t");       
       if ((edit_buffer[i] & BD_MASK)==0) {
         Serial.print("..");                
       } else {
         Serial.print(",B");       
       }
       if ((edit_buffer[i] & MD_MASK)==0) {
         Serial.print("..");                
       } else {
         Serial.print(",M");       
       }
       
       if ((edit_buffer[i] & HD_MASK)==0) {
         Serial.print("..");                
       } else {
         Serial.print(",H");       
       }
       
      Serial.print("\n");              
   } 

  
  
  
  for (analogInPin=1;analogInPin<6;analogInPin++) {
    // read the analog in value:
    int sensorValue = analogRead(analogInPin);
    // map it to the range of the analog out:
    int outputValue = map(sensorValue, 0, 1023, 0, 255);
    // change the analog out value:
    // print the results to the serial monitor:

    Serial.print("\tA");
    Serial.print(analogInPin);
    Serial.print("  = ");
    Serial.print(outputValue);    
 }

    Serial.print("\n");
  

    signed short clock_interval=clock_stop-clock_start;
    Serial.print("CLOCK COUNT: ");
    Serial.print(clock_count);
    Serial.print(" CLOCK TIME: ");
    Serial.print(clock_interval);
    Serial.print(" CLOCK START: ");
    Serial.print(clock_start);
    Serial.print(" CLOCK STOP: ");
    Serial.print(clock_stop);
    
    Serial.print("\n");
}   


}
