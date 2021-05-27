#include <Servo.h>
#include <DHT.h>

#define DHTPIN A2 // Pino do sensor de temp e umidade     
#define DHTTYPE DHT11 //DHT11 OU DHT22   
#define SERVO 9 // Porta Digital 9 PWM
#define LDR_POS1 A0 // Sensor de luminosidade na posicao 1 (FRENTE) --> ANALÓGICO
#define LDR_POS2 A1 // Sensor de luminosidade na posicao 2 (FUNDO) --> ANALÓGICO

///****************ARRUMAR NUMMEROS DOSPINOS ANTES DE USAR*************
#define Vermelho_LED2 3
#define Azul_LED2 6
#define Verde_LED2 5
#define Vermelho_LED1 12
#define Azul_LED1 8
#define Vermelho_LED3 4
#define Azul_LED3 7

//LED 1 --> FRENTE
//LED 2 --> MEIO
//LED 3 --> FUNDO

DHT dht(DHTPIN, DHTTYPE); //Inicializa o sensor temp/umid DHT11
Servo motor; // Variável Servo

//FLAGS
boolean MostrarMenuLoop = true; // FLAG PARA DETERMINAR SE VAI IMPRIMIR O MENU OU NAO NO LOOP
boolean UtilizaModuloBluetooth = true;
boolean FlagAjustaMilVars = true;

//--Variáveis globais
int pos=0; // Posição atual do Servo

//--Variaveis para determinar a data, hora e estacao do ano atuais
unsigned long esperaAtualizacao=0,minMil=60000;// mesMil=262800000,diaMil=86400000,horaMil=3600000,minMil=60000; //em milisegundos
unsigned long esperaAtualizacaoVar=0,minMilVar=0;//mesMilVar=262800000,diaMilVar=86400000,horaMilVar=3600000,minMilVar=60000; //em milisegundos
int mes=5,dia=20,hora=5,minuto=58; 
int estacao_ano = 1;// (VERAO,OUTONO,INVERNO,PRIMAVERA)= (0,1,2,3)

int RedLEDSIntensidades[] = {0,0,0};
int GreenLEDSIntensidades[] = {0};
int BlueLEDSIntensidades[] = {0,0,0};

int LDRintensidades[] = {0,0};
int LDRFaixas[] = {0,0};

float DadosDHT[] = {0,0,0}; // {temp,umid,indice_calor}

String modo = "NORM"; //MODO DE OPERACAO NORMAL (PADRAO)
String msg,cmd,valor_cmd,parametros_atuais = "";

 void ImprimirMenu(){
  Serial.println("*************************************");
  Serial.println("* 1)Modo de operação:              *");
  Serial.println("* Normal (padrão)   --> MO#NORM    *");
  Serial.println("* Cresc. acelerado  --> MO#CRES    *");
  Serial.println("* Estimular Floração--> MO#FLOR    *");
  Serial.println("* Dormência         --> MO#DORM    *");
  Serial.println("* 2) Data,hora e hemisfério        *");
  Serial.println("* > DT#dd/mm-hh                    *");
  Serial.println("* Ex: DT#10/05-18:25               *");
  Serial.println("* > Posição motor --> Ex: S1#74    *");
  Serial.println("* > Intensid. LEDs --> Ex: R1#157  *");
  Serial.println("* > Lig./Deslig. LED --> Ex: L2#0  *");
  Serial.println("* > Parâmetros atuais --> AT#0     *");
  Serial.println("************************************");
  }

  
 void ImprimirCondicoesAtuais( float* parametros, int lumin_ldr1, int lumin_ldr2){
  Serial.println("*       Cond. da estufa         *");
  Serial.print(dia);
  Serial.print("/");
  Serial.print(mes);
  Serial.print(" - ");
  Serial.print(hora);
  Serial.print(":");
  Serial.println(minuto);
  Serial.print("Modo: ");
  Serial.println(modo);  
  Serial.print("Umid: ");
  Serial.print(DadosDHT[1]);
  Serial.print(" % ");
  Serial.print("Temp: ");
  Serial.print(DadosDHT[0]);
  Serial.println(" *C");
  Serial.print("LDR1: ");
  Serial.print(lumin_ldr1);
  Serial.print("      LDR2: ");
  Serial.println(lumin_ldr2);
  Serial.print("Abertura: ");
  Serial.println(motor.read());
  Serial.print("LED1: R: ");
  Serial.print(RedLEDSIntensidades[0]);
  Serial.print(" G: 0");
  Serial.print(" B: ");
  Serial.println(BlueLEDSIntensidades[0]);
  Serial.print("LED2: R: ");
  Serial.print(RedLEDSIntensidades[1]);
  Serial.print(" G: ");
  Serial.print(GreenLEDSIntensidades[0]);
  Serial.print(" B: ");
  Serial.println(BlueLEDSIntensidades[1]);
  Serial.print("LED3: R: ");
  Serial.print(RedLEDSIntensidades[2]);
  Serial.print(" G: 0");
  Serial.print(" B: ");
  Serial.println(BlueLEDSIntensidades[2]);
  Serial.println("");
  }

  void SetarPosicaoEscotilha(int abertura){ // abertura --> valor em GRAUS da abertura desejada (-90 a 90 graus) 
 
  //abertura=abertura+90;
  abertura=180-abertura;
  int posicaoAtual = motor.read();
  abertura = floor(abertura*(1.0+5.0/90.0)); //AJUSTA VALOR PARA CONSIDERAR O ERRO DO MOTOR
 
  if(posicaoAtual < abertura ){ // abrir escotilha até que a abertura seja = 'abertura'
    for(pos = posicaoAtual; pos < abertura; pos++)
    {
      motor.write(pos);
      delay(10);// Movimenta a cada 10 ms --> abertura gradual
    }
  }
  else if (posicaoAtual > abertura){ // fechar escotilha até que a abertura seja = 'abertura'
    for(pos = posicaoAtual; pos >= abertura; pos--)
    {
      motor.write(pos);
      delay(10); // Movimenta a cada 10 ms --> fechamento gradual
    }
  }
 }

 float* RetornaParametrosDHT(){

  float temp = dht.readTemperature(); // Temperatura em C
  float umid = dht.readHumidity(); // Umidade

  if (isnan(umid) || isnan(temp)) { // CONFERE DADOS LIDOS
    Serial.println("Erro no sensor DHT");
    static float ParametrosDHT[3] = {-1.0,-1.0,-1.0};
    return ParametrosDHT;
  }
  
  float hic = dht.computeHeatIndex(temp, umid, false);
  static float ParametrosDHT[3] = {temp,umid,hic};
  //ATUALIZA VARS GLOBAIS
  DadosDHT[0] = temp;
  DadosDHT[1] = umid;
  DadosDHT[2] = hic;
  return ParametrosDHT;
 }


int RetornaCondicoesLuminosidade(byte Pino){ 

    int luminosidade = 0;
    if(Pino==A0)luminosidade = (1023 - analogRead(Pino)); // Lê o valor do sensor de luminosidade (analógico) (analog no arduino Uno A0->A5) (lê 10 bits)
    else luminosidade = analogRead(Pino);
 
    if(luminosidade > 450) { // Luminosidade ALTA
      return 2;
    }
    else if(luminosidade > 229 && luminosidade < 451) { // Luminosidade NORMAL
      return 1;
    }
    else { // Luminosidade BAIXA
      return 0;
    }
 }

 void SetarIluminacaoFinal(float* ParametrosCrescimento){

  //float tempIdeal = ParametrosCrescimento[0];
  //float umidadeIdeal = ParametrosCrescimento[1];
  //int R1 = (int) ParametrosCrescimento[2];
  //int G1 = (int) ParametrosCrescimento[3];
  //int B1 = (int) ParametrosCrescimento[4];
  //int R2 = (int) ParametrosCrescimento[5];
  //int B2 = (int) ParametrosCrescimento[6];
  //int R3 = (int) ParametrosCrescimento[7];
  //int B3 = (int) ParametrosCrescimento[8];
  String horario_ligar = String(ParametrosCrescimento[9]);
  int hora_ligar = (int) horario_ligar.substring(0,2).toInt();
  int min_ligar = (int) horario_ligar.substring(3).toInt();
  String horario_desligar = String(ParametrosCrescimento[10]);
  int hora_desligar = (int) horario_desligar.substring(0,2).toInt();
  int min_desligar = (int) horario_desligar.substring(3).toInt();

  //VARIAVEIS GLOBAIS DOS PARAMETROS
  //TEMP = DadosDHT[0]
  //UMID = DadosDHT[1]
  //HIC = DadosDHT[2]
  //LuminosidadePos1 = LDRFaixas[0]
  //LuminosidadePos2 = LDRFaixas[1]
  //ValorLDR1 = LDRintensidades[0]
  //ValorLDR2 = LDRintensidades[1]

  float Temperatura = DadosDHT[0];
  float Umidade = DadosDHT[1];

  //LED DE ANODO COMUM --> TEM QUE INVERTER A INTENSIDADE
  if((hora >= hora_ligar && minuto >= min_ligar) && ((hora < hora_desligar)|| (hora == hora_desligar && minuto < min_desligar))){
    analogWrite(Vermelho_LED2, 255-floor(ParametrosCrescimento[2]*(1.0-(((float)LDRintensidades[0]+(float)LDRintensidades[1])/2046.0))));
    RedLEDSIntensidades[1]=floor(ParametrosCrescimento[2]*(1.0-(((float)LDRintensidades[0]+(float)LDRintensidades[1])/2046.0)));
    analogWrite(Verde_LED2, 255-floor(ParametrosCrescimento[3]*(1.0-(((float)LDRintensidades[0]+(float)LDRintensidades[1])/2046.0))));
    GreenLEDSIntensidades[0]=floor(ParametrosCrescimento[3]*(1.0-(((float)LDRintensidades[0]+(float)LDRintensidades[1])/2046.0)));
    analogWrite(Azul_LED2, 255-floor(ParametrosCrescimento[4]*(1.0-(((float)LDRintensidades[0]+(float)LDRintensidades[1])/2046.0))));
    BlueLEDSIntensidades[1]=floor(ParametrosCrescimento[4]*(1.0-(((float)LDRintensidades[0]+(float)LDRintensidades[1])/2046.0)));
    //analogWrite(Vermelho_LED1, floor(ParametrosCrescimento[5]*(1.0-((float)LDRintensidades[0]/1023.0))));
    digitalWrite(Vermelho_LED1, LOW);
    RedLEDSIntensidades[0]=255;
    //analogWrite(Azul_LED1, floor(ParametrosCrescimento[6]*(1.0-((float)LDRintensidades[0]/1023.0))));
    digitalWrite(Azul_LED1, LOW);
    BlueLEDSIntensidades[0]=255;
    //analogWrite(Vermelho_LED3, floor(ParametrosCrescimento[7]*(1.0-((float)LDRintensidades[1]/1023.0))));
    digitalWrite(Vermelho_LED3, LOW);
    RedLEDSIntensidades[2]=255;
    //analogWrite(Azul_LED3, floor(ParametrosCrescimento[8]*(1.0-((float)LDRintensidades[1]/1023.0))));
    digitalWrite(Azul_LED3, LOW);
    BlueLEDSIntensidades[2]=255;
  }
  else {
    analogWrite(Vermelho_LED2, 255);
    analogWrite(Verde_LED2, 255);
    analogWrite(Azul_LED2, 255);
    //analogWrite(Vermelho_LED1, 0);
    digitalWrite(Vermelho_LED1, HIGH);
    //analogWrite(Azul_LED1, 0);
    digitalWrite(Azul_LED1, HIGH);
    //analogWrite(Vermelho_LED3, 0);
    digitalWrite(Vermelho_LED3, HIGH);
    //analogWrite(Azul_LED3, 0);
    digitalWrite(Azul_LED3, HIGH);
    RedLEDSIntensidades[1]=0;
    GreenLEDSIntensidades[0]=0;
    BlueLEDSIntensidades[1]=0;
    RedLEDSIntensidades[0]=0;
    BlueLEDSIntensidades[0]=0;
    RedLEDSIntensidades[2]=0;
    BlueLEDSIntensidades[2]=0;
  }

  if(Temperatura > ParametrosCrescimento[0]){ //Abrir escotilha
    int Abertura = (int)(Temperatura-ParametrosCrescimento[0])*(15); //15 graus de abertura para cada graus a mais na estufa
    SetarPosicaoEscotilha(Abertura);
  } else if(Umidade > ParametrosCrescimento[1]){// LuminosidadePos1 == 2 && LuminosidadePos1 == 2
    int Abertura = (int)((Umidade-ParametrosCrescimento[1])*(15)); //15 graus de abertura para cada graus a mais na estufa
    SetarPosicaoEscotilha(Abertura);
  }else{
    SetarPosicaoEscotilha(0);// Fechar a escotilha
  }

 }

 float* RetornaParametrosCrescimento(){ // Retorna {temperaturaIdeal,umidadeIdeal,R1,G1,B1,R2,B2,R3,B3,HORA_LIGAR,HORA_DESLIGAR}
  
  static float Parametros[] = {25.0,60.0,255,155,255,255,1,1,255,06.00,18.10}; // VALORES PADRAO CASO NAO CONSIGA DEFINIR

  if(modo == "NORM"){// Perido de crescimento NORMAL -- SEGUE AS ESTACOES DO ANO REAIS

    if(estacao_ano == 0){//Verão no hemisfério Sul (Crescimento)
      static float Parametros[] = {25.0,60.0,255,155,255,255,1,1,255,05.30,18.50}; // Todos os leds ligados
    }else if (estacao_ano == 1){//Outono no hemisfério Sul (Crescimento / Dormência)
      static float Parametros[] = {25.0,60.0,255,155,255,255,1,1,255,06.00,18.10};  // Todos os leds ligados
    } else if(estacao_ano == 2){//Inverno no hemisfério Sul (Dormência)
      static float Parametros[] = {22.0,40.0,255,155,255,255,1,1,255,06.30,17.45}; // Todos os leds ligados
    } else{//Primavera no hemisfério Sul (Floração / Crescimento)
      static float Parametros[] = {28.0,65.0,255,155,255,255,1,1,255,05.50,18.15}; // Todos os leds ligados
    }
    
  }else if(modo == "CRESC"){// Crescimento acelerado -- EQUIVALENTE AO VERAO (TEM PRIORIDADE SOBRE A ESTACAO DO ANO REAL)
    static float Parametros[] = {25.0,60.0,255,155,255,255,1,1,255,05.30,18.50}; // Leds Verdes desligados
  }else if(modo == "FLOR"){// Perido de floracao -- EQUIVALENTE A PRIMAVERA (TEM PRIORIDADE SOBRE A ESTACAO DO ANO REAL)
    static float Parametros[] = {28.0,65.0,255,155,255,255,1,1,255,05.50,18.15}; // Leds Verdes desligados
  }else if(modo == "DORM"){// Perido de dormencia -- EQUIVALENTE AO INVERNO (TEM PRIORIDADE SOBRE A ESTACAO DO ANO REAL)
    static float Parametros[] = {22.0,40.0,255,155,255,255,1,1,255,06.30,17.45}; // Somente Leds Vermelhos ligados
  }
  return Parametros;
 }
 
void setup ()
{
  //INICIALIZAR PINOS DOS LEDS COMO OUTPUT
  pinMode(LDR_POS1,INPUT); //ANALÓGICA
  pinMode(LDR_POS2,INPUT); //ANALÓGICA
  pinMode(Vermelho_LED2, OUTPUT); //DIGITAL PWM
  pinMode(Verde_LED2, OUTPUT); //DIGITAL PWM
  pinMode(Azul_LED2, OUTPUT); //DIGITAL PWM
  pinMode(Vermelho_LED1, OUTPUT); //DIGITAL
  pinMode(Azul_LED1, OUTPUT); //DIGITAL
  pinMode(Vermelho_LED3, OUTPUT); //DIGITAL
  pinMode(Azul_LED3, OUTPUT); //DIGITAL
  motor.attach(SERVO);
  Serial.begin(9600); // Inicia a porta serial, configura a taxa de dados para 9600 bps 
  dht.begin(); // Inicializa sensor DHT11/22
  motor.write(0); // Inicia motor posição zero
}
 
void loop()
{
  //FALTA ENVIAR A DATA E HORA PELO APP
  
  //Atualiza a data e hora
  unsigned long tempoExec = millis(); //Numero de milisegundos desde que comecou a exec o programa
  
  if(tempoExec - minMilVar >= minMil){//MINUTO
    minuto+=1;
    if(minuto==60){
      minuto=0;
      hora+=1;
      if(hora==24){
      hora=0;
      dia+=1;
      if((dia==32 && (mes==1 || mes==3 || mes==5|| mes==7 || mes==8 || mes==10 || mes==12)) || (dia==31 && (mes==4 || mes==6 || mes==9|| mes==11)) || (dia==29 && mes==2)){
        dia=1;
        mes+=1;
        if(mes==13){
          mes=1;
        }
        if(mes==3 && dia==21)estacao_ano = 1; //OUTONO
        if(mes==6 && dia==21)estacao_ano = 2; //INVERNO
        if(mes==9 && dia==21)estacao_ano = 3; //PRIMAVERA
        if(mes==12 && dia==21)estacao_ano = 0; //VERAO
      }
     }  
    }
      minMilVar = tempoExec;
    }  

    if(FlagAjustaMilVars && !UtilizaModuloBluetooth){
      FlagAjustaMilVars = false;
      mes>1?mes-=1:mes=12;
      dia>1?dia-=1:dia=30;
      hora>0?hora-=1:hora=23;
      minuto>0?minuto-=1:minuto=59;
  }  

  //MOSTRAR MENU INICIAL CASO SEJA NECESSARIO
  if(MostrarMenuLoop && !UtilizaModuloBluetooth)ImprimirMenu();
  MostrarMenuLoop = false; //Nao mostrara o menu no proximo loop

  //LENDO DADOS DO AMBIENTE
  int CondicaoLuminosidadeLDR1 = RetornaCondicoesLuminosidade(LDR_POS1); // BAIXA(0),NORMAL(1) OU ALTA(2) 
  int CondicaoLuminosidadeLDR2 = RetornaCondicoesLuminosidade(LDR_POS2); // " "   " "
  LDRFaixas[0] = CondicaoLuminosidadeLDR1;
  LDRFaixas[1] = CondicaoLuminosidadeLDR2;
  int luz_Frente = (1023 - analogRead(LDR_POS1)); //PRECISA INVERTER ESSE
  int luz_Fundo = analogRead(LDR_POS2);
  LDRintensidades[0] = luz_Frente;
  LDRintensidades[1] = luz_Fundo;
  float* ParametrosAmbiente = RetornaParametrosDHT();

  // LER MENSAGEM DO MONITOR SERIAL / APP ANDROID
  if (Serial.available() > 0){ // CHECA SE HÁ DADO DISPONÍVEL PARA SER LIDO
    
    msg += Serial.readString(); // LÊ A MENSAGEM TODA COMO UMA ÚNICA STRING --> ADD NA FILA (FIFO) DE COMANDOS
    
    do{//O BUFFER ACABA ENVIANDO VARIOS COMANDOS CONCATENADOS --> SEPARAR E ITERAR CADA UM

      cmd = msg.substring(0,2);//<S1-Servo1> , <R1-RED/LED1> , <L1-LED1> , <MO-Modo de Operacao>

      esperaAtualizacao = 600000; // SETA TIMER DE 1O MIN PARA CONTINUAR NO ESTADO SETADO PELO COMANDO
      esperaAtualizacaoVar=tempoExec;
      
      if(cmd == "S1"){ //SERVO MOTOR1
         valor_cmd = msg.substring(3,5); //EX msg = S1#75 --> valor_cmd = 75
         SetarPosicaoEscotilha(valor_cmd.toInt());
         msg=msg.substring(5); //LIMPA O ESSE COMANDO DO BUFFER --> FILA FIFO
      }else if(cmd == "R1"){//INTENSIDADE DO VERMELHO NO LED1
         valor_cmd = msg.substring(3,6); //EX msg = R1#099 --> valor_cmd = 99
         analogWrite(Vermelho_LED2, 255-valor_cmd.toInt());
         RedLEDSIntensidades[1]=valor_cmd.toInt();
         msg=msg.substring(6);
      }else if(cmd == "G1"){
         valor_cmd = msg.substring(3,6); 
         analogWrite(Verde_LED2, 255-valor_cmd.toInt());
         GreenLEDSIntensidades[0]=valor_cmd.toInt();
         msg=msg.substring(6);
      }else if(cmd == "B1"){
         valor_cmd = msg.substring(3,6);
         analogWrite(Azul_LED2, 255-valor_cmd.toInt());
         BlueLEDSIntensidades[1]=valor_cmd.toInt();
         msg=msg.substring(6);
      }else if(cmd == "R2"){
         valor_cmd = msg.substring(3,6);
         //analogWrite(Vermelho_LED1, valor_cmd.toInt());
         digitalWrite(Vermelho_LED1, valor_cmd.toInt()>0?LOW:HIGH);
         RedLEDSIntensidades[0]=valor_cmd.toInt()>0?255:0;
         msg=msg.substring(6);
      }else if(cmd == "B2"){
         valor_cmd = msg.substring(3,6);
         digitalWrite(Azul_LED1, valor_cmd.toInt()==0?HIGH:LOW);
         BlueLEDSIntensidades[0]=valor_cmd.toInt();
         msg=msg.substring(6);
      }else if(cmd == "R3"){
         valor_cmd = msg.substring(3,6);
         digitalWrite(Vermelho_LED3, valor_cmd.toInt()==0?HIGH:LOW);
         RedLEDSIntensidades[2]=valor_cmd.toInt();
         msg=msg.substring(6);
      }else if(cmd == "B3"){
         valor_cmd = msg.substring(3,6);
         //analogWrite(Azul_LED3, valor_cmd.toInt());
         digitalWrite(Azul_LED3, valor_cmd.toInt()>0?LOW:HIGH);
         BlueLEDSIntensidades[2]=valor_cmd.toInt()>0?255:0;
         msg=msg.substring(6);
      }else if(cmd == "L1"){//LIGAR/DESLIGAR LED1
         valor_cmd = msg.substring(3,4); //EX msg = L1#0 --> valor_cmd = 0
         analogWrite(Vermelho_LED2, valor_cmd=="0"?255:0);
         analogWrite(Verde_LED2, valor_cmd=="0"?255:0);
         analogWrite(Azul_LED2, valor_cmd=="0"?255:0);
         RedLEDSIntensidades[1]=(valor_cmd=="0"?0:255);
         GreenLEDSIntensidades[0]=(valor_cmd=="0"?0:255);
         BlueLEDSIntensidades[1]=(valor_cmd=="0"?0:255);
         msg=msg.substring(4);
      }else if(cmd == "L2"){
         valor_cmd = msg.substring(3,4);
         analogWrite(Vermelho_LED1, valor_cmd=="0"?255:0);
         digitalWrite(Azul_LED1, valor_cmd=="0"?HIGH:LOW);
         RedLEDSIntensidades[0]=(valor_cmd=="0"?0:255);
         BlueLEDSIntensidades[0]=(valor_cmd=="0"?0:255);
         msg=msg.substring(4);
      }else if(cmd == "L3"){
         valor_cmd = msg.substring(3,4);
         digitalWrite(Vermelho_LED3, valor_cmd=="0"?HIGH:LOW);
         analogWrite(Azul_LED3, valor_cmd=="0"?255:0);
         RedLEDSIntensidades[2]=(valor_cmd=="0"?0:255);
         BlueLEDSIntensidades[2]=(valor_cmd=="0"?0:255);
         msg=msg.substring(4);
      }else if(cmd == "MO"){//SETAR MODO DE OPERACAO
         valor_cmd = msg.substring(3,7); //EX msg = MO#NORM --> valor_cmd = NORM
         modo = valor_cmd;
         esperaAtualizacao = 0; // ZERA O TIMER (NÃO PRECISA PARA ESTE COMANDO) 
         esperaAtualizacaoVar = 0;
         msg=msg.substring(7);
      }else if(cmd == "DT"){//SETAR DATA ATUAL dd/mm-hh:mm
         valor_cmd = msg.substring(3,14); //EX msg = DT#01/05-15:33 --> valor_cmd = 01/05-15:33
         esperaAtualizacao = 0; // ZERA O TIMER (NÃO PRECISA PARA ESTE COMANDO) 
         esperaAtualizacaoVar = 0;
         dia = valor_cmd.substring(0,2).toInt();
         mes = valor_cmd.substring(3,5).toInt();
         hora = valor_cmd.substring(6,8).toInt();
         minuto = valor_cmd.substring(9).toInt();
         msg=msg.substring(14);
      }else if(cmd == "AT" && !UtilizaModuloBluetooth){//IMPRIMIR CONDICOES AMBIENTAIS ATUALIZADAS NO MONITOR SERIAL (SOMENTE SE NÃO ESTIVER USANDO BLUETOOTH)
         //valor_cmd = msg.substring(3,4); //EX msg = AT#0
         ImprimirCondicoesAtuais(ParametrosAmbiente,luz_Frente,luz_Fundo);
         esperaAtualizacao = 0; // ZERA O TIMER (NÃO PRECISA PARA ESTE COMANDO) 
         esperaAtualizacaoVar = 0;
         msg=msg.substring(4);
      }
      else{ //Limpar o buffer pq perdeu algum dado
        if (!UtilizaModuloBluetooth){
          Serial.println("Comando inexistente!");
          MostrarMenuLoop = true;//Chamar menu no prox loop
        }
        msg="";
        esperaAtualizacao = 0; // ZERA O TIMER (NÃO PRECISA PARA ESTE COMANDO) 
        esperaAtualizacaoVar = 0;
      }
    } while(msg.length()>2); //TEM Q EXEC TODOS OS COMANDOS ANTES DE CONTINUAR
    msg="";
  }

  if(tempoExec - esperaAtualizacaoVar >= esperaAtualizacao){
      esperaAtualizacaoVar=0;
      esperaAtualizacao=0;
      //Modifica o ambiente de acordo com os dados lidos
      float* ParametrosCrescimento = RetornaParametrosCrescimento();
      SetarIluminacaoFinal(ParametrosCrescimento);
    }

  if(UtilizaModuloBluetooth){ //Envia os dados atuais para o app
    parametros_atuais=(String(ParametrosAmbiente[0])+'#'+String(ParametrosAmbiente[1])+'#'+String(luz_Frente)+'#'+String(luz_Fundo)+
    '#'+String(RedLEDSIntensidades[1])+'#'+String(GreenLEDSIntensidades[0])+'#'+String(BlueLEDSIntensidades[1])+'#'+
    String(RedLEDSIntensidades[0])+'#'+String(BlueLEDSIntensidades[0])+'#'+String(RedLEDSIntensidades[2])+'#'
    +String(BlueLEDSIntensidades[2])+'#'+modo+'#'+String(motor.read())+'#'+String(dia)+'#'+String(mes)+'#'+String(hora)+'#'+String(minuto)+'#'+String(estacao_ano));
    Serial.println(parametros_atuais); // Envia mensagem para aplcativo android
    parametros_atuais=" ";
    }
    
  delay(100);// Para nao ficar analisando com tanta frequencia
}
