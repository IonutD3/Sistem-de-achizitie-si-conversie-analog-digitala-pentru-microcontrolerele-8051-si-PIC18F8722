/*
 * 02 - ADC PIC18F8722 cu calcul de tensiune
 *
 * AN1 este folosit ca intrare analogica. Rezultatul ADC este convertit
 * in tensiune presupunand Vref = 5 V si rezolutie de 10 biti.
 */
#include <string.h>
#include <xc.h>
#define _XTAL_FREQ 4000000
unsigned char c;

void init_USART(){
CSRC = 1;
BRGH = 1; 
SYNC = 0;
TXEN = 1;
BAUDCON1bits.BRG16 = 0;
SPBRG1 = 25;
CREN = 1;
TRISCbits.RC6 = 0;
TRISCbits.RC7 = 1;
SPEN = 1;}

void read_serial(){
while(!RCIF);
c = RCREG1;}

void write_serial(char c){
while(!TXIF);
TXREG = c;}

void write_mes(char cv[]){
for(int i=0;i<=strlen(cv);i++){
write_serial(cv[i]);}
}

void init_ADC(){
//Intrare RA0 - AN0   
CHS3 = 0;
CHS2 = 0;
CHS1 = 0;
CHS0 = 0;
//Intrare RA1 - AN1   
CHS3 = 0;
CHS2 = 0;
CHS1 = 0;
CHS0 = 1;
//Vref = 5V
VCFG1 = 0;
VCFG0 = 0;
//Intrarile RA0-RA15 analogice
PCFG3 = 0;
PCFG2 = 0;
PCFG1 = 0;
PCFG0 = 0;
//Intrarile RA0-RA15 analogice
PCFG3 = 0;
PCFG2 = 0;
PCFG1 = 0;
PCFG0 = 1;
//Aliniere la dreapta
ADFM = 1;
//Tacq = 12*Tad 12 *1.2us
ACQT2 =1;
ACQT1 =0;
ACQT0 =1;
//F = Fosc/32
ADCS2 = 0;
ADCS1 = 1;
ADCS0 = 0;
ADRESH = 0;
ADRESL = 0;
__delay_ms(50);
ADON = 1;}

unsigned int ADC_start(){
unsigned int ch=0;
//pornire conversie
GO = 1;
//terminare conversie
while(DONE);
ch = ((ADRESH  << 8) | ADRESL) ;
return (ch);}
void ADC_bin(int val){
char str[10];
for(int i=0;i<=9;i++){
str[i]= val %2;
val = val/2;}
for(int i=9;i>=0;i--){
write_serial(48+str[i]);}
}

void ADC_dec(int val){
char str[4];
for(int i=0;i<=3;i++){
str[i]= val %10;
val = val/10;}
for(int i=3;i>=0;i--){
write_serial(48+str[i]);}
}

void ADC_tri(int val){
char str[6];
float a=4.99;
int b=1023;
float c=((val*a)/b)*10000;
long int d=c;
for(int i=0;i<=5;i++){
str[i]=d%10;
d=d/10;}
for(int i=5;i>=0;i--){
write_serial(48+str[i]);}
}

void main(void){ 
unsigned int y;
init_USART();
init_ADC();
write_mes("Conversie:");
write_serial(0x0d);
while(1){ 
y = ADC_start();
for(int i=0;i<=20;i++)
__delay_ms(50);
ADC_bin(y);
write_serial(0x0d);
ADC_dec(y);
write_serial(0x0d);
ADC_tri(y);
write_serial(0x0d);}
}
