/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */


#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xbram.h"
#include "xparameters.h"
#include "xgpio.h"
#include "ff.h"
#include "xdevcfg.h"

#define FILE "test.txt"
XBram Bram;
XGpio Gpio;
static FATFS fatfs;
unsigned char TMPBUF[54]; //record bmp information

int Bram_ini(u16 DeviceId)
{
//	int Status;
	XBram_Config *ConfigPtr;

	/*
	 * Initialize the BRAM driver. If an error occurs then exit
	 */

	/*
	 * Lookup configuration data in the device configuration table.
	 * Use this configuration info down below when initializing this
	 * driver.
	 */
	ConfigPtr = XBram_LookupConfig(DeviceId);
	if (ConfigPtr == (XBram_Config *) NULL) {
		return XST_FAILURE;
	}


	return XST_SUCCESS;
}

int Bram_ini_Top(void)
{
	int bram0_status,bram1_status;
    bram0_status = Bram_ini(XPAR_BRAM_0_DEVICE_ID);
	if (bram0_status != XST_SUCCESS ) {
		xil_printf("Bram0 Initialization Failed\r\n");
	}
	else
	{
		xil_printf("Successfully Initialize Bram0\r\n");
	}

    bram1_status = Bram_ini(XPAR_BRAM_1_DEVICE_ID);
	if (bram1_status != XST_SUCCESS ) {
		xil_printf("Bram1 Initialization Failed\r\n");
	}
	else
	{
		xil_printf("Successfully Initialize Bram1\r\n");
	}
	return 0;
}

void bmp_load(char *bmp,u8 *frame,u32 stride)
{
    short y,x;
    short XImage,YImage;
    FIL fil;
    u32 iPixelAddr = 0;
    u32 BYTES_PIXEL;
    FRESULT res;
    unsigned char read_line_buf[1920 * stride];
    unsigned int br;

    res = f_open(&fil,bmp,FA_OPEN_EXISTING | FA_READ);
    if(res != FR_OK)
    {
        return;
    }
    res = f_read(&fil,&TMPBUF,54,&br);
    if(res != FR_OK)
    {
        return;
    }
    XImage = (unsigned short int)TMPBUF[19]*256 + TMPBUF[18]; //这里计算的是图像的Width和Height
    YImage = (unsigned short int)TMPBUF[23]*256 + TMPBUF[22];
    xil_printf("Width:%d   Height:%d\r\n",XImage,YImage);
    BYTES_PIXEL = YImage * stride;//这里BYTES_PIXEL指的是一列像素对应的Byte，YImage代表高度也就是图片一列像素点数
    //stride代表图片一个像素点需要8bits数据的地址数量，一个地址里面存一个Byte，因此一列像素对应需要YImage * stride
    //数量的Byte，对应需要这么多的地址空间。
    iPixelAddr = (YImage - 1) * stride;

    for(y = 0 ; y < YImage ; y++)
    {
    	f_read(&fil , read_line_buf , XImage * stride , &br);
    	for(x = 0 ; x < XImage ; x++)
    	{
    		frame[x * BYTES_PIXEL + iPixelAddr + 0] = read_line_buf[x * stride + 1];
    		frame[x * BYTES_PIXEL + iPixelAddr + 1] = read_line_buf[x * 3 + 1];
    		frame[x * BYTES_PIXEL + iPixelAddr + 2] = read_line_buf[x * 3 + 2];
    	}
    	iPixelAddr -= stride;
    }
    f_close(&fil);
}

void bmp_write(char * name,char *head_buf, u8 *data_buf , u32 stride)
{
	short x,y;
	short XImage;
	short YImage;
	FIL fil;
    u32 iPixelAddr = 0;
    FRESULT res;
    u32 BYTES_PIXEL;
    unsigned char write_line_buf[1920 * 3];
    unsigned int br;

    memset(&write_line_buf, 0 , 1920 * 3);

    res = f_open(&fil,name,FA_CREATE_ALWAYS | FA_WRITE);
    if(res != FR_OK)
    {
        return;
    }
    res = f_write(&fil, head_buf , 54 , &br);
    if(res != FR_OK)
    {
        return;
    }
    XImage = (unsigned short int)head_buf[19]*256 + head_buf[18]; //这里计算的是图像的Width和Height
    YImage = (unsigned short int)head_buf[23]*256 + head_buf[22];
 //   xil_printf("Width:%d   Height:%d\r\n",XImage,YImage);
    BYTES_PIXEL = YImage * stride;
    iPixelAddr = (YImage - 1) * stride;
    for(y = 0 ; y < YImage ; y++)
    {
    	for(x = 0 ; x < XImage ; x++)
    	{
    		write_line_buf[x*stride + 0] = data_buf[x * BYTES_PIXEL + iPixelAddr + 0];
    		write_line_buf[x*3 + 1] = data_buf[x * BYTES_PIXEL + iPixelAddr + 1];
    		write_line_buf[x*3 + 2] = data_buf[x * BYTES_PIXEL + iPixelAddr + 2];
    	}
        res = f_write(&fil, write_line_buf , XImage*stride , &br);
        if(res != FR_OK)
        {
            return;
        }
    	iPixelAddr -= stride;
    }
    f_close(&fil);
}

int Gpio_ini()
{
	int Status;

	/* Initialize the GPIO driver */
	Status = XGpio_Initialize(&Gpio, XPAR_GPIO_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Gpio Initialization Failed\r\n");
		return XST_FAILURE;
	}

// 0 output 1 input
	XGpio_SetDataDirection(&Gpio, 1 , 1);
	XGpio_SetDataDirection(&Gpio, 2 , 0);
	return 0;
}

int SD_Init()
{
    FRESULT rc;
    rc = f_mount(&fatfs,"",0);
    if(rc)
    {
        xil_printf("ERROR: f_mount returned %d\r\n",rc);
        return XST_FAILURE;
    }
    return XST_SUCCESS;
}

void SD_Init_Top(void)
{
	int rc;
    rc = SD_Init();
    if(XST_SUCCESS != rc)
    {
        xil_printf("fail to open SD Card~\n\r");
    }
    else
    {
        xil_printf("success to open SD Card~\n\r");
    }
}



int main()
{
	int Data,Data2;
	int Gpio_status;
	int membuf[28][28];
	static u32 stride = 24 / 8;  // stride 表示地址跨度，因为一个sd卡中的地址存储1Byte也就是8bits，那么24位的图片
	//一个像素点对应24bits也就是3Byte，所以地址跨度为3.
	u8 frameGet[28*28];
    init_platform();
    print("Hello World\n\r");
    Bram_ini_Top();
    SD_Init_Top();
    Gpio_ini();
	bmp_load("3.bmp",&frameGet,stride);
//	xil_printf("V%d V%d",frameGet[1],frameGet[2]);//输出两个像素点的值
//	bmp_write("5.bmp",&TMPBUF,&frameGet,stride);
	for (int j = 0;j < 28 ; j ++)
	{
	for (int i = 0;i < 28 ; i ++)
	{
		XBram_WriteReg(XPAR_BRAM_0_BASEADDR, (i + j * 28) * 4, *(frameGet+i*28*3+j*3));
	}
	}

//	for (int j = 0;j < 28 * 28 ; j ++)
//	{
//		Data = XBram_ReadReg(XPAR_BRAM_0_BASEADDR , j * 4);
//		xil_printf("%d,",Data);
//	}
//	Data = XBram_ReadReg(XPAR_BRAM_0_BASEADDR , 8);
//	XBram_WriteReg(XPAR_BRAM_0_BASEADDR, 4 , 256);
//	Data2 = XBram_ReadReg(XPAR_BRAM_0_BASEADDR , 8);
//	printf("%d   %d\n\r",Data,Data2);
	XGpio_DiscreteWrite(&Gpio , 2 , 1);
	Gpio_status = XGpio_DiscreteRead(&Gpio,1);
	while(Gpio_status == 0)
	{
		Gpio_status = XGpio_DiscreteRead(&Gpio,1);
	}
	XGpio_DiscreteWrite(&Gpio , 2 , 0);
	Data = XBram_ReadReg(XPAR_BRAM_1_BASEADDR , 0);
	Data2 = XBram_ReadReg(XPAR_BRAM_1_BASEADDR , 4);
	printf("result:%d\n\r",Data);

	print("Goodbye World\n\r");

    cleanup_platform();
    return 0;
}
