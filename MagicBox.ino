volatile int buttonState = 0;

void setup() {    
    pinMode(2, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    attachInterrupt(0, Click, RISING);
}

void loop() {
         
}

void Click(){
    buttonState = digitalRead(2);
    digitalWrite(LED_BUILTIN, buttonState);   
}
