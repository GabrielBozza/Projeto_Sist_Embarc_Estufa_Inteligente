#include <DHT.h>
#include <Servo.h>
 
#define SERVO 6 // Porta Digital 6 PWM
#define DHTPIN A2 // pino que estamos conectado
#define DHTTYPE DHT11 // DHT 11
#define ldr_pin1 A0
#define Vermelho_LED2 11
#define Azul_LED2 10
#define Verde_LED2 9
#define Vermelho_LED1 3
#define Azul_LED3 5

DHT dht(DHTPIN, DHTTYPE);
Servo motor; // Variável Servo
int pos; // Posição Servo
//int luminosidade;
//int CondicaoCrescimento=0;
//unsigned long Mes; //Guarda o valor do mes atual para determinar parametros ambientais da estufa de acordo com as estacoes do ano
//unsigned long OffsetMesAtual = 3; //Mes atual ao ligar o microcontrolador 

void setup() {
  pinMode(ldr_pin1,INPUT); //ANALÓGICA
  pinMode(Vermelho_LED2, OUTPUT); //DIGITAL PWM
  pinMode(Verde_LED2, OUTPUT); //DIGITAL PWM
  pinMode(Azul_LED2, OUTPUT); //DIGITAL PWM
  //pinMode(Vermelho_LED1, OUTPUT); //DIGITAL PWM
  //pinMode(Azul_LED3, OUTPUT); //DIGITAL PWM
  motor.attach(SERVO); //DIGITAL PWM
  Serial.begin(9600);
  motor.write(0); // Inicia motor posição zero
  dht.begin(); //ANALÓGICA
}

void loop() {

  int posicao_motor=0;
  double luminosidade_ldr1 = ( 1023 - analogRead( ldr_pin1 ));
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(t) || isnan(h)) 
  {
    Serial.println("Erro ao ler dados do sensor DHT11");
  } 
  else
  {
    Serial.print("Umidade: ");
    Serial.print(h);
    Serial.print(" % ");
    Serial.print("Temperatura: ");
    Serial.print(t);
    Serial.println(" *C");
    Serial.print("Luminosidade LDR1: ");
    Serial.println(luminosidade_ldr1);
    
  }

  if(luminosidade_ldr1 < 500 && t<30)//****CONSIDERAR A HORA DO DIA TBM
  {

    analogWrite(Vermelho_LED2, floor(255.0*(1.0-(luminosidade_ldr1/1023.0))));
    analogWrite(Verde_LED2, floor(55.0*(1.0-(luminosidade_ldr1/1023.0))));
    analogWrite(Azul_LED2, floor(255.0*(1.0-(luminosidade_ldr1/1023.0))));

    //analogWrite(Vermelho_LED1, 255*floor(1.0-(luminosidade_ldr1/1023.0)));
    //analogWrite(Azul_LED3, 255*floor(1.0-(luminosidade_ldr1/1023.0)));
  
    if(h>80)
    {
      SetarPosicaoEscotilha((h-80)*4.33);
      posicao_motor = (h-80)*4.33;
    }
    else
    {
      SetarPosicaoEscotilha(0);
      posicao_motor = 0;
     }
  }
  else if(luminosidade_ldr1 >= 500 || t>28){
    //DIMINUIR INTENSIDADE LEDS
    analogWrite(Vermelho_LED2, floor(255.0*(1.0-(luminosidade_ldr1/1023.0))));
    analogWrite(Verde_LED2, floor(25.0*(1.0-(luminosidade_ldr1/1023.0))));
    analogWrite(Azul_LED2, floor(155.0*(1.0-(luminosidade_ldr1/1023.0))));

    //analogWrite(Vermelho_LED1, 255*floor(1.0-(luminosidade_ldr1/1023.0)));
    //analogWrite(Azul_LED3, 255*floor(1.0-(luminosidade_ldr1/1023.0)));
    
    motor.write((t-25)*20);
    SetarPosicaoEscotilha((t-25)*20);
    posicao_motor = (t-25)*20;
    }

  Serial.print("Posicao escotilha: ");
  Serial.println(posicao_motor);
  Serial.print("Intensidades LEDs: R: ");
  Serial.print(floor(255.0*(1.0-(luminosidade_ldr1/1023.0))));
  Serial.print(" G: ");
  Serial.print(floor(25.0*(1.0-(luminosidade_ldr1/1023.0))));
  Serial.print(" B: ");
  Serial.println(floor(155.0*(1.0-(luminosidade_ldr1/1023.0))));
  Serial.println("********************************************************************");
  delay(2000);

}

/*void DeterminarMesAtual(){
  unsigned long MilisegDesdeInicio;
  unsigned long MilisegMes = 2628000000; //30.416*24*60*60*1000 --> Média de dias de um mês = 365/12 (30.416)
  MilisegDesdeInicio = millis(); // Função da biblioteca padrão
  Mes = (MilisegDesdeInicio/MilisegMes)+1+OffsetMesAtual; //Atualiza o valor da variável global que guarda o valor do mes atual  
}*/

void SetarPosicaoEscotilha(int abertura){
  // abertura --> valor em GRAUS da abertura desejada (-90 a 90 graus) 

  int posicaoAtual = motor.read();
  //Serial.print("Posicao atual");
  //Serial.println(posicaoAtual);
  
  
  if(posicaoAtual < abertura ){ // abrir escotilha até que a abertura seja = 'abertura'
    for(pos = posicaoAtual; pos < abertura; pos++)
    {
      motor.write(pos);
      delay(25);// Movimenta a cada 25 ms --> abertura gradual
    }
  }
  else if (posicaoAtual > abertura){ // fechar escotilha até que a abertura seja = 'abertura'
    for(pos = posicaoAtual; pos >= abertura; pos--)
    {
      motor.write(pos);
      delay(25); // Movimenta a cada 25 ms --> fechamento gradual
    }
  }
 }

 
 /*float* RetornaParametrosDHT(){

  float t = dht.readTemperature(); // Temperatura em C
  float h = dht.readHumidity(); // Umidade

  if (isnan(h) || isnan(t)) {
    Serial.println("Erro ao ler dados do sensor DHT no pino " + DHTPIN);
    static float Parametros[3] = {-1.0,-1.0,-1.0};
    return Parametros;
  }
  
  float hic = dht.computeHeatIndex(t, h, false);

  static float Parametros[3] = {t,h,hic};
  return Parametros;
 }*/

  /*int RetornaCondicoesLuminosidade(byte Pino){ // Ex: byte Pino = 0xA0
    
    luminosidade = analogRead(Pino); // Lê o valor do sensor de luminosidade (analógico) (analog no arduino Uno A0->A5) (lê 10 bits)
 
    if(luminosidade > 450) { // Luminosidade alta
      Serial.println("Luminosidade ALTA: " + luminosidade);
      return 2;
    }
    else if(luminosidade > 229 && luminosidade < 451) { // Luminosidade normal
      Serial.println("Luminosidade NORMAL: " + luminosidade);
      return 1;
    }
    else { // Luminosidade baixa
      Serial.println("Luminosidade BAIXA: " + luminosidade);
      return 0;
    }
 }*/
