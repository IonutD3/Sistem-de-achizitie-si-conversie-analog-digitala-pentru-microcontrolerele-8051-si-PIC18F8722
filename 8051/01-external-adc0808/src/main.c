/*
 * 01 - 8051 - ADC0808 extern
 *
 * 80C31 controleaza ADC0808N prin P0 si liniile de control de pe P2.
 * Timer0 genereaza clock-ul pentru ADC. Rezultatul este trimis prin UART.
 */
#include <reg51.h>
#define input P0
sbit ADD_A = P2^0;   
sbit ADD_B = P2^1;   
sbit ADD_C = P2^2;
sbit clk= P2^3;      
sbit ALE = P2^4;    
sbit OE = P2^5;                 
sbit SC = P2^6;  
sbit EOC = P2^7;   

void delay(unsigned int t){
unsigned int i,j;
for (i=t;i>0;i--)
for(j=0;j<122;j++);}
              
void timer0() interrupt 1{
// Toggle pentru clock-ul ADC0808.
clk=~clk;}

unsigned char read_adc(){
// Canalul 0.
unsigned char ch=0; 
EOC=1; 
OE=0;   
ALE=0;  
delay(2);  
ALE = 1;    
delay(2);
SC=0;      
delay(2);
SC = 1;   
delay(2);
ALE = 0;  
delay(2);
SC = 0;  
while(EOC==1);              
OE = 1;             
delay(2);
ch=input;            
OE = 0;
return (ch);}

void serial_Init(void){
TMOD = 0x20;                          
SCON = 0x50;                                                   
TH1 = 0xFD;                                        
TR1 = 1;}

void serial_write(unsigned char c){
SBUF = c;                                                        
while(TI == 0);                                                
TI = 0;}

void Convert_bin(int val){
char str[8];
int i = 0;
while(val){
str[i]=val%2;
val=val/2;
i++;}
for(i=7;i>=0;i--){
serial_write(48+str[i]);}
}

void Convert_dec(int val){
char str[4];
int i = 3;
while(val){
str[i]=val%10;
val=val/10;
i--;}
for(i=1;i<4;i++){
serial_write(48+str[i]);}
}

void main(){
unsigned int y;
ADD_C=0;  ADD_B=0; ADD_A=0;
serial_Init();
// Timer0: mod 2 pentru un overflow periodic.
input=0xff;        
TMOD=0x22;  
TH0=0xFD;
IE=0x82;
TR0=1;
while(1){
y=read_adc();           
delay(25);
Convert_bin(y); 
serial_write(0x0d);
Convert_dec(y);
serial_write(0x0d);
delay(25);}
}
