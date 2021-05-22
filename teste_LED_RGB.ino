int vermelhoPin = 11;
int verdePin = 5;
int azulPin = 3;

void setup()
{
  pinMode(vermelhoPin, OUTPUT);
  pinMode(verdePin, OUTPUT);
  pinMode(azulPin, OUTPUT);  
}

void loop()
{
  setColor(255, 0, 0);  // vermelho
  delay(1000);
  setColor(0, 255, 0);  // verde
  delay(1000);
  setColor(0, 0, 255);  // azul
  delay(1000);
  setColor(255, 255, 0);  // amarelo
  delay(1000);  
  setColor(80, 0, 80);  // roxo
  delay(1000);
}

void setCor(int vermelho, int verde, int azul)
{
  //TEM QUE INVERTER OS VALORES PQ ESTAMOS USANDO LEDS DE ANODO COMUM
  vermelho = 255 - vermelho;
  verde = 255 - verde;
  azul = 255 - azul;
  analogWrite(vermelhoPin, vermelho);
  analogWrite(verdePin, verde);
  analogWrite(azulPin, azul);  
}
