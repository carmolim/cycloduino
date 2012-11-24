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


#define speedReed         A0         // speed reed switch
#define cadenceReed       A1         // cadence reed switch


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
long speedSamplesSum      = 0;        // sum of all the speeds collected
long speedNumberSamples   = 0;        // total of revolutions made by the front wheel
float circumference       = 210;      // lenght of the tire
float kph                 = 0.00;     // speed in kph
float mph                 = 0.00;     // speed in mph
float topSpeed            = 0;        // top speed of the ride
float avgSpeed            = 0;        // average speed of the ride
int speedReedVal          = 0;        // ?? stores if the switch is open or closed // change to boolean?
int speedReedCounter      = 0;        // ??

// CADENCE VARIABLES

long cadenceTimer         = 0;        // time between one full rotation (in ms)
long cadenceSamplesSum    = 0;        // sum of all the speeds collected
long cadenceNumberSamples = 0;        // total of revolutions made by the front wheel
float cadence             = 0.00;     // actual cadence
float avgCadence          = 0;        // average cadence of the ride
float topCadence          = 0;        // top cadence fo the ride
int cadenceReedVal        = 0;        // stores if the switch is open or closed // change to boolean?
int cadenceReedCounter    = 0;        // ??


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


}

void loop()
{
  //print kph once a second
  displayKMH();
  displayCadence();

  //
  if(rideStarted)
  {
    rideTime++;
  }

  //
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


  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" | ");

  Serial.print("rotations S: ");
  Serial.print(speedNumberSamples);
  Serial.print(" | ");

  Serial.print("movingTime: ");
  Serial.print(movingTime);
  Serial.print(" | ");

  Serial.print("total time: ");
  Serial.print(rideTime);
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







