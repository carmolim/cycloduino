// cycloduino - ciclocomputer bike speedometer
// Augusto Carmo (carmolim) 2012 - https://github.com/carmolim/cycloduino
// Inspired on the work of Amanda Ghassae - http://www.instructables.com/id/Arduino-Bike-Speedometer/


/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 */


/*
 TODO

 - evitar leituras erradas quando o ima estiver passando com baixa velocidade ou parar na frente do REED
 - porque a leitura da cadência não está fazendo com que inicie a contagem do movingTime?

 
 LOG
 /// 

  - create a file name with date and time - I'll need a DS1302 or DS1307
  - think is going to be easier to just add a sequencial number and make a verification if the file name already exists

 
 SPEED
 /////
 
 OK - current speed
 OK - average speed  
 OK - top speed
 
 
 CADENCE
 ///////
 
 OK - how to calculate cadence
 OK - max cadence
 OK - average cadence
 

 CALORIES
 ////////

 The Journal of Sports Sciences provides a calorie expenditure formula for each gender.

 Men: Calories Burned = ((Age x 0.2017) + (Weight x 0.09036) + (Heart Rate x 0.6309) - 55.0969) x Time / 4.184
 Women: Calories Burned = ((Age x 0.074) -- (Weight x 0.05741) + (Heart Rate x 0.4472) - 20.4022) x Time / 4.184.

 Read more: http://www.livestrong.com/article/221621-formula-for-calories-burned-during-exercise/#ixzz2HWlKgfgJ


 Male: ((-55.0969 + (0.6309 x HR) + (0.1988 x W) + (0.2017 x A))/4.184) x 60 x T
 Female: ((-20.4022 + (0.4472 x HR) - (0.1263 x W) + (0.074 x A))/4.184) x 60 x T
 
 where:
 HR = Heart rate (in beats/minute) 
 W = Weight (in kilograms) 
 A = Age (in years) 
 T = Exercise duration time (in hours)

 other ref: http://www.shapesense.com/fitness-exercise/calculators/heart-rate-based-calorie-burn-calculator.aspx


 HEART RATE
 //////////

 Equation for Determination of Maximum Heart Rate Based on Age
 Maximum Heart Rate (beats/minute) = 208 - (0.7 x age)


 */


// LIBRARIES
////////////
#include <Adafruit_BMP085.h>                    // Barometer
#include <Adafruit_GFX.h>                       // LCD graphics
#include <Adafruit_PCD8544.h>                   // LCD
#include <Wire.h>
//#include <SD.h>


// INTERFACE
////////////

// pin 7 - Serial clock out (SCLK)
// pin 6 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)

Adafruit_PCD8544 display = Adafruit_PCD8544(3, 4, 5, 7, 6);

const int buttonPin        = 8;               // the number of the pushbutton pin
const int ledPin           = 13;              // the number of the LED pin
const int graphSteps       = 42;              // number of lines used for build the graph
int screen                 = 7;               // variable for reading the pushbutton status
int buttonState            = 0;               // variable for reading the pushbutton status
int graphPosition          = 0;               // stores the actual position in the array
int speedGraph[graphSteps];                   // stores the last 42 speed reads to draw the speed graphic


//CYCLES
////////

const int oneSecCycle      = 1000;            // 1 second
unsigned long before1Sec   = 0; 

const int buttonDebounce   = 200;             // 500 milisegundos
unsigned long beforeButton = 0; 


// SENSORS
//////////

const int speedReed        = A0;              // speed reed switch
const int cadenceReed      = A1;              // cadence reed switch


// BAROMETER
////////////

Adafruit_BMP085 bmp;                         // create a barometer object
int altitude               = 0;              // stores the actual altitude in meters
int lastAltitude           = 0;              // stores the last altitude value
int totalAscent            = 0;              // sum of all ascents
int maxAltitude            = 0;              // higher altitude in the ride
int minAltitude            = 900;            // lowest altitude in the ride
int filterAltitude         = 5;              // diference between altitude and last altitude


// LOG
//////

//File myFile;                                  // object to handle with the file in the SD
String logLine;                               // stores each log line before it is recorded in th SD
char logName[]            = "RIDE_00.csv";    // create an array that contains the name of our file.


// USER INFO
////////////

const int age              = 24;               // age in years of the user
float weight               = 77.5;             // weight in Kg


// HEART RATE
/////////////

float heartRate           = 0.00;             // current bpm
float avgHeartRate        = 0.00;             // average bpm
float maxHeartRate        = 0.00;             // max bpm
float minHeartRate        = 0.00;             // min bpm


// CALORIES
///////////

float caloriesBurned      = 0.00;             // total calories burned


// TOTAL MEASURES
/////////////////

const int maxReedCounter  = 80;               // min time (in ms) of one rotation (for debouncing)
float odometer            = 0;                // total distante
int loopCounter           = 0;                // how many times the loop run before the ride started


// PER RIDE
///////////

long rideTime             = 0;                // total time of the ride
long movingTime           = 0;                // only the moving time
long millisCount          = 0;                // stores the number of cicles runned in the interrupt
float rideDistance        = 0.00;             // total distance of the ride in Km
boolean rideStarted       = false;            // if the bike is moving = true
boolean moving            = false;            // if the bike is moving = true


// SPEED VARIBALES
//////////////////

long speedTimer           = 0;                 // time between one full rotation (in ms)
long speedNumberSamples   = 0;                 // total of revolutions made by the front wheel
float speedSamplesSum     = 0.00;              // sum of all the speeds collected
float circumference       = 210.0;             // lenght of the wheel
float kph                 = 0.00;              // speed in kph
float mph                 = 0.00;              // speed in mph
float topSpeed            = 0.00;              // top speed of the ride
float avgSpeed            = 0.00;              // average speed of the ride
int speedReedVal          = 0;                 // ?? stores if the switch is open or closed // change to boolean?
int speedReedCounter      = 0;                 // ??


// CADENCE VARIABLES
////////////////////

long cadenceTimer         = 0;                 // time between one full rotation (in ms)
long cadenceNumberSamples = 0;                 // total of revolutions made by the front wheel
float cadenceSamplesSum   = 0.00;              // sum of all the speeds collected
float cadence             = 0.00;              // actual cadence
float avgCadence          = 0.00;              // average cadence of the ride
float topCadence          = 0.00;              // top cadence fo the ride
int cadenceReedVal        = 0;                 // stores if the switch is open or closed // change to boolean?
int cadenceReedCounter    = 0;                 // ??


// TEMPERATURE
//////////////

float temperature        = 0.00;               // stores the temperature
float maxTemp            = 0.00;               // stores the maximum temperature of the ride
float minTemp            = 100.00;             // stores the minimum temperature of the ride
float avgTemp            = 0.00;               // stores the average temp of the ride
float tempSum            = 0.00;               // sum of all the temperature reads


//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

// SETUP

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////


void setup()
{
  // initializate the BMP085
  if (!bmp.begin())
  {
   Serial.println("Could not find a valid BMP085 sensor, check wiring!");
   while (1) {}
  }

  speedReedCounter = maxReedCounter;      // ?
  cadenceReedCounter = maxReedCounter;    // ?
  minAltitude = bmp.readAltitude(101500);

  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 

  // speed input
  pinMode(speedReed, INPUT);

  // cadence input                   
  pinMode(cadenceReed, INPUT); 

  // pushbutton input:
  pinMode(buttonPin, INPUT);   

  // LCD

  // init done
  display.begin();   

  // you can change the contrast around to adapt the display
  // for the best viewing!
  display.setContrast(50);


  // iniciates the array
  for (int i = 0; i < graphSteps; i++)
  {      
    speedGraph[i] = 0;     
  }


  // SD
  
  /*

  if (!SD.begin(4))
  {
    Serial.println("initialization failed!");
    return;
  }

  Serial.println("initialization done."); 
  */
/*
  TODO

  1 - search if there is a filename that starts with 01
  2 - if it finds a filename that starts with 01, find if there is a filename that starts with 02,03,04...
  3 - after find the last log add 1 to logCount and start a new log with this number

  REF
  http://arduino.cc/forum/index.php?topic=108264.0
  http://arduino.cc/forum/index.php?PHPSESSID=a96c1ff6fa88ecd4d0d0094696a1edeb&/topic,105997.0.html
  * - http://www.ladyada.net/make/logshield/lighttempwalkthru.html

  TEST
*/

/*
  // while the filename exists...
  while(SD.exists(logName))
  { 
    logCount += 1;                 // adds 1 to the counter
    logName[5] = logCount / 10 + '0';
    logName[6] = logCount % 10 + '0';
  }   

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open(logName, FILE_WRITE);  
*/


/*
  // create a new file
  for (uint8_t i = 0; i < 100; i++)
  {
    logName[5] = i/10 + '0';
    logName[6] = i%10 + '0';
    if (!SD.exists(logName))
    {
      // only open a new file if it doesn't exist
      myFile = SD.open(logName, FILE_WRITE); 
      break;  // leave the loop!
    }
  }


  // if the file opened okay, write to it
  if (myFile)
  {
    //Serial.print("Writing to log...");
    myFile.print("speed; avgSpeed; rotations S; cadence; avgCadence; rotations C; rideTime; movingTime; temperature;");
    myFile.println();

    // close the file:
    myFile.close();   
  }   */

  // WEIGHT

  // convert Kg to Lbs
  weight = weight * 2.20462262184877580723; 

 
  // TIMER SETUP - the timer interrupt allows precise timed measurements of the reed switch
  // for more info about configuration of arduino timers see http://arduino.cc/playground/Code/Timer1

  cli(); // stop interrupts

  // set timer1 interrupt at 1kHz
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 0;

  // set timer count for 1khz increments
  OCR1A = 1999; // = (1/1000) / ((1/(16*10^6))*8) - 1

  // turn on CTC mode
  TCCR1B |= (1 << WGM12);

  // Set CS11 bit for 8 prescaler
  TCCR1B |= (1 << CS11); 

  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei(); //allow interrupts

  //END TIMER SETUP

  Serial.begin(9600);

}// setup end


//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

// TIMER

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////


// Interrupt at freq of 1000 Hz to measure reed switch
ISR(TIMER1_COMPA_vect)
{

  //SPEED
  ///////

  // get val of A0
  speedReedVal = digitalRead(speedReed);

  // if reed switch is closed
  if (speedReedVal)
  {
    //min time between pulses has passed
    if (speedReedCounter == 0)
    {
      // min time between pulses has passed
      kph = (36*float(circumference))/float(speedTimer); // calculate kilometers per hour
  
      // reset speedTimer      
      speedTimer = 0;

      // reset speedReedCounter
      speedReedCounter = maxReedCounter;

      // increase number of samples by 1 - number of wheel rotations ajust the debouncer??
      speedNumberSamples++;  

      // starts the chronometer
      rideStarted = true;

      // the wheel is spinning
      moving = true;
    }

    else
    {
      if (speedReedCounter > 0)
      {
        // don't let speedReedCounter go negative
        speedReedCounter -= 1; // decrement speedReedCounter
      }
    }
  }

  else
  {
    // if reed switch is open
    if (speedReedCounter > 0)
    {
      // don't let speedReedCounter go negative
      speedReedCounter -= 1; // decrement speedReedCounter
    }
  }

  if (speedTimer > 2000)
  {
    // if no new pulses from reed switch- tire is still, set kmh to 0
    kph = 0; 

    // the bike is not moving
    moving = false;
  }

  else
  {
    speedTimer += 1; // increment speedTimer
  } 


  // CADENCE
  //////////

  // get val of A1
  cadenceReedVal = digitalRead(cadenceReed);

  // if reed switch is if closed
  if(cadenceReedVal)
  {
    //min time between pulses has passed
    if (cadenceReedCounter == 0)
    {
      // calculate rotations per minute 
      cadence = float(60000)/float(cadenceTimer);

      // reset timer
      cadenceTimer = 0;

      // reset reedCounter
      cadenceReedCounter = maxReedCounter;

      // increase number of samples by 1      
      cadenceNumberSamples++;  

      // starts the chronometer
      rideStarted = true;

      // the wheel is spinning
      moving = true;                                 
    }

    else
    {
      if(cadenceReedCounter > 0)
      {// don't let cadenceReedCounter go negative

        // decrement cadenceReedCounter
        cadenceReedCounter -= 1;    
      }
    }
  }

  // if reed switch is open
  else
  {
    // don't let cadenceReedCounter go negative
    if (cadenceReedCounter > 0)
    {   
      // decrement cadenceReedCounter
      cadenceReedCounter -= 1;
    }
  }

  if (cadenceTimer > 2000)
  {
    cadence = 0; 
  }

  else
  {
    cadenceTimer += 1; // increment timer
  } 


  // TIME
  ///////

  // if the timeCount is == to 1000 (1s)
  if(millisCount == 1000)
  {
    // if the ride started...
    if(rideStarted)
    {
      // increments 1 once a second
      rideTime += 1;  
    }
    
    // if the bike is moving...
    if(moving)
    {
      // increments 1 once a second
      movingTime += 1;
    }
    
    // reset the counter
    millisCount = 0; 
  }

  // increments 1 every cicle
  millisCount += 1;

} // end of timer


//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

// LOOP

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////


void loop()
{  

  // BUTTON
  /////////
  
  // button debounce
  if (millis() - beforeButton > buttonDebounce)
  {
     if (digitalRead(buttonPin) == HIGH)
     {
       screen += 1;
     }
    beforeButton = millis();
  }

  if(screen > 9)
  {
    screen = 0;
  }

  // 1 sec cycle
  if (millis() - before1Sec > oneSecCycle)
  {

    // if the ride started...
    if(rideStarted)
    {    
      // adds the actual temperature read to de sum
      tempSum += temperature;
      
      // save to log
      //saveToLog();

      // one more loop
      loopCounter += 1;  
    }
     
    // adds the actual speed to the correct postion in the array
    speedGraph[graphPosition] = (int) kph;

    // update the array position
    graphPosition += 1;    

    // if the graphPosition is at the end...
    if(graphPosition >= graphSteps-1)
    {
      // runs tru the array changin the position of the values "to the left" 
      for (int i = 0; i < graphSteps - 1; i++)
      {
        // current value equals to the next value
        speedGraph[i] = speedGraph[i+1];    
      }

      // now the graphPosition is always at the end
      graphPosition = graphSteps - 1;
    }

    
    // CALORIES
    ///////////

    caloriesBurned = (( (float) age * 0.2017) + (weight * 0.09036) + (heartRate * 0.6309) - 55.0969) * (movingTime / 60) / 4.184;


    // DISTANCE
    ///////////
    
    // Ride distance
    rideDistance = circumference * (float) speedNumberSamples / 100000;     // calculate distance in Km (1000 m)  


    // SPEED
    ////////

    // verifies if this speed is the top speed of the ride
    if(kph > topSpeed)
    {
      topSpeed = kph;
    }      

    // average speed
    speedSamplesSum += kph;                                // add the new calculate kph                                     
    avgSpeed = speedSamplesSum / (float) movingTime;       // calculate average speed

    // print kph once a second
    displayKMH();


    // CADENCE
    //////////

    // verifies if this cadence is the top cadence of the ride
    if(cadence > topCadence)
    {
      topCadence = cadence;
    }

    // average cadence 
    cadenceSamplesSum += cadence;                          // add the new calculate cadence
    avgCadence = cadenceSamplesSum / (float) movingTime;   // calculate average cadence

    // print cadence once a second
    displayCadence();
    

    // TEMPERATURE
    //////////////

    // update the actual temperature
    temperature = bmp.readTemperature();
    
    // calulate the avgTemp
    avgTemp = tempSum / (float) loopCounter;

    // verifies if this is the highest temperature recorded
    if(temperature > maxTemp)
    {
      maxTemp = temperature;
    }

    // verifies if this is the lowest temperature recorded
    if(temperature < minTemp)
    {
      minTemp = temperature;
    }

    // print temperatures once a second
    displayTemp();

    // display other data
    diplayOhterData();

    Serial.println(); // jump to the next line


    // ALTITUDE
    ///////////

    // you can get a more precise measurement of altitude
    // if you know the current sea level pressure which will
    // vary with weather and such. If it is 1015 millibars
    // that is equal to 101500 Pascals.
    altitude = bmp.readAltitude(101500);              

    // verifies if this altitude can be summed to the totalAscent
    if (altitude > lastAltitude && altitude - lastAltitude > filterAltitude)
    {
     totalAscent += altitude - lastAltitude;      
    } 

    // verifies if this is the highest altitude recorded
    if (altitude > maxAltitude)
    {
      maxAltitude = altitude;                 
    }           
      
    // verifies if this is the lowest altitude recorded
    if (altitude < minAltitude)
    {
      minAltitude = altitude;                 
    }

    // updates the value of lastAltitude
    lastAltitude = altitude;   



    // LCD SCREENS
    //////////////

    // screen 0 = speed
    if(screen == 0)
    {
      display.setTextColor(BLACK);
      display.setCursor(0,0);
      display.setTextSize(1);
      display.println("1 - speed");
      display.println();
      display.setTextSize(2);
      display.println(kph, 2);
      // display everything on LCD
      display.display();
    }

    // screen 1 = avgSpeed
    else if(screen == 1)
    {
      display.setTextColor(BLACK);
      display.setCursor(0,0);
      display.setTextSize(1);
      display.println("2 - avgSpeed");
      display.println();
      display.setTextSize(2);
      display.println(avgSpeed, 2);
      // display everything on LCD
      display.display();
    }

    // screen 2 = cadence
    else if(screen == 2)
      {  
        display.setTextColor(BLACK);
      display.setCursor(0,0);
      display.setTextSize(1);
      display.println("3 - RPM");
      display.println();
      display.setTextSize(2);
      display.println(cadence, 2);
      // display everything on LCD
      display.display();
    }

    // screen 3 = avgCadence
    else if(screen == 3)
    {
      display.setTextColor(BLACK);
      display.setCursor(0,0);
      display.setTextSize(1);
      display.println("4 - avgRPM");
      display.println();
      display.setTextSize(2);
      display.println(avgCadence, 2);
      // display everything on LCD
      display.display();
    }

    // screen 4 = temperature
    else if(screen == 4)
    {
      display.setTextColor(BLACK);
      display.setCursor(0,0);
      display.setTextSize(1);
      display.println("5 - temp");
      display.println();
      display.setTextSize(2);
      display.println(temperature, 2);

      // display everything on LCD
      display.display();
    }

    // screen 5 = avgTemperature
    else if(screen == 5)
    {
      display.setTextColor(BLACK);
      display.setCursor(0,0);
      display.setTextSize(1);
      display.println("6 - avgTemp");
      display.println();
      display.setTextSize(2);
      display.println(avgTemp, 2);

      // display everything on LCD
      display.display();
    }

    // screen 6 = summary
    else if(screen == 6)
    {
      display.setTextColor(BLACK);
      display.setCursor(0,0);
      display.setTextSize(1);

      display.print("speed:");
      display.println(kph, 2);

      display.print("avgS:");
      display.println(avgSpeed, 2); 

      display.print("rpm:");
      display.println(cadence, 1);

      display.print("temp:");
      display.println(temperature, 2);

      display.print("moving:");
      display.println(printTime(movingTime));
      
      // display everything on LCD
      display.display(); 
    }

    // screen 7 = speedGraph
    else if(screen == 7)
    {
      display.setTextColor(BLACK);
      display.setCursor(0,0);
      display.setTextSize(1);
      display.print("sGraph:");
      display.print(kph, 2);  
    
      // show the axis scale?
      const int topMargin = 20;
      float height = 0.00; 

      for (int i = 0; i < graphSteps; i++)
      {
        // map the value to the correct interval in display
        height = (int) map(speedGraph[i], 0, topSpeed, 48, topMargin);

        // create the line
        display.drawLine(i*2, 48, i*2, height, BLACK); // ok
      }
      
      // display everything on LCD
      display.display(); 
    } 

    // screen 8 = altitude
    else if(screen == 8)
    {
      display.setTextColor(BLACK);
      display.setCursor(0,0);
      display.setTextSize(1);
      display.println("8 - altitude");
      display.println();
      display.setTextSize(2);
      display.println(altitude);

      // display everything on LCD
      display.display(); 
    }

    // screen 9 = total ascent
    else if(screen == 9)
    {
      display.setTextColor(BLACK);
      display.setCursor(0,0);
      display.setTextSize(1);
      display.println("9 - ascent");
      display.println();
      display.setTextSize(2);
      display.println(totalAscent);

      // display everything on LCD
      display.display(); 
    } 




    //isto corre de segundo a segundo... 
    before1Sec = millis();    
  }// end of onSecCycle
  
  // clears the display for the next cycle
  display.clearDisplay();
}// end of loop


//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

// METHODS

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////


// return a readeable format of time
String printTime(long t)
{ 

  String time;                 // stores the time 0:0:0
  char temp[25];               // the calculations converded
  float h, m, s;               // variables for the time calculation

  h = t / 3600;                // calculates de hours 
  m = (t % 3600) / 60;         // calculates de minutes 
  s = t % 60;                  // calculates de seconds 

  dtostrf(h, 1, 0, temp);      // convert float hour to string and add to temp array
  time += temp;                // concatenate the hour in the string time
  time += ':';                 // concatenate ':' in the string time

  dtostrf(m, 1, 0, temp);      // convert float minute to string and add to temp array
  time += temp;                // concatenate the minute in the string time
  time += ':' ;                // concatenate ':' in the string time

  dtostrf(s, 1, 0, temp);      // convert float second to string and add to temp array
  time += temp;                // concatenate the second in the string time

  return time;                 // return time in this format: 0:0:0

} // end of printTime()


// method to save the log
void saveToLog()
{

  // ("speed; avgSpeed; rotations S; cadence; avgCadence; rotations C; rideTime; movingTime; temperature;");
  
  // holds temporary values
  char temp[55];

  // cleans logLine
  logLine = 0;
  
  // add the current speed
  dtostrf(kph, 1, 2, temp);
  logLine += temp;
  logLine += "; ";
  
  // adds average speed
  dtostrf(avgSpeed, 1, 2, temp);
  logLine += temp;
  logLine += "; ";
  
  // adds the number of wheel rotations
  logLine += String(speedNumberSamples);
  logLine += "; "; 

  // adds the current cadence
  dtostrf(cadence, 1, 2, temp);      
  logLine += temp;
  logLine += "; ";
  
  // adds average cadence
  dtostrf(avgCadence, 1, 2, temp);     
  logLine += temp;
  logLine += "; ";

  // adds number of pedal rotations
  logLine += String(cadenceNumberSamples);
  logLine += "; ";   
  
  // ride total time
  logLine += String(rideTime);
  logLine += "; ";
  
  // moving time
  logLine += String(movingTime);
  logLine += "; ";  
  
  // adds temperature
  dtostrf(temperature, 1, 2, temp);
  logLine += temp;
  logLine += "; ";  
  
  // adds average temperature
  dtostrf(avgTemp, 1, 2, temp);
  logLine += temp;
  logLine += "; ";
  
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
 /*
  myFile = SD.open(logName, FILE_WRITE);  

  // if the file opened okay, write to it:
  if (myFile)
  {
    //Serial.print("Writing to test.csv...");
    myFile.print(logLine);
    myFile.println();

    // close the file:
    myFile.close();    
    
  }   
  */
} // end of saveToLog



//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

// DISPLAY SERIAL MONITOR

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////


// method to display the speed data
void displayKMH()
{
  Serial.print("Speed: ");
  Serial.print(kph);
  Serial.print(" km/h");
  Serial.print(" | ");

  Serial.print("AvgSpeed: ");
  Serial.print(avgSpeed);
  Serial.print(" | ");

  Serial.print("rotations S: ");
  Serial.print(speedNumberSamples);
  Serial.print(" | ");
  
/*
  Serial.print("Avg Speed Mov Total: ");
  Serial.print(speedSamplesSum/(float)rideTime);
  Serial.print(" | ");

  Serial.print("speedSum: ");
  Serial.print(speedSamplesSum);
  Serial.print(" | ");  

  Serial.print("Top Speed ");
  Serial.print(topSpeed);
  Serial.print(" | ");
*/

} // end of displayKMH()

// method to display the cadence data
void displayCadence()
{
  Serial.print("Cadence: ");
  Serial.print(cadence);
  Serial.print(" | ");

  Serial.print("AvgCadence: ");
  Serial.print(speedSamplesSum/(float)movingTime);
  Serial.print(" | ");

  Serial.print("rotations C: ");
  Serial.print(cadenceNumberSamples);
  Serial.print(" | ");

/*
  Serial.print("Top Cadence: ");
  Serial.print(topCadence);
  Serial.print(" | ");
*/

} // end displayCadence()

// method to display the temp data
void displayTemp()
{
    
  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print(" | ");
  
  Serial.print("Avg Temp: ");
  Serial.print(avgTemp);
  Serial.print(" | ");
  
  Serial.print("Max Temp: ");
  Serial.print(maxTemp);
  Serial.print(" | ");
  
  Serial.print("Min Temp: ");
  Serial.print(minTemp);
  Serial.print(" | ");

} // end of display temp


// print other data
void diplayOhterData()
{
  Serial.print("Distance: ");
  Serial.print(rideDistance);
  Serial.print(" | ");

  Serial.print("movingTime: ");
  Serial.print(printTime(movingTime));
  Serial.print(" | ");

  Serial.print("time: ");
  Serial.print(printTime(rideTime));
  Serial.print(" | ");

  Serial.print("screen: ");
  Serial.print(screen);
  Serial.print(" | ");

/*
 Serial.print("speedReedCounter: ");
 Serial.print(speedReedCounter);
 Serial.print(" | ");
 
 Serial.print("speedReedVal: ");
 Serial.print(speedReedVal);
 Serial.print(" | ");
 
 Serial.print("speedTimer: ");
 Serial.print(speedTimer);
 Serial.print(" | ");
*/
}
