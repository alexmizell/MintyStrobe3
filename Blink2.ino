
/*

  Blink 2
  Firmware for MintyStrobe v3 programmable strobe light.
  by Alex Mizell, 08/26/2014

  Changelog:

    180623  Changed to rotary encoders and I2C display
    180626  Added duty cycle adjustment, strobe enable/disable, 
            fine adjust mode and auto-dimming backlight
  
 */

// library for atmega hardware timer, needed for solid strobe timing
#include <TimerOne.h>  

// the i2c version of this display requires only 2 wires for comm
#include <LiquidCrystal_I2C.h> 

// a bit more elegant than potentiometers
#include <Encoder.h> 

// this is for two purposes
// first to repeatedly read the encoders and buttons
// second to turn the backlight off
#include <SoftTimer.h> 

// globals and inits
 
int leftLED = 12;
int rightLED = 13;

LiquidCrystal_I2C lcd(0x3f,16,2);  // set the LCD address to 0x3f for a 16 chars and 2 line display

// digital pins 2 and 3 have hardware interrupts 
// which improves performance of the encoders
// each knob gets one interrupt and one regular pin
Encoder knobLeft(2, 4);  
Encoder knobRight(3, 5);

boolean leftButtonPressed = false;
boolean rightButtonPressed = false;

// the freq in ms at which the encoders and buttons 
// are read (this is mostly for center button debounce)
int encoderReadMillis = 150;

boolean isFlashing = true;
boolean lastIsFlashing = true;  // comparison to see if state changed

int lastBacklightEvent;
float hertz = 2; // default freq setting
long dutyCycle = 512; // 512 = 50% duty cycle, 1024 = 100% duty cycle
long dutyCycleMemory; // to remember last dutycycle setting when the strobe is turned off

boolean fineAdjMode = false;
boolean lastFineAdjMode = false;

// the pwm period is set in millionths of a second (microseconds)
// so to find that we divide one million (microseconds per second)
// by hertz (cycles per second) 
long period = 1000000 / hertz;  

// default values that are quickly changed
long positionLeft  = -999;
long positionRight = -999;

SoftTimer timer;  // create the timer object 

int inputID; // stores the encoder read timer ID (in case we ever wanted to stop it)
long newLeft, newRight; // create here so we don't have to re-instantiate every loop cycle

// this function is called ever x number of milliseconds on the software timer
// it is very short - just reads the knobs and buttons and puts that info in a variable
boolean readEncoders(EventBase* evt) {

  newLeft = knobLeft.read() /-4;    // divide by negative 4 to compensate for
  newRight = knobRight.read() /-4;  // quadrature encoding and also reverse knob direction why not
  
  if(analogRead(A2) < 120) {   // 120 just felt right
    leftButtonPressed = true;
  }
  
  if(analogRead(A1) < 120) { 
    rightButtonPressed = true;
  }

  return false;
}

// ignore this it's stupid
void callback()
{
  digitalWrite(10, digitalRead(10) ^ 1);
}

// the setup routine runs once when you press reset:
void setup() {

  // start reading encoders on a software timer
  inputID = timer.schedule(encoderReadMillis, readEncoders);

  pinMode(10, OUTPUT);               // overflow bit for timer 1, who cares
  Timer1.initialize(period);         // initialize hardware timer1
  Timer1.pwm(9, dutyCycle);          // setup pwm on pin 9, this is the meat
  Timer1.attachInterrupt(callback);  // attaches callback() as a timer overflow interrupt

  lcd.init();                        // initialize the lcd 
  lcd.backlight();                   // i don't know why i have to do this but the lcd doesn't work if i don't
  
  // turn on lcd backlight
  analogWrite(A3, 255);              // the backlight is actually connected to this pin
  
  lcd.setCursor(0,0);  // top left corner
  lcd.print("MintyStrobe v3");
  lcd.setCursor(0,1);
  lcd.print("by Alex Mizell");  //if you don't remove the // below then you'll never see this line because it's quickly overwritten
  //delay(2000);  // enable this line for "splash screen" but slower boot time, aint nobody got time for that
  
  //Serial.begin(115200);  // for debugging
  //Serial.println("Encoder Test:");
}

boolean turnOffBacklight(EventBase* evt){
  //Serial.println("backlight off");
  analogWrite(A3, 0);  // what do you think
}

void loop(){

  // check to see if isFlashing has changed since the last loop 
  // (i.e. has right button been pushed)
  if(lastIsFlashing != isFlashing){
    
    // strobe was on but it was just turned off
    if(!isFlashing) {
      
      // remember this dutycycle setting
      dutyCycleMemory = dutyCycle;
      
      // turn off strobes
      dutyCycle = 0;
      
      // make it so #1
      Timer1.setPeriod(period);
      Timer1.setPwmDuty(9, dutyCycle);
      
      //Serial.println("set pwm");

      // update display
      lcd.setCursor(0,1); // second line, far left
      lcd.print(hertz,2);
      lcd.print(" Hz D: ");
      lcd.print(dutyCycle);
      lcd.print("    ");
    }
  
    // if we're re-enabling strobe then reset 
    // the duty cycle from remembered setting
    if(isFlashing){
      
      dutyCycle = dutyCycleMemory;
      
      Timer1.setPeriod(period);
      Timer1.setPwmDuty(9, dutyCycle);
      
      //Serial.print("set pwm: ");
      //Serial.println(dutyCycle);
      
      // update display
      lcd.setCursor(0,1);
      lcd.print(hertz,2);
      lcd.print(" Hz D: ");
      lcd.print(dutyCycle);
      lcd.print("    ");
    }   

    lastIsFlashing = isFlashing;
  }

  // check to see if fineAdjMode has changed since the last loop
  // i.e. has the left button been pushed
  if(fineAdjMode != lastFineAdjMode){
    
    // fine adj was off but it was just turned on
    if(fineAdjMode) {
      
      //Serial.println("fine adjust on");
    }
  
    // if we're re-enabling strobe then reset 
    // the duty cycle from remembered setting
    if(!fineAdjMode){
      
      //Serial.println("fine adjust off");
    }   

    lastFineAdjMode = fineAdjMode;
  }

  // only process input if something has happened since the last poll
  if (newLeft != positionLeft || newRight != positionRight || leftButtonPressed || rightButtonPressed) {

    if(rightButtonPressed){
    
      // toggle isFlashing
      isFlashing = !isFlashing;
      
      //Serial.print("isFlashing: ");
      //Serial.println(isFlashing);
      
      rightButtonPressed = false;
    }

    if(leftButtonPressed){
    
      // toggle fineAdjust
      fineAdjMode = !fineAdjMode;
      
      //Serial.print("fineAdjMode: ");
      //Serial.println(fineAdjMode);
      
      // the button press is handled
      leftButtonPressed = false;
    }

    if(newLeft < positionLeft){
      // left knob turned ccw
      if(fineAdjMode){
        hertz -= 0.05;
      }
      else{
        hertz -= 1;
      }
    }

    if(newLeft > positionLeft){
      // left knob turned cw
      if(fineAdjMode){
        hertz += 0.05;
      }
      else{
        hertz += 1;
      }
    }

    if(newRight < positionRight){
      // right knob turned ccw
      dutyCycle -= 8;
    }

    if(newRight > positionRight){
      // right knob turned cw
      dutyCycle += 8;
    }

    // apply limits
    if(hertz < 0.05) { hertz = 0.05;}
    if(hertz > 60) { hertz = 60;}
    if(dutyCycle < 0) { dutyCycle = 0;}
    if(dutyCycle > 1024) {dutyCycle = 1024;}

    // remember last position
    // for comparison purposes
    positionLeft = newLeft;
    positionRight = newRight;

    //Serial.print("Left = ");
    //Serial.print(knobLeft.read());
    //Serial.print(", Right = ");
    //Serial.print(knobRight.read());
    //Serial.println();

    period = (1000000 / hertz);

    // cancel last backlight turn off event
    // until the last change is set
    timer.removeEvent(lastBacklightEvent);

    // turn backlight on
    analogWrite(A3, 255);

    // write to display
    lcd.setCursor(0,0);
    lcd.println("MintyStrobe v3        ");
    lcd.setCursor(0,1);
    lcd.print(hertz,2);
    lcd.print(" Hz D: ");
    lcd.print(dutyCycle);
    lcd.print("    ");
    
    // commit the hardware timer pwm changes
    Timer1.setPeriod(period);
    Timer1.setPwmDuty(9, dutyCycle);

    // wait 10 seconds then turn off backlight to conserve battery
    lastBacklightEvent = timer.once(turnOffBacklight, 10000);
  }

  // tick tock, clarise
  // https://youtu.be/4qZisaj1VWg
  timer.update();
}


