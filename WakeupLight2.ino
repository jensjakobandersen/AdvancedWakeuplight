/*******************************************************
WakeupLight2 
by
Jens Jakob Andersen
jayjay@zorck.dk
22 October 2020


Original code from:
Wakeup Light
by
Ralph Crützen

Hardware:
- Arduino Mega
- LCD + button shield
- DS3231 Real time clock (http://www.rinkydinkelectronics.com/library.php?id=73)
LED strip: 12V RGB LED:
https://ardustore.dk/produkt/led-strip-5050-rgb-12v-dc

LCD and button code:
  based on a sketch by Mark Bramwell, July 2010
  https://www.dfrobot.com/wiki/index.php/Arduino_LCD_KeyPad_Shield_(SKU:_DFR0009)
********************************************************/

#include <LiquidCrystal.h>
#include <DS3231.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

DS3231  rtc(SDA, SCL);

#define backlightPin 10

// LCD variables and constants
int lcdKey     = 0;
int adcKeyIn   = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

#define RED_PIN 44
#define GREEN_PIN 45
#define BLUE_PIN 46

// Color modes
#define cmStatic 0
#define cmAnimation 1
#define cmOff 2
#define cmPulse 3

Time t;

int alarmh = 0;
int alarmm = 0;
boolean alarmSet = false;

int r = 71;
int g = 20;
int b = 10;
int colorMode = cmAnimation;
boolean lightOn = true;

boolean toMainMenu = false;

uint16_t j = 0;

float RED_IDLES = 2;
float GREEN_IDLES = 4;
float BLUE_IDLES = 6;

  int RWidth = 80;
  int GWidth = 40;
  int BWidth = 90;

  int RBreath = 200;
  int GBreath = 40;
  int BBreath = 80;

  int RMin;
  int RMax;
  int GMin;
  int GMax;
  int BMin;
  int BMax;

  int RSpeed = 70;
  int GSpeed = 10;
  int BSpeed = 40;

  int RDelta = 1;
  int GDelta = 1;
  int BDelta = 1;


  
  
//   brightness level of each color (0..255)
  int Brightness_R;
  int Brightness_G;
  int Brightness_B;
  
//   direction of change in brightness level (-1 or +1)
  int Direction_R;
  int Direction_G;
  int Direction_B;
  
//   counter of loop cycles left to wait before changing the pin value
  int Timer_R;
  int Timer_G;
  int Timer_B;


void stripsetPixelColor(int i, int r, int g, int b)
  {
  analogWrite (RED_PIN, r);     // send the red brightness level to the red LED's pin
  analogWrite (GREEN_PIN, g);
  analogWrite (BLUE_PIN, b);
    
  
 } ;


void setup() {
  lcd.begin(16, 2);
    rtc.begin();
  Serial.begin(9600);
  
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  
  pinMode(backlightPin, OUTPUT);
  digitalWrite(backlightPin, HIGH);

  Brightness_R = 140;
  Brightness_G = 60;
  Brightness_B = 20;

 //  set the initial direction
  Direction_R = 1;
  Direction_G = -1;
  Direction_B = 1;

//   set the initial timer values to the full count
  Timer_R = RED_IDLES;
  Timer_G = GREEN_IDLES;
  Timer_B = BLUE_IDLES;
  
  stripsetPixelColor(1, 10,10,10);
  SetPulseParams();
  
  //strip.show();
  //rtc.setDOW(SUNDAY);         // Set Day-of-Week to SUNDAY
  //rtc.setTime(10, 35, 0);     // Set the time to 10:35:00 (24hr format)
  //rtc.setDate(6, 11, 2016);   // Set the date to November 6th, 2016
}

 void DoRainbow()
 {
   Timer_R = Timer_R - 1;
    Timer_G = Timer_G - 1;
    Timer_B = Timer_B - 1;

    
//     if a color's idle timer has expired, change its brightness level, reverse the direction of change if completely bright or dark, and reset the timer
      if (Timer_R == 0)
      {(Brightness_R = Brightness_R + Direction_R); (Timer_R = RED_IDLES);}
      if (Brightness_R == 0 or Brightness_R == 255) {Direction_R = Direction_R * -1;}
      
    
    if (Timer_G == 0) 
    {(Brightness_G = Brightness_G + Direction_G); (Timer_G = GREEN_IDLES);}  
    if (Brightness_G == 0 or Brightness_G == 255) {Direction_G = Direction_G * -1;} 

    if (Timer_B == 0) 
    {(Brightness_B = Brightness_B + Direction_B); (Timer_B = BLUE_IDLES);}
    if (Brightness_B == 0 or Brightness_B == 255) {Direction_B = Direction_B * -1;}   
     
    Brightness_B = constrain(Brightness_B, 0, 255);
    Brightness_G = constrain(Brightness_G, 0, 255);
    Brightness_R = constrain(Brightness_R, 0, 255); //brightness_r + direction_r?

    stripsetPixelColor (1,Brightness_R, Brightness_G, Brightness_B);
  
 }

void loop() {
  lcdKey = readLCDButtons();
  if (lcdKey == btnSELECT) {
    menuSetAlarm();
    toMainMenu = false;
    delay(300);
  }
   else if (lcdKey == btnLEFT) {
        colorMode = cmOff;
  }
  else if (lcdKey == btnRIGHT) {
        colorMode = cmPulse;
  }
  else if (lcdKey == btnDOWN) {
        colorMode = cmAnimation;
  }
  else if (lcdKey == btnUP) {
     wakeyWakey();
  }
  else if (lcdKey != btnNONE) {
    lightOn  = !lightOn;
    delay(300);
  }

  //if (colorMode == cmAnimation && !alarmSet && lightOn) 
  if (colorMode == cmAnimation && lightOn) 
  {
    DoRainbow();
  }
  //else if (colorMode == cmStatic && !alarmSet && lightOn) 
  else if (colorMode == cmStatic && lightOn) 
  {    
      stripsetPixelColor(1, r,g,b);   
  }
  else if (colorMode == cmOff)
  {
      stripsetPixelColor(1, 0,0,0);     
  }
  else if (colorMode == cmPulse)
  {
    Breathe();
  }
  //if (lightOn && !alarmSet)
  if (lightOn)
    digitalWrite(backlightPin, HIGH);
  else
    digitalWrite(backlightPin, LOW);
  
  lcd.setCursor(0,0);
  lcd.print("It is  "); // "It's"
  lcd.print(rtc.getTimeStr());
  lcd.setCursor(0,1);
  lcd.print("Alarm  "); // "Alarm"
  if (alarmSet) {
    if (alarmh < 10) lcd.print(0); lcd.print(alarmh); lcd.print(":");
    if (alarmm < 10) lcd.print(0); lcd.print(alarmm);
    t = rtc.getTime();
    if (alarmh == t.hour && alarmm == t.min) {
      wakeyWakey();
      alarmSet = false;
    }
  }
  else lcd.print("Off"); // "Off"
  delay(100);
}

//****************************
//** Alarm settings
//****************************

void wakeyWakey() {
  digitalWrite(backlightPin, HIGH);
  lcd.clear();
  lcd.print("Wake up!"); // "Wake up!"
  delay(500);
  for (int c = 0; c < 10; c++) 
  {
    lcd.setCursor(0,1);
    lcd.print(10 - c); lcd.print("   ");

    for(uint16_t i=0; i<200; i++) 
    {
      stripsetPixelColor(i, i,0,0);
      lcd.clear();
      lcd.print("Wake up! "); // "Wake up!"
      lcd.print (i);
      lcd.setCursor(0,1);
       lcd.print(rtc.getTimeStr());
              delay(200);
    
      lcdKey = readLCDButtons();
      if (lcdKey != btnNONE) return;
    }


for(uint16_t i=0; i<30; i++) 
    {
      stripsetPixelColor(i, 200,i,i+10);
      lcd.clear();
      
    lcd.print("Wake up! "); // "Wake up!"
      lcd.print (i);
      lcd.setCursor(0,1);
       lcd.print(rtc.getTimeStr());
            delay(300);
    
      lcdKey = readLCDButtons();
      if (lcdKey != btnNONE) return;
    }
    
    for(uint16_t i=0; i<55; i++) 
    {
      stripsetPixelColor(i, 200+i,i,i+30);
      lcd.clear();
      
    lcd.print("Wake up! "); // "Wake up!"
      lcd.print (i);
      lcd.setCursor(0,1);
       lcd.print(rtc.getTimeStr());
            delay(300);
    
      lcdKey = readLCDButtons();
      if (lcdKey != btnNONE) return;
    }

  for(uint16_t i=0; i<120; i++) 
    {
      stripsetPixelColor(i, 255,255,255);
      lcd.clear();
      
    lcd.print("Wake up! "); // "Wake up!"
      lcd.print (i);
      lcd.setCursor(0,1);
       lcd.print(rtc.getTimeStr());
            delay(300);
    
      lcdKey = readLCDButtons();
      if (lcdKey != btnNONE) return;
    }
    
  }
}

void SetPulseParams()
{
RMin = RBreath-(RWidth/2);
  if (RMin<0)
  {
     RMin=0;
  }
  
  RMax = (RWidth/2);
  RMax = RMax+RBreath;
  if (RMax>255)
  {
     RMax=255;
  }

   
  GMin = GBreath-(GWidth/2);
  if (GMin<0)
  {
     GMin=0;
  }
  GMax = GBreath+(GWidth/2);
  if (GMax>255)
  {
     GMax=255;
  }

  BMin = BBreath-(BWidth/2);
  if (BMin<0)
  {
     BMin=0;
  }
  BMax = BBreath+(BWidth/2);
  if (BMax>255)
  {
     BMax=255;
  }

/*

 If needed for debugging....

Serial.print ("Rmin: ");
Serial.print (RMin);
Serial.print (" - Rmax: ");
Serial.print (RMax);

Serial.print ("  Gmin: ");
Serial.print (GMin);
Serial.print (" - Gmax: ");
Serial.print (GMax);

Serial.print ("  Bmin: ");
Serial.print (BMin);
Serial.print (" - Bmax: ");
Serial.print (BMax);
*/

  
  RDelta = RWidth/RSpeed;  
  GDelta = GWidth/GSpeed;  
  BDelta = BWidth/BSpeed; 

  if (RDelta<1)
  {
    RDelta=1;
  }
  if (GDelta<1)
  {
    GDelta=1;
  }
  if (BDelta<1)
  {
    BDelta=1;
  }


}

void Breathe()  // aka Pulse
{
    
    for(uint16_t i=0; i<30; i++) 
    {
      stripsetPixelColor(1, RBreath,GBreath,BBreath);

     Serial.print (RBreath);
     Serial.print (" - ");
     Serial.print (GBreath);
     Serial.print (" - ");
     Serial.println (BBreath);
     /*
     Serial.print (" - xx : ");
     Serial.print (RDelta);
     Serial.print (" : ");
     Serial.print (GDelta);
     Serial.print (" : ");
     Serial.println (BDelta);

     */
      RBreath = RBreath + RDelta;
            
      if (RBreath >= RMax || RBreath <= RMin)
      {
        RDelta=RDelta*-1;
      }
      if (RBreath <1)
      {
      RBreath=0;
      } else if (RBreath >255)
      {
      RBreath=255;
      }
       
      GBreath = GBreath + GDelta;
      if (GBreath>=GMax || GBreath<=GMin)
      {
        GDelta=GDelta*-1;
      }

    if (GBreath <1)
      {
      GBreath=0;
      } else if (GBreath >255)
      {
      GBreath=255;
      }

      BBreath = BBreath + BDelta;
      if (BBreath>=BMax || BBreath<=BMin)
      {
        BDelta=BDelta*-1;
      }

  if (BBreath <1)
      {
      BBreath=0;
      } else if (BBreath >255)
      {
      BBreath=255;
      }
      delay(40);
          lcdKey = readLCDButtons();
      if (lcdKey != btnNONE) return;

      }
}


void menuSetAlarm() {
  lcd.clear();
  lcd.print("Set alarm"); // "Set alarm"
  delay(500);
  while (true) {
    lcdKey = readLCDButtons();
    while (lcdKey == btnNONE) {
      lcdKey = readLCDButtons();
    }
    if (lcdKey == btnSELECT) setAlarmOnOff();
    if (lcdKey == btnUP) menuClock();
    if (lcdKey == btnDOWN) menuSetTime();
    if (toMainMenu) {
      return;
    }
  }
}

void setAlarmOnOff() {
  lcd.clear();
  if (alarmSet) {
    lcd.print("> Alarm on"); // "Alarm on"
    lcd.setCursor(0,1);
    lcd.print("  Alarm off"); // "Alarm off"
  }
  else {
    lcd.print("  Alarm on");
    lcd.setCursor(0,1);
    lcd.print("> Alarm off");      
  }
  delay(500);
  while (true) {
    lcdKey = readLCDButtons();
    while (lcdKey == btnNONE) {
      lcdKey = readLCDButtons();
    }
    if (lcdKey == btnSELECT) {
      if (alarmSet) setAlarm();
      else {
        toMainMenu = true;
        lcd.clear();
        return;
      }
    }
    if (lcdKey == btnUP || lcdKey == btnDOWN) {
      if (alarmSet) {
        lcd.clear();
        lcd.print("  Alarm on");
        lcd.setCursor(0,1);
        lcd.print("> Alarm off");
        alarmSet = false;
        delay(500);        
      }
      else {
        lcd.clear();
        lcd.print("> Alarm on");
        lcd.setCursor(0,1);
        lcd.print("  Alarm off");
        alarmSet = true;
        delay(500);         
      }
    }
    if (toMainMenu) {
      return;
    }
  }  
}

void setAlarm() {
  lcd.clear();
  lcd.print("Set alarm"); // "Set alarm"
  lcd.setCursor(0,1);
  lcd.print(alarmh/10); lcd.print(alarmh%10); lcd.print(":");
  lcd.print(alarmm/10); lcd.print(alarmm%10);
  int hms[] = {alarmh/10, alarmh%10, alarmm/10, alarmm%10};
  int maxhms[] = {3, 10, 6, 10};
  delay(500);
  int digit = 0;
  int cursorPos;
  lcd.cursor();
  while (true) {
    lcd.setCursor(0,1);
    lcd.print(hms[0]); lcd.print(hms[1]); lcd.print(":");
    lcd.print(hms[2]); lcd.print(hms[3]);
    if (digit < 2) cursorPos = digit;
    else cursorPos = digit + 1;
    lcd.setCursor(cursorPos,1);
    delay(300);
    lcdKey = readLCDButtons();
    while (lcdKey == btnNONE) {
      lcdKey = readLCDButtons();
    }
    if (lcdKey == btnSELECT) {
      toMainMenu = true;
      alarmh = hms[0]*10 + hms[1];
      alarmm = hms[2]*10 + hms[3];
      lcd.noCursor();
      alarmSet = true;
      return;  
    }
    if (lcdKey == btnUP) {
      hms[digit]++;
      if (digit == 1 && hms[0] == 2 && hms[1] == 4) hms[1] = 0;
      else
        if (hms[digit] == maxhms[digit]) hms[digit] = 0;
    }
    if (lcdKey == btnDOWN) {
      hms[digit]--;
      if (digit == 1 && hms[0] == 2 && hms[1] == -1) hms[1] = 3;
      else      
        if (hms[digit] == -1) hms[digit] = maxhms[digit] - 1;    
    }      
    if (lcdKey == btnRIGHT) {
      digit++;
      if (digit == 4) digit = 0;
    }
    if (lcdKey == btnLEFT) {
      digit--;
      if (digit == -1) digit = 3;
    }  
  }  
}

//****************************
//** Time settings
//****************************

void menuSetTime() {
  lcd.clear();
  lcd.print("Set time"); // "Set time"
  delay(500);
  while (true) {
    lcdKey = readLCDButtons();
    while (lcdKey == btnNONE) {
      lcdKey = readLCDButtons();
    }
    if (lcdKey == btnSELECT) setTime();
    if (lcdKey == btnUP) menuSetAlarm();
    if (lcdKey == btnDOWN) menuSetColor();
    if (toMainMenu) {
      return;
    }
  }  
}

void setTime() {
  lcd.setCursor(0,1);
  lcd.print(rtc.getTimeStr());
  int h = t.hour; int m = t.min; int s = t.sec;
  int h1 = h/10;
  int h2 = h - h1*10;
  int m1 = m/10;
  int m2 = m - m1*10;
  int s1 = s/10;
  int s2 = s - s1*10;
  int hms[] = {h1, h2, m1, m2, s1, s2};
  int maxhms[] = {3, 10, 6, 10, 6, 10};
  delay(500);
  int digit = 0;
  int cursorPos;
  lcd.cursor();
  while (true) {
    lcd.setCursor(0,1);
    lcd.print(hms[0]); lcd.print(hms[1]); lcd.print(":");
    lcd.print(hms[2]); lcd.print(hms[3]); lcd.print(":");
    lcd.print(hms[4]); lcd.print(hms[5]);
    if (digit < 2) cursorPos = digit;
    else if (digit < 4) cursorPos = digit + 1;
    else cursorPos = digit + 2;
    lcd.setCursor(cursorPos,1);
    delay(300);
    lcdKey = readLCDButtons();
    while (lcdKey == btnNONE) {
      lcdKey = readLCDButtons();
    }
    if (lcdKey == btnSELECT) {
      toMainMenu = true;
      rtc.setTime(hms[0]*10 + hms[1], hms[2]*10 + hms[3], hms[4]*10 + hms[5]);
      lcd.noCursor();
      return;  
    }
    if (lcdKey == btnUP) {
      hms[digit]++;
      if (digit == 1 && hms[0] == 2 && hms[1] == 4) hms[1] = 0;
      else
        if (hms[digit] == maxhms[digit]) hms[digit] = 0;
    }
    if (lcdKey == btnDOWN) {
      hms[digit]--;
      if (digit == 1 && hms[0] == 2 && hms[1] == -1) hms[1] = 3;
      else      
        if (hms[digit] == -1) hms[digit] = maxhms[digit] - 1;    
    }      
    if (lcdKey == btnRIGHT) {
      digit++;
      if (digit == 6) digit = 0;
    }
    if (lcdKey == btnLEFT) {
      digit--;
      if (digit == -1) digit = 5;
    }  
  }  
}

//****************************
//** Color settings
//****************************

void WaitForKey (const char* prompt)
{
lcd.clear();
lcd.print (prompt);
lcd.setCursor(0,1);
lcd.print ("Pres right key.");
   
   while (true) 
   {
    lcdKey = readLCDButtons();
    while (lcdKey == btnNONE) 
    {
      lcdKey = readLCDButtons();
    }
     if (lcdKey == btnRIGHT) return;
   }
  
}


void SetUpPulse()
{

  WaitForKey ("Set color.");    
  setColor(RBreath,GBreath,BBreath, true);
  
WaitForKey ("Set velocity.");   
  setColor(RSpeed,GSpeed,BSpeed, false);

WaitForKey ("Set range.");   
  setColor(RWidth,GWidth,BWidth, false);


  SetPulseParams();
  
}

void menuSetColor() {
  lcd.clear();
  lcd.print("Set color");
  delay(500);
  while (true) {
    lcdKey = readLCDButtons();
    while (lcdKey == btnNONE) {
      lcdKey = readLCDButtons();
    }
    if (lcdKey == btnSELECT) setColorMode();
    if (lcdKey == btnUP) menuSetTime();
    if (lcdKey == btnDOWN) menuClock();
    if (toMainMenu) {
      return;
    }    
  }   
}



void setColorMode() {
  lcd.clear();
  if (colorMode == cmStatic) {
    lcd.print("> One color"); // "One color"
    lcd.setCursor(0,1);
    lcd.print("  Rainbow"); // "Rainbow"
  }
  else if (colorMode==cmAnimation) {
    lcd.print("  One color");
    lcd.setCursor(0,1);
    lcd.print("> Rainbow");      
  }
  else if (colorMode==cmOff) {
    lcd.print("  Rainbow");
    lcd.setCursor(0,1);
    lcd.print("> Off");      
  }
  else if (colorMode==cmPulse) {
    lcd.print("  Off");
    lcd.setCursor(0,1);
    lcd.print("> Pulse");      
  }


  
  delay(500);
  while (true) {
    lcdKey = readLCDButtons();
    while (lcdKey == btnNONE) {
      lcdKey = readLCDButtons();
    }
    if (lcdKey==btnRIGHT)
    {
      wakeyWakey();
    }

  if (lcdKey==btnLEFT)
    {
      colorMode = cmPulse;
      toMainMenu = true;
        return;
      
    }
    
    
    if (lcdKey == btnSELECT) 
    {
      if (colorMode == cmStatic) 
      {
        setColor(r,g,b, true);
        Brightness_R = r;
        Brightness_G = g;
        Brightness_B = b;
        
      }

  if (colorMode == cmPulse) 
      {
        SetUpPulse();
                
      }

      

      
      else 
      {
        toMainMenu = true;
        return;
      }
    }
    //if (lcdKey == btnUP || lcdKey == btnDOWN) {
    if (lcdKey == btnUP) 
    {
      if (colorMode == cmStatic) 
      {
        lcd.clear();
        lcd.print("> Pulse");
        lcd.setCursor(0,1);
        lcd.print("  One color");
        colorMode = cmPulse;
        delay(500);        
      }
      else if (colorMode == cmAnimation) 
      {
        lcd.clear();
        lcd.print("> One color");
        lcd.setCursor(0,1);
        lcd.print("  Rainbow");
        colorMode = cmStatic;
        delay(500);         
      }
      else if (colorMode == cmOff) 
      {
        lcd.clear();
        lcd.print("> Rainbow");
        lcd.setCursor(0,1);
        lcd.print("  Off");
        colorMode = cmAnimation;
        delay(500);         
      }
      else if (colorMode == cmPulse) 
      {
        lcd.clear();
        lcd.print("> Off");
        lcd.setCursor(0,1);
        lcd.print("  Pulse");
        colorMode = cmOff;
        delay(500);         
      }
    }

if (lcdKey == btnDOWN) 
    {
      if (colorMode == cmStatic) 
      {
        lcd.clear();
        lcd.print("  One Color");
        lcd.setCursor(0,1);
        lcd.print("> Rainbow");
        colorMode = cmAnimation;
        delay(500);        
      }
      else if (colorMode == cmAnimation) 
      {
        lcd.clear();
        lcd.print("  Rainbow");
        lcd.setCursor(0,1);
        lcd.print("> Off");
        colorMode = cmOff;
        delay(500);         
      }
      else if (colorMode == cmOff) 
      {
        lcd.clear();
        lcd.print("  Off");
        lcd.setCursor(0,1);
        lcd.print("> Pulse");
        colorMode = cmPulse;
        delay(500);         
      }
      else if (colorMode == cmPulse) 
      {
        lcd.clear();
        lcd.print("  Pulse");
        lcd.setCursor(0,1);
        lcd.print("> One color");
        colorMode = cmStatic;
        delay(500);         
      }
    }
    
    if (toMainMenu) {
      return;
    }
  }  
}

void setColor(int &r, int &g, int  &b, boolean ChangeLight) {

      if (ChangeLight)
      {
  stripsetPixelColor(1, r,g,b);
      }
  
  lcd.clear();
  lcd.print("Red Green Blue"); // "Red Green Blue"
  lcd.setCursor(0,1); lcd.print(r);
  lcd.setCursor(5,1); lcd.print(g);
  lcd.setCursor(11,1); lcd.print(b);
  int rgb[] = {r/100, (r%100)/10, (r%100) % 10, 
               g/100, (g%100)/10, (g%100) % 10,
               b/100, (b%100)/10, (b%100) % 10 };
  delay(500);
  int digit = 0;
  int cursorPos;
  lcd.cursor();
  while (true) {
    lcd.setCursor(0,1);  lcd.print(rgb[0]); lcd.print(rgb[1]); lcd.print(rgb[2]); 
    lcd.setCursor(5,1);  lcd.print(rgb[3]); lcd.print(rgb[4]); lcd.print(rgb[5]); 
    lcd.setCursor(11,1); lcd.print(rgb[6]); lcd.print(rgb[7]); lcd.print(rgb[8]); 
    if (digit < 3) cursorPos = digit;
    else if (digit < 6) cursorPos = digit + 2;
    else cursorPos = digit + 5;
    lcd.setCursor(cursorPos,1);
    delay(300);
    lcdKey = readLCDButtons();
    while (lcdKey == btnNONE) {
      lcdKey = readLCDButtons();
    }
    if (lcdKey == btnSELECT) {
      toMainMenu = true;
      lcd.noCursor();
      lcd.clear();
      return;  
    }
    if (lcdKey == btnUP) {
      for (int i = 0; i <= 6; i = i + 3) {      
        if (digit == i   && ((rgb[i] + 1)*100 + rgb[i+1]*10 + rgb[i+2]) < 256 && rgb[digit] < 9)
          rgb[i]++;
        if (digit == i+1 && (rgb[i]*100 + (rgb[i+1] + 1)*10 + rgb[i+2]) < 256 && rgb[digit] < 9)
          rgb[i+1]++;
        if (digit == i+2 && (rgb[i]*100 + rgb[i+1]*10 + (rgb[i+2] + 1)) < 256 && rgb[digit] < 9)
          rgb[i+2]++;
      }
      r = rgb[0]*100 + rgb[1]*10 + rgb[2];
      g = rgb[3]*100 + rgb[4]*10 + rgb[5];
      b = rgb[6]*100 + rgb[7]*10 + rgb[8];
       if (ChangeLight)
      {
  stripsetPixelColor(1, r,g,b);
      }
      
    }
    if (lcdKey == btnDOWN) {
      rgb[digit]--;
      if (rgb[digit] == -1) rgb[digit] = 0;
      r = rgb[0]*100 + rgb[1]*10 + rgb[2];
      g = rgb[3]*100 + rgb[4]*10 + rgb[5];
      b = rgb[6]*100 + rgb[7]*10 + rgb[8];
        if (ChangeLight)
      {
  stripsetPixelColor(1, r,g,b);
      }
      
    }      
    if (lcdKey == btnRIGHT) {
      digit++;
      if (digit == 9) digit = 0;
    }
    if (lcdKey == btnLEFT) {
      digit--;
      if (digit == -1) digit = 8;
    }  
  }   
}


//****************************
//** Menu back to clock
//****************************

void menuClock() {
  lcd.clear();
  lcd.print("Show clock"); // "Show clock"
  delay(500);
  while (true) {
    lcdKey = readLCDButtons();
    while (lcdKey == btnNONE) {
      lcdKey = readLCDButtons();
    }
    if (lcdKey == btnSELECT) {
      toMainMenu = true;
      return;  
    }
    if (lcdKey == btnUP) menuSetColor();
    if (lcdKey == btnDOWN) menuSetAlarm();    
  }
}

 int readLCDButtons() {
  adcKeyIn = analogRead(0);
  // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
  // we add approx 50 to those values and check to see if we are close
  if (adcKeyIn > 1000)
    return btnNONE;
  else {
    digitalWrite(backlightPin, HIGH);
  }
  if (adcKeyIn < 50)   return btnRIGHT;  
  if (adcKeyIn < 250)  return btnUP; 
  if (adcKeyIn < 450)  return btnDOWN; 
  if (adcKeyIn < 650)  return btnLEFT; 
  if (adcKeyIn < 850)  return btnSELECT;  
  return btnNONE;  // when all others fail, return this...
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return random(255);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return random(255);
  } else {
   WheelPos -= 170;
   return random(255);
  }
}
