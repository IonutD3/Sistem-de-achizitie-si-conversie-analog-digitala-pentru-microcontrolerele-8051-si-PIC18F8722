# Sistem de achiziție și conversie analog-digitală pentru microcontrolerele — 8051 / PIC18F8722

---

# 🇷🇴 Română

## 1. Descrierea proiectului

Acest proiect implementează și simulează trei aplicații embedded pentru **achiziția și conversia semnalelor analogice în valori digitale**, folosind două familii diferite de microcontrolere.

Prima implementare utilizează un **microcontroler 80C31** împreună cu un convertor analog-digital extern **ADC0808N**. Celelalte două implementări folosesc **convertorul ADC intern de 10 biți al microcontrolerului PIC18F8722**.

În toate implementările, rezultatul conversiei este procesat în firmware și transmis prin **UART**, astfel încât datele pot fi monitorizate într-un terminal serial.

Proiectul urmărește în mod direct interacțiunea dintre:

- perifericele ADC;
- registrele microcontrolerului;
- temporizatoare;
- întreruperi;
- semnale de control hardware;
- procesarea valorilor numerice;
- comunicația serială UART;
- simularea circuitului în Proteus.

---

## 2. Obiective

Principalele obiective ale proiectului sunt:

- configurarea unui ADC extern și controlul acestuia printr-un microcontroler 8051;
- generarea semnalului de clock pentru ADC0808 folosind Timer0;
- implementarea unei rutine de citire a conversiei ADC;
- configurarea ADC-ului intern al PIC18F8722;
- efectuarea conversiilor pe 10 biți;
- reprezentarea rezultatului ADC în format binar și zecimal;
- calcularea tensiunii corespunzătoare unei valori ADC;
- transmiterea datelor prin UART;
- verificarea funcționării prin simulări Proteus.

---

# 3. Implementarea 80C31 + ADC0808

## 3.1 Arhitectura

Prima aplicație folosește un **80C31** și un convertor extern **ADC0808N**.

Magistrala de date a ADC-ului este conectată la portul `P0`, iar semnalele de control sunt mapate pe `P2`.

| Pin / port | Semnal | Rol |
|---|---|---|
| `P0` | `input` | Magistrala de date ADC |
| `P2.0` | `ADD_A` | Selectarea canalului |
| `P2.1` | `ADD_B` | Selectarea canalului |
| `P2.2` | `ADD_C` | Selectarea canalului |
| `P2.3` | `clk` | Clock ADC0808 |
| `P2.4` | `ALE` | Address Latch Enable |
| `P2.5` | `OE` | Output Enable |
| `P2.6` | `SC` | Start Conversion |
| `P2.7` | `EOC` | End Of Conversion |

În `main()`, liniile de adresare sunt inițializate cu:

```c
ADD_C = 0;
ADD_B = 0;
ADD_A = 0;
```

Prin urmare, aplicația selectează **canalul 0 al ADC0808**.

---

## 3.2 Generarea clock-ului ADC

Clock-ul ADC0808 este generat software prin întreruperea Timer0:

```c
void timer0() interrupt 1 {
    clk = ~clk;
}
```

La fiecare overflow al Timer0, starea pinului `P2.3` este inversată.

Timer0 este configurat în modul 2:

```c
TMOD = 0x22;
TH0 = 0xFD;
IE = 0x82;
TR0 = 1;
```

Astfel, Timer0 generează periodic tranzițiile necesare semnalului de clock al ADC0808.

---

## 3.3 Secvența de conversie

Funcția:

```c
unsigned char read_adc()
```

implementează secvența de control a conversiei.

Fluxul este:

1. pregătirea semnalelor `EOC`, `OE` și `ALE`;
2. activarea `ALE`;
3. activarea semnalului `SC`;
4. dezactivarea semnalelor de start;
5. așteptarea terminării conversiei prin `EOC`;
6. activarea `OE`;
7. citirea rezultatului de pe `P0`;
8. dezactivarea `OE`;
9. returnarea valorii digitale de 8 biți.

Așteptarea finalizării conversiei este realizată prin:

```c
while(EOC == 1);
```

Rezultatul este apoi preluat prin:

```c
ch = input;
```

---

## 3.4 Conversia rezultatului în binar și zecimal

Rezultatul ADC este transmis în două forme.

Funcția:

```c
Convert_bin(y);
```

transformă valoarea în reprezentarea binară pe 8 biți.

Funcția:

```c
Convert_dec(y);
```

transformă rezultatul într-o reprezentare zecimală pe trei cifre.

Cele două valori sunt transmise succesiv prin UART, separate prin `Carriage Return`:

```c
serial_write(0x0d);
```

---

## 3.5 Comunicația UART

UART-ul este configurat în:

```c
void serial_Init(void)
```

prin:

```c
TMOD = 0x20;
SCON = 0x50;
TH1 = 0xFD;
TR1 = 1;
```

Timer1 este utilizat pentru generarea baud rate-ului, iar transmisia efectivă este realizată prin:

```c
SBUF = c;
while(TI == 0);
TI = 0;
```

Astfel, fiecare caracter este transmis și se așteaptă finalizarea transmisiei înainte de trimiterea următorului caracter.

---

# 4. Implementarea PIC18F8722 – ADC binar și zecimal

## 4.1 Configurarea sistemului

A doua aplicație utilizează ADC-ul intern al microcontrolerului **PIC18F8722**.

Frecvența de lucru este definită în cod prin:

```c
#define _XTAL_FREQ 4000000
```

deci firmware-ul utilizează o frecvență de oscilație de **4 MHz**.

Canalul ADC selectat este `AN0`:

```c
CHS3 = 0;
CHS2 = 0;
CHS1 = 0;
CHS0 = 0;
```

Tensiunea de referință este configurată folosind referințele interne ale ADC-ului:

```c
VCFG1 = 0;
VCFG0 = 0;
```

iar rezultatul ADC este aliniat la dreapta:

```c
ADFM = 1;
```

---

## 4.2 Timpul de achiziție și clock-ul ADC

Codul configurează timpul de achiziție prin:

```c
ACQT2 = 1;
ACQT1 = 0;
ACQT0 = 1;
```

și clock-ul ADC prin:

```c
ADCS2 = 0;
ADCS1 = 1;
ADCS0 = 0;
```

Comentariile din cod indică utilizarea unui timp de achiziție de:

```text
Tacq = 12 × Tad
```

și a unui clock ADC derivat din:

```text
Fosc / 32
```

ADC-ul este activat după configurare:

```c
ADON = 1;
```

---

## 4.3 Pornirea și citirea conversiei

Funcția:

```c
unsigned int ADC_start()
```

pornește conversia prin:

```c
GO = 1;
```

și așteaptă finalizarea acesteia:

```c
while(DONE);
```

După finalizarea conversiei, registrele `ADRESH` și `ADRESL` sunt combinate într-o singură valoare de 10 biți:

```c
ch = ((ADRESH << 8) | ADRESL);
```

Această valoare este returnată către `main()`.

---

## 4.4 Reprezentarea binară

Funcția:

```c
ADC_bin(int val)
```

descompune valoarea ADC în biți succesivi folosind împărțiri repetate la 2.

Rezultatul este transmis pe 10 poziții:

```text
XXXXXXXXXX
```

corespunzătoare rezoluției de 10 biți a convertorului.

---

## 4.5 Reprezentarea zecimală

Funcția:

```c
ADC_dec(int val)
```

extrage cifrele zecimale prin operații modulo 10 și le transmite prin UART.

În `main()`, rezultatul fiecărei conversii este transmis succesiv:

```text
valoare binară
valoare zecimală
```

cu un `Carriage Return` între rezultate.

Înaintea buclei principale este transmis și mesajul:

```text
Conversie:
```

---

# 5. Implementarea PIC18F8722 – calculul tensiunii

A treia aplicație pornește de la aceeași arhitectură PIC18F8722, dar adaugă conversia rezultatului ADC într-o valoare de tensiune.

## 5.1 Canalul analogic

În această implementare, codul configurează selecția canalului astfel încât `CHS` să ajungă la valoarea corespunzătoare canalului **AN1**:

```c
CHS3 = 0;
CHS2 = 0;
CHS1 = 0;
CHS0 = 1;
```

Configurarea `PCFG` este realizată, de asemenea, direct prin registrele ADC.

---

## 5.2 Calculul tensiunii

Funcția:

```c
ADC_tri(int val)
```

transformă rezultatul ADC într-o valoare de tensiune.

Calculul din cod este:

```c
float a = 4.99;
int b = 1023;
float c = ((val * a) / b) * 10000;
```

Prin urmare, firmware-ul utilizează relația:

```text
V = ADC × 4.99 / 1023
```

iar rezultatul este scalat cu `10000` pentru a putea fi transmis prin UART ca valoare numerică cu patru zecimale.

De exemplu, intern:

```text
ADC result
     ↓
ADC × 4.99
     ↓
÷ 1023
     ↓
× 10000
     ↓
integer representation
     ↓
UART
```

---

## 5.3 Datele transmise

În această variantă, pentru fiecare conversie sunt transmise trei reprezentări:

```text
valoare binară
valoare zecimală
valoare de tensiune
```

Fiecare este urmată de:

```c
write_serial(0x0d);
```

pentru delimitarea datelor în terminalul serial.

---

# 6. Comunicația UART pe PIC18F8722

Ambele implementări PIC folosesc modulul USART al microcontrolerului.

Configurația include:

```c
CSRC = 1;
BRGH = 1;
SYNC = 0;
TXEN = 1;
BAUDCON1bits.BRG16 = 0;
SPBRG1 = 25;
CREN = 1;
SPEN = 1;
```

Liniile `RC6` și `RC7` sunt configurate pentru transmisie și recepție:

```c
TRISCbits.RC6 = 0;
TRISCbits.RC7 = 1;
```

Transmisia unui caracter este realizată prin:

```c
while(!TXIF);
TXREG = c;
```

iar funcția `write_mes()` permite transmiterea unor șiruri de caractere.

---

# 7. Structura proiectului

```text
Sistem-de-achizitie-si-conversie-analog-digitala-pentru-microcontrolerele-8051-si-PIC18F8722/
│
├── README.md
│
├── 8051/
│   └── 01-external-adc0808/
│       ├── proteus/
│       │   └── project.pdsprj
│       └── src/
│           └── main.c
│
└── pic18f8722/
    │
    ├── 01-adc-binary-decimal/
    │   ├── proteus/
    │   │   └── project.pdsprj
    │   └── src/
    │       └── main.c
    │
    └── 02-adc-voltage/
        ├── proteus/
        │   └── project.pdsprj
        └── src/
            └── main.c
```

---

# 8. Tehnologii și instrumente

### Microcontrolere și periferice

- 80C31 / 8051
- PIC18F8722
- ADC0808N
- ADC intern de 10 biți
- USART / UART
- Timer0
- Timer1

### Limbaje și toolchain

- C pentru sisteme embedded
- Keil / 8051 C toolchain
- Microchip XC8
- registre hardware ale microcontrolerelor

### Simulare

- Proteus Design Suite
- simulare ADC
- simulare comunicație UART
- monitorizare serială

---

# 9. Concepte de sisteme embedded demonstrate

Proiectul demonstrează practic următoarele concepte:

- configurarea registrelor perifericelor;
- acces direct la registrele microcontrolerului;
- conversie analog-digitală;
- ADC extern versus ADC integrat;
- rezoluție ADC de 8 biți și 10 biți;
- selectarea canalelor analogice;
- configurarea tensiunii de referință;
- configurarea timpului de achiziție;
- configurarea clock-ului ADC;
- generarea unui semnal de clock cu Timer0;
- utilizarea întreruperilor;
- controlul semnalelor `ALE`, `OE`, `SC` și `EOC`;
- transmitere serială UART;
- conversia numerelor în reprezentări binare și zecimale;
- calcul numeric pe microcontroler;
- simularea și verificarea unui sistem embedded în Proteus.

---

# 10. Fluxul general al datelor

## Implementarea 80C31

```text
Semnal analogic
       │
       ▼
   ADC0808N
       │
   8-bit result
       │
       ▼
     P0
       │
       ▼
     80C31
       │
       ├── conversie binară
       │
       └── conversie zecimală
       │
       ▼
      UART
       │
       ▼
Terminal serial
```

## Implementarea PIC18F8722

```text
Semnal analogic
       │
       ▼
 ADC intern 10-bit
       │
       ▼
 ADRESH + ADRESL
       │
       ▼
  Rezultat ADC
       │
       ├── binar
       │
       ├── zecimal
       │
       └── tensiune
       │
       ▼
      UART
       │
       ▼
Terminal serial
```

---

# 11. Simulare

Pentru fiecare dintre cele trei implementări există câte un proiect **Proteus**.

Simulările sunt utilizate pentru verificarea:

- conexiunilor hardware;
- funcționării ADC-ului;
- secvenței de conversie;
- semnalelor de control;
- generării clock-ului;
- citirii rezultatului digital;
- procesării valorilor în firmware;
- comunicației UART.

Astfel, proiectul permite testarea logicii firmware împreună cu modelul hardware simulat, fără a depinde exclusiv de execuția pe placa fizică.

---

# 12. Concluzie

Proiectul implementează un flux complet de achiziție și procesare a unui semnal analogic la nivel de microcontroler.

Cele trei aplicații evidențiază două abordări hardware diferite: utilizarea unui **ADC0808 extern cu 80C31** și utilizarea **ADC-ului integrat al PIC18F8722**. Rezultatele conversiilor sunt procesate la nivel de firmware și transmise prin UART, iar ultima implementare extinde procesarea prin calcularea unei valori de tensiune.

Din punct de vedere al programării embedded, proiectul pune accent pe configurarea directă a registrelor, temporizare, întreruperi, controlul perifericelor și comunicația hardware-software.

---

# 🇬🇧 English

## 1. Project Overview

This project implements and simulates three embedded applications for **analog signal acquisition and analog-to-digital conversion**, using two different microcontroller architectures.

The first implementation uses an **80C31 microcontroller** together with an external **ADC0808N** converter. The other two implementations use the **10-bit internal ADC peripheral of the PIC18F8722**.

In all implementations, the ADC result is processed in firmware and transmitted through **UART**, allowing the converted data to be monitored using a serial terminal.

The project focuses on the interaction between:

- ADC peripherals;
- microcontroller registers;
- timers;
- interrupts;
- hardware control signals;
- numerical data processing;
- UART communication;
- Proteus-based hardware simulation.

---

# 2. Objectives

The main objectives of the project are:

- configuring and controlling an external ADC using an 8051-family microcontroller;
- generating the ADC0808 clock using Timer0;
- implementing an ADC conversion and readout routine;
- configuring the internal ADC peripheral of the PIC18F8722;
- performing 10-bit ADC conversions;
- representing ADC results in binary and decimal formats;
- converting an ADC result into a voltage value;
- transmitting measurement data through UART;
- validating the implementations through Proteus simulations.

---

# 3. 80C31 + ADC0808 Implementation

## 3.1 Architecture

The first application uses an **80C31 microcontroller** together with an external **ADC0808N**.

The ADC data bus is connected to port `P0`, while the control signals are mapped to `P2`.

| Port / Pin | Signal | Purpose |
|---|---|---|
| `P0` | `input` | ADC data bus |
| `P2.0` | `ADD_A` | Channel selection |
| `P2.1` | `ADD_B` | Channel selection |
| `P2.2` | `ADD_C` | Channel selection |
| `P2.3` | `clk` | ADC0808 clock |
| `P2.4` | `ALE` | Address Latch Enable |
| `P2.5` | `OE` | Output Enable |
| `P2.6` | `SC` | Start Conversion |
| `P2.7` | `EOC` | End Of Conversion |

The `main()` function initializes the channel-selection lines as:

```c
ADD_C = 0;
ADD_B = 0;
ADD_A = 0;
```

which selects **ADC channel 0**.

---

## 3.2 ADC Clock Generation

The ADC0808 clock is generated through a Timer0 interrupt:

```c
void timer0() interrupt 1 {
    clk = ~clk;
}
```

Each Timer0 overflow toggles `P2.3`, generating the clock transitions required by the ADC.

Timer0 is configured in mode 2:

```c
TMOD = 0x22;
TH0 = 0xFD;
IE = 0x82;
TR0 = 1;
```

---

## 3.3 Conversion Sequence

The function:

```c
unsigned char read_adc()
```

implements the ADC control sequence.

The conversion process consists of:

1. preparing the `EOC`, `OE`, and `ALE` signals;
2. enabling `ALE`;
3. generating the start-conversion sequence;
4. disabling the conversion-control signals;
5. waiting for the `EOC` signal;
6. enabling the ADC output;
7. reading the result from `P0`;
8. disabling `OE`;
9. returning the 8-bit conversion result.

Conversion completion is detected with:

```c
while(EOC == 1);
```

The digital result is then read using:

```c
ch = input;
```

---

## 3.4 Binary and Decimal Conversion

The ADC result is transmitted in two representations.

The function:

```c
Convert_bin(y);
```

generates an 8-bit binary representation.

The function:

```c
Convert_dec(y);
```

generates a decimal representation using three positions.

Both values are transmitted through UART and separated using:

```c
serial_write(0x0d);
```

---

## 3.5 UART Communication

The UART interface is initialized by:

```c
void serial_Init(void)
```

using:

```c
TMOD = 0x20;
SCON = 0x50;
TH1 = 0xFD;
TR1 = 1;
```

Timer1 is used as the baud-rate generator.

Character transmission is implemented through:

```c
SBUF = c;
while(TI == 0);
TI = 0;
```

This ensures that each character is transmitted before the next one is written to the serial buffer.

---

# 4. PIC18F8722 – Binary and Decimal ADC

## 4.1 ADC Configuration

The second application uses the **PIC18F8722 internal 10-bit ADC**.

The oscillator frequency is defined as:

```c
#define _XTAL_FREQ 4000000
```

corresponding to a **4 MHz system clock**.

The ADC channel is configured as `AN0`:

```c
CHS3 = 0;
CHS2 = 0;
CHS1 = 0;
CHS0 = 0;
```

The ADC reference configuration is set through:

```c
VCFG1 = 0;
VCFG0 = 0;
```

and the conversion result is right-aligned:

```c
ADFM = 1;
```

---

## 4.2 Acquisition Time and ADC Clock

The acquisition time is configured through:

```c
ACQT2 = 1;
ACQT1 = 0;
ACQT0 = 1;
```

while the ADC clock is configured using:

```c
ADCS2 = 0;
ADCS1 = 1;
ADCS0 = 0;
```

The source code specifies:

```text
Tacq = 12 × Tad
```

and an ADC clock derived from:

```text
Fosc / 32
```

The ADC is then enabled with:

```c
ADON = 1;
```

---

## 4.3 Starting and Reading a Conversion

The function:

```c
unsigned int ADC_start()
```

starts a conversion with:

```c
GO = 1;
```

and waits for completion using:

```c
while(DONE);
```

The two ADC result registers are combined into a single 10-bit value:

```c
ch = ((ADRESH << 8) | ADRESL);
```

The resulting value is returned to the main application.

---

## 4.4 Binary Representation

The function:

```c
ADC_bin(int val)
```

converts the ADC value into individual binary digits using repeated division by 2.

The resulting 10-bit representation is transmitted through UART:

```text
XXXXXXXXXX
```

---

## 4.5 Decimal Representation

The function:

```c
ADC_dec(int val)
```

extracts decimal digits using modulo-10 operations and transmits them through UART.

For each ADC conversion, the application therefore outputs:

```text
binary representation
decimal representation
```

with a carriage return separating the values.

The program also sends the initial message:

```text
Conversie:
```

when the application starts.

---

# 5. PIC18F8722 – Voltage Calculation

The third application extends the PIC18F8722 ADC implementation by converting the ADC result into a voltage value.

## 5.1 Analog Channel

The source code configures the ADC channel-selection bits so that `CHS` corresponds to **AN1**:

```c
CHS3 = 0;
CHS2 = 0;
CHS1 = 0;
CHS0 = 1;
```

The ADC input configuration is also controlled through the `PCFG` registers.

---

## 5.2 Voltage Calculation

The function:

```c
ADC_tri(int val)
```

converts the ADC result into a voltage representation.

The calculation implemented in the source code is:

```c
float a = 4.99;
int b = 1023;
float c = ((val * a) / b) * 10000;
```

Therefore, the voltage conversion is based on:

```text
V = ADC × 4.99 / 1023
```

The result is multiplied by `10000` so that the calculated voltage can be transmitted as a scaled integer representation corresponding to four decimal places.

The processing flow is:

```text
ADC result
     ↓
ADC × 4.99
     ↓
÷ 1023
     ↓
× 10000
     ↓
integer representation
     ↓
UART
```

---

## 5.3 Output Format

For every conversion, the application transmits three representations:

```text
binary value
decimal value
voltage value
```

Each value is followed by:

```c
write_serial(0x0d);
```

to delimit the measurements in the serial terminal.

---

# 6. PIC18F8722 UART Configuration

Both PIC implementations use the microcontroller's USART peripheral.

The configuration includes:

```c
CSRC = 1;
BRGH = 1;
SYNC = 0;
TXEN = 1;
BAUDCON1bits.BRG16 = 0;
SPBRG1 = 25;
CREN = 1;
SPEN = 1;
```

The serial pins are configured through:

```c
TRISCbits.RC6 = 0;
TRISCbits.RC7 = 1;
```

Character transmission is performed using:

```c
while(!TXIF);
TXREG = c;
```

The `write_mes()` function provides string transmission over the same UART interface.

---

# 7. Project Structure

```text
Sistem-de-achizitie-si-conversie-analog-digitala-pentru-microcontrolerele-8051-si-PIC18F8722/
│
├── README.md
│
├── 8051/
│   └── 01-external-adc0808/
│       ├── proteus/
│       │   └── project.pdsprj
│       └── src/
│           └── main.c
│
└── pic18f8722/
    │
    ├── 01-adc-binary-decimal/
    │   ├── proteus/
    │   │   └── project.pdsprj
    │   └── src/
    │       └── main.c
    │
    └── 02-adc-voltage/
        ├── proteus/
        │   └── project.pdsprj
        └── src/
            └── main.c
```

---

# 8. Technologies and Tools

### Microcontrollers and Peripherals

- 80C31 / 8051
- PIC18F8722
- ADC0808N
- 10-bit internal ADC
- USART / UART
- Timer0
- Timer1

### Programming and Toolchain

- Embedded C
- Keil / 8051 C toolchain
- Microchip XC8
- Direct microcontroller register configuration

### Simulation

- Proteus Design Suite
- ADC simulation
- UART communication simulation
- Serial terminal monitoring

---

# 9. Embedded Systems Concepts Demonstrated

The project demonstrates practical implementation of:

- peripheral register configuration;
- direct hardware register access;
- analog-to-digital conversion;
- external versus integrated ADC peripherals;
- 8-bit and 10-bit ADC resolution;
- analog channel selection;
- voltage-reference configuration;
- ADC acquisition-time configuration;
- ADC clock configuration;
- timer-based clock generation;
- interrupt handling;
- ADC0808 `ALE`, `OE`, `SC`, and `EOC` control;
- UART serial transmission;
- binary and decimal number conversion;
- numerical processing on a microcontroller;
- hardware/firmware co-simulation in Proteus.

---

# 10. Data Flow

## 80C31 Implementation

```text
Analog Signal
      │
      ▼
  ADC0808N
      │
  8-bit Result
      │
      ▼
     P0
      │
      ▼
    80C31
      │
      ├── Binary conversion
      │
      └── Decimal conversion
      │
      ▼
     UART
      │
      ▼
Serial Terminal
```

## PIC18F8722 Implementation

```text
Analog Signal
      │
      ▼
 10-bit Internal ADC
      │
      ▼
 ADRESH + ADRESL
      │
      ▼
  ADC Result
      │
      ├── Binary
      │
      ├── Decimal
      │
      └── Voltage
      │
      ▼
     UART
      │
      ▼
Serial Terminal
```

---

# 11. Simulation

Each implementation includes a dedicated **Proteus project** corresponding to the source code.

The simulations are used to verify:

- hardware connections;
- ADC operation;
- conversion timing;
- ADC control signals;
- clock generation;
- digital result acquisition;
- firmware-level data processing;
- UART communication.

This allows the firmware and the simulated hardware configuration to be tested together before deployment to physical hardware.

---

# 12. Conclusion

The project implements a complete analog acquisition and processing workflow at microcontroller level.

The three applications demonstrate two different hardware approaches: an **external ADC0808 controlled by an 80C31** and the **integrated ADC peripheral of the PIC18F8722**. The conversion results are processed in firmware and transmitted through UART, while the third implementation extends the processing stage by calculating a corresponding voltage value.

From an embedded software perspective, the project focuses on direct register configuration, timing, interrupts, peripheral control, numerical processing, and hardware-software communication.

---

# 👤 Autor / Author

**IonutD**
