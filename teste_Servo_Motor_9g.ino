#include <Servo.h>
 
#define SERVO 6 // Porta Digital 6 PWM
 
Servo motor; // Variável Servo
int pos; // Posição Servo
 
void setup ()
{
  motor.attach(SERVO);
  Serial.begin(9600);
  motor.write(0); // Inicia motor posição zero (sem erro)
}
 
void loop()
{
  for(pos = 0; pos < 90; pos++)
  {
    motor.write(pos);
    delay(15);
  }

  SetarPosicao(90);
  Serial.println(motor.read());//95
  SetarPosicao(60);
  Serial.println(motor.read());//63
  SetarPosicao(10);
  Serial.println(motor.read());//10

  delay(1000);

}

void SetarPosicao(int pos){
    int posicao_ajustada = floor(pos*(1.0+5.0/90.0));
    motor.write(posicao_ajustada);
  }
