#include "F28x_Project.h"
#include "F2837xD_Ipc_drivers.h"
#include <stdint.h>
#include <math.h>

#include "Peripheral\Peripheral.h"
#include "Comm\Comm.h"
#include "Test_setting.h"

//***********************************************************************//
//                          P r o t o t y p e                            //
//***********************************************************************//
void task_table (Uint32 * counter);

//***********************************************************************//
//                             D e f i n e                               //
//***********************************************************************//

#define NUM_TASK_MEMBER (Uint16)(sizeof(struct_task_period) / sizeof(Uint16))
#define TASK_MAX_COUNT *((Uint16 *)&task_period + (NUM_TASK_MEMBER-1))

//***********************************************************************//
//                     G l o b a l  V a r i a b l e s                    //
//***********************************************************************//
const struct_task_period task_period = {
    .count_5kHz  = 1,
    .count_1kHz  = 5,
    .count_100Hz = 50,
    .count_10Hz  = 500,
    .count_1Hz   = 5000,
};

extern Uint16 data_rx[SIZEOFDATA_RX];
extern Uint16 data_tx[SIZEOFDATA_TX/2];

//***********************************************************************//
//                        S h a r e d  M e m o r y                       //
//***********************************************************************//

//***********************************************************************//
//                          F u n c t i o n s                            //
//***********************************************************************//
void main(void)
{
	InitSysCtrl(); //Initialize System Control:PLL, WatchDog, enable Peripheral Clocks

	Init_Peripheral();

	Enable_Peripheral();

//	Enable_Comm();

	for(;;); //Idle Loop
}

#pragma CODE_SECTION(SCIA_RX_isr, "ramfuncs")
__interrupt void SCIA_RX_isr(void)
{
    Receive_from_USB();
    data_tx[0] = data_rx[3];
    data_tx[1] = data_rx[3];

    GpioDataRegs.GPCTOGGLE.bit.GPIO81 = 1;

    SciaRegs.SCIFFRX.bit.RXFIFORESET=1; //RESET RX FIFO

    SciaRegs.SCIFFRX.bit.RXFFINTCLR = 1; //clear INT1 flag
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP9; //clear INT Group9
}


//#pragma CODE_SECTION(cpu_timer_5kHz, "ramfuncs")
__interrupt void cpu_timer_5kHz(void)
{
	CpuTimer0.InterruptCount++; //Start counting

    task_table(&CpuTimer0.InterruptCount);
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; //clear INT Group1
}

void task_table (Uint32 * counter)
{
//    if (*counter % (Uint32)task_period.count_5kHz == 0)
//    {
//
//    }

    if (*counter % (Uint32)task_period.count_1kHz == 0)
    {
        Report_to_USB();
    }

//    if (*counter % (Uint32)task_period.count_100Hz == 0)
//    {
//
//    }
//
//    if (*counter % (Uint32)task_period.count_10Hz == 0)
//    {
//        GpioDataRegs.GPATOGGLE.bit.GPIO31 = 1;
//    }
//
    if (*counter % (Uint32)task_period.count_1Hz == 0)
    {
    	GpioDataRegs.GPCTOGGLE.bit.GPIO73 = 1;
    }

    if (*counter % (Uint32)TASK_MAX_COUNT == 0)
    {
        *counter = 0; //reset Timer0 counter
    }
}