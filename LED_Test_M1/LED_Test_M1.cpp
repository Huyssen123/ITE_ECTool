#define  TOOLS_VER   "V1.2"

/* Copyright (C)Copyright 2005-2020 zxq Telecom. All rights reserved.

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
#include <tchar.h>

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
UINT8 PM_STATUS_PORT66          =0x66;
UINT8 PM_CMD_PORT66             =0x66;
UINT8 PM_DATA_PORT62            =0x62;
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


UINT8 ECRamReadExt_Direct(unsigned short iIndex)
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

void setFontSize(int FontSize)
{
    CONSOLE_FONT_INFOEX info = {0};
    info.cbSize       = sizeof(CONSOLE_FONT_INFOEX);
    info.dwFontSize.X = FontSize/2; //
    info.dwFontSize.Y = FontSize;   //
    //info.FontFamily = TMPF_DEVICE;//TMPF_VECTOR/TMPF_FIXED_PITCH/TMPF_TRUETYPE;
    info.FontWeight   = 200;
    //wcscpy(info.FaceName, L"Lucida Console");
    //wcscpy(info.FaceName, L"Consolas");
    
    SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), NULL, &info);
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

#define Tool_Debug  1

//==================================================================================================

//=======================================Tool info==================================================
#define  TOOLS_NAME  "LED Test Tool"
#define  CopyRight   "(C)Copyright 2019-2030 ZXQ Telecom."
UINT8 EC_CHIP_ID1;
UINT8 EC_CHIP_ID2;
UINT8 EC_CHIP_Ver;
UINT8 Console_col;
UINT8 Console_line;
    
UINT8  EC_Tool_Cmd=0;
typedef void (*Cmd_FUN_P)(void);
//==================================================================================================

void ToolInit(void)
{   
    #if Tool_Debug
    // ITE IT-557x chip is DLM architecture for EC  RAM and It's support 6K/8K RAM.
    // If used RAM less  than 4K, you can access EC RAM form 0x000--0xFFF by 4E/4F IO port
    // If used RAM more than 4K, RAM address change to 0xC000
    // If you want to access EC RAM by 4E/4F IO port, you must set as follow register first
    // REG_1060[BIT7]
    EC_CHIP_ID1 = EC_RAM_READ(0x2000);
    EC_CHIP_ID2 = EC_RAM_READ(0x2001);
    if(0xFF==EC_CHIP_ID1)
    {
        EC_ADDR_PORT = 0x2E;
        EC_DATA_PORT = 0x2F;
        EC_CHIP_ID1 = EC_RAM_READ(0x2000);
        EC_CHIP_ID2 = EC_RAM_READ(0x2001);
    }
    //printf("CHIP_ID=%X%X\n", EC_CHIP_ID1, EC_CHIP_ID2);
    
    if(0x55==EC_CHIP_ID1)
    {
        EC_CHIP_Ver = EC_RAM_READ(0x1060);
        EC_CHIP_Ver = EC_CHIP_Ver | 0x80;
        EC_RAM_WRITE(0x1060, EC_CHIP_Ver);
    }
    #endif
    
    // Modify Console size    
    system("chcp 437");
    system("mode con cols=200 lines=40");
}

void Help(void)
{
    printf("=======================================================\n");
    printf("=             %s Version : %s         =\n", TOOLS_NAME, TOOLS_VER);
    printf("=======================================================\n");
    printf("=         %s     =\n", CopyRight);
    
    printf("=                 All Rights Reserved.                =\n");
    printf("=                             --%s           =\n", __DATE__);
    printf("=                                                     =\n");
    printf("=======================================================\n");
}


typedef struct LED_Info_struct
{
    UINT8 LED_number;
    UINT8 LED_name[32];
    UINT8 LED_enable;
}LED_Info_Struct;

enum InfoNameEnum
{
    LED_1=0,
    LED_2,
    LED_3,
    LED_4,
    LED_5,
    LED_6,
    LED_7,
    LED_8,
    LED_9,
    LED_A,
    LED_B,
    LED_C,
    LED_D,
    LED_E,
    INFONAMECOUNT
};
LED_Info_Struct LED_Info[] =
{
    {LED_1, "³äµç°×µÆ", 0},
    {LED_2, "³äµçéÙµÆ", 0},
    {LED_3, "µçÔ´ºìµÆ", 0},
    {LED_4, "µçÔ´ÂÌµÆ", 0},
    {LED_5, "µçÔ´À¶µÆ", 0},
    {LED_6, "¼üÅÌ±³¹â", 0},
    {LED_7, "´óÐ´°×µÆ", 0},
    {LED_8, "Êý×Ö°×µÆ", 0},
    {LED_9, "¹¦ÄÜ¼üµÆ", 0},
    {LED_A, "ÉãÏñÍ·µÆ", 0},
    {LED_B, "±êÊ¶µÆ",   0},
    {LED_C, "¼üÅÌºìµÆ", 0},
    {LED_D, "¼üÅÌÂÌµÆ", 0},
    {LED_E, "¼üÅÌÀ¶µÆ",   0},
};

UINT8 Display_X;
UINT8 Display_Y;UINT8 Y_Base;


UINT8 Display_Chinese(UINT8 *Str, UINT8 UI_x, UINT8 UI_y)
{
    FILE *pHZK16File = NULL;
    UINT8 Dot_Matrix_buffer[32];
    int offset;
    UINT8 Char_index;
    UINT8 i, j, k;
    
    if((pHZK16File=fopen("hzk16", "rb")) == NULL)
    {
        fprintf(stderr, "Open error hzk16\n");
        return 1;
    }
    
    Display_X = UI_x;
    Display_Y = UI_y;
    Y_Base = UI_y;
    for(Char_index=0; Str[Char_index]!=0; Char_index+=2)
    {
        offset = (94*(unsigned int)(Str[Char_index]-0xA0-1) + (Str[Char_index+1]-0xA0-1))*32;
        fseek(pHZK16File, offset, SEEK_SET);
        fread(Dot_Matrix_buffer, 1, 32, pHZK16File);
        
        SetPosition_X_Y(Display_X, Display_Y);
        for(k=0; k<8; k++)
        {
            for(j=0; j<2; j++)
            {
                for(i=0; i<8; i++)
                {
                    if(Dot_Matrix_buffer[k*4+j]&(1<<(7-i)))
                    {
                        printf("%c",(Dot_Matrix_buffer[k*4+j+2]&(1<<(7-i)))?0xDB:0xDF);
                    }
                    else
                    {
                        printf("%c",(Dot_Matrix_buffer[k*4+j+2]&(1<<(7-i)))?0xDC:0x20);
                    }
                }
            }
            Display_Y ++;
            SetPosition_X_Y(Display_X, Display_Y);
        }
        
        Display_X += 22;
        Display_Y = Y_Base;
    }
    
   fclose(pHZK16File);
   return 0;
}

UINT8 Read_Cfg_File(void)
{
    FILE *pLEDCfgFile = NULL;
    int  HexNum;
    int  i;
    char *str;
    char *pStrLine;
    char StrLine[1024];
    char StrNum[16];
    int  InfoIndex=0;
    
    if((pLEDCfgFile = fopen("LEDTest.cfg","r")) == NULL)
    {
        printf("LEDTest.cfg not exist\n");
        return 1;
    }
    
    while (!feof(pLEDCfgFile))
    {   
        // Read one line data
        fgets(StrLine,1024,pLEDCfgFile);
        //printf("%s", StrLine);
        
        pStrLine = StrLine;
        if(('$'==StrLine[0]) && (('1'==StrLine[1])))
        {
            //InfoIndex = (StrLine[3]-'0')*10 + (StrLine[4]-'0');
            
            while(('#' != (*pStrLine++)));
            
            i=0;
            while(('#' != (*pStrLine)))
            {
                LED_Info[InfoIndex].LED_name[i] = *pStrLine++;
                printf("%#04X ", LED_Info[InfoIndex].LED_name[i]);
                i++;
            }
            
            pStrLine++;
            while(('#' != (*pStrLine++)));
            HexNum = (int)strtol(pStrLine, &str, 10);
            //printf("%#X\n",HexNum);
            if(0x00==HexNum)
            {
                LED_Info[InfoIndex].LED_enable = 0;
            }
            else if(0x01==HexNum)
            {
                LED_Info[InfoIndex].LED_enable = 1;
            }

            InfoIndex++;
        }
    }
    
    #if 0
    printf("\n\n\n");
    for(i=0; i<INFONAMECOUNT; i++)
    {
        printf("LED_%d %d %s\n", i+1, LED_Info[i].LED_enable, LED_Info[i].LED_name);
    }
    #endif
    
    fclose(pLEDCfgFile);
    
    return 0;
}

//==================================================================================================
// This define EC mail box EC RAM address
#define MBox_Cmd        0xC150
#define MBox_SubCmd     0xC151
#define MBox_CmdState   0xC152
#define MBox_Data1      0xC153
#define MBox_Data2      0xC154
#define MBox_Data3      0xC155
#define MBox_Data4      0xC156
#define MBox_Data5      0xC157

UINT8 LED_Exit_Test(void)
{
    #if Tool_Debug
    UINT8 wait_count;
    EC_RAM_WRITE(MBox_SubCmd, 0x02);
    EC_RAM_WRITE(MBox_Cmd,    0x30);
    _sleep(100); // ms
    
    for(wait_count=0; wait_count<10; wait_count++)
    {
        if(0x55==EC_RAM_READ(MBox_CmdState))
        {
            break;
        }
        _sleep(100); // ms
    }
    
    if(wait_count<10)
    {
        return 0;
    }
    #endif
    
    return 1;
}

UINT8 Send_LED_Test_Cmd(UINT8 led_id)
{
    UINT8 wait_count;
    UINT8 led_blink=0x07;
    char press_key=0;
    
    srand((unsigned)time(NULL));
    led_blink = 0x01 & rand();
    if(0x01==led_blink)
    {
        led_blink = 0x07; // LED blink
    }
    else
    {
        led_blink = 0x03; // LED On
    }
    
    //printf("led_blink=%d\n", led_blink);
    
    #if Tool_Debug
    EC_RAM_WRITE(MBox_Data1, led_id);
    EC_RAM_WRITE(MBox_Data2, led_blink);
    EC_RAM_WRITE(MBox_SubCmd, 0x01); // sub command
    EC_RAM_WRITE(MBox_Cmd,    0x30); // main command
    _sleep(100); // ms
    
    for(wait_count=0; wait_count<10; wait_count++)
    {
        if(0x55==EC_RAM_READ(MBox_CmdState))
        {
            break;
        }
        _sleep(100); // ms
    }
    #endif
    
    // Wait S/L Key
    press_key=getch();
    
    if(0x07==led_blink)  // LED blink
    {
        if((0x73==press_key) || (0x53==press_key)) // S or s
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else if(0x03==led_blink)  // LED on
    {
        if((0x6C==press_key) || (0x4C==press_key)) // L or l
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
    
    return 1;
}

UINT8 Test_LED(void)
{
    int  i;
    UINT8 Chinese_Str1[]="²âÊÔÊ§°Ü";
    UINT8 Chinese_Str2[]="²âÊÔ³É¹¦";
    UINT8 Chinese_Str3[]={0xC9,0xC1, 0xCB,0xB8, 0xB0,0xB4, 0xA3,0xD3, 0x00};  // ÉÁË¸°´S
    UINT8 Chinese_Str4[]={0xB3,0xA4, 0xC1,0xC1, 0xB0,0xB4, 0xA3,0xCC, 0x00};  // ³¤ÁÁ°´L
    
    for(i=0; i<INFONAMECOUNT; i++) // Polling test support LED 
    {
        if(LED_Info[i].LED_enable)
        {
            system("cls");
            // Display chinese
            if(Display_Chinese(LED_Info[i].LED_name, 0, 2))
            {
                return 1;
            }

            Display_Chinese(Chinese_Str3, 0, 12);
            Display_Chinese(Chinese_Str4, 0, 23);
            
            // Send LED test command
            if(Send_LED_Test_Cmd(i))
            {
                system("cls");
                LED_Exit_Test();
                SetTextColor(EFI_LIGHTRED, EFI_BLACK);
                Display_Chinese(Chinese_Str1, 0, 2);
                SetTextColor(EFI_WHITE, EFI_BLACK);
                return 1;
            }
        }
    }
    
    system("cls");
    LED_Exit_Test();
    SetTextColor(EFI_LIGHTGREEN, EFI_BLACK);
    Display_Chinese(Chinese_Str2, 0, 2);
    SetTextColor(EFI_WHITE, EFI_BLACK);
    return 0;  //Test Pass
}



int main(int Argc, char *Argv[])
{
    char IOInitOK=0;
    int i;

    if(Argc>1)
    {
        goto ArgcError;
    }
    
    
    IOInitOK = InitializeWinIo();
    if(IOInitOK)
    {
        //SetTextColor(EFI_LIGHTGREEN, EFI_BLACK);
        //printf("WinIo OK\n");
    }
    else
    {
        SetTextColor(EFI_LIGHTRED, EFI_BLACK);
        printf("Error during initialization of WinIo\n");
        goto IOError;
    }
    
    SetTextColor(EFI_WHITE, EFI_BLACK);
    ToolInit();
    
    // Read LED test config File
    if(Read_Cfg_File())
    {
        goto IOError;
    }
    
    // Start test LED
    if(Test_LED())
    {
        goto IOError;
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