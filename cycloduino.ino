// cycloduino - ciclocomputer bike speedometer
// Augusto Carmo (carmolim) 2012 - https://github.com/carmolim/cycloduino
// Inspired on the work of Amanda Ghassae - http://www.instructables.com/id/Arduino-Bike-Speedometer/

// TEMP - http://bildr.org/2011/07/ds18b20-arduino/

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 */


/*
 TODO
 
 LOG
 ///
 
 - create a file name with date and time
 
 SPEED
 /////
 
 OK - current speed
 OK - average speed  
 OK - top speed
 
 
 CADENCE
 ///////
 
 OK - how to calculate cadence
 OK - max cadence
 OK - average cadenc
 
 */


// LIBRARIES
////////////

#include <OneWire.h>
#include <SD.h>


// SENSORS
//////////

#define speedReed          A0           // speed reed switch
#define cadenceReed        A1           // cadence reed switch
int tempSensor           = 2;           // DS18S20 Signal pin on digital 2


// LOG
//////

File myFile;                            // object to handle with the file in the SD
String logLine;                         // stores each log line before it is recorded in th SD


// TOTAL MEASURES
/////////////////
float odometer            = 0;          // total distante
int maxReedCounter        = 300;        // min time (in ms) of one rotation (for debouncing)


// PER RIDE
///////////

boolean rideStarted       = false;      // if the bike is moving = true
boolean moving            = false;      // if the bike is moving = true
long rideTime             = 0;          // total time of the ride
long movingTime           = 0;          // only the moving time
long millisCount          = 0;          // stores the number of cicles runned in the interrupt
float distance            = 0.00;       // total distance of the ride in Km


// SPEED VARIBALES
//////////////////

long speedTimer           = 0;          // time between one full rotation (in ms)
long speedNumberSamples   = 0;          // total of revolutions made by the front wheel
float speedSamplesSum     = 0;          // sum of all the speeds collected
float circumference       = 210;        // lenght of the wheel
float kph                 = 0.00;       // speed in kph
float mph                 = 0.00;       // speed in mph
float topSpeed            = 0;          // top speed of the ride
float avgSpeed            = 0;          // average speed of the ride
int speedReedVal          = 0;          // ?? stores if the switch is open or closed // change to boolean?
int speedReedCounter      = 0;          // ??


// CADENCE VARIABLES
////////////////////

long cadenceTimer         = 0;          // time between one full rotation (in ms)
long cadenceNumberSamples = 0;          // total of revolutions made by the front wheel
float cadenceSamplesSum   = 0;          // sum of all the speeds collected
float cadence             = 0.00;       // actual cadence
float avgCadence          = 0;          // average cadence of the ride
float topCadence          = 0;          // top cadence fo the ride
int cadenceReedVal        = 0;          // stores if the switch is open or closed // change to boolean?
int cadenceReedCounter    = 0;          // ??


// TEMPERATURE
//////////////

OneWire ds(tempSensor);                 // on digital pin 2
float temperature        = 0.00;        // stores the temperature
float maxTemp            = 0.00;        // stores the maximum temperature of the ride
float minTemp            = 100.00;      // stores the minimum temperature of the ride
float avgTemp            = 0.00;        // stores the average temp of the ride
float tempSum            = 0.00;        // sum of all the temperature reads

void setup()
{
  speedReedCounter = maxReedCounter;

  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
  pinMode(53, OUTPUT);
  pinMode(speedReed, INPUT);            // speed input
  pinMode(cadenceReed, INPUT);          // cadence input


<<<<<<< HEAD
/* did´t work without the SD shield ?
=======
/* didn´t work without the SD shield ?
>>>>>>> Measuring time more precisely, speed calculations fix

  // SD

  if (!SD.begin(4))
  {
    Serial.println("initialization failed!");
    return;
  }

  Serial.println("initialization done."); 

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("log.csv", FILE_WRITE);  

  // if the file opened okay, write to it:
  if (myFile)
  {
    //Serial.print("Writing to log.csv...");
    myFile.print("speed; avgSpeed; rotations S; cadence; avgCadence; rotations C; rideTime; movingTime; temperature;");
    myFile.println();

    // close the file:
    myFile.close();   
  }   

*/

  // TIMER SETUP- the timer interrupt allows precise timed measurements of the reed switch
  //for more info about configuration of arduino timers see http://arduino.cc/playground/Code/Timer1

  cli(); //stop interrupts

  //set timer1 interrupt at 1kHz
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


ISR(TIMER1_COMPA_vect)
{// Interrupt at freq of 1kHz to measure reed switch


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
<<<<<<< HEAD
      kph = (37.76*float(circumference))/float(speedTimer); //calculate kilometers per hour why 37.76?
=======
      kph = (36*float(circumference))/float(speedTimer); // calculate kilometers per hour
>>>>>>> Measuring time more precisely, speed calculations fix

      // reset speedTimer      
      speedTimer = 0;

      // reset speedReedCounter
      speedReedCounter = maxReedCounter;

      // increase number of samples by 1 - number of wheel rotations ajust the debouncer??
      speedNumberSamples++;  

      // starts the timer
      rideStarted = true;

      // the wheel is spinning
      moving = true;
    }

    else
    {
      if (speedReedCounter > 0)
      {// don't let speedReedCounter go negative
        speedReedCounter -= 1; // decrement speedReedCounter
      }
    }
  }

  else
  {
    // if reed switch is open
    if (speedReedCounter > 0)
    {// don't let speedReedCounter go negative
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

}

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

void loop()
{  
 
  // if the ride started...
  if(rideStarted)
  {    
    // adds the actual temperature read to de sum
    tempSum += temperature;
    
    // save to log
    saveToLog();
  }


  // DISTANCE
  ///////////
  
  // Ride distance
  distance = circumference * (float)speedNumberSamples / 100000;     // calculate distance in Km (1000 m)  


  // SPEED
  ////////

  // verifies if this speed is the top speed of the ride
  if(kph > topSpeed)
  {
    topSpeed = kph;
  }      

  // average speed
  speedSamplesSum += kph;                                // add the new calculate kph                                     
  avgSpeed = speedSamplesSum/(float)movingTime;          // calculate average speed

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
  avgCadence = cadenceSamplesSum/(float)movingTime;      // calculate average cadence

  // print cadence once a second
  displayCadence();
  

  // TEMPERATURE
  //////////////

  // update the actual temperature
  temperature = getTemp();
  
  // calulate the avgTemp
  avgTemp = tempSum/(float)rideTime;

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

  
  // print other data

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" | ");

  Serial.print("movingTime: ");
  Serial.print(printTime(movingTime));
  Serial.print(" | ");

  Serial.print("time: ");
  Serial.print(printTime(rideTime));
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

  Serial.println(); // jump to the next line

  delay(1000); // waits for 1s for the next loop

}// end of loop


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
  myFile = SD.open("log.csv", FILE_WRITE);  

  // if the file opened okay, write to it:
  if (myFile)
  {
    //Serial.print("Writing to test.csv...");
    myFile.print(logLine);
    myFile.println();

    // close the file:
    myFile.close();    
  }   
} // end of saveToLog


// method to get temp method
float getTemp()
{
  //returns the temperature from one DS18S20 in DEG Celsius

  byte data[12];
  byte addr[8];

  if (!ds.search(addr))
  {
    // no more sensors on chain, reset search
    ds.reset_search();
    return -1000;
  }

  if (OneWire::crc8( addr, 7) != addr[7])
  {
    Serial.println("CRC is not valid!");
    return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28)
  {
    Serial.print("Device is not recognized");
    return -1000;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = ds.reset();
  ds.select(addr);  
  ds.write(0xBE); // Read Scratchpad


  for (int i = 0; i < 9; i++)
  { // we need 9 bytes
    data[i] = ds.read();
  }

  ds.reset_search();

  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;

  return TemperatureSum;
} // end of getTemp()