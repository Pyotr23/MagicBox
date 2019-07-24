
const byte piezoPin = 3;                        // пин пищалки
const byte buttonPin = 12;                      // пин кнопки
const unsigned int delayInMs = 50;              // задержка между итерациями основного цикла
const unsigned int frequencyGz = 15000;         // частота звучания пищалки
const unsigned int preCodeTime = 3000;          // длительность нажатия кнопки в мс для включения режима записи мелодии
const unsigned int preCodeTolerancePercent = 20;// точность длительности нажатия кнопки для включения режима записи
const unsigned int melodyDurationInMs = 10000;  // время записи мелодии

int firstDuration;        // длительность нажатия для включения режима записи в мс
int downThreshold;        // верхняя граница допустимой длительности нажатия кнопки для включения режима записи в мс
int upThreshold;          // нижняя граница допустимой длительности нажатия кнопки для включения режима записи в мс
unsigned int counter = 0; // счётчик длительности паузы/нажатия

bool clicking = false;    // есть ли нажатие на данной итерации
bool prevClick = false;   // было ли нажатие на предыдущей итерации
bool preCode = false;     // старт вычисления первого нажатия
bool writeMelody = false; // идёт ли запись мелодии

// int writingMelodyCounter;       // счётчик количества итераций записи мелодии
int melodyDurationInCount = -1; // счётчик количества итераций записи мелодии
int preCodeCounter = 1;         // счётчик количества итераций нажатия для включения режима записи

void setup() {  
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(piezoPin, OUTPUT);

//  melodyDurationInCount = melodyDurationInMs / delayInMs;                       
  downThreshold = preCodeTime - preCodeTime / 100 * preCodeTolerancePercent;
  upThreshold = preCodeTime + preCodeTime / 100 * preCodeTolerancePercent;
}

void loop() {     
    clicking = digitalRead(buttonPin);
    digitalWrite(LED_BUILTIN, clicking);
  
    if (clicking){                            // если есть нажатие, то обрабатываем его
        tone(piezoPin, frequencyGz); 
      
        if (!preCode)                          
            preCode = true;      
        else{                                   // начинается обработка длинного первого нажатия
            if (prevClick){                     // если продолжается нажатие, то идёт подсчёт тактов с нажатием
                preCodeCounter++;
                if ((preCodeCounter * delayInMs) % 1000 == 0)
                    Serial.println("Секундa"); 
            }
        }    
        prevClick = true;
    }  
    else{
        noTone(piezoPin); 
        if (preCode){                           // если первое нажатие закончено, то обсчитываем его
            firstDuration = preCodeCounter * delayInMs;
            Serial.println(firstDuration);
            if ((firstDuration >= downThreshold) && (firstDuration <= upThreshold)){
                Serial.println("Открыто!");
                writeMelody = true;
                melodyDurationInCount = -1;
            }
            Serial.println();           
        }      
        preCodeCounter = 0;
        preCode = false;
        prevClick = false;
    }      
    if (writeMelody){
        melodyDurationInCount++;
        if (melodyDurationInMs <= melodyDurationInCount * delayInMs){
            Serial.println("Запись мелодии окончена.");
            writeMelody = false;
        }
    }
    delay(delayInMs);  
}
