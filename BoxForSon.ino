
const byte piezoPin = 3;
const byte buttonPin = 12;
const unsigned int delayInMs = 50;
const unsigned int frequencyGz = 15000;
const unsigned int preCodeTime = 3000;
const unsigned int preCodeTolerancePercent = 20;
const unsigned int melodyDurationInSec = 20;

int firstDuration;
int downThreshold;
int upThreshold;
unsigned int counter = 0;

bool clicking = false;
bool prevClick = false;
bool preCode = false;
bool writeMelody = false;

int writingMelodyCounter; 
int melodyDurationInCount;
int preCodeCounter = 1;

void setup() {  
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(piezoPin, OUTPUT);

  melodyDurationInCount = melodyDurationInSec * 1000 / delayInMs;
  downThreshold = preCodeTime - preCodeTime / 100 * preCodeTolerancePercent;
  upThreshold = preCodeTime + preCodeTime / 100 * preCodeTolerancePercent;
}

void loop() {     
  clicking = digitalRead(buttonPin);
  digitalWrite(LED_BUILTIN, clicking);
  
  if (clicking){
    tone(piezoPin, frequencyGz); 
    if (!preCode){ 
       preCode = true;       
    }
    else{
        if (prevClick){          
            preCodeCounter++;
            if ((preCodeCounter * delayInMs) % 1000 == 0)
                Serial.println("Секундa"); 
        }
    }    
    prevClick = true;
  }  
  else{
      noTone(piezoPin); 
      if (preCode){      
          firstDuration = preCodeCounter * delayInMs;
          Serial.println(firstDuration);
          if ((firstDuration >= downThreshold) && (firstDuration <= upThreshold)){
              Serial.println("Открыто!");
              writeMelody = true;
              melodyDurationInCount = 0;
          }
          Serial.println(); 
      }      
      preCodeCounter = 0;
      preCode = false;
      prevClick = false;
  }      
  delay(delayInMs);
  if (melodyDurationInCount <= )
}
