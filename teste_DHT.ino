#include <DHT.h>
 
#define DHTPIN A0
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
 
void setup() 
{
  Serial.begin(9600);
  dht.begin(); //Inicializar
}
 
void loop() 
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (isnan(t) || isnan(h)) //Testa se os valores foram lidos corretamente
  {
    Serial.println("Problema ao ler dados do sensor DHT11");
  } 
  else
  {
    Serial.print("Umidade: ");
    Serial.print(h);
    Serial.print(" % ");
    Serial.print("Temperatura: ");
    Serial.print(t);
    Serial.println(" *C");
  }
  delay(2000);
}
