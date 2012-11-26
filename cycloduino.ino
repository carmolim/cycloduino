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
 
 
 SPEED
 /////////////////
 
 OK - current speed
 OK - average speed  
 OK - top speed
 
 
 CADENCE
 ////////////////
 
 OK - how to calculate cadence
 OK - max cadence
 OK - average cadenc
 
 */

#include <OneWire.h>

// SENSORS
//////////

#define speedReed          A0        // speed reed switch
#define cadenceReed        A1        // cadence reed switch
int tempSensor            = 2;       // DS18S20 Signal pin on digital 2





// TOTAL MEASURES
/////////////////
float odometer            = 0;       // total distante
int maxReedCounter        = 300;     // min time (in ms) of one rotation (for debouncing)


// PER RIDE
///////////

boolean rideStarted       = false;    // if the bike is moving = true
boolean moving            = false;    // if the bike is moving = true
long rideTime             = 0;        // total time of the ride
long movingTime           = 0;        // only the moving time
float distance            = 0.00;     // total distance of the ride in Km


// SPEED VARIBALES

long speedTimer           = 0;        // time between one full rotation (in ms)
long speedNumberSamples   = 0;        // total of revolutions made by the front wheel
float speedSamplesSum     = 0;       // sum of all the speeds collected
float circumference       = 210;      // lenght of the tire
float kph                 = 0.00;     // speed in kph
float mph                 = 0.00;     // speed in mph
float topSpeed            = 0;        // top speed of the ride
float avgSpeed            = 0;        // average speed of the ride
int speedReedVal          = 0;        // ?? stores if the switch is open or closed // change to boolean?
int speedReedCounter      = 0;        // ??

// CADENCE VARIABLES

long cadenceTimer         = 0;        // time between one full rotation (in ms)
long cadenceNumberSamples = 0;        // total of revolutions made by the front wheel
float cadenceSamplesSum   = 0;        // sum of all the speeds collected
float cadence             = 0.00;     // actual cadence
float avgCadence          = 0;        // average cadence of the ride
float topCadence          = 0;        // top cadence fo the ride
int cadenceReedVal        = 0;        // stores if the switch is open or closed // change to boolean?
int cadenceReedCounter    = 0;        // ??


// TEMPERATURE

OneWire ds(tempSensor); // on digital pin 2

float temperature        = 0.00;    // stores the temperature
float maxTemp            = 0.00;    // stores the maximum temperature of the ride
float minTemp            = 0.00;    // stores the minumum temperature of the ride
float avgTemp            = 0.00;    // stores the average temp of the ride
float tempSum            = 0.00;    // sum of all the temperature reads

void setup()
{
  speedReedCounter = maxReedCounter;

  pinMode(speedReed, INPUT);            // speed input
  pinMode(cadenceReed, INPUT);          // cadence ipunt


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

  // get val of A0
  speedReedVal = digitalRead(speedReed);

  if (speedReedVal)
  {
    // if reed switch is closed
    if (speedReedCounter == 0)
    {

      //min time between pulses has passed
      kph = (37.76*float(circumference))/float(speedTimer); //calculate kilometers per hour why 37.76?

      // reset speedTimer      
      speedTimer = 0;

      //reset speedReedCounter
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
        speedReedCounter -= 1;//decrement speedReedCounter
      }
    }
  }

  else
  {
    // if reed switch is open
    if (speedReedCounter > 0)
    {//don't let speedReedCounter go negative
      speedReedCounter -= 1; //decrement speedReedCounter
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

  // verifies if this speed is the top speed of the ride
  if(kph > topSpeed)
  {
    topSpeed = kph;
  }    

  // CADENCE
  /////////////


  // get val of A1
  cadenceReedVal = digitalRead(cadenceReed);

  // if reed switch is closed
  if (cadenceReedVal)
  {
    if (cadenceReedCounter == 0)
    {// min time between pulses has passed

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

  // verifies if this cadence is the top cadence of the ride
  if(cadence > topCadence)
  {
    topCadence = cadence;
  }

}

void displayKMH()
{
  Serial.print("Speed: ");
  Serial.print(kph);
  Serial.print(" km/h");
  Serial.print(" | ");

  Serial.print("Avg Speed Mov: ");
  Serial.print(avgSpeed);
  Serial.print(" | ");

  Serial.print("Avg Speed Mov Total: ");
  Serial.print(speedSamplesSum/(float)rideTime);
  Serial.print(" | ");

  Serial.print("speedSamplesSum: ");
  Serial.print(speedSamplesSum);
  Serial.print(" | ");  

  Serial.print("rotations S: ");
  Serial.print(speedNumberSamples);
  Serial.print(" | ");

  /*
  Serial.print("Top Speed ");
   Serial.print(topSpeed);
   Serial.print(" | ");
   */
}


void displayCadence()
{
  Serial.print("Cadence: ");
  Serial.print(cadence);
  Serial.print(" | ");

  Serial.print("Avg Cadence Mov: ");
  Serial.print(avgCadence);
  Serial.print(" | ");

  Serial.print("Top Cadence: ");
  Serial.print(topCadence);
  Serial.print(" | ");

  Serial.print("rotations C: ");
  Serial.print(cadenceNumberSamples);
  Serial.print(" | ");

} // ende displayCadence()

void loop()
{  

  //print kph once a second
  displayKMH();
  displayCadence();

  // increments 1 every period of 1s
  if(rideStarted)
  {
    rideTime++;
  }

  // increments 1 every period of 1s
  if(moving)
  {
    movingTime++;
  }

  // AVERAGE SPEED  
  speedSamplesSum += kph;                                // add the new calculate kph                                     
  avgSpeed = speedSamplesSum/(float)movingTime;          // calculate average speed

  // AVERAGE CADENCE  
  cadenceSamplesSum += cadence;                          // add the new calculate cadence
  avgCadence = cadenceSamplesSum/(float)movingTime;      // calculate average cadence

    // RIDE DISTANCE
  distance = circumference * (float)speedNumberSamples / 100000;     // calculate distance in Km  



  // TEMPERATURE

  // update the actual temperature
  // temperature = getTemp();

  // adds the actual temperature readind to de sum
  tempSum += temperature;

  // calulate the avgTemp
  avgTemp = tempSum/rideTime;

  // verifies that this is the highest temperature recorded
  if(temperature > maxTemp)
  {
    maxTemp = temperature;
  }

  // verifies that this is the lowest temperature recorded
  if(temperature < minTemp)
  {
    minTemp = temperature;
  }

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" | ");

  Serial.print("movingTime: ");
  Serial.print(printTime(movingTime));
  Serial.print(" | ");

  Serial.print("total time: ");
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

  Serial.println();

  delay(1000); 

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


  return time;                  // return time in this format: 0:0:0

}


// Get temp method
float getTemp()
{
  //returns the temperature from one DS18S20 in DEG Celsius

  byte data[12];
  byte addr[8];

  if ( !ds.search(addr))
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











