#include <xc.h>
#include <p18f4520.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "conbits.h"
#include "uart_layer.h"

#define ADC_RES (5.0/1023.0)

void interrupt high_isr(void);
void interrupt low_priority low_isr(void);

uint8_t print_buffer[256] = {0}; // buffer to print stuff to serial
bool adc_done = false;

void print_adc_value_volt(void){     
    float adc_volt = ((ADRESH << 8) | (ADRESL)) * ADC_RES;
    sprintf(print_buffer,"\rADC Volt: %.2f",adc_volt);
    uart_send_string(print_buffer);
}

void set_led_after_com(void){
    if(CMCONbits.C2OUT){
        LATBbits.LB7 = 1;
        LATBbits.LB0 = 0;
    }
    if(CMCONbits.C1OUT){
        LATBbits.LB7 = 0;
        LATBbits.LB0 = 1;
    }
}

void main(void){
    
    OSCCONbits.IDLEN = 1;
    OSCCONbits.IRCF = 0x07;
    OSCCONbits.SCS = 0x03;
    while(OSCCONbits.IOFS!=1); // 8Mhz

    TRISB=0;    // b port as output
    LATB=0x00; // b port low
    
    TRISAbits.RA0 = 1;
    TRISAbits.RA1 = 1;
    TRISEbits.RE2 = 1;
    
    //LATB=0x01; // b port low
  
    uart_init(51,0,1,0);//baud 9600
    
    //LATB=0x02; // b port low
    
    //T2CONbits.T2OUTPS = 15; //0.5us *16*16*8 = 1.024ms
    //T2CONbits.T2CKPS  = 2;

    ADCON1bits.VCFG0 = 0;
    ADCON1bits.VCFG1 = 0;
    ADCON1bits.PCFG = 0x0B;
    
    //LATB=0x04; // b port low
    
    ADCON2bits.ADFM = 1;
    ADCON2bits.ACQT = 0x04;
    ADCON2bits.ADCS = 0x04;
    
    ADCON0bits.CHS = 7;
    ADCON0bits.ADON = 1;
    
    //LATB=0x08; // b port low

    CVRCONbits.CVRSS    = 0;
    CVRCONbits.CVRR     = 1;
    CVRCONbits.CVROE    = 0;
    CVRCONbits.CVR      = 0x0F; // 3.125V
    
    //LATB=0x10; // b port low
    
    CMCONbits.CM    = 6;
    CMCONbits.CIS   = 0;
    CMCONbits.C1INV = 0;
    CMCONbits.C2INV = 1;
    
    //LATB=0x20; // b port low
    
    IPR2bits.CMIP   = 1;
    PIE2bits.CMIE   = 1;
    
    //IPR1bits.TMR2IP = 0;
    //PIE1bits.TMR2IE = 1;
    
    IPR1bits.ADIP = 1;
    PIE1bits.ADIE = 1;
    
    //LATB=0x40; // b port low
    __delaywdt_ms(2000);
    sprintf(print_buffer,"\n\rProgram start\n\r");
    uart_send_string(print_buffer); // everything works in setup
   // LATB=0x80; // b port low
    
    RCONbits.IPEN   = 1; 
    INTCONbits.GIEH = 1; 
    INTCONbits.GIEL = 1;// base interrupt setup
    
    CVRCONbits.CVREN    =   1;
   // T2CONbits.TMR2ON    =   1;
    
    for(;;){
        
        if(!adc_done){
            //OSCCONbits.IDLEN = 1;
            ADCON0bits.GODONE = 1; 
        }
        if(adc_done){
            print_adc_value_volt();
            adc_done = false;
            LATBbits.LB4 = ~LATBbits.LB4;
           // OSCCONbits.IDLEN = 0;
           // T2CONbits.TMR2ON = 1;
        }
        Sleep();
    } 
}

void interrupt high_isr(void){
    INTCONbits.GIEH = 0;
    if(PIR2bits.CMIF == 1){
        set_led_after_com();
        PIR2bits.CMIF = 0; 
    }
    if(PIR1bits.ADIF){
        adc_done = true;
        PIR1bits.ADIF = 0;
    }
    INTCONbits.GIEH = 1;
}

void interrupt low_priority low_isr(void){
    INTCONbits.GIEH = 0;
    if(PIR1bits.TMR2IF == 1){
       /// T2CONbits.TMR2ON = 0;
       // ADCON0bits.GODONE = 1;
        PIR1bits.TMR2IF = 0;
    }  
    INTCONbits.GIEH = 1;
}



