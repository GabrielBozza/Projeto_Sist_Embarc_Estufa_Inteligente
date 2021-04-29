const int ldr_pin = A0;

void setup() {
  
    pinMode(ldr_pin,INPUT);
    Serial.begin(9600);
}

void loop() {
   Serial.print("Luminosidade: ");
   Serial.println( 1023 - analogRead( ldr_pin )); //LDR INVERTIDO NO MODULO COMPRADO, SOH INVERTI O VALOR
   delay(1000);
}
