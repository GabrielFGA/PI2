//Ultima versão do codigo para a apresentação 2 de PI2.
//Quando atinge uma temperatura, o triac corta parte da onda.

#include <msp430.h>

#define STDBY BIT5
#define TRIAC BIT0
#define ADC_pin_rede BIT6
#define ADC_pin_termometro BIT7
#define ADC_channel_rede INCH_6
#define ADC_channel_termometro INCH_7

unsigned volatile long val_temp = 0, val_rede = 0;
int k = 0;

/*void ADC_config(void){

    ADC10AE0 |= BIT1;
    ADC10CTL1 = INCH_1+SHS_0+ADC10DIV_0+ADC10SSEL_3+CONSEQ_1;               //in canal 7, divide por 1, seleciona internal ocsilator, capta todos os canais uma vez
    ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON;                              //VCC-GND, sample-hold x4, adc on
    ADC10CTL1 &= ~ADC10DF;                                                  //setar modo binário comum
    ADC10DTC0 |= ADC10CT;                                                   //habilita transf. continua
}*/

int ADC_read(void){

    ADC10CTL0 |= ENC + ADC10SC;                                             //enable conversion, start conversion
    ADC10CTL0 &= ~ADC10IFG;
    ADC10CTL0 &= ~ADC10SC;
    ADC10CTL0 &= ~ENC;                                                      //reseta flag de conversão A/D, reseta SC, reseta enable conversion
    while ((ADC10CTL1 & ADC10BUSY) == 0x01);                                //espera a covnersão acabar

    return (ADC10MEM);

}

void BASIC_config(){

    WDTCTL = WDTPW | WDTHOLD;                                               // stop watchdog timer
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;                                                   //atribui 1MHz ao SMCLK

}

int main(void)
{
    P1DIR &= 0x00;
    P1DIR |= TRIAC;              //define o triac como saída
    P1OUT &= 0x00;
    BASIC_config();

    while(1){

        ADC10AE0 |= BIT7;                                                       //termômetro
        ADC10CTL1 = INCH_7+SHS_0+ADC10DIV_0+ADC10SSEL_3+CONSEQ_2;               //in canal 7, divide por 1, seleciona internal ocsilator, capta todos os canais uma vez
        ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON;                              //VCC-GND, sample-hold x4, adc on
        ADC10CTL1 &= ~ADC10DF;                                                  //setar modo binário comum
        ADC10DTC0 |= ADC10CT;                                                   //habilita transf. continua
        ADC_read();
        val_temp = ADC10MEM;

        ADC10AE0 |= BIT6;                                                       //crossing zero detector
        ADC10CTL1 = INCH_6+SHS_0+ADC10DIV_0+ADC10SSEL_3+CONSEQ_2;               //in canal 7, divide por 1, seleciona internal ocsilator, capta todos os canais uma vez
        ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON;                              //VCC-GND, sample-hold x4, adc on
        ADC10CTL1 &= ~ADC10DF;                                                  //setar modo binário comum
        ADC10DTC0 |= ADC10CT;                                                   //habilita transf. continua
        ADC_read();
        val_rede = ADC10MEM;

        if(val_temp > 380){
            k = 200;                //calibrar aqui
        }else{
            k = 534;                //calibrar aqui
        }

        if(val_rede > k){      
            P1OUT |= TRIAC;
        }else{
            P1OUT &= ~TRIAC;
        }
    }
}
