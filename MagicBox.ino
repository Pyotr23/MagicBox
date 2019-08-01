
const byte piezoPin = 3;                        // пин пищалки
const byte buttonPin = 12;                      // пин кнопки
const byte delayInMs = 50;                      // задержка между итерациями основного цикла
const unsigned int frequencyGz = 15000;         // частота звучания пищалки
const unsigned int preCodeTime = 3000;          // длительность нажатия кнопки в мс для включения режима записи мелодии
const unsigned int preCodeTolerancePercent = 20;// точность длительности нажатия кнопки для включения режима записи
const unsigned int melodyDurationInMs = 10000;  // время записи мелодии
const byte durationsQuantity = 50;              // наибольшее число пауз и нажатий в мелодии
const byte musicTolerancePercent = 50;          // погрешность воспроизведения интервалов мелодии, в %

// int firstDuration;                  // длительность нажатия для включения режима записи в мс
int downThreshold;                  // верхняя граница допустимой длительности нажатия кнопки для включения режима записи в мс
int upThreshold;                    // нижняя граница допустимой длительности нажатия кнопки для включения режима записи в мс
int counter = -1;                   // счётчик длительности паузы/нажатия
byte durations[durationsQuantity];  // массив длительностей нажатий/пауз мелодии
byte notes[durationsQuantity];      // воспроизведённая мелодия
byte currentDuration = 0;           // индекс текущей длительности в массиве длительностей

bool clicking = false;    // есть ли нажатие на данной итерации
bool prevClick = false;   // было ли нажатие на предыдущей итерации
bool preCode = false;     // старт вычисления первого нажатия
bool writeMelody = false; // идёт ли запись/прослушивание мелодии
bool listenMelody = false;// идёт ли прослушивание мелодии

int melodyDurationInCount = -1; // счётчик количества итераций записи мелодии
int preCodeCounter = 1;         // счётчик количества итераций нажатия для включения режима записи
int melodyLengthInCount;        // количество тактов для записи мелодии

void setup() {  
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(piezoPin, OUTPUT);
                     
  downThreshold = (preCodeTime - preCodeTime / 100 * preCodeTolerancePercent) / delayInMs;
  upThreshold = (preCodeTime + preCodeTime / 100 * preCodeTolerancePercent) / delayInMs;
  melodyLengthInCount = melodyDurationInMs / delayInMs;
  PrintArray(durations);
}

void loop() {     
    clicking = digitalRead(buttonPin);
    digitalWrite(LED_BUILTIN, clicking);
  
    if (clicking){                            // если есть нажатие, то обрабатываем его
        tone(piezoPin, frequencyGz); 

        if (writeMelody){                                                   // если идёт запись мелодии или прослушивание 
            Serial.println(counter);
            counter++;
            if ((counter != 0) && !prevClick){                              // если идёт счётчик и первый такт нажатия кнопки,
              if (listenMelody)                                             // то записываем паузу в массив нужный
                    notes[currentDuration] = counter;
                else
                    durations[currentDuration] = counter;
                Serial.print("В массив записана пауза длительностью ");
                Serial.println(counter);
                currentDuration++;
                counter = 0;
            }
            melodyDurationInCount++;
            if (melodyDurationInCount >= melodyLengthInCount){    // если количество тактов мелодии превысило необходимое количество тактов,
              if (listenMelody){                                  // то печатаем получившийся массив и повторяем мелодию
                    Serial.println("Прослушивание окончено.");
                    PrintArray(notes);
                    ReplayMelody(notes, delayInMs);
                }
                else {
                    Serial.println("Запись мелодии окончена.");
                    PrintArray(durations);
                    ReplayMelody(durations, delayInMs);
                }                  
                currentDuration = 0;
                counter = -1;
                writeMelody = false;
                listenMelody = false;
            }                            
        }
        else{
            if (!preCode)                           // начинаем обсчитывать первое нажатие, если его не было 
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
        if (writeMelody){                                                     // если идёт запись/прослушивание мелодии
            Serial.println(counter);          
            if (counter != -1){                                               // если не самое начало слушания мелодии
                counter++;
                if (prevClick){                                               // если кнопка только что отжата, то запись в массив
                    if (listenMelody)
                        notes[currentDuration] = counter;
                    else
                        durations[currentDuration] = counter;
                    Serial.print("В массив записано нажатие длительностью ");
                    Serial.println(counter);
                    currentDuration++;
                    counter = 0;
                }            
            }               
            
            melodyDurationInCount++;
            if (melodyDurationInCount >= melodyLengthInCount){        // если конец мелодии, то конец
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
                currentDuration = 0;
                counter = -1;
                writeMelody = false;
                listenMelody = false;
            }
        }
        else{
            if (preCode){                           // если первое нажатие закончено, то обсчитываем его                
                Serial.println(preCodeCounter * delayInMs);
                writeMelody = true;
                melodyDurationInCount = 0;
                if ((preCodeCounter >= downThreshold) && (preCodeCounter <= upThreshold)){
                    Serial.println("Пошла запись мелодии!");     
                    ResetArray(durations);               
                }
                else{
                    ResetArray(notes);
                    Serial.print("Пошло слушание мелодии. Записано первым значение ");  
                    listenMelody = true;
                    Serial.println(preCodeCounter);
                    notes[0] = preCodeCounter;
                    currentDuration = 1;
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

bool ComparisonArrays(byte first[], byte second[]){
    bool result = true;
    byte i = 0;
    int tolerance;
    int delta;
    while ((first[i] != 0) && (second[i] != 0)){
        tolerance = first[i] * musicTolerancePercent / 100 + 1;
        delta = first[i] - second[i];
        if (abs(delta) > tolerance){
            result = false;
            return;  
        }
        i++;                
    }  
    return result;
}

void ResetArray(byte arr[durationsQuantity]){
    for (int i = 0; i < durationsQuantity; i++){
        arr[i] = 0;
    }
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
