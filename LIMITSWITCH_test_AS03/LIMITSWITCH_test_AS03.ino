void setup() {
  pinMode(6,INPUT_PULLUP);//pb
  pinMode(5,INPUT_PULLUP);
  pinMode(9,INPUT_PULLUP);//limit
  pinMode(8,INPUT_PULLUP);//cloth
  Serial.begin(9600);

}

void loop() {
  
  


  int a = digitalRead(5);
  Serial.print("EMG");Serial.println(a);
  int b = digitalRead(6);
  Serial.print("PB");Serial.println(b);

  int sen = digitalRead(8);
  Serial.print("Sensor");Serial.println(sen);
  int limit = digitalRead(9);
  Serial.print("LIMIT=");Serial.println(limit);
}
