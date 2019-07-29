
const byte piezoPin = 3;                        // пин пищалки
const byte buttonPin = 12;                      // пин кнопки
const byte delayInMs = 50;                      // задержка между итерациями основного цикла
const unsigned int frequencyGz = 15000;         // частота звучания пищалки
const unsigned int preCodeTime = 3000;          // длительность нажатия кнопки в мс для включения режима записи мелодии
const unsigned int preCodeTolerancePercent = 20;// точность длительности нажатия кнопки для включения режима записи
const unsigned int melodyDurationInMs = 10000;  // время записи мелодии
const byte durationsQuantity = 50;              // наибольшее число пауз и нажатий в мелодии

int firstDuration;                  // длительность нажатия для включения режима записи в мс
int downThreshold;                  // верхняя граница допустимой длительности нажатия кнопки для включения режима записи в мс
int upThreshold;                    // нижняя граница допустимой длительности нажатия кнопки для включения режима записи в мс
int counter = -1;                   // счётчик длительности паузы/нажатия
byte durations[durationsQuantity];  // массив длительностей нажатий/пауз мелодии
byte notes[durationsQuantity];      // воспроизведённая мелодия
byte currentDuration = 0;           // индекс текущей длительности в массиве длительностей

bool clicking = false;    // есть ли нажатие на данной итерации
bool prevClick = false;   // было ли нажатие на предыдущей итерации
bool preCode = false;     // старт вычисления первого нажатия
bool writeMelody = false; // идёт ли запись мелодии
bool listenMelody = false;// идёт ли прослушивание мелодии

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
  PrintArray(durations);
}

void loop() {     
    clicking = digitalRead(buttonPin);
    digitalWrite(LED_BUILTIN, clicking);
  
    if (clicking){                            // если есть нажатие, то обрабатываем его
        tone(piezoPin, frequencyGz); 

        if (writeMelody){
            Serial.println(counter);
            counter++;
            if ((counter != 0) && !prevClick){
                if (listenMelody)
                    notes[currentDuration] = counter;
                else
                    durations[currentDuration] = counter;
                Serial.print("В массив записана пауза длительностью ");
                Serial.println(counter);
                currentDuration++;
                counter = 0;
            }
            melodyDurationInCount++;
            if (melodyDurationInMs <= melodyDurationInCount * delayInMs){
                if (listenMelody){
                    Serial.println("Прослушивание окончено.");
                    PrintArray(notes);
                    ReplayMelody(notes, delayInMs);
                }
                else {
                    Serial.println("Запись мелодии окончена.");
                    PrintArray(durations);
                    ReplayMelody(durations, delayInMs);
                }                
                // durations[currentDuration] = counter;
                currentDuration = 0;
                counter = -1;
                writeMelody = false;
                listenMelody = false;
            }                            
        }
        else{
            if (!preCode)                          
                preCode = true;      
            else{                                   // начинается обработка длинного первого нажатия
                if (prevClick){                     // если продолжается нажатие, то идёт подсчёт тактов с нажатием
                    preCodeCounter++;
                    if ((preCodeCounter * delayInMs) % 1000 == 0)
                        Serial.println("Секундa"); 
                }
            }                
        }        
    }  
    else{
        noTone(piezoPin); 
        if (writeMelody){  
            Serial.println(counter);          
            if (counter != -1){
                counter++;
                if (prevClick){
                    if (listenMelody)
                        notes[currentDuration] = counter;
                    else
                        durations[currentDuration] = counter;
                    Serial.print("В массив записана нажатие длительностью ");
                    Serial.println(counter);
                    currentDuration++;
                    counter = 0;
                }            
            }               
            
            melodyDurationInCount++;
            if (melodyDurationInMs <= melodyDurationInCount * delayInMs){
                if (listenMelody){
                    Serial.println("Прослушивание окончено.");
                    PrintArray(notes);
                    ReplayMelody(notes, delayInMs);
                }
                else {
                    Serial.println("Запись мелодии окончена.");
                    PrintArray(durations);
                    ReplayMelody(durations, delayInMs);
                }                
                // durations[currentDuration] = counter;
                currentDuration = 0;
                counter = -1;
                writeMelody = false;
                listenMelody = false;
            }
        }
        else{
            if (preCode){                           // если первое нажатие закончено, то обсчитываем его
                firstDuration = preCodeCounter * delayInMs;
                Serial.println(firstDuration);
                writeMelody = true;
                melodyDurationInCount = 0;
                if ((firstDuration >= downThreshold) && (firstDuration <= upThreshold)){
                    Serial.println("Пошла запись мелодии!");                    
                }
                else{
                    Serial.println("Пошло слушание мелодии.");  
                    listenMelody = true;
                    notes[0] = preCodeCounter;
                    counter = 0;
                }
                Serial.println();           
            }      
            preCodeCounter = 0;
            preCode = false;            
        }        
    }

    prevClick = clicking;
    delay(delayInMs);  
}

void PrintArray(byte arr[durationsQuantity]){
    Serial.println();
    for (int i = 0; i < 50; i++){
        Serial.print(arr[i]) ;
        Serial.print(" "); 
    }  
    Serial.println();
}

void ReplayMelody(byte arr[durationsQuantity], byte delayMs){
    bool clickButton = true;
    int j = 0;
    for (int i = 0; i < 50; i++){
        if (arr[i] == 0)
            return;
        if (clickButton)
            tone(piezoPin, frequencyGz);
        else
            noTone(piezoPin);
        while (j < arr[i]){
            delay(delayMs);
            j++;
        }
        clickButton = !clickButton;
        j = 0;                
    }   
}
