#include <msp430.h>
#define ADC_pin BIT3
#define ADC_channel INCH_3


void ADC_config(void){

    ADC10AE0 |= ADC_pin;                                                    //ADC10 enable no pin 3
    ADC10CTL0 = SREF_0 + ADC10SHT_0 + ADC10ON;                              //VCC-GND, sample-hold x4, adc on
    ADC10CTL1 = ADC_channel+SHS_0+ADC10DIV_0+ADC10SSEL_0+CONSEQ_2+ADC10DF;  //BIT3 como entrada/TA out1/div 1 /SMCLK
    ADC10DTC0 |= ADC10CT;                                                   //habilita transf. continua

}

void ADC_read(void){

    ADC10CTL0 |= ENC + ADC10SC;                                         //enable conversion, start conversion
    ADC10CTL0 &= ~ADC10IFG + ~ADC10SC + ~ENC;                           //reseta flag de conversão A/D, reseta SC, reseta enable conversion

}

void BASIC_config(){

    WDTCTL = WDTPW | WDTHOLD;                                           // stop watchdog timer
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;                                               //atribui 1MHz ao SMCLK

}

int main(void)
{
    P1DIR = BIT0;
    P1OUT &= 0x00;

    BASIC_config();
    ADC_config();

    while(1){

        ADC_read();

        if((ADC10MEM>>6) > 520){                                        //Necessário deslocar para corrigir a posição dos bits
            P1OUT |= BIT0;
        }else{
            P1OUT &= ~BIT0;
        }

    }

    return 0;
}
