#include <Servo.h>

const byte piezoPin = 3;                        // пин пищалки
const byte buttonPin = 12;                      // пин кнопки
const byte servoPin = 11;                       // пин сервы
const byte delayInMs = 50;                      // задержка между итерациями основного цикла
const unsigned int frequencyGz = 15000;         // частота звучания пищалки
const unsigned int preCodeTime = 3000;          // длительность нажатия кнопки в мс для включения режима записи мелодии
const unsigned int preCodeTolerancePercent = 20;// точность длительности нажатия кнопки для включения режима записи
const unsigned int melodyDurationInMs = 10000;  // время записи мелодии
const byte durationsQuantity = 50;              // наибольшее число пауз и нажатий в мелодии
const byte musicTolerancePercent = 50;          // погрешность воспроизведения интервалов мелодии, в %
const unsigned int signalizeDelayInMs = 500;    // длительность тактов при сигнализировании начала записи мелодии
const byte recordingStart[] = {0, 1, 0, 1, 2};  // последовательность тактов сигнала при начале записи, 2 - окончание массива
const byte success[] = {0, 1, 0, 1, 0, 1, 2};   // последовательность тактов сигнала при верном повторении
const byte failure[] = {0, 1, 1, 1, 2};         // последовательность тактов сигнала при неверном повторении
const byte openDegree = 0;                      // градусов сервы при открытии
const byte closeDegree = 90;                    // градусов сервы при закрывании

Servo servo;

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
bool isSuccess = false;   // успешна ли попытка открывания
bool isOpen = true;       // открыта ли шкатулка

int melodyDurationInCount = -1; // счётчик количества итераций записи мелодии
int preCodeCounter = 1;         // счётчик количества итераций нажатия для включения режима записи
int melodyLengthInCount;        // количество тактов для записи мелодии

void setup() {  
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(piezoPin, OUTPUT);
  servo.attach(servoPin);
  servo.write(openDegree);
                     
  downThreshold = (preCodeTime - preCodeTime / 100 * preCodeTolerancePercent) / delayInMs;
  upThreshold = (preCodeTime + preCodeTime / 100 * preCodeTolerancePercent) / delayInMs;
  melodyLengthInCount = melodyDurationInMs / delayInMs;  
}

void loop() {     
    clicking = digitalRead(buttonPin);
    digitalWrite(LED_BUILTIN, clicking);
  
    if (clicking){                            // если есть нажатие, то обрабатываем его
        tone(piezoPin, frequencyGz); 

        if (!isOpen){
            if (writeMelody){                                                   // если идёт запись мелодии или прослушивание 
                //Serial.println(counter);
                counter++;
                if ((counter != 0) && !prevClick){                              // если идёт счётчик и первый такт нажатия кнопки,
                    if (listenMelody)                                             // то записываем паузу в массив нужный
                        notes[currentDuration] = counter;
                    else
                        durations[currentDuration] = counter;
                    //Serial.print("В массив записана пауза длительностью ");
                    //Serial.println(counter);
                    currentDuration++;
                    counter = 0;
                }
                melodyDurationInCount++;
                if (melodyDurationInCount >= melodyLengthInCount){    // если количество тактов мелодии превысило необходимое количество тактов,
                    if (listenMelody){                                  // то печатаем получившийся массив и повторяем мелодию
                        Serial.println("Прослушивание окончено.");                    
                        isSuccess = ComparisonArrays(durations, notes);             
                        if (isSuccess)
                            Signalize(piezoPin, frequencyGz, success, signalizeDelayInMs); 
                        else
                            Signalize(piezoPin, frequencyGz, failure, signalizeDelayInMs); 
                        // ReplayMelody(notes, delayInMs);
                    }
                    else {
                        Serial.println("Запись мелодии окончена.");                    
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
            servo.write(closeDegree);
            delay(3000);
        } 
        isOpen = !isOpen; 
        if (isOpen)
            Serial.println("Открылась");
        else
            Serial.println("Закрылась");              
    }  
    else{
        noTone(piezoPin); 
        if (writeMelody){                                                     // если идёт запись/прослушивание мелодии
            //Serial.println(counter);          
            if (counter != -1){                                               // если не самое начало слушания мелодии
                counter++;
                if (prevClick){                                               // если кнопка только что отжата, то запись в массив
                    if (listenMelody)
                        notes[currentDuration] = counter;
                    else
                        durations[currentDuration] = counter;
                    //Serial.print("В массив записано нажатие длительностью ");
                    //Serial.println(counter);
                    currentDuration++;
                    counter = 0;
                }            
            }               
            
            melodyDurationInCount++;
            if (melodyDurationInCount >= melodyLengthInCount){        // если конец мелодии, то конец
                if (listenMelody){
                    Serial.println("Прослушивание окончено.");   
                    isSuccess = ComparisonArrays(durations, notes);             
                    if (isSuccess)
                        servo.write(openDegree);
                        // Signalize(piezoPin, frequencyGz, success, signalizeDelayInMs); 
                    else
                        Signalize(piezoPin, frequencyGz, failure, signalizeDelayInMs); 
                    ReplayMelody(notes, delayInMs);
                }
                else {
                    Serial.println("Запись мелодии окончена.");                    
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
                // Serial.println(preCodeCounter * delayInMs);
                writeMelody = true;
                melodyDurationInCount = 0;
                if ((preCodeCounter >= downThreshold) && (preCodeCounter <= upThreshold)){
                    Serial.println("Пошла запись мелодии!");                      
                    Signalize(piezoPin, frequencyGz, recordingStart, signalizeDelayInMs);  
                    ResetArray(durations);               
                }
                else{
                    ResetArray(notes);
                    Serial.println("Пошло слушание мелодии.");  
                    listenMelody = true;                    
                    notes[0] = preCodeCounter;
                    currentDuration = 1;
                    counter = 0;
                }                          
            }      
            preCodeCounter = 0;
            preCode = false;            
        }        
    }

    prevClick = clicking;
    delay(delayInMs);  
}

void Signalize(byte piezoPin, int freq, byte melody[], int delayInMs){  
    int i = 0;
    while(melody[i] != 2){        
        if (melody[i] == 0)
            noTone(piezoPin);
        else
            tone(piezoPin, freq);
        i++;
        delay(delayInMs);
    };   
    noTone(piezoPin); 
}

bool ComparisonArrays(byte first[], byte second[]){
    Serial.print("Началось сравнение массивов. ");
    Serial.print("Допустимое отклонение каждой длительности - ");
    Serial.print(musicTolerancePercent);
    Serial.println("%.");
    PrintArray(durations);
    PrintArray(notes);    
    byte i = 0;
    int tolerance;
    int delta;
    int ostatok;
    while (!((first[i] == 0) || (second[i] == 0))){   
        ostatok = first[i] % 2;
        tolerance = first[i] * musicTolerancePercent / 100 + ostatok;          
        delta = first[i] - second[i];
        Serial.print(delta); 
        Serial.print(" ");     
        if (abs(delta) > tolerance){
            Serial.println();
            Serial.println("Превышен допустимый порог.");          
            return false;  
        }
        i++;                
    }  
    if (first[i] != second[i]){     
        Serial.println();
        Serial.println("Мелодии разной длины."); 
        return false;
    }
    Serial.println("Массивы похожи.");    
    return true;
}

void ResetArray(byte arr[durationsQuantity]){
    for (int i = 0; i < durationsQuantity; i++){
        arr[i] = 0;
    }
}

void PrintArray(byte arr[durationsQuantity]){    
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
