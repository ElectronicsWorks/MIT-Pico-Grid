#include "F28x_Project.h"
#include "Comm.h"
#include "../Peripheral/SCI.h"

//***********************************************************************//
//                     G l o b a l  V a r i a b l e s                    //
//***********************************************************************//

Uint16 usb_tx[SIZEOFUSB_TX] = {0};
Uint16 usb_rx[SIZEOFUSB_RX] = {0};

extern Uint16 device_flag[NUMOFDEVICE];
extern Uint16 cast_flag[NUMOFDEVICE];
extern Uint16 RS485_addr[NUMOFDEVICE];
extern Uint16 RS485_tx[NUMOFDEVICE][SIZEOFRS485_TX];
extern Uint16 RS485_rx[NUMOFDEVICE][SIZEOFRS485_RX];

float32 lp[NUMOFDEVICE] = {100.0f, 100.0f, 100.0f};
float32 lq[NUMOFDEVICE] = {100.0f, 100.0f, 100.0f};

//***********************************************************************//
//                          F u n c t i o n s                            //
//***********************************************************************//
void USB_TX(void)
{
    SCIA_xmit(BEGIN);
    Uint16 i;
    for(i=0;i<SIZEOFUSB_TX;i++)
    {
        SCIA_xmit(usb_tx[i]);
//        usb_tx[i] = 0; //reset USB_TX data
    }

    SCIA_xmit(END);
}

void USB_RX (void)
{
	Uint16 i;
	for (i = 0; i < SIZEOFUSB_RX; i++)
	{
		Uint16 j;
		j = (SIZEOFUSB_RX - 1) - (SciaRegs.SCIFFRX.bit.RXFFST - 1) % SIZEOFUSB_RX;

		usb_rx[i] = (j == i)? SciaRegs.SCIRXBUF.all : usb_rx[i]; //If data sequence matches, load the data; otherwises, do nth.
	}
}

void USB_TO_RS485Interpreter(void)
{
    //LabView command -  0: command; 1: address; 2: N/A; 3:value
    Uint16 addr = 0;
    Uint16 i = 0;
    addr = usb_rx[1];
    //Matching Address
    if (addr == ADDR_HOST){
        if (usb_rx[0] == 191) GpioDataRegs.GPACLEAR.bit.GPIO0 = 1;
        if (usb_rx[0] == 192) GpioDataRegs.GPASET.bit.GPIO0 = 1;
        if (usb_rx[0] == 193) GpioDataRegs.GPACLEAR.bit.GPIO1 = 1;
        if (usb_rx[0] == 194) GpioDataRegs.GPASET.bit.GPIO1 = 1;
        if (usb_rx[0] == 195) GpioDataRegs.GPACLEAR.bit.GPIO2 = 1;
        if (usb_rx[0] == 196) GpioDataRegs.GPASET.bit.GPIO2 = 1;
        if (usb_rx[0] == 181) lp[0] = usb_rx[3];
        if (usb_rx[0] == 182) lq[0] = usb_rx[3];
        if (usb_rx[0] == 183) lp[1] = usb_rx[3];
        if (usb_rx[0] == 184) lq[1] = usb_rx[3];
        if (usb_rx[0] == 185) lp[2] = usb_rx[3];
        if (usb_rx[0] == 186) lq[2] = usb_rx[3];
    }
    else{
        for (i = 0; i < NUMOFDEVICE; i++)
        {
            if (addr == ADDR_BROADCAST || RS485_addr[i] == addr){
                RS485_tx[i][0] = usb_rx[0]; // command
                RS485_tx[i][1] = usb_rx[3]; // value
                device_flag[i] = 1;
                if (addr == ADDR_BROADCAST)
                    cast_flag[i] = 1;
            }
        }
    }
}
