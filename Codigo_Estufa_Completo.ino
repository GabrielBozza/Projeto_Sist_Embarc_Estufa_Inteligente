#include <Servo.h>
#include <DHT.h>

#define DHTPIN 2 // Pino do sensor de temp e umidade     
#define DHTTYPE DHT11 //DHT11 OU DHT22   
#define SERVO 6 // Porta Digital 6 PWM
#define LDR_POS1 A0 // Sensor de luminosidade na posicao 1 (a definir) --> ANALÓGICO
#define LDR_POS2 A1 // Sensor de luminosidade na posicao 1 (a definir) --> ANALÓGICO

DHT dht(DHTPIN, DHTTYPE); //Inicializa o sensor temp/umid DHT11
Servo motor; // Variável Servo

//Declaração das funções
float* RetornaParametrosCrescimento(int CondicaoCrescimento);
void LigarConjuntoLeds(int conjLeds[]);
void SetarPosicaoEscotilha(int abertura);
void SetarIluminacaoFinal(int LuminosidadePos1, int LuminosidadePos2, float Temperatura, float Umidade,
 float IndiceCalor,float* ParametrosCrescimento,void* SetarPosicaoEscotilha(int abertura));

//Variáveis globais
int pos; // Posição Servo
int luminosidade;
int CondicaoCrescimento=0;
int RedLEDS[] = {11,12,13}; //Portas digitais dos LEDS Vermelhos
int GreenLEDS[] = {8,9,10}; //Portas digitais dos LEDS Verdes
int BlueLEDS[] = {5,6,7}; //Portas digitais dos LEDS Azuis
unsigned long Mes; //Guarda o valor do mes atual para determinar parametros ambientais da estufa de acordo com as estacoes do ano
unsigned long OffsetMesAtual = 3; //Mes atual ao ligar o microcontrolador 
 
void setup ()
{
  motor.attach(SERVO);
  Serial.begin(9600); // Inicia a porta serial, configura a taxa de dados para 9600 bps 
  dht.begin(); // Inicializa sensor DHT11/22
  motor.write(0); // Inicia motor posição zero
}
 
void loop()
{
  int CondicaoLuminosidadeLDR1 = RetornaCondicoesLuminosidade(LDR_POS1);
  int CondicaoLuminosidadeLDR2 = RetornaCondicoesLuminosidade(LDR_POS2);
  float* ParametrosAmbiente = RetornaParametrosDHT();
  //int CondicaoCrescimento = DeterminarCondicoesCrescimento();
  //CondicaoCrescimento = 1; //Escolhido pelo usuario
  Serial.print("Condição de crescimento atual: ");
  PrintarCondicaoCrescimento();
  Serial.println("Digite a nova condição de crescimento caso deseje mudar:");
  if (Serial.available() > 0) {
    CondicaoCrescimento = Serial.read(); //Usuário escolhe mudar ou não a condição de crescimento das plantas
    Serial.print("Condição de crescimento selecionada: ");
    PrintarCondicaoCrescimento();
    //Serial.println(CondicaoCrescimento, DEC);
  }
  delay(3000); //Espera usuário digitar
  DeterminarMesAtual();
  float* ParametrosCrescimento = RetornaParametrosCrescimento(CondicaoCrescimento);
  SetarIluminacaoFinal(CondicaoLuminosidadeLDR1, CondicaoLuminosidadeLDR2, ParametrosAmbiente[0], ParametrosAmbiente[1],ParametrosAmbiente[2], ParametrosCrescimento);
  Serial.println("Dados atualizados");
  delay(2000);// Para nao ficar analisando com tanta frequencia
}

void DeterminarMesAtual(){
  unsigned long MilisegDesdeInicio;
  unsigned long MilisegMes = 2628000000; //30.416*24*60*60*1000 --> Média de dias de um mês = 365/12 (30.416)
  MilisegDesdeInicio = millis(); // Função da biblioteca padrão
  Mes = (MilisegDesdeInicio/MilisegMes)+1+OffsetMesAtual; //Atualiza o valor da variável global que guarda o valor do mes atual  
}

void SetarPosicaoEscotilha(int abertura){
  // abertura --> valor em GRAUS da abertura desejada (-90 a 90 graus) 

  int posicaoAtual = motor.read();
  
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

 void PrintarCondicaoCrescimento(){

  switch(CondicaoCrescimento){
    default:
    case 0:
      Serial.println("Condição de crescimento PADRÃO");
      break;
    case 1:
      Serial.println("Condição de crescimento PARA FLORAÇÃO");
      break;
    case 2:
      Serial.println("Condição de crescimento PARA DORMÊNCIA");
      break;  
  }
  }

 void DeterminarCondicoesCrescimento(int condicao){// Modificar esta funcao para receber o valor pela COM
  
  if(condicao == 0){// Perido de crescimento
    
    Serial.println("Modo de Crescimento selecionado");
    
  }else if(condicao == 1){// Perido de floracao
    
    Serial.println("Modo de Floração selecionado");
    
  }else if(condicao == 2){// Perido de dormencia
    
    Serial.println("Modo de Dormência selecionado");
    
  }
  else {// Crescimento padrao
    
    Serial.println("Modo Padrão selecionado");
    
  }
 }

 void MostrarCondicoesTemperaturaUmidade(){

  float h = dht.readHumidity(); // Umidade
  float t = dht.readTemperature(); // Temperatura em C
  float f = dht.readTemperature(true); // Temperatura em F

  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Erro ao ler dados do sensor DHT no pino " + DHTPIN);
    return;
  }

  float hif = dht.computeHeatIndex(f, h);
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print("Umidade: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperatura: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Índice de calor: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");

  return;
 }


 float* RetornaParametrosDHT(){

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
 }


 int RetornaCondicoesLuminosidade(byte Pino){ // Ex: byte Pino = 0xA0
    
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
 }

 void SetarIluminacaoFinal(int LuminosidadePos1, int LuminosidadePos2, float Temperatura, float Umidade,
 float IndiceCalor,float* ParametrosCrescimento){

  float tempIdeal = ParametrosCrescimento[0];
  float umidadeIdeal = ParametrosCrescimento[1];
  int LEDSVermelhos = (int) ParametrosCrescimento[2];
  int LEDSVerdes = (int) ParametrosCrescimento[3];
  int LEDSAzuis = (int) ParametrosCrescimento[4];

  if(LuminosidadePos1 == 0 || LuminosidadePos1 == 1 || LuminosidadePos2 == 0 || LuminosidadePos2 == 1){ //Exemplo de uma possível condicao e seu resultado no sistema
    
    LigarConjuntoLeds(RedLEDS);
    LigarConjuntoLeds(GreenLEDS);
    LigarConjuntoLeds(BlueLEDS);
    
    if(Temperatura > tempIdeal){ //Abrir escotilha
      
      int Abertura = (int)(Temperatura-tempIdeal)*(15); //15 graus de abertura para cada graus a mais na estufa
      SetarPosicaoEscotilha(Abertura);
      
    }
  } else if(Umidade > umidadeIdeal){// LuminosidadePos1 == 2 && LuminosidadePos1 == 2
    
    int Abertura = (int)((Umidade-umidadeIdeal)*(15)); //15 graus de abertura para cada graus a mais na estufa
    SetarPosicaoEscotilha(Abertura);
    
  }else{
    
    SetarPosicaoEscotilha(0);// Fechar a escotilha
    
  }
 }

 float* RetornaParametrosCrescimento(int CondicaoCrescimento){ // Retorna {temperaturaIdeal,umidadeIdeal,LedsVermelhos,LedsVerdes,LedsAzuis}
    
  if(CondicaoCrescimento == 0){// Perido de crescimento
    
    static float Parametros[] = {25.0,60.0,1,1,1}; // Todos os leds ligados
    return Parametros;
    
  }else if(CondicaoCrescimento == 1){// Perido de floracao
    
    static float Parametros[] = {28.0,65.0,1,0,1}; // Leds Verdes desligados
    return Parametros;
    
  }else if(CondicaoCrescimento == 2){// Perido de dormencia
    
    static float Parametros[] = {22.0,40.0,1,0,0}; // Somente Leds Vermelhos ligados
    return Parametros;
    
  }
  else {// Crescimento padrao
    
    if(Mes<4){ //Verão no hemisfério Sul (Crescimento)
      static float Parametros[] = {25.0,60.0,1,1,1}; // Todos os leds ligados
      return Parametros;
    }
    else if(Mes>=4 && Mes<7){ //Outono no hemisfério Sul (Crescimento / Dormência)
      static float Parametros[] = {25.0,60.0,1,1,1}; // Todos os leds ligados
      return Parametros;
    }
    else if(Mes>=7 && Mes<10){ //Inverno no hemisfério Sul (Dormência)
      static float Parametros[] = {22.0,40.0,1,0,0}; // Somente Leds Vermelhos ligados
      return Parametros;
    }
    else{ //Primavera no hemisfério Sul (Floração / Crescimento)
      static float Parametros[] = {28.0,65.0,1,0,1}; // Leds Verdes desligados
      return Parametros;
    }
  }

 }

  void LigarConjuntoLeds (int ConjuntoLED[]){
    digitalWrite(ConjuntoLED[1],HIGH);
    digitalWrite(ConjuntoLED[2],HIGH);
    digitalWrite(ConjuntoLED[3],HIGH);
 }

   void DesligarConjuntoLeds (int ConjuntoLED[]){
    digitalWrite(ConjuntoLED[1],LOW);
    digitalWrite(ConjuntoLED[2],LOW);
    digitalWrite(ConjuntoLED[3],LOW);
 }
 
