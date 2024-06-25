
//*****************************************************************************************
// Simple PWM For Both Switches
//*****************************************************************************************
// Header file
#include "F28x_Project.h"

////////  Vande  /////////

volatile float frequency_inc = 1.0; // starting frequency is 1 Hz
volatile float frequency_dec = 10.0; // starting frequency is 10 Hz
//
// Function Prototypes
//
__interrupt void xINT1_isr(void);
__interrupt void xINT2_isr(void);
__interrupt void cpu_timer0_isr(void);
void main(void)
{

    ////////  Vande  /////////

    // This line of statement will initialize all the CPU  clocks of micro controller
    InitSysCtrl();

    // First Disable all the interrupts which was declare earlier
    DINT;

    /* Initialize peripheral interrupt expansion control register which will be used to addressed
   any  particular interrupt in PIE vector table*/
    InitPieCtrl();

    // Disable interrupt Enable Register
    IER = 0x0000;
    // Disable interrupt Flag Register
    IFR = 0x0000;

    /*Initialize Pie assignment which will consist of
      12 Core Interrupt and 16 sub-interrupt of each Core Interrupt*/
    InitPieVectTable();

    ////////  Vande  /////////

    // This is needed to write to EALLOW protected registers
    EALLOW;
    //Configure Timer0 interrupt in pie vector table
    PieVectTable.TIMER0_INT = &cpu_timer0_isr;
    // Configure XINT1 in pie Vector Table
    PieVectTable.XINT1_INT = &xINT1_isr;
    // Configure XINT2 in pie Vector Table
    PieVectTable.XINT2_INT = &xINT2_isr;
    // This is needed to disable write to EALLOW protected registers
    EDIS;

    // Initialize all the CPU Timers
    InitCpuTimers();

    // Configure CPU-Timer 0 to __interrupt every 1000 milliseconds:
    // 60MHz CPU Frequency
    ConfigCpuTimer(&CpuTimer0, 60, 1000000);

    //Configure Timer control register with its address
    CpuTimer0Regs.TCR.all = 0x4001;

    ////////  Vande  /////////

    EALLOW;
    //Configure GPIO7,GPIO18
    GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 0;
    GpioCtrlRegs.GPAMUX2.bit.GPIO18 = 0;
    //Configure GPIO67,GPIO22
    GpioCtrlRegs.GPCMUX1.bit.GPIO67 = 0;
    GpioCtrlRegs.GPAMUX2.bit.GPIO22 = 0;

    // Enable GPIO7  for logic analyzer as an Output
    GpioCtrlRegs.GPADIR.bit.GPIO7 = 1;
    // Enable GPIO18  for logic analyzer as an Output
    GpioCtrlRegs.GPADIR.bit.GPIO18 = 1;

    // Enable GPIO 67 as an Input  which will be use for Switch 1
    GpioCtrlRegs.GPCDIR.bit.GPIO67 = 0;
    // Enable GPIO 22 as an Input which will be use for Switch 2
    GpioCtrlRegs.GPADIR.bit.GPIO22 = 0;

    //Configure GPIO34 and GPIO31 as an Output
    GpioCtrlRegs.GPBMUX1.bit.GPIO34 = 0;
    GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1;
    GpioCtrlRegs.GPAMUX2.bit.GPIO31 = 0;
    GpioCtrlRegs.GPADIR.bit.GPIO31 = 1;

    //Configure GPIO31 ,GPIO34 ,GPIO7,GPIO18 initially set to Low and High
    //Initially Blue LED will be turn OFF
    GpioDataRegs.GPASET.bit.GPIO31 = 1;
    //initially RED LED will be turn ON
    GpioDataRegs.GPBSET.bit.GPIO34 = 0;
    GpioDataRegs.GPASET.bit.GPIO7 = 1;
    GpioDataRegs.GPASET.bit.GPIO18 = 0;
    EDIS;



    // Enable CPU INT1 which is connected to CPU-Timer 0
    IER |= M_INT1; // M- means to Enable

    ////////  Vande  /////////

    // Enable TINT0 ,XINT1,XINT2   in the PIE: Group 1 __interrupt 4,5,7
    // Enable the PIE block
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
    // Enable PIE Group 1 INT4
    PieCtrlRegs.PIEIER1.bit.INTx4 = 1;
    // Enable PIE Group 1 INT5
    PieCtrlRegs.PIEIER1.bit.INTx5 = 1;
    /* Address Core interrupt 1 and Sub-Interrupt 7
     which in PIE vector table is Timer 0 interrupt*/
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;

    // Enable global Interrupts and higher priority real-time debug events:
    // Enable Global __interrupt INTM
    EINT;
    // Enable Global realtime __interrupt DBGM
    ERTM;

    ////////  Vande  /////////

    //enable xINT1,xINT2
    XintRegs.XINT1CR.bit.ENABLE = 1;            // Enable XINT1
    XintRegs.XINT2CR.bit.ENABLE = 1;            // Enable XINT2
    //Configure GPIO67,GPIO22 FOR xINT1,xINT2
    GPIO_SetupXINT1Gpio(67);
    GPIO_SetupXINT2Gpio(22);

    ////////  Vande  /////////

    // Check switch state for falling edge
    XintRegs.XINT1CR.bit.POLARITY = 1;          // Rising edge interrupt
    XintRegs.XINT2CR.bit.POLARITY = 0;          // Falling edge interrupt

    // set Timer0 period for desired frequency in this it is 1 SEC
    CpuTimer0Regs.PRD.all = 100e6 ;
    // start Timer0
    CpuTimer0Regs.TCR.bit.TSS = 0;

    while(1){
        //Wait for CPU to wake-up
        asm("IDLE:");
    }
}

////////  Vande  /////////

// ISR for Gpio(67)
__interrupt void xINT1_isr(void)
{

    // Increase frequency by 1Hz
    frequency_inc += 1.0;
    if(frequency_inc >= 10.0)
        // limit frequency to 10 Hz
        frequency_inc = 10.0;
    // update Timer0 period
    CpuTimer0Regs.PRD.all = 50e6 / frequency_inc;

    // clear interrupt flag
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

////////  Vande  /////////

// ISR for Gpio(22)
__interrupt void xINT2_isr(void)
{

    // Decrease frequency by 1Hz
    frequency_dec -= 1.0;
    if(frequency_dec <= 1.0)
        // limit frequency to 10 Hz
        frequency_dec = 1.0;
    // update Timer0 period
    CpuTimer0Regs.PRD.all = 50e6 / frequency_dec;

    // clear interrupt flag
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

////////  Vande  /////////

__interrupt void cpu_timer0_isr(void)
{
    // toggle LED pin
    GpioDataRegs.GPATOGGLE.bit.GPIO31= 1;
    // toggle LED pin
    GpioDataRegs.GPBTOGGLE.bit.GPIO34= 1;
    // toggle pwm pin
    GpioDataRegs.GPATOGGLE.bit.GPIO7= 1;
    // toggle pwm pin
    GpioDataRegs.GPATOGGLE.bit.GPIO18= 1;

    // clear interrupt flag
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;

}






