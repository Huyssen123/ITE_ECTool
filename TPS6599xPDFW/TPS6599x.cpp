#define  TOOLS_VER   "V0.2"


//*****************************************
// TPS6599x PD FW Update Tool Version : 0.1
// 1. First release
//    a. PD firmware backup
//    b. PD firmware version read
//*****************************************

/* Copyright (C)Copyright 2005-2020 ZXQ Telecom. All rights reserved.

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
//typedef unsigned int    UINT16;
#define TRUE            1
#define FALSE           0
//==================================================================================================

//==========================The hardware port to read/write function================================
#define READ_PORT(port,data2)  GetPortVal(port, &data2, 1);
#define WRITE_PORT(port,data2) SetPortVal(port, data2, 1)
//==================================================================================================

//======================================== PM channel ==============================================
#define PM_STATUS_PORT66          0x66
#define PM_CMD_PORT66             0x66
#define PM_DATA_PORT62            0x62
#define PM_OBF                    0x01
#define PM_IBF                    0x02
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
    Send_cmd_by_PM(0x81);     // 6C port write 0x81
    Send_data_by_PM(index);   // 68 port write data address
    Send_data_by_PM(data);    // 68 port write data
}
//--------------read EC RAM------------------------------------/
BYTE EC_ReadByte_PM(BYTE index)
{
    BYTE data;
    Send_cmd_by_PM(0x80);       // 6C port write 0x80
    Send_data_by_PM(index);     // 68 port write data address
    data = Read_data_from_PM(); // read 68 port, get data
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

//===============================Console control interface==========================================
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
//==================================================================================================

//  Example:
//  ECRamWrite_Direct(0x51,0x90);
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
//ECRamRead_Direct
//ECRamWrite_Direct
//ECRamReadExt_Direct
//ECRamWriteExt_Direct

//60/64
//EC_ReadByte_KBC
//EC_WriteByte_KBC

//62/66
//EC_ReadByte_PM
//EC_WriteByte_PM

#define  EC_RAM_WRITE  ECRamWriteExt_Direct
#define  EC_RAM_READ   ECRamReadExt_Direct

//==================================================================================================

//=======================================Tool info==================================================
#define  TOOLS_NAME  "TPS6599x FW Update"
#define  ITE_IC      "TPS6599x"
#define  CopyRight   "(C)Copyright 2005-2020 ZXQ Telecom."
#define  TOOLS_AUTHOR "Morgen(zxqchongchi@gmail.com)"
#define  DEBUG       1
#define  ESC         0x1B



#define PD_FW_RAM_Addr          0xD1F4
#define PD_Control_Addr         0xD1A4
#define PD_FW_Size              0x8000

#define I2C_RW_RAM_Addr         0xD2A0
#define I2C_State_RAM_Addr      0xD2A1
#define EEPROM_Addr_1           0xD2A8
#define EEPROM_Addr_2           0xD2A9
#define EEPROM_Addr_3           0xD2AA
#define EEPROM_Addr_4           0xD2AB
#define EEPROM_Data             0xD2B0


#define PD_I2C_Read      0x55
#define PD_I2C_Write     0xAA
#define PD_I2C_RW_OK     0xAA
#define PD_I2C_RW_Busy   0x55
#define PD_I2C_Free      0

BYTE   FW_Data[0x800][0x10];


BYTE Read_PDFW_ToBuffer(char *FileName)
{
    UINT16 len1;
    UINT16 i;
    UINT16 read_count;
    FILE   *debuglog;
    FILE   *pPDFW_File;
    
    
    #if DEBUG
    printf("File Name : %s\n",FileName);
    
    debuglog = fopen("Debug.log","w");
    if(debuglog == NULL)
    {
        printf("Creat debuglog file Fail\n\n");
        return(FALSE);
    }
    #endif
    
    if((pPDFW_File = fopen((const char*)FileName, "rb")) == NULL)
    {
        printf("open PD FW file wrong!\n");
        return(FALSE);
    }
    
    read_count = 0;
    while (!feof(pPDFW_File))
    {
        len1=fread(FW_Data[read_count], 1, 0x10, pPDFW_File);

        #if DEBUG
        if(!(read_count%0x10))
        {
            fprintf(debuglog, "\n%02X\n",read_count);
            fprintf(debuglog, "\n");
        }
        for(i=0; i<16; i++)
        {
            fprintf(debuglog, "%02X ", FW_Data[read_count][i]);
        }
        fprintf(debuglog, "\n");
        #endif
        
        read_count++;
    }
    
    while(read_count<0x800)
    {
         for(i=0; i<16; i++)
        {
            FW_Data[read_count][i] = 0xFF;
        }
        
        #if DEBUG
        if(!(read_count%0x10))
        {
            fprintf(debuglog, "\n%02X\n",read_count);
            fprintf(debuglog, "\n");
        }
        for(i=0; i<16; i++)
        {
            fprintf(debuglog, "%02X ", FW_Data[read_count][i]);
        }
        fprintf(debuglog, "\n");
        #endif
        
        read_count++;
    }
    
    #if DEBUG
    printf("FWSize=[%d]Byte\n", read_count*0x10);
    fclose(debuglog);
    #endif

    fclose(pPDFW_File);
    
    return(TRUE);
}

void Update_PD_FW(void)
{
    BYTE I2C_Status=0;
    BYTE i;
    BYTE error_count;
    UINT16 read_count=0;
    BYTE addr_1=0;
    BYTE addr_2=0;
    BYTE addr_3=0;
    BYTE addr_4=0;
    
    for(read_count=0; read_count<0x800;read_count++)
    {
        I2C_Status = EC_RAM_READ(I2C_State_RAM_Addr);
        if(PD_I2C_Free==I2C_Status)
        {
            for(i=0; i<16; i++)
            {
                EC_RAM_WRITE(EEPROM_Data+i, FW_Data[read_count][i]);
            }
            read_count++;
            for(i=0; i<16; i++)
            {
                EC_RAM_WRITE(EEPROM_Data+i+16, FW_Data[read_count][i]);
            }
            
            EC_RAM_WRITE(EEPROM_Addr_1, addr_1);
            EC_RAM_WRITE(EEPROM_Addr_2, addr_2);
            EC_RAM_WRITE(EEPROM_Addr_3, addr_3);
            EC_RAM_WRITE(EEPROM_Addr_4, addr_4);
            EC_RAM_WRITE(I2C_RW_RAM_Addr, PD_I2C_Write);
            _sleep(20);   // millisecond
            //printf("read_count=%#X  addr=%#X-%#X-%#X\n", read_count, addr_3, addr_2, addr_1);
            
            while(1)
            {
                I2C_Status = EC_RAM_READ(I2C_State_RAM_Addr);
                if(PD_I2C_RW_OK==I2C_Status)
                {
                    addr_1 = addr_1+0x20;
                    if(0==addr_1)
                    {
                        addr_2++;
                        if(0==addr_2)
                        {
                            addr_3++;
                        }
                        if(!(addr_2%4))
                        {
                            printf("@");
                            printf("[%02d%%]\b\b\b\b\b", ((read_count*0x10)*100)/0x8000);
                        }
                    }
                    EC_RAM_WRITE(I2C_State_RAM_Addr, PD_I2C_Free);
                    error_count=0;
                    break;
                }
                
                _sleep(5);   // millisecond
                error_count++;
                if(error_count>100)
                {
                    return;
                }
            }
        }
    }
}


void Write_Buffer_ToFile(void)
{
    FILE   *pPDFW_BK_File;
    UINT16 read_count;
    
    if((pPDFW_BK_File = fopen("PD_FW_Backup.bin", "wb")) == NULL)
    {
        printf("open PD FW file wrong!\n");
        return;
    }
    
    for(read_count=0; read_count<0x800; read_count++)
    {
        fwrite(FW_Data[read_count],1,16,pPDFW_BK_File);
    }
    
    fclose(pPDFW_BK_File);
}

BYTE Read_PD_FW(void)
{
     BYTE I2C_Status=0;
     BYTE i;
     BYTE error_count;
     UINT16 read_count;
     BYTE addr_1=0;
     BYTE addr_2=0;
     BYTE addr_3=0;
     BYTE addr_4=0;
     
     printf(" Read PD FW : [....................................]\r Read PD FW : [");
     
     for(read_count=0; read_count<0x800; read_count++)
     {
         I2C_Status = EC_RAM_READ(I2C_State_RAM_Addr);
         if(PD_I2C_Free==I2C_Status)
         {
            EC_RAM_WRITE(EEPROM_Addr_1, addr_1);
            EC_RAM_WRITE(EEPROM_Addr_2, addr_2);
            EC_RAM_WRITE(EEPROM_Addr_3, addr_3);
            EC_RAM_WRITE(EEPROM_Addr_4, addr_4);
            EC_RAM_WRITE(I2C_RW_RAM_Addr, PD_I2C_Read);
            _sleep(20);   // millisecond
            
            while(1)
            {
                I2C_Status = EC_RAM_READ(I2C_State_RAM_Addr);
                if(PD_I2C_RW_OK==I2C_Status)
                {
                    for(i=0; i<16; i++)
                    {
                        FW_Data[read_count][i] = EC_RAM_READ(EEPROM_Data+i);
                    }
                    
                    addr_1 = addr_1+0x10;
                    if(0==addr_1)
                    {
                        addr_2++;
                        if(0==addr_2)
                        {
                            addr_3++;
                        }
                        if(!(addr_2%4))
                        {
                            printf("@");
                            printf("[%02d%%]\b\b\b\b\b", ((read_count*0x10)*100)/0x8000);
                        }
                    }
                    EC_RAM_WRITE(I2C_State_RAM_Addr, PD_I2C_Free);
                    error_count=0;
                    break;
                }
                
                _sleep(5);   // millisecond
                error_count++;
                if(error_count>100)
                {
                    return 0;
                }
            }
        }
    }
    
    printf("[%02d%%]\b\b\b\b\b", 100);
    return 1;
}


void Read_FW_Ver(void)
{
    BYTE Ver_Byte1=0;
    BYTE Ver_Byte2=0;
    BYTE Ver_Byte3=0;
    BYTE Ver_Byte4=0;

    Ver_Byte1 = EC_RAM_READ(PD_FW_RAM_Addr);
    Ver_Byte2 = EC_RAM_READ(PD_FW_RAM_Addr+1);
    Ver_Byte3 = EC_RAM_READ(PD_FW_RAM_Addr+2);
    Ver_Byte4 = EC_RAM_READ(PD_FW_RAM_Addr+3);
    
    printf("PD_FW_Version: %02X.%02X.%02X.%02X", Ver_Byte4,Ver_Byte3,Ver_Byte2,Ver_Byte1);
}

void help(void)
{
    printf("============================================================\n");
    printf("=         %s FW Update Utility Version : %s        =\n",ITE_IC,TOOLS_VER);
    printf("=        %s            =\n",CopyRight);
    printf("=                 All Rights Reserved.                     =\n");
    printf("=                             --%s                =\n", __DATE__);
    printf("=                                                          =\n");
    printf("=      [/R_FW]            Read Current PD FW binary        =\n");
    printf("=      [/R_VER]           Read Current PD FW Version       =\n");
    printf("=      [/W_FW  PDFW.bin]  Update PD FW                     =\n");
    printf("============================================================\n");
}

//==================================================================================================
UINT16 main(UINT16 argc, char *argv[])
{
    BYTE IOInitOK=0;
    BYTE PD_Action=0;
    BYTE PD_Control_status=0;
    
    if (argc == 1)
    {
        goto ArgcError;
    }
    
    if(!strcmp("/R_FW",argv[1]))
    {
        PD_Action=1;
    }
    if(!strcmp("/R_VER",argv[1]))
    {
        PD_Action=2;
    }
    if(!strcmp("/W_FW",argv[1]))
    {
        PD_Action=3;
    }
    
    system("cls");
    
    printf("============================================================\n");
    printf("=         %s FW Update Utility Version : %s        =\n",ITE_IC,TOOLS_VER);
    printf("=                             --%s                =\n", __DATE__);
    printf("=                                                          =\n");
    printf("============================================================\n");
    
    // Init IO port
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
    
    //EC must stop PD register access after receive this command
    PD_Control_status = EC_RAM_READ(PD_Control_Addr);
    EC_RAM_WRITE(PD_Control_Addr, PD_Control_status|0x04);
    
    if(3==PD_Action)
    {
        if(TRUE == Read_PDFW_ToBuffer(argv[2]))
        {
            Update_PD_FW();
            
            //Send command for reset PD
            PD_Control_status = EC_RAM_READ(PD_Control_Addr);
            EC_RAM_WRITE(PD_Control_Addr, PD_Control_status|0x08);
            _sleep(50);   // millisecond
        }
    }
    else if(2==PD_Action)
    {
        Read_FW_Ver();
    }
    else if(1==PD_Action)
    {
        if(Read_PD_FW())
        {
            Write_Buffer_ToFile();
        }
        else
        {
            printf("Backup PD FW error\n");
        }
        
    }
    else
    {
        printf("Not support\n");
    }
    
    //EC can access PD register after receive this command
    PD_Control_status = EC_RAM_READ(PD_Control_Addr);
    EC_RAM_WRITE(PD_Control_Addr, PD_Control_status&0xFB);
    
    goto end;
    
ArgcError:
    ShutdownWinIo();
    help();
    return 1;

IOError:
    ShutdownWinIo();
    return 1;
    
end:
ShutdownWinIo();
    return 0;
}