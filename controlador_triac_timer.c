//Este programa deve ser capaz de detectar a fase da senóide e ativar o disparo do triac
//tem como entrada a amplitude da rede em seu ADC e como saída o pulso de disparo do triac


#include <msp430.h> 
#define ADC_pin BIT4
#define ADC_channel INCH_4
#define DISPARO BIT0

unsigned int val = 0;

void ADC_config(void){

    ADC10AE0 |= ADC_pin;                                                    //ADC10 enable no ADC_pin
    ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON;                              //VCC-GND, sample-hold x4, adc on
    ADC10CTL1 = ADC_channel+SHS_0+ADC10DIV_0+ADC10SSEL_3+CONSEQ_2;          //ADC_channel como entrada, TA out1, div 1, SMCLK
    ADC10CTL1 &= ~ADC10DF;                                                  //setar modo binário comum
    ADC10DTC0 |= ADC10CT;                                                   //habilita transf. continua

}


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
    TACCTL0 = OUTMOD_7;                             //reset/set
    TACCTL1 = OUTMOD_7;


}

void TIMER_clear(){
    TA0CTL = TACLR;                                 //clear TA0R
    TA0CTL = TASSEL_2 + ID_0 + MC_1;                //SMCLK, /1, up mode
    TACCTL0 = OUTMOD_7;
}

int main(void)
{   
    P1DIR |= DISPARO;
    P1OUT &= 0x00;

    TIMER_config();
    BASIC_config();
    ADC_config();
    TACCR0 = 1000-1;                                          //Se TACCR0 for muito pequeno a potencia é maxima. O valor máximo de TACCR0
                                                              //é de 8000 (8ms) e pouco, que é aproximadamente 1/120 s. 
    while(1){

        ADC_read();
        val = ADC10MEM;
        TIMER_config();

        if(val>100 & val<500){                                //define intervalo de disparo. Se for apenas uma condição no if, essa condição pode se
            TIMER_clear();                                    //repetir no mesmo if, por isso o intervalo é fechado.
            while((TA0CTL & TAIFG) == 0);                     //espera atingir TACCR0
            P1OUT |= DISPARO;
            TA0CTL &= ~TAIFG;

        }else{
           P1OUT &= ~DISPARO;
        }
    }
}
