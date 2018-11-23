//Este programa deve ser capaz de detectar a fase da senóide e ativar o disparo do triac
//tem como entrada a amplitude da rede em seu ADC e como saída o pulso de disparo do triac
//OBS: 7000 é o valor de TACCR0 que deixa a potência mínima, não 8000.

#include <msp430.h> 
#define STDBY BIT3
#define REDE BIT2
#define TERMOMETRO BIT1
#define DISPARO BIT0
#define ADC_rede_ch INCH_2
#define ADC_termo_ch INCH_1


unsigned int val_rede = 0;
unsigned int T_alvo = 0;
unsigned int T_atual = 0;

int ADC_read(void){
    ADC10CTL0 |= ENC + ADC10SC;                                             //enable conversion, start conversion
    ADC10CTL0 &= ~ADC10IFG;
    ADC10CTL0 &= ~ADC10SC;
    ADC10CTL0 &= ~ENC;                                                      //reseta flag de conversão A/D, reseta SC, reseta enable conversion
    while ((ADC10CTL1 & ADC10BUSY) == 0x01);                                //espera a covnersão acabar
    return (ADC10MEM);
}

void BASIC_config(){
    WDTCTL = WDTPW | WDTHOLD;                                           // stop watchdog timer
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;                                               //atribui 1MHz ao SMCLK
}

void TIMER_config(){
    TA0CTL = TASSEL_2 + ID_0 + MC_1;                //SMCLK, /1, up mode
    TACCTL0 = OUTMOD_7;                             //reset/set, cap/comp interrupt enabled
    TACCTL1 = OUTMOD_7;
}

void TIMER_clear(){
    TA0CTL = TACLR;                                 //clear TA0R
    TA0CTL = TASSEL_2 + ID_0 + MC_1;                //SMCLK, /1, up mode
    TACCTL0 = OUTMOD_7;
}

int main(void)
{
    P1DIR &= 0x00;
    P1DIR |= DISPARO;
    P1OUT &= 0x00;

    TIMER_config();
    BASIC_config();
    TACCR0 = 50-1;
    T_alvo = 0;

    while(1){

        TIMER_config();

        ADC10AE0 |= TERMOMETRO;                                                 //termômetro
        ADC10CTL1 = ADC_termo_ch+SHS_0+ADC10DIV_0+ADC10SSEL_3+CONSEQ_2;         //in canal 7, divide por 1, seleciona internal ocsilator, capta todos os canais uma vez
        ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON;                              //VCC-GND, sample-hold x4, adc on
        ADC10CTL1 &= ~ADC10DF;                                                  //setar modo binário comum
        ADC10DTC0 |= ADC10CT;                                                   //habilita transf. continua
        ADC_read();
        T_atual = ADC10MEM;

        ADC10AE0 |= REDE;                                                       //crossing zero detector
        ADC10CTL1 = ADC_rede_ch+SHS_0+ADC10DIV_0+ADC10SSEL_3+CONSEQ_2;          //in canal 7, divide por 1, seleciona internal ocsilator, capta todos os canais uma vez
        ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON;                              //VCC-GND, sample-hold x4, adc on
        ADC10CTL1 &= ~ADC10DF;                                                  //setar modo binário comum
        ADC10DTC0 |= ADC10CT;                                                   //habilita transf. continua
        ADC_read();
        val_rede = ADC10MEM;

        if((P1IN & STDBY) == 0){
            T_alvo = 900;
        }else{
            T_alvo = 800;
        }

        if(val_rede>100 & val_rede<300){                                        //define intervalo de disparo. Se for apenas uma condição no if, essa condição pode se
            TIMER_clear();                                                      //repetir no mesmo if, por isso o intervalo é fechado.
            while((TA0CTL & TAIFG) == 0);                                       //espera atingir TACCR0
            P1OUT |= DISPARO;
            TA0CTL &= ~TAIFG;

        }else{
           P1OUT &= ~DISPARO;
        }
        if(T_atual < T_alvo){
            TACCR0 = (7200-26*(T_alvo-T_atual));
        }else{
            TACCR0 = 7500; //(8000 + (T_atual-T_alvo));
        }
    }
}


