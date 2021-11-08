/////////////////////////////////////////////////////////////////////////
///////////////////////       DEFINES       /////////////////////////////
/////////////////////////////////////////////////////////////////////////

#include <hidef.h>           /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include <string.h>
#include <stdio.h>

/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
///////////////////////   GLOBAL DEFINES    /////////////////////////////
/////////////////////////////////////////////////////////////////////////

#define LCD_DATA PORTK
#define LCD_CTRL PORTK
#define RS 0x01
#define EN 0x02

#define INPUT_OFFSET     10
#define SET_OFFSET       48
#define CURR_TEMP_OFFSET 53

/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
/////////////////////// FUNCTION PROTOTYPES /////////////////////////////
/////////////////////////////////////////////////////////////////////////

void initGPIO(void);

void initLCD(void);

void initErrMsg(void);

unsigned char scanKeypad(void);

unsigned int getTmpSetting(void);

void inputErr(void);

void printErr(void);

unsigned int parseTemp(void);

void initTempSensor(void);

unsigned char readTempSensor(void);

void writeDisplay(void);

void storeTempF2Arr(unsigned int tempF);

////////////////////////////// UTILS ////////////////////////////////////

void MSDelay(unsigned int itime);

void COMWRT4(unsigned char);

void DATWRT4(unsigned char);

void clrDisp(void);

void clearInputArray(void);

void clearSetArray(void); 

void cursorReturn(void);

char* int2char3dig (unsigned int num);

/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
///////////////////////// GLOBAL VARIABLES //////////////////////////////
/////////////////////////////////////////////////////////////////////////

const unsigned char keypad[4][4] = {
'1','2','3','A',
'4','5','6','B',
'7','8','9','C',
'*','0','#','D'
};


unsigned char dispArr[64] = {
'S', 'e', 't', ' ', 'T', 'e', 'm', 'p',        // Row 1 
':', ' ', ' ', ' ', ' ', ' ', ' ', ' ',        // Row 1
' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',        // Not displayed
' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',        // Not displayed
' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',        // Not displayed
'S', 'E', 'T', 'T', 'I', 'N', 'G', ':',        // Row 2
' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',        // Row 2
' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '         // Not displayed
};

const unsigned char errMsg[64] = {
'E', 'R', 'R', ':', ' ', 'E', 'n', 't',
'e', 'r', ' ', 't', 'e', 'm', 'p', ' ',
' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',        // Not displayed
' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',        // Not displayed
' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',        // Not displayed
't', 'h', 'e', 'n', ' ', '\'', 'A', '\'',
'k', 'e', 'y', ' ', '.', '.', '.', ' ',
' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '         // Not displayed
};

unsigned char column, row;

unsigned int setTemp = 999;

unsigned char currTempF = 999;

/////////////////////////////////////////////////////////////////////////

void main(void){                          //OPEN MAIN
   
   unsigned int tempSetting = 999;
   unsigned int tempF = 999;
   
   unsigned int i;
   
   char *currTmpArr;
   
   initGPIO();                            // Initialize GPIO systems
   initLCD();                             // Initialize LCD module
   
   initTempSensor();
   clearInputArray();   
   
   while(1){
      
      //  PORTB = 0xFF; // LEDS on (working)
   
      writeDisplay();
            
      tempSetting = getTmpSetting();
      
      if (tempSetting != 999){            // Error if 999
      
        setTemp = tempSetting;    

      }
      
      tempF = readTempSensor();
      
      if (tempF != 999){                  // Error if 999
      
        currTempF = tempF;
        
        currTmpArr = int2char3dig(currTempF);
        
        for (i = 0; i < 3; i++){
        
          dispArr[i + CURR_TEMP_OFFSET] = currTmpArr[i];
        
        }
      
      }
      
   } 
}

/////////////////////////////////////////////////////////////////////////
////////////////////////  LOCAL FUNCTIONS  //////////////////////////////
/////////////////////////////////////////////////////////////////////////

void initGPIO(void){
  DDRB = 0xFF;                           // MAKE PORTB OUTPUT
  DDRJ = 0xFF;                           // PTJ as output for Dragon12+ LEDs
  DDRA = 0x0F;                           // MAKE ROWS INPUT AND COLUMNS OUTPUT
  PTJ=0x0;                               //Allow the LEDs to dsiplay data on PORTB pins
  DDRP |=0x0F;                           // RGB LED OUTPUTS
  PTP |=0x0F;                            // TURN OFF 7SEG LED
  DDRK = 0xFF;                           // PORTK IS LCD MODULE
  
  return;
}

void initLCD(void){
  DDRK = 0xFF;   
  COMWRT4(0x33);   // reset sequence provided by data sheet
  MSDelay(1);
  COMWRT4(0x32);   // reset sequence provided by data sheet
  MSDelay(1);
  COMWRT4(0x28);   // Function set to four bit data length
                   // 2 line, 5 x 7 dot format
  MSDelay(1);
  COMWRT4(0x06);   // entry mode set, increment, no shift
  MSDelay(1);
  COMWRT4(0x0C);   // Display set, disp on, cursor off, blink off
  MSDelay(1);
  COMWRT4(0x01);   // Clear display
  MSDelay(1);
  COMWRT4(0x80);   // set start posistion, home position
  MSDelay(1);
  
  return;
}

void initTempSensor(void){
    
    ATD0CTL2 = 0x80;                      //Turn on ADC,..No Interrupt
    MSDelay(5);
    ATD0CTL3 = 0x08;                      //one conversion, no FIFO
    ATD0CTL4 = 0xEB;                      //8-bit resolu, 16-clock for 2nd phase,
                                          //prescaler of 24 for Conversion Freq=1MHz
}

unsigned char readTempSensor(void){

    unsigned char tempReading = 999;
    unsigned char tempC = 0;
    unsigned char tempF = 0;

    ATD0CTL5 = 0x85;                      //Channel 5 (right justified, unsigned,single-conver,one chan only) 

    while(!(ATD0STAT0 & 0x80));
    tempReading = ATD0DR0L;
    tempC = tempReading / 4;
    tempC = tempC - 20;                   // Correct temp in C now
    
    tempF = tempC * 9 / 5 + 32;
    
    
    PORTB = ATD0DR0L;                     //dump it on LEDs
    MSDelay(2);                           //optional
    
    return tempF;
}

void storeTempF2Arr(unsigned int tempF){
    
    

}

unsigned char scanKeypad(void){

   unsigned char keyIn = ' ';

   while(1){                              
                              
         PORTA = PORTA | 0x0F;            //COLUMNS SET HIGH
         row = PORTA & 0xF0;              //READ ROWS
         
         if (row == 0x00){                //NO KEY PRESSED
            keyIn = ' ';
            break;
         }

      // Secondary task determines location of key pressed   
      do{                                 
         do{                              
            MSDelay(1);                   
            row = PORTA & 0xF0;           //READ ROWS
         }while(row == 0x00);             //CHECK FOR KEY PRESS //CLOSE do3
         
         MSDelay(10);                     //WAIT FOR DEBOUNCE
         row = PORTA & 0xF0;
      }while(row == 0x00);                //FALSE KEY PRESS //CLOSE do2

      while(1){                           //OPEN while(1)
      
         PORTA &= 0xF0;                   //CLEAR COLUMN
         PORTA |= 0x01;                   //COLUMN 0 SET HIGH
         row = PORTA & 0xF0;              //READ ROWS
         if(row != 0x00){                 //KEY IS IN COLUMN 0
            column = 0;
            break;                        //BREAK OUT OF while(1)
         }
         
         PORTA &= 0xF0;                   //CLEAR COLUMN
         PORTA |= 0x02;                   //COLUMN 1 SET HIGH
         row = PORTA & 0xF0;              //READ ROWS
         if(row != 0x00){                 //KEY IS IN COLUMN 1
            column = 1;
            break;                        //BREAK OUT OF while(1)
         }

         PORTA &= 0xF0;                   //CLEAR COLUMN
         PORTA |= 0x04;                   //COLUMN 2 SET HIGH
         row = PORTA & 0xF0;              //READ ROWS
         if(row != 0x00){                 //KEY IS IN COLUMN 2
            column = 2;
            break;                        //BREAK OUT OF while(1)
         }
         
         PORTA &= 0xF0;                   //CLEAR COLUMN
         PORTA |= 0x08;                   //COLUMN 3 SET HIGH
         row = PORTA & 0xF0;              //READ ROWS
         if(row != 0x00){                 //KEY IS IN COLUMN 3
            column = 3;
            break;                        //BREAK OUT OF while(1)
         }
         
      row = 0;                            //KEY NOT FOUND        
      break;                              //step out of while(1) loop to not get stuck
      
      }                                   //end while(1)

      if(row == 0x10){
         MSDelay(1);
         keyIn = (keypad[0][column]);     
 
      }
      else if(row == 0x20){
         MSDelay(1);
         keyIn = (keypad[1][column]);
 
      }
      else if(row == 0x40){
         MSDelay(1);
         keyIn = (keypad[2][column]);
 
      }
      else if(row == 0x80){
         MSDelay(1);
         keyIn = (keypad[3][column]);
 
      } 
      else if (row == 0x00){
         keyIn = ' ';
        
      }

      do{
         MSDelay(10);
         PORTA = PORTA | 0x0F;            //COLUMNS SET HIGH
         row = PORTA & 0xF0;              //READ ROWS
      }while(row != 0x00);                //MAKE SURE BUTTON IS NOT STILL HELD
  
      if (keyIn != ' '){
        
        return keyIn;
      }
      
   }                                      //CLOSE WHILE(1)
   
  return keyIn; 
      
}

unsigned int getTmpSetting(void){
  unsigned char keyIn = ' ';
  unsigned int charCnt = 0;  
  unsigned int tempSetting = 0;
  unsigned int i = 0;

  while (1){
    
    keyIn = scanKeypad();
    
    if(keyIn != ' ' && keyIn != 'A' && charCnt != 4){
      
      dispArr[charCnt + INPUT_OFFSET] = keyIn;   // Save char to array
      writeDisplay();                            // Keep up with echoing user inputs
      charCnt++;
      
    } else if (charCnt == 4) {                   // Error if temp input is >4 digits
      charCnt = 0;
      inputErr();
      return 999;                                // Return error
      
    }
    
    if (keyIn == 'A'){
    
      for(i = charCnt; i < 4; i++){             // Clear rest of char array
        dispArr[i + INPUT_OFFSET] = ' ';      
      }
    
      tempSetting = parseTemp();      
      clearInputArray();

      break;
            
    }
  }
      
  return tempSetting;
}

unsigned int parseTemp(void){
  unsigned int i, arrSize, tempSetting;
  
  arrSize = 0;
  tempSetting = 999;                           // Error value
  
  for (i = 0; i < 4; i++){
    if ((dispArr[i + INPUT_OFFSET] != ' ') && (dispArr[i + INPUT_OFFSET] != 'A')){ 
      arrSize++;
      dispArr[i + SET_OFFSET] = dispArr[i + INPUT_OFFSET];        // Sets up set array also
    } else {
      dispArr[i + SET_OFFSET] = ' ';                              
    }
  }

  switch (arrSize){
  
    case 3 :
      tempSetting = (100 * ((dispArr[0 + INPUT_OFFSET]) - 48)) + (10 * ((dispArr[1 + INPUT_OFFSET]) - 48)) + ((dispArr[2 + INPUT_OFFSET]) - 48);            
      break;
        
    case 2 :
      tempSetting = (10 * ((dispArr[0 + INPUT_OFFSET]) - 48)) + ((dispArr[1 + INPUT_OFFSET]) - 48);
      break;
      
    case 1 :
      tempSetting = (dispArr[0 + INPUT_OFFSET]) - 48;
      break;
      
    default:    
      inputErr();
      break;  
  }
  
  
  return tempSetting;

}

void inputErr(void){
  
  clrDisp();
  printErr();
  clrDisp();
  clearInputArray();
  clearSetArray();
  
  return;

}

void printErr(void){

  int i = 0;
  int idx = 0;
   
  for (idx = 0; idx < 4; idx++){           // Blink msg 4 times
    
    clrDisp();
      
    for (i = 0; i < 64; i++){
      
      DATWRT4(errMsg[i]);
          
    }
     
    MSDelay(100);
  }
  return;
}

void writeDisplay(void){  
  int i;
   
  clrDisp();
   
  for(i = 0; i < 64; i++){          
    DATWRT4(dispArr[i]);                  // Write character to display
  }
  cursorReturn();                         // Cursor to home position
}



/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
//////////////////////// UTILITY FUNCTIONS //////////////////////////////
/////////////////////////////////////////////////////////////////////////

void MSDelay(unsigned int itime){
unsigned int i; unsigned int j;
   for(i=0;i<itime;i++)
      for(j=0;j<4000;j++);
}

void COMWRT4(unsigned char command){
        
        unsigned char x;
        
        x = (command & 0xF0) >> 2;        // shift high nibble to center of byte for Pk5-Pk2
        LCD_DATA =LCD_DATA & ~0x3C;       // clear bits Pk5-Pk2
        LCD_DATA = LCD_DATA | x;          // sends high nibble to PORTK
        MSDelay(1);
        LCD_CTRL = LCD_CTRL & ~RS;        // set RS to command (RS=0)
        MSDelay(1);
        LCD_CTRL = LCD_CTRL | EN;         // set enable
        MSDelay(5);
        LCD_CTRL = LCD_CTRL & ~EN;        // unset enable to capture command
        MSDelay(15);                      // wait
        x = (command & 0x0F)<< 2;         // shift low nibble to center of byte for Pk5-Pk2
        LCD_DATA =LCD_DATA & ~0x3C;       // clear bits Pk5-Pk2
        LCD_DATA =LCD_DATA | x;           // send low nibble to PORTK
        LCD_CTRL = LCD_CTRL | EN;         // set enable
        MSDelay(5);
        LCD_CTRL = LCD_CTRL & ~EN;        // unset enable to capture command
        MSDelay(15);
  }

void DATWRT4(unsigned char data){
        
        unsigned char x;
               
        x = (data & 0xF0) >> 2;
        LCD_DATA =LCD_DATA & ~0x3C;                     
        LCD_DATA = LCD_DATA | x;
        MSDelay(1);
        LCD_CTRL = LCD_CTRL | RS;
        MSDelay(1);
        LCD_CTRL = LCD_CTRL | EN;
        MSDelay(1);
        LCD_CTRL = LCD_CTRL & ~EN;
        MSDelay(5);
       
        x = (data & 0x0F)<< 2;
        LCD_DATA =LCD_DATA & ~0x3C;                     
        LCD_DATA = LCD_DATA | x;
        LCD_CTRL = LCD_CTRL | EN;
        MSDelay(1);
        LCD_CTRL = LCD_CTRL & ~EN;
        MSDelay(5);
  }
  
void clrDisp(void){
  COMWRT4(0x01);   // Clear display
}

void clearInputArray(void){
  int i;
  for (i = 0; i < 4; i++){
    dispArr[i + INPUT_OFFSET] = ' ';
  }
}

void clearSetArray(void){
  int i;
  for (i = 0; i < 4; i++){
    dispArr[i + SET_OFFSET] = ' ';
  }
}

void cursorReturn(void){

  COMWRT4(0x02);              // Return cursor home 

  return;
}

char* int2char3dig (unsigned int N){
  char charArray[3];
  
  sprintf(charArray, "%d", N);
  
  return charArray; 
}

/////////////////////////////////////////////////////////////////////////