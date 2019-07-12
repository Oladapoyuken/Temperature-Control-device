#include <EEPROM.h>

#include <DHT.h>

#include <Keypad.h>

//#include <LiquidCrystal.h>

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>


//temperature sensor pin
#define dht_pin 3
#define dht_type DHT11

//Mode indicators
#define heater_led A0
#define cooler_led A3

//buttons pins
#define move_btn A2
#define select_btn A1

//mode relays
#define cooler_relay 4 
#define heater_relay 2

//buzzer
#define buz 13

//LiquidCrystal lcd(2,3,A1,A2,A3,A4);
LiquidCrystal_I2C lcd(0x3F,2,1,0,4,5,6,7,3,POSITIVE);


//constructing "Degree" character
byte degree[8] = {
  0b00011,
  0b00011,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

double keypadInput = 0;

float savedTemp = 0;
float currentTemp = 0;

int savedTemp_len = 0;
int currentTemp_len = 0;

boolean state = false;
boolean preState = false;

//pin D12 is vacant

DHT dht(dht_pin, dht_type);

const byte rows = 4;
const byte cols = 3;
char keys[rows][cols] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'.','0','-'}
};
byte rowPins[rows] = {5,6,7,8};
byte colPins[cols] = {9,10,11};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols);


boolean checkLastMode(){
  boolean lastState = true;
  int opt = 0;
  float temp = 0;
  EEPROM.get(0, opt);
  EEPROM.get(1, temp);
  
  if(opt == 1){
    //start with the cooler
    savedTemp = temp;
    lastState = true;
    state = true;
  }
  else{
    //start with the heater
    savedTemp = temp;
    lastState = false;
    state = false;
  }

  return lastState;
}


void startSystem(){

  if(checkLastMode() == true){
    lcd.setCursor(0,0);
    lcd.print("Cooler: ");
    lcd.print(savedTemp);
    lcd.write(byte(0));
    lcd.print("C");
  }
  else{
    lcd.setCursor(0,0);
    lcd.print("Heater: ");
    lcd.print(savedTemp);
    lcd.write(byte(0));
    lcd.print("C");
  }
  
}


void setup() {
  Serial.begin(9600);

  lcd.begin(16,2);
  lcd.backlight();

  pinMode(move_btn, INPUT);
  pinMode(select_btn, INPUT);
  pinMode(cooler_relay, OUTPUT);
  pinMode(heater_relay, OUTPUT);

  pinMode(buz, OUTPUT);
  pinMode(heater_led, OUTPUT);
  pinMode(cooler_led, OUTPUT);

  digitalWrite(cooler_relay, HIGH);
  digitalWrite(heater_relay, HIGH);

  dht.begin();

  startSystem();
  
  lcd.createChar(0, degree);
}

void sound(){
  digitalWrite(buz, HIGH);
  delay(100);
  digitalWrite(buz, LOW);
  delay(100);
}

void loop() {
  if(analogRead(move_btn) > 500){
    delay(200);
    
    sound();
    Serial.println("MOVE");
    while(true){
       changeMode();
       break;
    }
  }
  
  getTemperature();
  
  if(state == true){
    startDevice('c');
    digitalWrite(heater_relay, HIGH);
    //analogWrite(heater_led, 0);
  }
  else{
    startDevice('h');
    digitalWrite(cooler_relay, HIGH);
    //analogWrite(cooler_led, 0);
  }

  
}

void startDevice(char x){
  switch(x){
    case 'c':{
      if(currentTemp >= savedTemp){
        digitalWrite(cooler_relay, LOW);
        analogWrite(cooler_led, 1000);
      }
      break;
    }
    case 'h':{
      if(currentTemp <= savedTemp){
        digitalWrite(heater_relay, LOW);
        analogWrite(cooler_led, 1000);
      }
      break;
    }
    default:{
      break;
    }
  }
}




//get current temperature of room
void getTemperature(){
  float temp = dht.readTemperature();
  currentTemp = temp;
  
  lcd.setCursor(0,1);
  lcd.print("Current: ");
  lcd.print(temp);
  lcd.write(byte(0));
  lcd.print("C");
  
  Serial.print("Current: ");
  Serial.print(temp);
  Serial.println("'C");
  delay(200);
}

//Selection menu, choose between heater and cooler
//void selection_menu(){
//  lcd.clear();
//  lcd.print("SELECT ->HEATER");
//  lcd.setCursor(9,1);
//  lcd.print("COOLER");
//}


// change between Heater and cooler
void changeMode(){

  while(true){
    if(analogRead(move_btn) > 500){
      
      analogWrite(heater_led, 0);
      analogWrite(cooler_led, 0);

      
      
      delay(200);
      sound();
      if(state == true){
        lcd.clear();
        lcd.print("SELECT:->HEATER");
        lcd.setCursor(9,1);
        lcd.print("COOLER");
        state = false;
      }
      else{
        lcd.clear();
        lcd.print("SELECT:  HEATER");
        lcd.setCursor(7,1);
        lcd.print("->COOLER");
        state = true;
      }
    }
    if(analogRead(select_btn) > 600 ){
      Serial.println("SELECT");
      delay(200);
      sound();
      selectMode();
      break;
    }
    
  }
  digitalWrite(heater_relay, HIGH);
  digitalWrite(cooler_relay, HIGH);
  preState = true;
}

void selectMode(){
  if(state == false){   
    //insert temperature to start heater
    
    Serial.println("Enter Temperature: ");
    setHeaterTemp(getInput());
    
  }
  else {
    //insert temperature to start cooler
    
    Serial.println("Enter Temperature: ");
    setCoolerTemp(getInput());
  }
}

////set cooler temperature and start sensor
void setCoolerTemp(double temp){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("COOLER: ");
  lcd.print(temp);
  lcd.write(byte(0));
  lcd.print("C");
  savedTemp = temp;
  EEPROM.put(0,1);
  EEPROM.put(1,savedTemp);
}

//set heater temperature and start sensor
void setHeaterTemp(double temp){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("HEATER: ");
  lcd.print(temp);
  lcd.write(byte(0));
  lcd.print("C");
  savedTemp = temp;
  EEPROM.put(0,0);
  EEPROM.put(1,savedTemp);
}





//get data from keypad
double getInput(){
  lcd.clear();
  lcd.print("Enter Temp: ");
  while(true){

    String str = "";
    char keyPress = keypad.getKey();
    int counter = 0; 
    
      if (keyPress){
        sound();
        Serial.println(keyPress);
        str = str + keyPress;
        lcd.print(keyPress);
        while(counter < 3){
          
          if(analogRead(select_btn) > 500){
            delay(200);
            sound();
            counter = 3;
          }
          
          char keyPress = keypad.getKey();
          if(keyPress){
            sound();
            Serial.println(keyPress);
            str = str + keyPress;
            lcd.print(keyPress);
            counter++;
            delay(200);
          } 
        }
        if(counter == 3){
          char temp[10] = "";
          //savedTemp_len = sizeof(temp);
          str.toCharArray(temp, sizeof(str));
          keypadInput = atof(temp);
          Serial.println(keypadInput);
          
          break;
        }
      }
  }
  return keypadInput;
}
