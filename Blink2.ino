#include <LiquidCrystalFast.h>

/*

  Blink 2
  Firmware for MintyStrobe v3 programmable strobe light.
  by Alex Mizell, 08/26/2014
  
 */
 
// Pin 13 has an LED connected on most Arduino boards.
// Pin 11 has the LED on Teensy 2.0
// Pin 6  has the LED on Teensy++ 2.0
// Pin 13 has the LED on Teensy 3.0
// give it a name:
int led = 13;

// initialize the library with the numbers of the interface pins
LiquidCrystalFast lcd(12, 10, 11, 5, 4, 3, 2);
         // LCD pins: RS  RW  EN  D4 D5 D6 D7

int maxfreq = 30;
int minfreq = 1;
float freq = 20;

int potpin = 2;
int potval = 0;
int oldpotval = 0;

// the setup routine runs once when you press reset:
void setup() {
  
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);

  // set up the LCD's number of rows and columns: 
  lcd.begin(16, 2);
  
  // Print a message to the LCD.
  lcd.println("MintyStrobe v3");
  lcd.println("By Alex Mizell");
  
  Serial.begin(57600);
    
}

// the loop routine runs over and over again forever:
void loop() {
  
  oldpotval = potval;
  potval = analogRead(potpin); 
  
  freq = ((((maxfreq-minfreq) * potval) / 1024.0 ) + minfreq);
  
  if(potval!=oldpotval){
    
    //Serial.println(freq, 2);
    lcd.clear();
    lcd.println("MintyStrobe v3");
    lcd.print(freq,2);
    lcd.println(" Hz");

  }
  
  
  
  digitalWrite(led, HIGH);    // turn the LED on (HIGH is the voltage level)
  delay((1000.0/freq)/3);     // wait for one third of freq
  digitalWrite(led, LOW);     // turn the LED off by making the voltage LOW
  delay(((1000.0/freq)/3)*2); // wait for two thirds of freq
  

}
