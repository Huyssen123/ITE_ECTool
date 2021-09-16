#define  TOOLS_VER   "V1.0"

//******************************************************************************
// BatteryTool Version : 1.0
//******************************************************************************


/* Copyright (C)Copyright 2005-2020 ZXQ Telecom. All rights reserved.

   Author: Morgen Zhu
   
   Description:These functions of this file are reference only in the Windows!
   It can read/write ITE-EC RAM by 
   ----PM-port(62/66)
   ----KBC-port(60/64)
   ----EC-port(2E/2F or 4E/4F)
   ----Decicated I/O Port(301/302/303)

    Using VS2012 X86 cmd tool to compilation
    For windows-32/64bit
*/


//=============================== Include file =================================
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <windows.h>
#include <time.h>
#include <conio.h>
#include <wincon.h>
#include <Winbase.h>

using namespace std;
#include "winio.h"
//#pragma comment(lib,"WinIo.lib")       // For 32bit
#pragma comment(lib,"WinIox64.lib")    // For 64bit
//==============================================================================


//=============================== Type Define ==================================
typedef unsigned char   BYTE;
typedef unsigned char   UINT8;
#define TRUE            1
#define FALSE           0
//==============================================================================


//=================== The hardware port to read/write function =================
#define READ_PORT(port,data2)  GetPortVal(port, &data2, 1);
#define WRITE_PORT(port,data2) SetPortVal(port, data2, 1)
//==============================================================================


//============================= PM channel =====================================
UINT8 PM_STATUS_PORT66          =0x66;
UINT8 PM_CMD_PORT66             =0x66;
UINT8 PM_DATA_PORT62            =0x62;
#define PM_OBF                    0x01
#define PM_IBF                    0x02

/* wait EC PM channel port66 output buffer full */
void Wait_PM_OBF (void)
{
    DWORD data;
    READ_PORT(PM_STATUS_PORT66, data);
    while(!(data & PM_OBF))
    {
        READ_PORT(PM_STATUS_PORT66, data);
    }
}

/* wait EC PM channel port66 input buffer empty */
void Wait_PM_IBE (void)
{
    DWORD data;
    READ_PORT(PM_STATUS_PORT66, data);
    while(data & PM_IBF)
    {
        READ_PORT(PM_STATUS_PORT66, data);
    }
}

/* send command by EC PM channel */
void Send_cmd_by_PM(BYTE Cmd)
{
    Wait_PM_IBE();
    WRITE_PORT(PM_CMD_PORT66, Cmd);
    Wait_PM_IBE();
}

/* send data by EC PM channel */
void Send_data_by_PM(BYTE Data)
{
    Wait_PM_IBE();
    WRITE_PORT(PM_DATA_PORT62, Data);
    Wait_PM_IBE();
}

/* read data from EC PM channel */
BYTE Read_data_from_PM(void)
{
    DWORD data;
    Wait_PM_OBF();
    READ_PORT(PM_DATA_PORT62, data);
    return(data);
}

/* write EC RAM */
void EC_WriteByte_PM(BYTE index, BYTE data)
{
    Send_cmd_by_PM(0x81);
    Send_data_by_PM(index);
    Send_data_by_PM(data);
}

/* read EC RAM */
BYTE EC_ReadByte_PM(BYTE index)
{
    BYTE data;
    Send_cmd_by_PM(0x80);
    Send_data_by_PM(index);
    data = Read_data_from_PM();
    return data;
}
//==============================================================================


//=============================== KBC channel ==================================
UINT8 KBC_STATUS_PORT64         =0x64;
UINT8 KBC_CMD_PORT64            =0x64;
UINT8 KBC_DATA_PORT60           =0x60;
#define KBC_OBF                   0x01
#define KBC_IBF                   0x02

/* wait EC KBC channel port64 output buffer full */
void Wait_KBC_OBF (void)
{
    DWORD data;
    READ_PORT(KBC_STATUS_PORT64, data);
    while(!(data & KBC_OBF))
    {
        READ_PORT(KBC_STATUS_PORT64, data);
    }
}

/* wait EC KBC channel port64 output buffer empty */
void Wait_KBC_OBE (void)
{
    DWORD data;
    READ_PORT(KBC_STATUS_PORT64, data);
    while(data & KBC_OBF)
    {
        READ_PORT(KBC_DATA_PORT60, data);
        READ_PORT(KBC_STATUS_PORT64, data);
    }
}

/* wait EC KBC channel port64 input buffer empty */
void Wait_KBC_IBE (void)
{
    DWORD data;
    READ_PORT(KBC_STATUS_PORT64, data);
    while(data & KBC_IBF)
    {
        READ_PORT(KBC_STATUS_PORT64, data);
    }
}

/* send command by EC KBC channel */
void Send_cmd_by_KBC (BYTE Cmd)
{
    Wait_KBC_OBE();
    Wait_KBC_IBE();
    WRITE_PORT(KBC_CMD_PORT64, Cmd);
    Wait_KBC_IBE();
}

/* send data by EC KBC channel */
void Send_data_by_KBC (BYTE Data)
{
    Wait_KBC_OBE();
    Wait_KBC_IBE();
    WRITE_PORT(KBC_DATA_PORT60, Data);
    Wait_KBC_IBE();
}

/* read data from EC KBC channel */
BYTE Read_data_from_KBC(void)
{
    DWORD data;
    Wait_KBC_OBF();
    READ_PORT(KBC_DATA_PORT60, data);
    return(data);
}

/* Write EC RAM via KBC port(60/64) */
void EC_WriteByte_KBC(BYTE index, BYTE data)
{
    Send_cmd_by_KBC(0x81);
    Send_data_by_KBC(index);
    Send_data_by_KBC(data);
}

/* Read EC RAM via KBC port(60/64) */
BYTE EC_ReadByte_KBC(BYTE index)
{
    Send_cmd_by_KBC(0x80);
    Send_data_by_KBC(index);
    return Read_data_from_KBC();
}
//==============================================================================


//======================= ITE EC Direct Access interface =======================
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
UINT8 EC_ADDR_PORT              = 0x4E;   // 0x2E or 0x4E
UINT8 EC_DATA_PORT              = 0x4F;   // 0x2F or 0x4F

// Read EC RAM via EC port(2E/2F or 4E/4F)
UINT8 EC_RAM_Read_Direct(unsigned short iIndex)
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

// Write EC RAM via EC port(2E/2F or 4E/4F)
void EC_RAM_Write_Direct(unsigned short iIndex, BYTE data)
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
//==============================================================================


//=======================   Decicated I/O Port Operation =======================
// Need EC code configuration and need BIOS decode I/O
#define HIGH_BYTE_PORT    0x301
#define LOW_BYTE_PORT     (HIGH_BYTE_PORT+1)
#define DATA_BYTE_PORT    (HIGH_BYTE_PORT+2)  // Decicated I/O Port
UINT8   High_Byte =0;

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
//==============================================================================


//======================== Console control interface ===========================
#define EFI_BLACK                 0x00
#define EFI_BLUE                  0x01
#define EFI_GREEN                 0x02
#define EFI_RED                   0x04
#define EFI_BRIGHT                0x08

#define EFI_CYAN                  (EFI_BLUE | EFI_GREEN)
#define EFI_MAGENTA               (EFI_BLUE | EFI_RED)
#define EFI_BROWN                 (EFI_GREEN | EFI_RED)
#define EFI_LIGHTGRAY             (EFI_BLUE | EFI_GREEN | EFI_RED)
#define EFI_LIGHTBLUE             (EFI_BLUE | EFI_BRIGHT)
#define EFI_LIGHTGREEN            (EFI_GREEN | EFI_BRIGHT)
#define EFI_LIGHTCYAN             (EFI_CYAN | EFI_BRIGHT)
#define EFI_LIGHTRED              (EFI_RED | EFI_BRIGHT)
#define EFI_LIGHTMAGENTA          (EFI_MAGENTA | EFI_BRIGHT)
#define EFI_YELLOW                (EFI_BROWN | EFI_BRIGHT)
#define EFI_WHITE                 (EFI_BLUE | EFI_GREEN | EFI_RED | EFI_BRIGHT)

void SetTextColor(UINT8 TextColor, UINT8 BackColor)
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hOut, (TextColor|(BackColor<<4)));
}

void SetPosition_X_Y(UINT8 PositionX, UINT8 PositionY)
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos={PositionX,PositionY};
    SetConsoleCursorPosition(hOut, pos);
}

void SetToolCursor()
{
    system("cls");
    system("color 07");
    
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO CursorInfo;  
    GetConsoleCursorInfo(handle, &CursorInfo);
    CursorInfo.bVisible = false;
    SetConsoleCursorInfo(handle, &CursorInfo);
}

void ClearToolCursor()
{
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);  
    CONSOLE_CURSOR_INFO CursorInfo;  
    GetConsoleCursorInfo(handle, &CursorInfo);
    CursorInfo.bVisible = true;
    SetConsoleCursorInfo(handle, &CursorInfo);
    
    SetTextColor(EFI_LIGHTGRAY, EFI_BLACK);
    system("cls");
}
//==============================================================================


//========================== EC RAM Access Interface ===========================
//  Example:
//  EC_RAM_Write_Direct(0x51,0x90);
//  EC_WriteByte_KBC(0x52,0x91);
//  EC_WriteByte_PM(0x53,0x93);
//  EC_WriteByte_DeIO(0x54,0x94);
//  printf("%d\n", ECRamRead_Direct(0x51));
//  printf("%d\n", EC_ReadByte_KBC(0x52));
//  printf("%d\n", EC_ReadByte_PM(0x53));
//  printf("%d\n", EC_ReadByte_DeIO(0x54));

// 0x301
//EC_ReadByte_DeIO
//EC_WriteByte_DeIO

//0x2E
//EC_RAM_Read_Direct
//EC_RAM_Write_Direct

//60/64
//EC_ReadByte_KBC
//EC_WriteByte_KBC

//62/66
//EC_ReadByte_PM
//EC_WriteByte_PM

#define  EC_RAM_WRITE  EC_RAM_Write_Direct
#define  EC_RAM_READ   EC_RAM_Read_Direct
//==============================================================================


//====================================== Tool info =============================
#define  TOOLS_NAME  "IT8987 FanView"
#define  ITE_IC      "ITE8987"
#define  CopyRight   "(C)Copyright 2005-2020 ZXQ Telecom."
unsigned char  EC_RAM_RW_Flag=0;
unsigned char  IO_RW_Flag=0;
//==============================================================================



void ToolInit(void)
{
    UINT8 EC_CHIP_ID1;
    UINT8 EC_CHIP_ID2;
    UINT8 REG_1060;
    
    /*
    ITE IT-557x chip is DLM architecture for EC  RAM and It's support 6K/8K RAM.
    If used RAM less than 4K, you can access EC RAM form 0x000--0xFFF by 4E/4F
    IO port.
    If used RAM more than 4K, RAM address change to 0xC000.
    If you want to access EC RAM by 4E/4F IO port, you must set as follow register first
    REG_1060[BIT7] = 1
    */
    
    EC_CHIP_ID1 = EC_RAM_READ(0x2000);
    EC_CHIP_ID2 = EC_RAM_READ(0x2001);

    if(0xFF==EC_CHIP_ID1)
    {
        EC_ADDR_PORT = 0x2E;
        EC_DATA_PORT = 0x2F;
        EC_CHIP_ID1 = EC_RAM_READ(0x2000);
        EC_CHIP_ID2 = EC_RAM_READ(0x2001);
    }
    printf("CHIP_ID=%X%X\n", EC_CHIP_ID1, EC_CHIP_ID2);
    
    if(0x55==EC_CHIP_ID1)
    {
        REG_1060 = EC_RAM_READ(0x1060);
        REG_1060 = REG_1060 | 0x80;
        EC_RAM_WRITE(0x1060, REG_1060);
    }
}

void Help(void)
{
    printf("===============================================================\n");
    printf("=                 ITE EC Utility Version : %-4s               =\n",TOOLS_VER);
    printf("=                                     --%-16s      =\n", __DATE__);
    printf("=                                                             =\n");
    printf("=    /W  /[6266/686C/4E4F/2E2F]  Address(Hex)  Data(Hex)      =\n");
    printf("=    /R  /[6266/686C/4E4F/2E2F]  Address(Hex)                 =\n");
    printf("=                                                             =\n");
    printf("===============================================================\n");
}


UINT8 Set_Access_IO(int Argc, char **Argv)
{
    IO_RW_Flag = 0;

    if(Argc<3)
    {
        printf("Please input access IO port\n");
        return 0;
    }
    else if(!strcmp("/6266", Argv[2]))
    {
        PM_STATUS_PORT66 =0x66;
        PM_CMD_PORT66    =0x66;
        PM_DATA_PORT62   =0x62; // Set port is 686C
    }
    else if(!strcmp("/686C", Argv[2]))
    {
        PM_STATUS_PORT66 =0x6C;
        PM_CMD_PORT66    =0x6C;
        PM_DATA_PORT62   =0x68; // Set port is 686C
    }
    else if(!strcmp("/4E4F", Argv[2]))
    {
        EC_ADDR_PORT = 0x4E;
        EC_DATA_PORT = 0x4F;
        IO_RW_Flag = 1;
    }
    else if(!strcmp("/2E2F", Argv[2]))
    {
        EC_ADDR_PORT = 0x2E;
        EC_DATA_PORT = 0x2F;
        IO_RW_Flag = 1;
    }
    else
    {
        printf("Access IO port input error\n");
        return 0;
    }

    return 1;
}

void RAM_Access(int Argc, char **Argv)
{
    unsigned short RAM_Address;
    unsigned short RAM_Data;
    char *str;
    
    if(Argc>3)
    {
        RAM_Address = (unsigned short)strtol(Argv[3], &str, 16);
    }
    else
    {
        printf("Please input read or write address\n");
        return;
    }
    
    // Write
    if(1==EC_RAM_RW_Flag)
    {
        if(Argc>4)
        {
            RAM_Data = (unsigned short)strtol(Argv[4], &str, 16);
        }
        else
        {
            printf("Please input wriye data\n");
            return;
        }

        if(IO_RW_Flag)// Read/Write RAM by 4E4F
        {
            EC_RAM_WRITE(RAM_Address, RAM_Data);
        }
        else
        {
            EC_WriteByte_PM(RAM_Address, RAM_Data);
        }
        printf("Index = %#X \n",RAM_Address);
        printf("Data  = %#X \n",RAM_Data);
    }
    // Read
    else if(2==EC_RAM_RW_Flag)
    {
        if(IO_RW_Flag)// Read/Write RAM by 4E4F
        {
            RAM_Data = EC_RAM_READ(RAM_Address);
        }
        else
        {
            RAM_Data = EC_ReadByte_PM(RAM_Address);
        }
        printf("Index = %#X \n",RAM_Address);
        printf("Data  = %#X \n",RAM_Data);
    }
    else
    {
        printf("Please set read or write flag\n");
    }
}


int main(int Argc, char *Argv[])
{
    char IOInitOK=0;
    int i;

    if(1==Argc)
    {
        goto ArgcError;
    }
    
    // Check read or write
    EC_RAM_RW_Flag=0;
    if(!strcmp("/W", Argv[1]))
    {
        EC_RAM_RW_Flag=1;
    }
    else if(!strcmp("/R", Argv[1]))
    {
        EC_RAM_RW_Flag=2;
    }
    else
    {
        goto ArgcError;
    }
    
    // Init IO
    IOInitOK = InitializeWinIo();
    if(IOInitOK)
    {
        SetTextColor(EFI_LIGHTGREEN, EFI_BLACK);
        printf("WinIo OK\n");
    }
    else
    {
        SetTextColor(EFI_LIGHTRED, EFI_BLACK);
        printf("Error during initialization of WinIo\n");
        goto IOError;
    }
    SetTextColor(EFI_WHITE, EFI_BLACK);

    //ToolInit();

    if(Set_Access_IO(Argc, Argv))
    {
        RAM_Access(Argc, Argv);
        goto end;
    }
    else
    {
        goto ArgcError;
    }
    
    goto end;

ArgcError:
    Help();
    ShutdownWinIo();
    return 1;
    
IOError:
    ShutdownWinIo();
    return 1;
    
end:
ShutdownWinIo();
    return 0;
}