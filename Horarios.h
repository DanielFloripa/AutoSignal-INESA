/*______________________________________________
*      SISTEMA AUTOMACAO DE SINAL              |
*      Colégio   Santo    Antônio              |
***********************************************|
*      DANIEL S. CAMARGO                       |
*      daniel@colmeia.udesc.br                 |
*      Inicio : 02/07/2013                     |
*      Update: 01/10/2014
*_____________________________________________*/

#include <inttypes.h> // para trabalhar com diferentes tamanhos de inteiros
#define SEGUNDOS_NO_DIA  86400
#define ANO_INICIAL      1900
#define MINUTOS_SEGUNDOS 3600
#define INTERVALO        3600000 //intervalo do update_NTP = 60 min
#define TMP_ALARME       3       // em segundos

static uint8_t horario[] = //hora, minuto,
       { 8, 30,
         9, 00,
        10, 00,
        11, 00,
        12, 00,
        13, 00, 
        14, 00,
        15, 00,
        16, 00,
        17, 00,
        18, 00,
        19, 00,
        19, 30,
        20, 00,
        21, 30,
        23, 10,
        23, 15};

static uint8_t feriado[] = //dia, mês,
        {1, 1, //Dia da Paz
         9, 3, //Niver Joinville
        21, 4, //Tirandentes
         1, 5, //Trabalhador
         7, 9, //Independencia
        12, 10,//Nossa Senhora
         2, 11,//Finados
        15, 11,//República
        25, 12,//Natal
        31, 12};//Véspera Ano Novo

static uint8_t iniHV[]={  //ano, dia, //Inicio Horrario de verao em outubro
            13, 20, 
            14, 19,    
            15, 18,   
            16, 16,    
            17, 15,   
            18, 21}; 
            
static uint8_t fimHV[]={ //ano, dia, //Fim Horario de verao em fevereiro
            14, 16,    
            15, 22,   
            16, 21,    
            17, 19,   
            18, 18}; 

