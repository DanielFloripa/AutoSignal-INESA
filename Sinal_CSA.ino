#include "Horarios.h"
#include <EtherCard.h>
#include <Wire.h>
#include <DS1307new.h>
#define RELE 9

/**************************************************************************************************/
/*******************************  V  A  R  I  A  V  E  I  S  **************************************/
/**************************************************************************************************/
//char dateTime[20], date[10], time[10];
static uint8_t diasPorMes[] = {
  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
int RTC[7], ntp_ano, ntp_mes, ntp_dia, ntp_hora, ntp_minuto, ntp_segundo;
unsigned long lastTime = 0;
static byte srcPort = 0;
uint32_t timeStamp; //= 3601504760; // <- apenas um exemplo de timestamp
bool requestSent;
byte Ethernet::buffer[700];

/**************************************************************************************************/
/***************************  C  O  N  F  I  G  U  R  A  V  E  L  *********************************/
/**************************************************************************************************/
static byte mymac[] = {0x01, 0x1A, 0x4B, 0x38, 0x0C, 0x5C};
static byte mymask[] = {0xff, 0xff, 0xfa, 0x00};
static byte ntpServer[] = {0xc1, 0xcc, 0x72, 0xe8};// csa = 193, 204, 114, 232//  outro = {200,160,7,186};
/*****************************************************************************************************/

void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando...");
  ether.begin(sizeof Ethernet::buffer, mymac, 10);
  ether.dhcpSetup();

  /* Descomente para debuger das configuracoes, 
   * imprime informacoes na Serial */
  
  /*if (!ether.begin(sizeof Ethernet::buffer, mymac, 10))
   Serial.println("Falha ao acessar controlador Ethernet!!!");
   else
   Serial.println("Controlador Ethernet inicializado.");
   if (!ether.dhcpSetup())
   Serial.println("Falha ao obter configuracao do DHCP!!!");
   else
   Serial.println("Configuracao DHCP completa!");
   ether.printIp("Endereco IP:\t", ether.myip);
   ether.printIp("NetMask: \t", ether.mymask);
   ether.printIp("Gateway: \t", ether.gwip);*/
  requestSent = false;
  DS1307.begin();
  DS1307.setDate(13, 10, 30, 03, 01, 00, 00);
  pinMode(RELE, OUTPUT);
}
/*****************************************************************************************************/
void loop() {
   ether.packetLoop(ether.packetReceive());
   if (requestSent && ether.ntpProcessAnswer(&timeStamp, srcPort)) {
   ///tone(8, 4000,30);
   //Serial.println("Resposta NTP recebida!\n");
   setDate_ts(timeStamp + MINUTOS_SEGUNDOS * horarioVerao());
   requestSent = false;
   }
   unsigned long m_time = millis();
   if (m_time - lastTime > INTERVALO) { // usa delay sem parar a execucao do programa
   ///tone(8, 500,30); // funcao tone para piezo ligado ao pino D8
   lastTime = m_time;
   ether.ntpRequest(ntpServer, srcPort);
   requestSent = true;
   }
  getDataHora();
  //mostraDataHora() ;
  alarme();
}

/* Funcao que verifica e configura o vetor RTC */
void getDataHora(){
  if (RTC[5] > 59) /* Se estiver fora do padrao, configure */
    DS1307.setDate(13, 10, 30, 03, 01, 00, 00);
  else {    /* Se estiver dentro, pegue os valores */
    DS1307.getDate(RTC); /* RTC[x] -> [0]=ANO, [1]=MES, [2]=DIA, [3]=DIA-SEMANA, [4]=HORA, [5]=MINUTO, [6]=SEGUNDO  */
    //sprintf(dateTime, "%02d/%02d/%02d %02d:%02d", RTC[2], RTC[1], RTC[0], RTC[4], RTC[5]);
    //sprintf(date, "%02d/%02d/%02d", RTC[2], RTC[1], RTC[0]);
    //sprintf(time, "%02d:%02d:%02d", RTC[4], RTC[5], RTC[6]);
  }
}

/*  Funcao que imprime na porta serial */
/* // Descomente para debug
void mostraDataHora( ) {
  if((millis() % 1000) <= 1){ // operacao modulo, se o resto da divisao for 1, imprime
    Serial.print(date);
    Serial.print(" -> ");
    Serial.println(time);
  }
}*/

bool notFeriado() { //Percorre o vetor feriado[]
  for (int i = 0; i <= (sizeof(feriado)/2); i += 2) {
    if (RTC[2] == feriado[i] && RTC[1] == feriado[i + 1]){ //se dia atual é Feriado && mês atual é Feriado
      //Serial.print("Feriado, ");
      return false;
    }
  }
  return true;
}

bool notFimDeSemana() {
  if ((RTC[3] == 1) || (RTC[3] == 7))//Se domingo ou sábado
    return false;
  return true;
}

bool isHorario() { //Percorre o vetor horario[]
  for (int i = 0; i <= (sizeof(horario)/2); i += 2) {
    if (RTC[4] == horario[i] && RTC[5] == horario[i + 1] && RTC[6] <= TMP_ALARME)
      return true;
  }
  return false;
}

/* Funcao verifica as tres condicoes para tocar o alarme:
 * Deve estar no horario, nao deve ser findi, nem feriado */
void alarme() {
  if ((isHorario() && notFimDeSemana() && notFeriado())){
    digitalWrite(RELE, HIGH);
    //tone(8,1000,30);
  }
  else
    digitalWrite(RELE, LOW);
}

/* Funcao que configura a data e hora a partir do timestamp */
void setDate_ts(uint32_t timeStamp) {
  /* Extraindo o ano */
  ntp_ano = ANO_INICIAL;
  while (true){
    uint32_t segundos;
    if (isAnoBissexto(ntp_ano)) 
      segundos = SEGUNDOS_NO_DIA * 366;
    else 
      segundos = SEGUNDOS_NO_DIA * 365;
    if (timeStamp >= segundos) {
      timeStamp -= segundos;
      ntp_ano++;
    } 
    else break;
  }
  
  /* Extraindo o mes */
  ntp_mes = 0;
  while (true) {
    uint32_t segundos = SEGUNDOS_NO_DIA * diasPorMes[ntp_mes];
    if (isAnoBissexto(ntp_ano) && ntp_mes == 1) 
      segundos = SEGUNDOS_NO_DIA * 29;
    if (timeStamp >= segundos) {
      timeStamp -= segundos;
      ntp_mes++;
    } 
    else break;
  }
  ntp_mes++;
  
  /* Extraindo o dia */
  ntp_dia = 1;
  while (true) {
    if (timeStamp >= SEGUNDOS_NO_DIA) {
      timeStamp -= SEGUNDOS_NO_DIA;
      ntp_dia++;
    } 
    else break;
  }
  
  /* Extrai hora, min, e seg */
  ntp_hora = timeStamp / MINUTOS_SEGUNDOS;
  ntp_minuto = (timeStamp - (uint32_t) ntp_hora * MINUTOS_SEGUNDOS) / 60;
  ntp_segundo = (timeStamp - (uint32_t) ntp_hora * MINUTOS_SEGUNDOS) - ntp_minuto * 60;
  int atraso = ntp_segundo - RTC[6];
  //Serial.print("Tempo de atraso RTT = ");
  //Serial.println(atraso);
  DS1307.setDate((ntp_ano-2000), ntp_mes, ntp_dia, getDiaSemana() , ntp_hora, ntp_minuto, ntp_segundo);
  //imprimeData();
}

bool isAnoBissexto(int ano) {
  return (ano % 4 == 0 && (ano % 100 != 0 || ano % 400 == 0));
}

int getDiaSemana( ) {  //Deve retornar o dia da semana
  int i=0, vec[11]={
    0  };
  vec[i] = ((12 - ntp_mes) / 10);
  vec[++i] = (ntp_ano - 2000) - vec[0];
  vec[++i] = ntp_mes + (12 * vec[0]);
  vec[++i] = vec[1] / 100;
  vec[++i] = vec[3] / 4;
  vec[++i] = 2 - vec[3] + vec[4];
  vec[++i] = 365.25 * vec[1];
  vec[++i] = 30.6001 * (vec[2] + 1);
  vec[++i] = (vec[5] + vec[6]) + (vec[7] + ntp_dia) + 5;
  vec[++i] = vec[8] % 7;
  if (vec[9]==0) return 7;
  else return vec[9];
}


/* // Descomente para usar debug pela Serial 
void imprimeData( ){
  Serial.println("\n\tHora atualizada: ");
  Serial.print(ntp_dia);
  Serial.print("/");
  Serial.print(ntp_mes);
  Serial.print("/");
  Serial.print(ntp_ano);
  switch (getDiaSemana()) { //Testa o resultado e retorna
  case 7: 
    Serial.print(", Sabado, "); 
    break;
  case 1: 
    Serial.print(", Domingo, "); 
    break;
  case 2: 
    Serial.print(", Segunda, "); 
    break;
  case 3: 
    Serial.print(", Terca, "); 
    break;
  case 4: 
    Serial.print(", Quarta, "); 
    break;
  case 5: 
    Serial.print(", Quinta, "); 
    break;
  case 6: 
    Serial.print(", Sexta, "); 
    break;
  }
  Serial.print(ntp_hora);
  Serial.print(":");
  Serial.print(ntp_minuto);
  Serial.print(":");
  Serial.println(ntp_segundo);
}
*/

int horarioVerao(){ //Seguindo o Decreto N° 6.558 de 08/09/2008.
  int ret = -3;
  if(RTC[1] >= 10 || RTC[1] <= 2){
    for (int i = 0; i < 12; i += 2) {
      if(((RTC[0] == iniHV[i]) && (RTC[1] == 10) && (RTC[2] >= iniHV[i+1])) || 
        ((RTC[0] == iniHV[i]) && (RTC[1] > 10)) || 
        ((RTC[0] == fimHV[i]) && (RTC[1] < 2)) ||
        ((RTC[0] == fimHV[i]) && (RTC[1] == 2) && (RTC[2] < fimHV[i+1])))
        ret = -2;
    }
  }
  return ret;
}

/**  E  O  F  **/

