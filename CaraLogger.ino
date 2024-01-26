
#define ON   LOW
#define OFF  HIGH

#define ignition_out 10

void setup() {
  pinMode(ignition_out, OUTPUT);
}

void Ignition(bool state)
{
   digitalWrite(relay, state);
}




void loop() {
    Ignition(ON);
    delay(1000);  
    Ignition(OFF);
     delay(1000);  
}
