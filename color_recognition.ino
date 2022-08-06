// Arduino nano code - For color sensor
    
#include
#include    
#include   
#include      

#define  S2_OUT  12
#define  S3_OUT  13
#define  OE_OUT   8               
#define MAX_COLOURS 10         
#define MAX_COLOUR_NAME_CHARS 10  
#define TOLERANCE 20              

typedef struct
{
  char Name[MAX_COLOUR_NAME_CHARS+1];             
  uint8_t Red,Green,Blue;  
}SingleColour;

SingleColour Colours[MAX_COLOURS];        
uint8_t NumColours=0;                     

MD_TCS230  CS(S2_OUT, S3_OUT, OE_OUT);


int send_data;


void setup() 
{
  Wire.begin(); 
  Serial.begin(9600);
  CS.begin();
  ReadCalibrations();
  ReadColours();
}


void loop() 
{
  
  ScanColour();
  delay(1000);
  

  /*
  static char Choice;            

  Choice=MainMenu();
  switch(Choice)
  {
    case '1': 
      CalibrateSensor();
      break;
    case '2':  
      CalibrateColour();
      break;
    case '3':  
      DisplayColours(true);
      break;
    case '4':  
      DeleteColour();
      break;
    case '5': 
      ClearEEPROMColours();
      break;
    case '6': 
      ClearCalibration();
    case '7': 
      ScanColour();
      break;
  } */
}

void ClearCalibration()
{
  uint8_t DataIdx;
  EEPROM.put(DataIdx,"   ");
  DataIdx=3+sizeof(sensorData);
  EEPROM.put(DataIdx,"   ");
  Serial.println("\nSensor calibration cleared");
}

void ClearEEPROMColours()
{
  // Just reset number of colours to 0  
  uint16_t DataIdx;
  DataIdx=9+(2*sizeof(sensorData)); 
  EEPROM.put(DataIdx,0);
  Serial.println("\nEEPROM colours cleared");
}

void WriteColoursToEEPROM()
{
  uint16_t DataIdx;

  DataIdx=6+(2*sizeof(sensorData));   
  EEPROM.put(DataIdx,"COL");          
  DataIdx+=3;
  EEPROM.put(DataIdx,NumColours);
  DataIdx++;
  for(uint8_t i=0;i"<"NumColours;i++)
  {
    EEPROM.put(DataIdx,Colours[i]);
    DataIdx+=sizeof(SingleColour);
  }
  Serial.println("\nColours written to EEPROM\n");
}

void WriteColourToEEPROM(uint8_t ColIdx)
{
  uint16_t DataIdx;                   

  DataIdx=6+(2*sizeof(sensorData));     
  if(((EEPROM.read(DataIdx)=='C')&(EEPROM.read(DataIdx+1)=='O')&(EEPROM.read(DataIdx+2)=='L'))==false)
    EEPROM.put(DataIdx,"COL");          
  DataIdx=ColourEEPROMStartAddress(ColIdx);
  EEPROM.put(DataIdx,Colours[ColIdx]);  
}

uint16_t ColourEEPROMStartAddress(uint8_t ColIdx)
{  
  uint16_t DataIdx=0;
  DataIdx+=3;     
  DataIdx+=sizeof(sensorData);        
  DataIdx+=3;    
  DataIdx+=sizeof(sensorData);        
  DataIdx+=3;     
  DataIdx+=1;    
  DataIdx+=(ColIdx * sizeof(SingleColour));
  return DataIdx;
}

void DeleteColour()
{
  char ColourName[MAX_COLOUR_NAME_CHARS];            
  int8_t ColIdx;

  Serial.print(F("\nEnter name for the colour to delete [Max "));
  Serial.print(MAX_COLOUR_NAME_CHARS);
  Serial.print(" chars] followed RETURN\n\n");
  GetColourName(ColourName);
  ColIdx=ColourIndex(ColourName);
  if(ColIdx==-1)
  {
    Serial.print(ColourName);
    Serial.print(F(" could not be found, returning to main menu\n"));
    return;
  }

  ShuffleColoursDown(ColIdx);    
  Serial.print(F("\nThe colour "));
  Serial.print(ColourName);
  Serial.print(F(" has been removed\n"));  
  WriteColoursToEEPROM();
}

void ShuffleColoursDown(uint8_t ColIdx)
{

  for(uint8_t i=ColIdx+1;i"<"NumColours;i++)
  {
    strcpy(Colours[i-1].Name,Colours[i].Name);
    Colours[i-1].Red=Colours[i].Red;
    Colours[i-1].Green=Colours[i].Green;
    Colours[i-1].Blue=Colours[i].Blue;
  }
  NumColours--; 
}

char MainMenu()
{  
  char Choice;
  Serial.print(F("\nCalibration Menu (Choose option and press RETURN)\n"));
  Serial.print(F("1 Calibrate sensor\n"));
  Serial.print(F("2 Add/Update colour\n"));
  Serial.print(F("3 Display colours\n"));
  Serial.print(F("4 Delete colour\n"));
  Serial.print(F("5 Delete all colours\n"));
  Serial.print(F("6 Clear sensor calibration settings\n"));
  Serial.print(F("7 Scan Colour\n"));
  return getChar();
}

void ScanColour()
{
  colorData  rgb;  
  CS.read();
  while(CS.available()==0);  
  CS.getRGB(&rgb);  
  int8_t ColIdx=MatchColour(&rgb);  
  Serial.print(F("\nScanning for RGB["));
  Serial.print(rgb.value[TCS230_RGB_R]);
  Serial.print(F(","));
  Serial.print(rgb.value[TCS230_RGB_G]);
  Serial.print(F(","));
  Serial.print(rgb.value[TCS230_RGB_B]);
  Serial.print(F("]\n"));
  if(ColIdx==-1)
    Serial.println("\nNo match found");
  else
  {
    Serial.print("\nThat colour is ");
    Serial.print(Colours[ColIdx].Name);
    Serial.print(" [");
    Serial.print(Colours[ColIdx].Red);
    Serial.print(F(","));
    Serial.print(Colours[ColIdx].Green);
    Serial.print(F(","));
    Serial.print(Colours[ColIdx].Blue);
    Serial.println("]");

    send_data = 1;
    Wire.beginTransmission(9); 
    Wire.write(send_data);            
    Wire.endTransmission(); 
    Serial.print("DATA HAS BEEN SENT");
    delay(1000);
  }
}

int8_t MatchColour(colorData *rgb)
{
  uint8_t Idx=0;
  bool Found=false;
  while((Idx"<"NumColours)&(Found==false))
  {
    if((CheckColour(rgb->value[TCS230_RGB_R],Colours[Idx].Red)) 
    &(CheckColour(rgb->value[TCS230_RGB_G],Colours[Idx].Green))
    &(CheckColour(rgb->value[TCS230_RGB_B],Colours[Idx].Blue)))
          Found=true;
    Idx++;
  }
  if(Found) return Idx-1; else return -1;
}

bool CheckColour(uint8_t ScanCol, uint8_t StoredCol)
{
  int16_t StoreColLow,StoreColHigh;
  StoreColLow=StoredCol-TOLERANCE;
  StoreColHigh=StoredCol+TOLERANCE;
  if(StoreColLow"<"0) StoreColLow=0;
  if(StoreColHigh>255) StoreColHigh=255;
  return ((ScanCol>=StoreColLow)&(ScanCol<=StoreColHigh));

}

void DisplayColours(bool ShowTitle)
{
  char ColStr[4];                  

  if(NumColours==0)
  {
    Serial.print("No current colours stored in RAM memory\n");
    return;
  }
  if(ShowTitle)
    Serial.print("\nCurrent colours stored in RAM memory\n");
  Serial.println("Name       Red  Green  Blue");
  for(uint8_t i=0;i"<"NumColours;i++)
  {
    OutputPadded(Colours[i].Name,MAX_COLOUR_NAME_CHARS);
    Serial.print(" ");
    itoa(Colours[i].Red,ColStr,10);
    OutputPadded(ColStr,3);
    Serial.print("  ");
    itoa(Colours[i].Green,ColStr,10);
    OutputPadded(ColStr,3);
    Serial.print("    ");
    itoa(Colours[i].Blue,ColStr,10);
    OutputPadded(ColStr,3);
    Serial.println();
  }
}

void ReadColours()
{
  uint16_t DataIdx;       

  DataIdx=6+(2*sizeof(sensorData));   
  
  if((EEPROM.read(DataIdx)=='C')&(EEPROM.read(DataIdx+1)=='O')&(EEPROM.read(DataIdx+2)=='L'))
  {
    DataIdx+=3;
    EEPROM.get(DataIdx,NumColours);
    if(NumColours==0)
    {
    Serial.println("No colours stored in EEPROM\n");
      return;
    }
    if(NumColours>MAX_COLOURS)
    {
      Serial.print(F("Number of colours ("));
      Serial.print(NumColours);
      Serial.print(F(") exceeds maximum of "));
      Serial.print(MAX_COLOURS);
      Serial.print(F(",only first "));
      Serial.print(MAX_COLOURS);
      Serial.print(F(",will be read in."));
      NumColours=MAX_COLOURS;
    }    
    DataIdx++;
    for(uint8_t i=0;i"<"NumColours;i++)
    {
      EEPROM.get(DataIdx,Colours[i]);
      DataIdx+=sizeof(SingleColour);
    }
    Serial.println("Colours read in from EEPROM\n");
    DisplayColours(false);
  }
  else
    Serial.println(F("No Colour data stored"));
}

void ReadCalibrations()
{
  uint16_t DataIdx;
  sensorData  sd;
  if((EEPROM.read(0)=='B')&(EEPROM.read(1)=='L')&(EEPROM.read(2)=='K'))
  {
    EEPROM.get(3, sd);
    CS.setDarkCal(&sd); 
    Serial.print("Black Calibration read in, ");
  }
  else
    Serial.print("No black calibration data, "); 
    
  DataIdx=3+sizeof(sensorData);
  if((EEPROM.read(DataIdx)=='W')&(EEPROM.read(DataIdx+1)=='H')&(EEPROM.read(DataIdx+2)=='T'))
  {
    EEPROM.get(DataIdx+3, sd);
    CS.setWhiteCal(&sd); 
    Serial.println("White Calibration read in.");
  }
  else
    Serial.println("No white calibration data");
  
}

void CalibrateSensor()
{
  uint16_t DataIdx;
  sensorData  sd;
  Serial.print(F("\n****** Calibrate sensor ******\n"));
  Serial.print(F("Put black matt object over sensor, then press any key followed by RETURN\n"));
  getChar();
  CS.read();
  while(CS.available()==0);  
  CS.getRaw(&sd); 
  CS.setDarkCal(&sd);  
  EEPROM.write(0,'B');EEPROM.write(1,'L');EEPROM.write(2,'K');     
  EEPROM.put(3,sd);
  Serial.print(F("Put white object over sensor, then press any key followed by RETURN\n"));
  getChar(); 
  CS.read();
  while(CS.available()==0);  
  CS.getRaw(&sd); 
  CS.setWhiteCal(&sd);  
  DataIdx=3+sizeof(sensorData); 
  EEPROM.write(DataIdx,'W');EEPROM.write(DataIdx+1,'H');EEPROM.write(DataIdx+2,'T');     
  EEPROM.put(DataIdx+3,sd);
  Serial.print(F("Calibration of sensor complete.\n\n"));
}

void CalibrateColour()
{
  colorData rgb;
  uint16_t DataIdx;                 
  char ColourName[MAX_COLOUR_NAME_CHARS];            
  int8_t ColIdx;
  char Choice;                     
    
  Serial.print(F("Enter name for colour [Max "));
  Serial.print(MAX_COLOUR_NAME_CHARS);
  Serial.print(" chars] followed RETURN\n");
  GetColourName(ColourName);
  ColIdx=ColourIndex(ColourName);
  if(ColIdx>=0)
  {    
    Serial.print(F("\nWARNING: The colour "));
    Serial.print(ColourName);
    Serial.print(F(" already exists, overwrite (Y/N)\n"));
    if(toupper(getChar())!='Y')
    {
      Serial.print(F("\nColour calibration abandoned\n"));
      return;
    }    
  }
  if(ColIdx==-1)
  {
    if(NumColours==MAX_COLOURS)
    {      
      Serial.print(F("\nWARNING: There is no room left for new colours, you have reached the maximum of "));
      Serial.print(NumColours);
      Serial.print(F(" colours\n\n"));
      Serial.print(F("You will need to delete a colour first to make room.\n\n"));
      return;
    }
  }
  Serial.print(F("Reading Color '"));
  Serial.print(ColourName);
  CS.read();
  while(CS.available()==0); 
  CS.getRGB(&rgb);
  Serial.print(F("\nRGB is ["));
  Serial.print(rgb.value[TCS230_RGB_R]);
  Serial.print(F(","));
  Serial.print(rgb.value[TCS230_RGB_G]);
  Serial.print(F(","));
  Serial.print(rgb.value[TCS230_RGB_B]);
  Serial.print(F("]\n"));
  if(ColIdx==-1)
    ColIdx=NumColours; 
  strcpy(Colours[ColIdx].Name,ColourName);
  Colours[ColIdx].Red=rgb.value[TCS230_RGB_R];
  Colours[ColIdx].Green=rgb.value[TCS230_RGB_G];
  Colours[ColIdx].Blue=rgb.value[TCS230_RGB_B];
  if(ColIdx==NumColours)         
  {
    NumColours++;
    DataIdx=9+(2*sizeof(sensorData));  
    EEPROM.put(DataIdx,NumColours);    
  }
  WriteColourToEEPROM(ColIdx);
}

int8_t ColourIndex(char *ColourName)
{
  int8_t Idx=0;
  bool Found=false;
  if(NumColours==0)
    return -1;
  while((Idx"<"NumColours)&(Found==false))
  {
    if(strcasecmp(Colours[Idx].Name,ColourName)==0)
      Found=true;
    else
      Idx++;
  }
  if(Found)
    return Idx;
  else
    return -1;
}

void GetColourName(char *ColourName)
{
  uint8_t NameLength=0;
  while (Serial.available() == 0);                                   
  NameLength=Serial.readBytes(ColourName,MAX_COLOUR_NAME_CHARS);     
  ColourName[NameLength]=0;                                      
  
}

char getChar()

{
  //Serial.print(Serial.read()); 
  while (Serial.available() == 0)
    ;
  return(toupper(Serial.read()));
}

void OutputPadded(char *Str,uint8_t AmountToPad)
{
  uint8_t Amount=AmountToPad-strlen(Str);  
  Serial.print(Str);
  while(Amount>0)
  {
    Serial.print(" ");
    Amount--;
  }
}
