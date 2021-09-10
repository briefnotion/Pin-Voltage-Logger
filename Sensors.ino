// ***************************************************************************************
// *
// *    Core       | Everything within this document is proprietary to Core Dynamics.
// *    Dynamics   | Any unauthorized duplication will be subject to prosecution.
// *
// *    Department : (R+D)^2
// *       Sub Dept: Programming
// *    Location ID: 856-45B
// *                                                      (c) 2856 - 2857 Core Dynamics
// ***************************************************************************************
// *
// *  PROJECTID: <UNASSIGNED>    Revision: 00000000.01
// *  TEST CODE:                 QACODE: UNASSIGNED        CENSORCODE: <NOTAPPROVED>
// *
// ***************************************************************************************
// *  Programmer Notes:
// *
// *            Coder:  <CODER>
// *    Support Coder:  None
// *  --- Apnd:
// *    Other Sources:  None
// *
// *  Description:
// *    Report voltage values of pin A1-A7 through serial com port, in comma deliminated 
// *	log format
// *
// ***************************************************************************************
// *
// * V 0.01 _210910
// *      - Published to HUB
// *
// ***************************************************************************************


// Testing and Debugging
#define BOOTEST       false
#define RESTTIME      3000            
#define TESTRESTTIME  100

#define SENSORCOUNT     7
#define SENSOR_A1       A1
#define SENSOR_A2       A2
#define SENSOR_A3       A3
#define SENSOR_A4       A4
#define SENSOR_A5       A5
#define SENSOR_A6       A6
#define SENSOR_A7       A7

// ---------------------------------------------------------------------------------------
// CLASSES

class Sensor
{
  private:
  String strSENSOR_TYPE = "";
  String strSENSOR_DESC = "";
  
  unsigned int intMAX = 0;
  unsigned int intMIN = 0;

  unsigned long uintACCUM = 0;
  unsigned long uintSAMPLE_SIZE = 0;


  public:
  
  void define(String strSensor_Type, String strSensor_Desc)
  {
    strSENSOR_TYPE = strSensor_Type;
    strSENSOR_DESC = strSensor_Desc;
  }

  String type()
  {
    return strSENSOR_TYPE;
  }

  String desc()
  {
    return strSENSOR_DESC;
  }

  void record(int intValue)
  {
    uintACCUM = uintACCUM + intValue;
    uintSAMPLE_SIZE++;

    if (uintSAMPLE_SIZE == 1)
    {
      intMAX = intValue;
      intMIN = intValue;
    }
    else
    {
      if(intValue > intMAX)
      {
        intMAX = intValue;
      }
      
      if(intValue < intMIN)
      {
        intMIN = intValue;
      }
    }
  }

  void reset_vals()
  {
    intMAX = 0;
    intMIN = 0;
    uintACCUM = 0;
    uintSAMPLE_SIZE = 0;
  }

  unsigned long get_average()
  {
    return (unsigned long)(uintACCUM / uintSAMPLE_SIZE);
  }

  unsigned int get_sample_size()
  {
    return uintSAMPLE_SIZE;
  }

  unsigned int get_accum()
  {
    return uintACCUM;
  }

  unsigned int get_min()
  {
    return intMIN;
  }

  unsigned int get_max()
  {
    return intMAX;
  }
};


// ***************************************************************************************
// MAIN PROGRAM
// ***************************************************************************************

// GLOBAL
unsigned long tmeCurrentMillis = millis();
unsigned long tmePrevMillis = 0;
int intRestTime = RESTTIME;  // 16 = 60 fps     // Do not check for update until rest 
                                                //  time is passed.

// Onboard LED to signify data being sent to LED strip.
const int ledPin =  LED_BUILTIN;

void setup()
{

  /*

  MQ List
  
  MQ-2 - Methane, Butane, LPG, Smoke
  MQ-3 - Alcohol, Ethanol, Smoke
  MQ-4 - Methane, CNG Gas
  MQ-5 - Natural gas, LPG
  MQ-6 - LPG, butane
  MQ-7 - Carbon Monoxide
  MQ-8 - Hydrogen Gas
  MQ-9 - Carbon Monoxide, flammable gasses
  MQ131 - Ozone
  MQ135 - Air Quality (Benzene, Alcohol, smoke)
  MQ136 - Hydrogen Sulphide gas
  MQ137 - Ammonia
  MQ138 - Benzene, Toluene, Alcohol, Propane, Formaldehyde gas, Hydrogen
  MQ214 - Methane, Natural Gas
  MQ216 - Natural gas, Coal Gas
  MQ303A - Alcohol, Ethanol, smoke
  MQ306A - LPG, butane
  MQ307A - Carbon Monoxide
  MQ309A - Carbon Monoxide, flammable gas

  */

}


// ---------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------
// MAIN LOOP
void loop()
//  Main Loop:
{
  // ---------------------------------------------------------------------------------------
  // Setup

  String strCommentBuffer = "";
  String strComment = "";
  boolean booCommentActive = false;

  // Serial Input
  char bytIn = 0;

  // Timing and Communication
  unsigned long tmeCurrentMillis = 0;
  tmeCurrentMillis = millis();
  Serial.begin(115200);

  // Open serial communications if in debug mode.
  if (BOOTEST == true)
  {
    //Serial.println("Program Start");
    intRestTime = TESTRESTTIME;
  }
  else
  {
    // Normal
    intRestTime = RESTTIME;
  }

  // ---------------------------------------------------------------------------------------
  // Sensor Setup
  // Analog Sensor Array

  Sensor Sensors[SENSORCOUNT];

  Sensors[0].define("MQ-6", "LPG Butane");
  Sensors[1].define("MQ-136", "Hydrogen Sulphide Gas");
  Sensors[2].define("MQ-135", "Air Quality (Benzene Alcohol Smoke)");
  Sensors[3].define("MQ-9", "Carbon Monoxide FlammableGasses");
  Sensors[4].define("MQ-7", "Carbon Monoxide");
  Sensors[5].define("MQ-4", "Methane CNGGas");
  Sensors[6].define("MQ-3", "Alcohol Ethanol Smoke");
  
  int intSensors[SENSORCOUNT];
  
  // Define Sensors
  pinMode(SENSOR_A1, INPUT);  // MQ-6 - LPG, butane 
  pinMode(SENSOR_A2, INPUT);  // MQ-136 = Hydrogen Sulfide Gas 
  pinMode(SENSOR_A3, INPUT);  // MQ-135 - Air Quality (Benzene, Alcohol, smoke)
  pinMode(SENSOR_A4, INPUT);  // MQ-9 - Carbon Monoxide, flammable gasses
  pinMode(SENSOR_A5, INPUT);  // MQ-7 = carbonic monoxide (CO)
  pinMode(SENSOR_A6, INPUT);  // MQ-4 = methane and natural gas
  pinMode(SENSOR_A7, INPUT);  // MQ-3 = ethanol

  // ---------------------------------------------------------------------------------------
  
  //  Get current time.  This will be our timeframe to work in.
  tmeCurrentMillis = millis();

  // ---------------------------------------------------------------------------------------  
  // Print Header

  Serial.print("Time (ms)");
  Serial.print(", est SampleSize");
  
  for (int x=0; x<SENSORCOUNT; x++)
  {
    Serial.print(", ");
    Serial.print(Sensors[x].type());
    Serial.print(" - ");
    Serial.print(Sensors[x].desc());
    Serial.print(", ");
    Serial.print(Sensors[x].type());
    Serial.print("- min");
    Serial.print(", ");
    Serial.print(Sensors[x].type());
    Serial.print("- max");
  }
  Serial.print(", ");
  Serial.print("Comment");
  Serial.println();

  // END Setup
  // ---------------------------------------------------------------------------------------
  

  // ---------------------------------------------------------------------------------------
  // Constant Loop
  while (true)
  {
    //  Get current time.  This will be our timeframe to work in.
    tmeCurrentMillis = millis();

    // ---------------------------------------------------------------------------------------
    // Constant Running at every cycle
    Sensors[0].record(analogRead(SENSOR_A1));
    Sensors[1].record(analogRead(SENSOR_A2));
    Sensors[2].record(analogRead(SENSOR_A3));
    Sensors[3].record(analogRead(SENSOR_A4));
    Sensors[4].record(analogRead(SENSOR_A5));
    Sensors[5].record(analogRead(SENSOR_A6));
    Sensors[6].record(analogRead(SENSOR_A7));

    // Simple time set
    while (Serial.available()>0)
    {
      bytIn = Serial.read();

      if (booCommentActive == true)
      {
        if ((bytIn == 13) or (bytIn == '/'))  //In case you term doesnt support CR, end comment
                                              // mode with "/", like it was started.
        {
          strComment = strCommentBuffer;
          strCommentBuffer = "";
          booCommentActive = false;
        }
        else
        {
          if ((bytIn >=32) && (bytIn <=127))
          {
            strCommentBuffer = strCommentBuffer + bytIn;
          }
        }
      }
      else
      {
        switch (bytIn)
        {
          case 'h':
            strComment = "Help Guide: '/' - Enter Comment, '/' or CR- End Comment. Delay: 0 - none, 1 .1sec, 2 .3sec, 3 1sec, 4 3sec, 5 10sec, 6 1min, 7 10min, 8 30min, 9 1hr";
            break;
          case '/':
            booCommentActive = true;
            //strComment = "Comment Mode";
            break;
          case '0':
            intRestTime = 0;				// No Delay
            strComment = "Sample Size No Delay";
            break;
          case '1':
            intRestTime = 100;			// .1 sec Delay
            strComment = "Sample Size .1 sec";
            break;
          case '2':
            intRestTime = 300;			// .3 sec Delay
            strComment = "Sample Size .3 sec";
            break;
          case '3':
            intRestTime = 1000;			// 1 sec Delay
            strComment = "Sample Size 1 sec";
            break;
          case '4':
            intRestTime = 3000;			// 3 sec Delay
            strComment = "Sample Size 3 sec";
            break;
          case '5':
            intRestTime = 10000;			// 10 sec Delay
            strComment = "Sample Size 10 sec";
            break;
          case '6':
            intRestTime = 60000;			// 1 min Delay
            strComment = "Sample Size 1 min";
            break;
          case '7':
            intRestTime = 60000 * 10;		// 10 min Delay
            strComment = "Sample Size 10 min";
            break;
          case '8':
            intRestTime = 600000 * 30;	// 30 min Delay
            strComment = "Sample Size 30 min";
            break;
          case '9':
            intRestTime = 600000 * 60;	// 1 hr Delay
            strComment = "Sample Size 1 hr";
            break;          
        } // end Switch
      } // end if not booCommentActive
    }

    
    // ---------------------------------------------------------------------------------------
    
    // ---------------------------------------------------------------------------------------
    // Timed Loop
    if (tmeCurrentMillis - tmePrevMillis >= intRestTime)
    {
      tmePrevMillis = tmeCurrentMillis;
  
  
      // --- Grabbing Data From Hardware inputs ---
   
      // Print Time
      Serial.print(tmeCurrentMillis);

      // Print Sample Size from first sensor.  Should be same for all Sensors.
      Serial.print(",  ");
      Serial.print(Sensors[0].get_sample_size());

      // Print values of each sensor
      for (int x=0; x < SENSORCOUNT; x++)
      {
        Serial.print(",  ");
        Serial.print(Sensors[x].get_average());
        Serial.print(",  ");
        Serial.print(Sensors[x].get_min());
        Serial.print(",  ");
        Serial.print(Sensors[x].get_max());

        Sensors[x].reset_vals();
      }

      // Print Comment and clear
      Serial.print(",  ");
      Serial.print(strComment); 
      strComment = "";     
  
      Serial.println();
  
      //  If we made it to this part of the code then we need to
      //    tell the LED hardware that it has a change to commit.
  
      // --- Execute LED Hardware Changes If Anything Was Updated ---
      //if ((booUpdates0 == true) || (booUpdates1 == true))
      {
        //  Turn on onboad LED when communicating with LED Hardware.
        digitalWrite(ledPin, HIGH);
        //  Update LED Hardware with changes.
  
        //  Turn off onboad LED when communication complete.
        digitalWrite(ledPin, LOW);
      }
  
      // Debug Routines ---
      // Slow delay if in testing mode.
      if (BOOTEST == true)
      {
        intRestTime = TESTRESTTIME;
      }
    }   // End Delayless Loop
  }
}
