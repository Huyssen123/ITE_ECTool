#define  TOOLS_VER   "V0.2"
#define Vendor       "XXXX"

//*****************************************
// WinFlash EC tool Version : 0.2
// 1. Add only back eFlash function
// 2. Support 25C/25D port
//*****************************************

//*****************************************
// WinFlash EC tool Version : 0.1
// 1. First Release
//    a. Update IT8987 eFlash (128K)
//    b. Update SPI flash (64K, 128K)
//*****************************************

/* Copyright (C)Copyright 2005-2020 XXXX Telecom. All rights reserved.

   Author: Morgen Zhu
   
   Description:These functions of this file are reference only in the Windows!
   It can read/write ITE-EC RAM by 
   ----PM-port(62/66)
   ----KBC-port(60/64)
   ----EC-port(2E/2F or 4E/4F)
   ----Decicated I/O Port(301/302/303)
*/


// Using VS2012 X86 cmd tool to compilation
// For windows-32/64bit

//=======================================Include file ==============================================
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <windows.h>
#include <time.h>
#include <conio.h>
#include <wincon.h>
#include <Powrprof.h>
#include <Winbase.h>

using namespace std;
#include "winio.h"
//#pragma comment(lib,"WinIo.lib")       // For 32bit
#pragma comment(lib,"WinIox64.lib")    // For 64bit
//==================================================================================================

//========================================Type Define ==============================================
typedef unsigned char   BYTE;
typedef unsigned char   UINT8;
#define TRUE            1
#define FALSE           0
//==================================================================================================

//==========================The hardware port to read/write function================================
#define READ_PORT(port,data2)  GetPortVal(port, &data2, 1);
#define WRITE_PORT(port,data2) SetPortVal(port, data2, 1)
//==================================================================================================

//======================================== PM channel ==============================================
WORD PM_STATUS_PORT66          =0x66;
WORD PM_CMD_PORT66             =0x66;
WORD PM_DATA_PORT62            =0x62;
#define PM_OBF                  0x01
#define PM_IBF                  0x02
//------------wait EC PM channel port66 output buffer full-----/
void Wait_PM_OBF (void)
{
    DWORD data;
    READ_PORT(PM_STATUS_PORT66, data);
    while(!(data& PM_OBF))
    {
        READ_PORT(PM_STATUS_PORT66, data);
    }
}

//------------wait EC PM channel port66 input buffer empty-----/
void Wait_PM_IBE (void)
{
    DWORD data;
    READ_PORT(PM_STATUS_PORT66, data);
    while(data& PM_IBF)
    {
        READ_PORT(PM_STATUS_PORT66, data);
    }
}

//------------send command by EC PM channel--------------------/
void Send_cmd_by_PM(BYTE Cmd)
{
    Wait_PM_IBE();
    WRITE_PORT(PM_CMD_PORT66, Cmd);
    Wait_PM_IBE();
}

//------------send data by EC PM channel-----------------------/
void Send_data_by_PM(BYTE Data)
{
    Wait_PM_IBE();
    WRITE_PORT(PM_DATA_PORT62, Data);
    Wait_PM_IBE();
}

//-------------read data from EC PM channel--------------------/
BYTE Read_data_from_PM(void)
{
    DWORD data;
    Wait_PM_OBF();
    READ_PORT(PM_DATA_PORT62, data);
    return(data);
}
//--------------write EC RAM-----------------------------------/
void EC_WriteByte_PM(BYTE index, BYTE data)
{
    Send_cmd_by_PM(0x81);
    Send_data_by_PM(index);
    Send_data_by_PM(data);
}
//--------------read EC RAM------------------------------------/
BYTE EC_ReadByte_PM(BYTE index)
{
    BYTE data;
    Send_cmd_by_PM(0x80);
    Send_data_by_PM(index);
    data = Read_data_from_PM();
    return data;
}
//==================================================================================================

//================================KBC channel=======================================================
#define KBC_STATUS_PORT64         0x64
#define KBC_CMD_PORT64            0x64
#define KBC_DATA_PORT60           0x60
#define KBC_OBF                   0x01
#define KBC_IBF                   0x02
// wait EC KBC channel port64 output buffer full
void Wait_KBC_OBF (void)
{   
    DWORD data;
    READ_PORT(KBC_STATUS_PORT64, data);
    while(!(data& KBC_OBF))
    {
        READ_PORT(KBC_STATUS_PORT64, data);
    }
}

// wait EC KBC channel port64 output buffer empty
void Wait_KBC_OBE (void)
{
    DWORD data;
    READ_PORT(KBC_STATUS_PORT64, data);
    while(data& KBC_OBF)
    {
        READ_PORT(KBC_DATA_PORT60, data);
        READ_PORT(KBC_STATUS_PORT64, data);
    }
}

// wait EC KBC channel port64 input buffer empty
void Wait_KBC_IBE (void)
{
    DWORD data;
    READ_PORT(KBC_STATUS_PORT64, data);
    while(data& KBC_IBF)
    {
        READ_PORT(KBC_STATUS_PORT64, data);
    }
}

// send command by EC KBC channel
void Send_cmd_by_KBC (BYTE Cmd)
{
    Wait_KBC_OBE();
    Wait_KBC_IBE();
    WRITE_PORT(KBC_CMD_PORT64, Cmd);
    Wait_KBC_IBE();
}

// send data by EC KBC channel
void Send_data_by_KBC (BYTE Data)
{
    Wait_KBC_OBE();
    Wait_KBC_IBE();
    WRITE_PORT(KBC_DATA_PORT60, Data);
    Wait_KBC_IBE();
}

// read data from EC KBC channel
BYTE Read_data_from_KBC(void)
{
    DWORD data;
    Wait_KBC_OBF();
    READ_PORT(KBC_DATA_PORT60, data);
    return(data);
}
// Write EC RAM via KBC port(60/64)
void EC_WriteByte_KBC(BYTE index, BYTE data)
{
    Send_cmd_by_KBC(0x81);
    Send_data_by_KBC(index);
    Send_data_by_KBC(data);
}

// Read EC RAM via KBC port(60/64)
BYTE EC_ReadByte_KBC(BYTE index)
{
    Send_cmd_by_KBC(0x80);
    Send_data_by_KBC(index);
    return Read_data_from_KBC();
}
//==================================================================================================

//=======================================EC Direct Access interface=================================
//Port Config:
//  BADRSEL(0x200A) bit1-0  Addr    Data
//                  00      2Eh     2Fh
//                  01      4Eh     4Fh
//
//              01      4Eh     4Fh
//  ITE-EC Ram Read/Write Algorithm:
//  Addr    w   0x2E
//  Data    w   0x11
//  Addr    w   0x2F
//  Data    w   high byte
//  Addr    w   0x2E
//  Data    w   0x10
//  Addr    w   0x2F
//  Data    w   low byte
//  Addr    w   0x2E
//  Data    w   0x12
//  Addr    w   0x2F
//  Data    rw  value
UINT8 EC_ADDR_PORT = 0x4E;   // 0x2E or 0x4E
UINT8 EC_DATA_PORT = 0x4F;   // 0x2F or 0x4F
UINT8 High_Byte    = 0;
// Write EC RAM via EC port(2E/2F or 4E/4F)
void ECRamWrite_Direct(unsigned short iIndex, BYTE data)
{
    DWORD data1;
    data1 = data;
    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x11);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, High_Byte); // High byte

    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x10);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, iIndex);  // Low byte

    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x12);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, data1);
}

UINT8 ECRamRead_Direct(UINT8 iIndex)
{
    DWORD data;
    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x11);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, High_Byte); // High byte

    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x10);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, iIndex);  // Low byte

    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x12);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    READ_PORT(EC_DATA_PORT, data);
    return(data);
}


unsigned char ECRamReadExt_Direct(unsigned short iIndex)
{
    DWORD data;
    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x11);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, iIndex>>8); // High byte

    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x10);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, iIndex&0xFF);  // Low byte

    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x12);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    READ_PORT(EC_DATA_PORT, data);
    return(data);
}

void ECRamWriteExt_Direct(unsigned short iIndex, BYTE data)
{
    DWORD data1;
    data1 = data;
    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x11);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, iIndex>>8);    // High byte

    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x10);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, iIndex&0xFF);  // Low byte

    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x12);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, data1);
}
//==================================================================================================

//=======================================Decicated I/O Port Operation===============================
// Need EC code configuration and need BIOS decode I/O
#define HIGH_BYTE_PORT    0x301
#define LOW_BYTE_PORT     (HIGH_BYTE_PORT+1)
#define DATA_BYTE_PORT    (HIGH_BYTE_PORT+2)  // Decicated I/O Port

BYTE EC_ReadByte_DeIO(BYTE iIndex)
{
    DWORD data;
    SetPortVal(HIGH_BYTE_PORT, High_Byte, 1);
    SetPortVal(LOW_BYTE_PORT, iIndex, 1);
    GetPortVal(DATA_BYTE_PORT, &data, 1);
    return data;
}

void EC_WriteByte_DeIO(BYTE iIndex, BYTE data)
{
    SetPortVal(HIGH_BYTE_PORT, High_Byte, 1);
    SetPortVal(LOW_BYTE_PORT, iIndex, 1);
    SetPortVal(DATA_BYTE_PORT, data, 1);
}
//==================================================================================================




/**************************TYPE define**************************************************/
typedef unsigned char   BYTE;
#define TRUE         1
#define FALSE        0
#define TOOL_DEBUG   1
/**************************TYPE define end**********************************************/

/**************************Variable define**********************************************/
BYTE SPIFlashID[5];
BYTE *str1;
BYTE *str2;
BYTE *str3;
BYTE *str4;

BYTE  BackUp=0;     // Default is not backup
BYTE  FW_Size=0;   // Default is 64K
BYTE  ResetFlag=0;  // Default is not reset

FILE   *pECFile;
FILE   *pBackFile;
/**************************Variable define end******************************************/

//***************************The Command of EC addition SPI Flash************************/
// commonly used
#define SPICmd_WRSR            0x01   // Write Status Register
#define SPICmd_PageProgram     0x02   // To Program Page, 1-256 bytes data to be programmed
                                      // into memory in a single operation
#define SPICmd_READ            0x03   // Read Data Bytes from Memory at Normal ReadMode
#define SPICmd_WRDI            0x04   // Write diaable
#define SPICmd_RDSR            0x05   // Read Status Register
#define SPICmd_WREN            0x06   // Write Enable
#define SPICmd_FastRead        0x0B   // Read Data Bytes from Memory at Fast Read Mode
#define SPICmd_ChipErase       0xC7   // Chip Erase
#define SPICmd_JEDEC_ID        0x9F   // JEDEC ID READ command 
                                      //(manufacturer and product ID of devices)
#define SPICmd_EWSR            0x50   // Enable write Status Register

//PMC SPI Flash
#define SPICmd_PMCDeviceID1    0xAB   // Read Manufacturer and Product ID
#define SPICmd_PMCDeviceID2    0x90   // Read Manufacturer and Device ID

//Winband SPI Flash
#define SPICmd_WBDeviceID      0x4B   // Read unique ID

// ITE eFlash cmd
#define SPICmd_1KSectorErase   0xD7    // Sector Erase, 1K bytes
#define SPICmd_AAIBytePro      0xAF
#define SPICmd_AAIWordPro      0xAD
/****************************************************************************************/

/***************************The Flash manufacturer ID************************************/
#define SSTID                   0xBF
#define WinbondID               0xEF  // Verification
#define AtmelID                 0x9F
#define STID                    0x20
#define SpansionID              0x01
#define MXICID                  0xC2
#define AMICID                  0x37
#define EONID                   0x1C
#define ESMTID                  0x8C
#define PMCID                   0x7F  // Verification
#define GDID                    0xC8  // Verification
#define ITEID                   0xFF  // Verification
/****************************************************************************************/
typedef struct SPIDeviceInfo
{
    BYTE device_id;
    BYTE *device_string;
    BYTE *device_size_string;
}*pSPIDeviceInfo;

struct SPIFlashIDInfo
{
    BYTE vendor_id;
    BYTE *vendor_name;
    pSPIDeviceInfo device_info;
};

/*******************************SPI Flash Description************************************/
// You can add new flash described in here

// GigaDevice Flash
struct SPIDeviceInfo GDFlash[]=
{
    {0x10, (BYTE*)"GD25D05B", (BYTE*)"64K"         },
    {0x11, (BYTE*)"GD25D10B", (BYTE*)"128K"        },
    {0x00, NULL, NULL   }
};

// PMC Flash
struct SPIDeviceInfo PMCFlash[]=
{
    {0x20, (BYTE*)"Pm25LD512C", (BYTE*)"64K"            },
    {0x7B, (BYTE*)"Pm25LV512A", (BYTE*)"unknow size"    },
    {0x7C, (BYTE*)"Pm25LV010A", (BYTE*)"unknow size"    },
    {0x7D, (BYTE*)"Pm25LV020",  (BYTE*)"unknow size"    },
    {0x7E, (BYTE*)"Pm25LV040",  (BYTE*)"unknow size"    },
    {0x00, NULL, NULL   }
};

//Winbond Flash
struct SPIDeviceInfo WBFlash[]=
{
    {0x10, (BYTE*)"W25X05CL", (BYTE*)"8K"    },
    {0x11, (BYTE*)"W25X10CL", (BYTE*)"128K"  },
    {0x12, (BYTE*)"W25X20CL", (BYTE*)"256K"  },
    {0x14, (BYTE*)"W25Q80DV", (BYTE*)"1M"    },
    {0x15, (BYTE*)"W25Q16",   (BYTE*)"2M"    },
    {0x16, (BYTE*)"W25Q32",   (BYTE*)"4M"    },
    {0x00, NULL, NULL   }
};

// ITE e-flash
struct SPIDeviceInfo ITEFlash[]=
{
    {0xFE, (BYTE*)"ITE e-flash", (BYTE*)"128K"         },
    {0x00, NULL, NULL   }
};

struct SPIFlashIDInfo  aSPIFlashIDInfo[]=
{
    {   GDID,       (BYTE*)"GigaDevice" , (pSPIDeviceInfo)&GDFlash},
    {   WinbondID,  (BYTE*)"Winbond"    , (pSPIDeviceInfo)&WBFlash},
    {   PMCID,      (BYTE*)"PMC"        , (pSPIDeviceInfo)&PMCFlash},
    {   ITEID,      (BYTE*)"ITE"        , (pSPIDeviceInfo)&ITEFlash},
    {   0x00,       NULL,  NULL}
};
/*******************************SPI Flash Description end********************************/


/****************************************Flash operation-interface*************************/
#define  _EnterFollowMode   0x01
#define  _ExitFollowMode    0x05
#define  _SendCmd           0x02
#define  _SendByte          0x03
#define  _ReadByte          0x04

void FollowMode(BYTE mode)
{
    Send_cmd_by_PM(mode);
}

void SendCmdToFlash(BYTE cmd)
{
    Send_cmd_by_PM(_SendCmd);
    Send_cmd_by_PM(cmd);
}

void SendByteToFlash(BYTE data)
{
	Send_cmd_by_PM(_SendByte);
	Send_cmd_by_PM(data);
}

BYTE ReadByteFromFlash(void)
{
	Send_cmd_by_PM(_ReadByte);
	return(Read_data_from_PM());
}

void WaitFlashFree(void)
{
	FollowMode(_EnterFollowMode);
	SendCmdToFlash(SPICmd_RDSR);
	while(0x00 != (ReadByteFromFlash()&0x01));
	FollowMode(_ExitFollowMode);
}

void FlashWriteEnable(void)
{
	WaitFlashFree();
	FollowMode(_EnterFollowMode);
	SendCmdToFlash(SPICmd_WRSR);
	SendByteToFlash(0x00);
	
	FollowMode(_EnterFollowMode);
	SendCmdToFlash(SPICmd_WREN);
	
	FollowMode(_EnterFollowMode);
	SendCmdToFlash(SPICmd_RDSR);
	while(0x00 == (ReadByteFromFlash()&0x02));
	FollowMode(_ExitFollowMode);
}

void FlashWriteDisable(void)
{
	WaitFlashFree();
	FollowMode(_EnterFollowMode);
	SendCmdToFlash(SPICmd_WRDI);
	
	WaitFlashFree();
	FollowMode(_EnterFollowMode);
	SendCmdToFlash(SPICmd_RDSR);
	while(0x00 != (ReadByteFromFlash()&0x02));
	FollowMode(_ExitFollowMode);
}

// For ITE e-flash
void FlashStatusRegWriteEnable(void)
{
    WaitFlashFree();
    FollowMode(_EnterFollowMode);
    SendCmdToFlash(SPICmd_WREN);

    FollowMode(_EnterFollowMode);
    SendCmdToFlash(SPICmd_EWSR);

}
/****************************************Flash operation-interface*************************/

/******************************************************************************************/
void Read_SPIFlash_JEDEC_ID(void)
{
	BYTE index;
	
	WaitFlashFree();
	FollowMode(_EnterFollowMode);
	SendCmdToFlash(SPICmd_JEDEC_ID);
	for(index=0x00;index<3;index++)
	{
		SPIFlashID[index]=ReadByteFromFlash();
	}
	FollowMode(_ExitFollowMode);
}

void Show_FlashInfo(void)
{
	BYTE i,j;
	struct SPIDeviceInfo *ptmp;
	printf("Flash ID          : %X %X %X\n", SPIFlashID[0], SPIFlashID[1], SPIFlashID[2]);
	for(i=0;aSPIFlashIDInfo[i].vendor_id!=NULL;i++)
	{
		if(aSPIFlashIDInfo[i].vendor_id == SPIFlashID[0])
		{
			ptmp=aSPIFlashIDInfo[i].device_info;
			printf("Flash Manufacture : %s\n", aSPIFlashIDInfo[i].vendor_name);
			for(j=0;(ptmp+j)->device_id != 0;j++)
			{
				if((ptmp+j)->device_id == SPIFlashID[2])
				{
					printf("Flash Model       : %s\n", (ptmp+j)->device_string);
					printf("Flash Size        : %s\n", (ptmp+j)->device_size_string);
					return;
				}
			}
			printf("Unknow Flash Model\n");
			return;
		}
	}
	printf("Unknow Flash\n");
	return;
}

void Flash_BackUp(void)
{
    WORD counter;
    BYTE *strBK1;
    BYTE *strBK2;
    BYTE *strBK3;
    BYTE *strBK4;
	char tmp[64];
	
	time_t t = time(0);
    strftime( tmp, sizeof(tmp), "%Y-%m-%d[%X]",localtime(&t) );
	tmp[13] = '.';
	tmp[16] = '.';
	strcat(tmp,"eFlashBK.bin");
		
    printf("   Back up...       : ");
    if((pBackFile = fopen(tmp, "wb")) == NULL)
    {
        printf("Creat backup file wrong!\n");
        return;
    }
    strBK1=(UINT8 *)malloc(0x8000);
    strBK2=(UINT8 *)malloc(0x8000);
    strBK3=(UINT8 *)malloc(0x8000);
    strBK4=(UINT8 *)malloc(0x8000);
    
    WaitFlashFree();
    FollowMode(_EnterFollowMode);
    SendCmdToFlash(SPICmd_FastRead);
    SendByteToFlash(0x00);   // addr[24:15]
    SendByteToFlash(0x00);   // addr[8:14]
    SendByteToFlash(0x00);   // addr[0:7]
    SendByteToFlash(0x00);   // fast read dummy byte
    
    for(counter=0x0000;counter<0x8000;counter++)  //read 32k
    {
        strBK1[counter]=ReadByteFromFlash();
        if(0 == counter%0x800)
        {
            printf("#");
        }
    }
    
    for(counter=0x0000;counter<0x8000;counter++)  //read 32k
    {
        strBK2[counter]=ReadByteFromFlash();
        if(0 == counter%0x800)
        {
            printf("#");
        }
    }
    
    if(128==FW_Size)
    {
        for(counter=0x0000;counter<0x8000;counter++)  //read 32k
        {
            strBK3[counter]=ReadByteFromFlash();
            if(0 == counter%0x800)
                printf("#");
        }

        for(counter=0x0000;counter<0x8000;counter++)  //read 32k
        {
            strBK4[counter]=ReadByteFromFlash();
            if(0 == counter%0x800)
                printf("#");
        }
    }
    
    FollowMode(_ExitFollowMode);
    
    fwrite(strBK1,1,0x8000,pBackFile);
    fwrite(strBK2,1,0x8000,pBackFile);
    
    if(128==FW_Size)
    {
        fwrite(strBK3,1,0x8000,pBackFile);
        fwrite(strBK4,1,0x8000,pBackFile);
    }

    printf("   -- BackUp OK. \n\n");

    free(strBK1);
    free(strBK2);
    free(strBK3);
    free(strBK4);
    fclose(pBackFile);
}

//==================================================================================================
// For ITE e-flash
void Block_1K_Erase(BYTE addr2,BYTE addr1,BYTE addr0)
{
    FlashStatusRegWriteEnable();
    FlashWriteEnable();

    WaitFlashFree();
    FollowMode(_EnterFollowMode);
    SendCmdToFlash(SPICmd_1KSectorErase);
    SendByteToFlash(addr2);
    SendByteToFlash(addr1);
    SendByteToFlash(addr0);
    WaitFlashFree();
}

// For ITE 128K e-flash
void ITE_eFlash_Erase(void)
{
    unsigned int i,j;
    printf("   Eraseing...      : ");
    
    for(i=0; i<0x02; i++)           // 2*64K
    {
        for(j=0; j<0x100; j+=0x04)  // 64K
        {
            Block_1K_Erase((BYTE)i, (BYTE)j, 0x00);
            if(0 == j%0x8)
                printf("#");
        }
    }
    
    printf("   -- Erase OK. \n\n");
}

// For ITE 128K e-flash
BYTE ITE_eFlash_Erase_Verify(void)
{
    unsigned int counter;
    BYTE Dat;
    BYTE i;
    printf("   Erase Verify...  : ");
    
    FlashWriteDisable();
    WaitFlashFree();
    FollowMode(_EnterFollowMode);
    SendCmdToFlash(SPICmd_FastRead);
    SendByteToFlash(0x00);   // addr[24:15]
    SendByteToFlash(0x00);   // addr[8:14]
    SendByteToFlash(0x00);   // addr[0:7]
    SendByteToFlash(0x00);   // fast read dummy byte
    
    for(i=0; i<4; i++)
    {
        for(counter=0x0000;counter<0x8000;counter++)  // 32K
        {
            Dat=ReadByteFromFlash();
            if(Dat!=0xFF) //verify Byte is all 0xFF, otherwise  error
            {
                WaitFlashFree();
                printf(" Dat is: %d \n",Dat);
                printf(" Counter is: %d \n",counter);
                printf(" Block is: %d \n",i);
                printf(" -- Verify Fail. \n");
                return(FALSE);
            }
            if(0 == counter%0x800)
                printf("#");
        }
    }
    
    WaitFlashFree();
    printf("   -- Verify OK. \n\n");
    return(TRUE);
}

// Only for 128KB ITE eFlash program
// e-Flash program:
// 1. send start address
// 2. send word program command(ITE e-flash only support this command)
// 3. send tow byte
// 4. wait flsh free
// 5. got to setp 1
void ITE_eFlash_Program(void)
{
    unsigned int i;
    printf("   Programing...    : ");

    FlashStatusRegWriteEnable();
    FlashWriteEnable();
    FollowMode(_EnterFollowMode);
    SendCmdToFlash(SPICmd_AAIWordPro);
    SendByteToFlash(0x00);
    SendByteToFlash(0x00);
    SendByteToFlash(0x00);
    SendByteToFlash(str1[0]);
    SendByteToFlash(str1[1]);
    WaitFlashFree();
    
    for(i=2; i<0x8000; i+=2)
    {
        FollowMode(_EnterFollowMode);
        SendCmdToFlash(SPICmd_AAIWordPro);
        SendByteToFlash(str1[i]);
        SendByteToFlash(str1[i+1]);
        WaitFlashFree();
        if(0 == i%0x800)
            printf("#");
    }
    
    for(i=0; i<0x8000; i+=2)
    {
        FollowMode(_EnterFollowMode);
        SendCmdToFlash(SPICmd_AAIWordPro);
        SendByteToFlash(str2[i]);
        SendByteToFlash(str2[i+1]);
        WaitFlashFree();
        if(0 == i%0x800)
            printf("#");
    }
    
    for(i=0; i<0x8000; i+=2)
    {
        FollowMode(_EnterFollowMode);
        SendCmdToFlash(SPICmd_AAIWordPro);
        SendByteToFlash(str3[i]);
        SendByteToFlash(str3[i+1]);
        WaitFlashFree();
        if(0 == i%0x800)
            printf("#");
    }
    
    for(i=0; i<0x8000; i+=2)
    {
        FollowMode(_EnterFollowMode);
        SendCmdToFlash(SPICmd_AAIWordPro);
        SendByteToFlash(str4[i]);
        SendByteToFlash(str4[i+1]);
        WaitFlashFree();
        if(0 == i%0x800)
            printf("#");
    }

    FlashWriteDisable();
    printf("   -- Programing OK. \n\n");
}
//==================================================================================================

void Erase_Flash(void)
{
    WORD counter;
    
    printf("   Eraseing...      : ");
    
    FlashWriteEnable();
    WaitFlashFree();
    FollowMode(_EnterFollowMode);
    SendCmdToFlash(SPICmd_ChipErase);
    FollowMode(_ExitFollowMode);
    
    for(counter=0x00;counter<0x20;counter++)
    {
        printf("#");
    }
    printf("   -- Erase OK. \n\n");
}

BYTE Erase_Flash_Verify()
{
    WORD counter;
    printf("   Erase Verify...  : ");
    
    FlashWriteDisable();
    WaitFlashFree();
    FollowMode(_EnterFollowMode);
    SendCmdToFlash(SPICmd_FastRead);
    SendByteToFlash(0x00);   // addr[24:15]
    SendByteToFlash(0x00);   // addr[8:14]
    SendByteToFlash(0x00);   // addr[0:7]
    SendByteToFlash(0x00);   // fast read dummy byte
    
    for(counter=0x0000;counter<0x8000;counter++)
    {
        if(ReadByteFromFlash()!=0xFF) //verify Byte is all 0xFF, otherwise  error
        {
            WaitFlashFree();
            printf(" -- Verify Fail 1. \n");
            return(FALSE);
        }
        if(0 == counter%0x800)
        {
            printf("#");
        }
    }
    
    for(counter=0x0000;counter<0x8000;counter++)
    {
        if(ReadByteFromFlash()!=0xFF) //verify Byte is all 0xFF, otherwise  error
        {
            WaitFlashFree();
            printf(" -- Verify Fail 2. \n");
            return(FALSE);
        }
        if(0 == counter%0x800)
        {
            printf("#");
        }
    }
    
    if(128==FW_Size)
    {
        for(counter=0x0000;counter<0x8000;counter++)
        {
            if(ReadByteFromFlash()!=0xFF) //verify Byte is all 0xFF, otherwise  error
            {
                WaitFlashFree();
                printf(" -- Verify Fail 3. \n");
                return(FALSE);
            }
            if(0 == counter%0x800)
            {
                printf("#");
            }
        }
    
        for(counter=0x0000;counter<0x8000;counter++)
        {
            if(ReadByteFromFlash()!=0xFF) //verify Byte is all 0xFF, otherwise  error
            {
                WaitFlashFree();
                printf(" -- Verify Fail 4. \n");
                return(FALSE);
            }
            if(0 == counter%0x800)
            {
                printf("#");
            }
        }
    }

    WaitFlashFree();
    printf("   -- Verify OK. \n\n");
    return(TRUE);
}

BYTE Read_ECFile_ToBuf(char *FileName)
{
    WORD len1;
    
    if((pECFile = fopen((const char*)FileName, "rb")) == NULL)
    {
        printf("open EC file wrong!\n");
        return(FALSE);
    }
    
    len1=fread(str1, 1, 0x8000, pECFile);
    printf("Read Byte: %X\n", len1);
    if(0x8000 != len1)
    {
        printf("Read EC File error 1\n");
        return(FALSE);
    }
    
    len1=fread(str2, 1, 0x8000, pECFile);
    printf("Read Byte: %X\n", len1);
    if(0x8000 != len1)
    {
        printf("Read EC File error 2\n");
        return(FALSE);
    }
    
    if(128==FW_Size)
    {
        len1=fread(str3,1,0x8000,pECFile);
        printf("Read Byte: %X\n", len1);
        if(0x8000 != len1)
        {
            printf("Read EC File error 3\n");
            return(FALSE);
        }
        
        len1=fread(str4,1,0x8000,pECFile);
        printf("Read Byte: %X\n", len1);
        if(0x8000 != len1)
        {
            printf("Read EC File error 4\n");
            return(FALSE);
        }
    }
    fclose(pECFile);
    
    return(TRUE);
 
}

// only for 64KB Flash program
void Program_Flash(void)
{
    WORD counter;
    WORD counter2;
    WORD counter3;
    printf("   Programing...    : ");
    
    for(counter2=0x0000;counter2<0x80;counter2++)   // 32Kb    // 128 block
    {
        FlashWriteEnable();
        WaitFlashFree();
        FollowMode(_EnterFollowMode);
        SendCmdToFlash(SPICmd_PageProgram);
        SendByteToFlash(0x00);
        SendByteToFlash(0x00+(BYTE)counter2);
        SendByteToFlash(0x00);
        for(counter=0x0000;counter<0x100;counter++)   // block 256Byte
        {
            SendByteToFlash(str1[counter+(counter2<<8)]);
        }
        WaitFlashFree();
        if(0 == counter2%0x8)
        {
            printf("#");
        }
    }

    for(counter2=0x0000;counter2<0x80;counter2++)   // 32Kb
    {
        FlashWriteEnable();
        WaitFlashFree();
        FollowMode(_EnterFollowMode);
        SendCmdToFlash(SPICmd_PageProgram);
        SendByteToFlash(0x00);
        SendByteToFlash(0x00+(BYTE)(counter2+0x80));
        SendByteToFlash(0x00);
        for(counter=0x0000;counter<0x100;counter++)
        {
            SendByteToFlash(str2[counter+(counter2<<8)]);
        }
        WaitFlashFree();
        if(0 == counter2%0x8)
        {
            printf("#");
        }
    }
    
    if(128==FW_Size)
    {
        counter3 = 1;
        for(counter2=0x0000;counter2<0x80;counter2++)   // 32Kb    // 128 block
        {
            FlashWriteEnable();
            WaitFlashFree();
            FollowMode(_EnterFollowMode);
            SendCmdToFlash(SPICmd_PageProgram);
            SendByteToFlash(counter3);
            SendByteToFlash(0x00+(BYTE)counter2);
            SendByteToFlash(0x00);
            for(counter=0x0000;counter<0x100;counter++)   // block 256Byte
            {
                SendByteToFlash(str3[counter+(counter2<<8)]);
            }
            WaitFlashFree();
            if(0 == counter2%0x8)
                printf("#");
        }

        for(counter2=0x0000;counter2<0x80;counter2++)   // 32Kb
        {
            FlashWriteEnable();
            WaitFlashFree();
            FollowMode(_EnterFollowMode);
            SendCmdToFlash(SPICmd_PageProgram);
            SendByteToFlash(counter3);
            SendByteToFlash(0x00+(BYTE)(counter2+0x80));
            SendByteToFlash(0x00);
            for(counter=0x0000;counter<0x100;counter++)
            {
                SendByteToFlash(str4[counter+(counter2<<8)]);
            }
            WaitFlashFree();
            if(0 == counter2%0x8)
                printf("#");
        }
    }
    
    FlashWriteDisable();
    printf("   -- Programing OK. \n\n");
}

BYTE Program_Flash_Verify(void)
{
    WORD counter;
    printf("   Program Verify...: ");

    FlashWriteDisable();
    WaitFlashFree();
    FollowMode(_EnterFollowMode);
    SendCmdToFlash(SPICmd_FastRead);
    SendByteToFlash(0x00);   // addr[24:15]
    SendByteToFlash(0x00);   // addr[8:14]
    SendByteToFlash(0x00);   // addr[0:7]
    SendByteToFlash(0x00);   // fast read dummy byte
    
    for(counter=0x0000;counter<0x8000;counter++)
    {
        if(ReadByteFromFlash()!=str1[counter]) //verify Byte
        {
            WaitFlashFree();
            printf(" -- Verify Fail. \n"); 
            return(FALSE);
        }
        if(0 == counter%0x800)
        {
            printf("#");
        }
    }
    
    for(counter=0x0000;counter<0x8000;counter++)
    {
        if(ReadByteFromFlash()!=str2[counter]) //verify Byte is all 0xFF, otherwise  error
        {
            WaitFlashFree();
            printf(" -- Verify Fail. \n"); 
            return(FALSE);
        }
        if(0 == counter%0x800)
        {
            printf("#");
        }
    }
    
    if(128==FW_Size)
    {
        for(counter=0x0000;counter<0x8000;counter++)
        {
            if(ReadByteFromFlash()!=str3[counter]) //verify Byte
            {
                WaitFlashFree();
                printf(" -- Verify Fail. \n");
                return(FALSE);
            }
            if(0 == counter%0x800)
                printf("#");
        }

        for(counter=0x0000;counter<0x8000;counter++)
        {
            if(ReadByteFromFlash()!=str4[counter]) //verify Byte is all 0xFF, otherwise  error
            {
                WaitFlashFree();
                printf(" -- Verify Fail. \n");
                return(FALSE);
            }
            if(0 == counter%0x800)
                printf("#");
        }
    }

	WaitFlashFree();
	printf("   -- Verify OK. \n");
	return(TRUE);
}

BYTE CheckECFile(void)
{
    BYTE i=0;
    BYTE VersionString[] = "ITE EC-V14.0 CF03-T";  // VersionString[] this string match in the bin file
    while (*(VersionString+i))
    {
        if (*(str1+0x50+i) == *(VersionString+i))
        {
            i=i+1;
            continue;
        }
        printf("\nThis bin file doesn't match the current project!\n");
        return 0;
    }
    return 1;
}


void Show_Version(void)
{
  printf("*************************************************************\n");
  printf("**            EC Flash Utility Version : %s              **\n",TOOLS_VER);
  printf("**                                                         **\n");
  printf("**   (C)Copyright %s Telecom Technology Co.,Ltd        **\n",Vendor);
  printf("**                 All Rights Reserved.                    **\n");
  printf("**                                                         **\n");
  printf("**                 Modified by Morgen                      **\n");
  printf("*************************************************************\n");
}

void help(void)
{
    printf("=======================================================\n");
    printf("=         ITE EC Flash Utility Version : %s         =\n",TOOLS_VER);
    printf("=   (C)Copyright %s Telecom Technology Co.,Ltd    =\n",Vendor);
    printf("=                 All Rights Reserved.                =\n");
    printf("=                             --%s           =\n", __DATE__);
    printf("=                                                     =\n");
    printf("=                                                     =\n");
    printf("=  FTEFI [/128] ... [/R] xxxx.bin                     =\n");
    printf("=   /B      Back up flash data                        =\n");
    printf("=   /64     Size of FW is 64K                         =\n");
    printf("=   /128    Size of FW is 128K(Default)               =\n");
    printf("=   /R      Reset EC after update                     =\n");
    printf("=   /686C   update e-flash by port 686C(Default 6266) =\n");
	printf("=   /25C update e-flash by port 25C/25D(Default 6266) =\n");
    printf("=======================================================\n");
}

int main( int argc, char *argv[] )
{
    bool IOInitOK;
    BYTE eFlashBackOnly=0;
    unsigned int i;
    
    if(1 == argc)
    {
        help();
        goto ArgcError;
    }
    BackUp=0;     // Default is not backup
    FW_Size=128;  // Default is 128K
    ResetFlag=0;  // Default is not reset
    PM_STATUS_PORT66 =0x66;
    PM_CMD_PORT66    =0x66;
    PM_DATA_PORT62   =0x62; // Default port is 6266
    
    for(i=1; i<argc; i++)
    {
        if(!strcmp("/B",argv[i]) || !strcmp("/b",argv[i]))
        {
            BackUp=1;  // Back up the chip FW when update FW
        }
        
        if(!strcmp("/R",argv[i]) || !strcmp("/r",argv[i]))
        {
            ResetFlag=1;  // Flag reset ec after update
        }
        
        if(!strcmp("/64",argv[i]))
        {
            FW_Size=64;
        }
        
        if(!strcmp("/128",argv[i]))
        {
            FW_Size=128;
        }
        
        if(!strcmp("/686C",argv[i]) || !strcmp("/686c",argv[i]))
        {
            PM_STATUS_PORT66 =0x6C;
            PM_CMD_PORT66    =0x6C;
            PM_DATA_PORT62   =0x68; // Set port is 686C
        }
        
        if(!strcmp("/25C",argv[i]) || !strcmp("/25c",argv[i]))
        {
            PM_STATUS_PORT66 =0x25D;
            PM_CMD_PORT66    =0x25D;
            PM_DATA_PORT62   =0x25C; // Set port is 25C/25D
        }
    }
    
    // init winio for IO access
    IOInitOK = InitializeWinIo();
    if(IOInitOK)
    {
        printf("WinIo OK.\n");
    }
    else
    {
        printf("Error during initialization of WinIo.\n");
        goto IOError;
    }
    
    // load EC file
    // malloc 32K *n memory
    str1=(BYTE *)malloc(0x8000);
    str2=(BYTE *)malloc(0x8000);
    str3=(BYTE *)malloc(0x8000);
    str4=(BYTE *)malloc(0x8000);
    
    if(FALSE == Read_ECFile_ToBuf(argv[i-1]))
    {
        if(0==BackUp)
        {
            printf("Read EC file Fail\n");
            goto ReadFileError;
        }
        eFlashBackOnly = 1;
    }
    
    // clrscr(); clear screen. only in the TC (conio.h)
    // other C is  system("cls")  (stdlib.h);
    system("cls");

    if(TOOL_DEBUG) printf("Send 0xDC command to EC at IO[%X]\n", PM_CMD_PORT66);
    // Command for EC enter flash function
    Send_cmd_by_PM(0xDC);    // port 66H call EC function: ITE_Flash_Utility()

    if(TOOL_DEBUG) printf("Wait 0x33 data from IO[%X]\n",PM_DATA_PORT62);
    // ACK, 62port, EC will send data 0x33 to Host
    // from Host Interface PM Channel 1 Data Out Port
    while(0x33 != Read_data_from_PM());
    if(TOOL_DEBUG) printf("Received 0x33\n");
    
    if(TOOL_DEBUG) printf("Read JEDEC_ID\n");
    // Read Flash JEDEC ID
    Read_SPIFlash_JEDEC_ID();
    if(TOOL_DEBUG) printf("Read JEDEC_ID OK\n");
    
    // show Flash Info
    Show_Version();      // display tools version
    Show_FlashInfo();    // display vendor/device/size info
	if(1==eFlashBackOnly)
	{
		printf("EC File           : %s \n\n", "unknows");
	}
	else
	{
		printf("EC File           : %s \n\n", argv[i-1]);
	}

    // back EC Flash Data
    if(BackUp)
    {
        Flash_BackUp();       // read flash(64k) data to back
		
		// Only back eFlash
		if(1==eFlashBackOnly)
		{
			goto end;
		}
    }

    // ITE e-flash
    if(0xFF == SPIFlashID[0])
    {
        // flash erase
        ITE_eFlash_Erase();
        
        if(FALSE == ITE_eFlash_Erase_Verify())
        {
            printf("For ITE e-Flash\n");
            printf("Verify Erase fail,please choose other ways of erase and program\n");
            goto end;
        }
        
        // flash program
        ITE_eFlash_Program();
    }
    // SPI flash
    else
    {
        // SPI flash erase
        Erase_Flash();
        
        // flash erase verify
        if(FALSE ==Erase_Flash_Verify())
        {
            printf("For SPI Flash\n");
            printf("Verify Erase fail,please choose other ways of erase and program\n");
            goto end;
        }
        
        // program flash
        Program_Flash();
    }

    // program verify
    if(FALSE == Program_Flash_Verify())
    {
        printf("Verify program fail,please choose other ways of erase and program\n");
        goto end;
    }

end:
    FollowMode(_ExitFollowMode);
    if(ResetFlag)
    {
        printf("Please wait, EC will reset\n");
        _sleep(2000);
        Send_cmd_by_PM(0xFE);           // exit FlashECCode and reset EC
    }
    else
    {
        _sleep(2000);
        Send_cmd_by_PM(0xFC);           // exit FlashECCode
    }

end2:
	free(str1);
	free(str2);
end1:
	ShutdownWinIo();
	Sleep(1);
	return 0;

ReadFileError:
    free(str1);
    free(str2);
    free(str3);
    free(str4);
IOError:
	ShutdownWinIo();
ArgcError:
	printf("Please try again\n");
	return 1;
}