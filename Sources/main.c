//  Michael Nguyen
//  Fan simulator
#include <hidef.h>      /* common defines and macros */
#include <mc9s12dg256.h>     /* derivative information */
#pragma LINK_INFO DERIVATIVE "mc9s12dg256b"
#include "queue.h"
#include "main_asm.h" /* interface to the assembly module */
#include <string.h>
#include <stdio.h>
/////////////////////////////////////////////////////////////
      // variables
////////////////////////////////////////////////////////////
unsigned short dtime ;
int pos = 0,pitch = 250,pitch_jump = 250,cycle = 1, speed, i, total ,pressed=0, tempF,ix, briteness = 250, brightVal, value, running;
char val, c, a;

  
void output_string(char phrase[]);
//////////////////////////////////////////////////////////
         //interrupts
/////////////////////////////////////////////////////////
void interrupt 25 handler() {  //25 is port H interrupt
    pressed=1; // press happened   to give current temp

    PIFH = 0xFF;        //clears portH interrupt flag           
}
  

void interrupt 13 timer_handler() {       //channel 5
  tone(pitch);
}
//////////////////////////////////////////////////////////
         //main function
/////////////////////////////////////////////////////////
void main(void) {
  //variable and phrases for SCI
  char ch=0,pin[4],enter,b,pin2[4],c,ch2=0;
  int i,j;
  int isSame =1, inLoop=1;
  char* phrase = "Please SET a 4 digit pin\n";
  char* phrase2 = "\nRe-enter your pin to unlock program\n";
  char* phrase3 = "\nPin match\nEnjoy the super duper cool FAN program\n";
  char* phrase4 = "\nWrong pin Try again\n";


  PLL_init();        // set system clock frequency to 24 MHz 
  lcd_init();
  initq();
  SCI0_init(9600); 
  enter = inchar0(); 
  keypad_enable();
  seg7_disable(); 
  led_enable(); 
  SW_enable();
  motor0_init();
  ad0_enable();
  DDRM  = 0xFF;       
  ix=0;             //enable RTI and system interrupts
  dtime=1;
  _asm CLI;                  //enable system interrupts
  PIEH = 0xFF;               //enable PTH interrupt
  DDRA = 0x0F;
  sound_init();


//////////////////////////////////////////////////////////
         //pressing enter so to establish SCI connection
/////////////////////////////////////////////////////////

  if(enter = '\r'){//Press enter to start
  output_string(phrase); 
  }

  for(i=0;i<4;i++) {        // storing 4 digit pin
    
     ch=getkey();
     b= hex2asc(ch);
     pin[i] = b;
     ch ='*';
     outchar0(ch);
     wait_keyup();  
  }
  while(inLoop){        // entering a loop to capture pin again
    output_string(phrase2);     
    for(j=0;j<4;j++) 
    {        
         ch2=getkey();
         c = hex2asc(ch2);
         pin2[j] = c;
         outchar0(c);
         wait_keyup();  
    }    
    for(i=0;i<4;i++) 
    {     
      if(pin[i]!=pin2[i]){
        isSame = 0;        
      }
    }
    if(isSame)
    {        
      output_string(phrase3);  
      inLoop=0;
    } 
    else
    {
      output_string(phrase4);
      isSame=1;
    }
  
   
  }//end inLoop  
//////////////////////////////////////////////////////////
         //main while loop
/////////////////////////////////////////////////////////   
  while(1)
  {
   char kp;
   running=0;
   kp = getkey();   
   leds_on(SW1_dip());

    
   if(PORTA == 0xEE||PORTA == 0xDE||PORTA == 0xBE)//1 4 7
   {     
     clear_lcd();
     val = 100;
     motor0(val);
     set_lcd_addr(0x48);
     type_lcd("Low");
     PTM = 0X04;       //port pm2  red
     sound_on();
     ms_delay(100);
     sound_off();     
     running=0;
     dtime=1;     
   } 
   else if(PORTA == 0xED||PORTA == 0xDD||PORTA == 0xBD)//2 5 8
   {  
     clear_lcd();
     set_lcd_addr(0x48);
     type_lcd("Medium");
     val = 175;
     motor0(val);
     PTM = 0X80;//port pm7    blue
     sound_on();
     ms_delay(100);
     sound_off();
     ms_delay(100);
     sound_on();
     ms_delay(100);
     sound_off();
     running=0;     
     dtime=1;
   } 
   else if(PORTA == 0xEB||PORTA == 0xDB||PORTA == 0xBB) // 3 6 9 
   { 
     clear_lcd();
     set_lcd_addr(0x48);
     type_lcd("High");
     val = 240;
     motor0(val);
     PTM = 0X02;    //port pm1      green2
     sound_on();
     ms_delay(100);
     sound_off();
     ms_delay(100);
     sound_on();
     ms_delay(100);
     sound_off();
     ms_delay(100);
     sound_on();
     ms_delay(100);
     sound_off();    
     running=0;     
     dtime=1;          
   } 
   else if(PORTA ==0x7E )//* enable Potentiometer takeover
   {     
   running=1;
   sound_on();
   ms_delay(1000);
   sound_off();
   clear_lcd();
   set_lcd_addr(0x00);
   type_lcd("Manual mode");
   PTM = 0X86;           
     while(running)
     {
       value = ad0conv(2);  //   Ad0 channel2
       value>>2; 
       motor0(value);
       kp = keyscan();
       if(PORTA == 0x7B )// # disable takeover
       {    
         running = 0;     
       } 
     }     
   }
   else if(PORTA == 0x7D)// 0 display light status
   { 
     ad1_enable();
     brightVal = ad1conv(3);     //Ad1 channel 4 photosensor
     brightVal = 1023 - brightVal;
     briteness = brightVal; 
     set_lcd_addr(0x00);
    
     if(briteness>500)
     {   
       type_lcd("Bright");
     } 
     else
     {
       type_lcd("Dark  ");
     }
   }
    
   else
   {
     clear_lcd();
     set_lcd_addr(0x48);
     type_lcd("Off");
     PTM = 0x00;        
     leds_off();
     pos=0;     
     running=0;
     pressed=0; 
   }

   if(pressed==1)
   {
     val = ad0conv(5);    // on board temperature display temp
     val = val >> 1;
     tempF = val*9/5 +32;
     set_lcd_addr(0x40);
     write_int_lcd(tempF);     
   }
   dtime=1;  
   cycle++;
   pitch = cycle*pitch_jump;  
     
   if (cycle == 12) 
   {
     cycle = 1;    //restart pitch   
   }       
  }//close while(1) 
}  //mainvoid


////////////////////////////////////////////////////////////////
//output phrase class
////////////////////////////////////////////////////////////////
void output_string(char phrase[]) 
{
    int i=0;
    while(phrase[i] != 0) 
    {     
      outchar0(phrase[i]);
      i++;
    } 
}

//////////////////////////////////////////////////////////
 // code need revision- cool ideas that need more thought
/////////////////////////////////////////////////////////
/* 
RTI_init();
void interrupt 7 function() {  //rti display dark or bright
    dtime--;
    if(dtime==0){
      brightVal = ad0conv(4);     //channel 4 photosensor
      set_lcd_addr(0x00);
    
      if(brightVal > briteness){   

         type_lcd("Bright");
     
      } else{

         type_lcd("Dark");
       
      }
    }
  clear_RTI_flag();        //clears portH interrupt flag           
}
////////////////////////////////////////////////////
#define _DO	 2554
#define _RE	 2279
#define _MI	 2148
#define _FA  1989
#define _SOL 1706
#define _LA	 1572
#define _SI	 1324

int firstSong[15]  = {_LA,_LA,_SOL,_LA,_LA,_SOL,_SOL,_LA,_SOL,_LA,_LA,_LA,_SOL,_SI,_DO };  // 15 notes to each song
int secondSong[15] = {_DO,_RE,_MI,_FA,_SOL,_LA,_SI,_DO,_RE,_MI,_DO,_RE,_MI,_FA,_SOL };
int thirdSong[15]  = {_LA,_LA,_SOL,_LA,_SOL,_LA,_SI,_DO,_RE,_MI,_SI,_FA,_DO,_SI,_FA};




if((PTM & 0x01)==0x01){
      ms_delay(10);
      for(i = 0; i < 15; ++i){ 
         pitch=thirdSong[i];       //  Sound Third Song
         sound_on();
         ms_delay(500);      //  Delay duration
         sound_off();
      }
        
          
     
   }

 */





