#define  TOOLS_VER   "V3.2"

/* Copyright (C)Copyright 2005-2020 ZXQ Telecom. All rights reserved.

   Author: 
   
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
typedef unsigned int    UINT32;
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
#define  TOOLS_NAME  "EC Test Tool"
#define  CopyRight   "(C)Copyright 2020 ZXQ Telecom."
UINT8 EC_CHIP_ID1;
UINT8 EC_CHIP_ID2;
UINT8 EC_CHIP_Ver;

//==================================================================================================

void ToolInit(void)
{
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
}

//==================================================================================================

extern UINT8 Hello_Cmd(void);
extern UINT8 NULL_Cmd(void);
extern UINT8 Get_EC_Version(void);
extern UINT8 S5_AutoPowerOn(void);
extern UINT8 Battery_Control(void);
extern UINT8 Get_BatteryVoltage(void);
extern UINT8 Get_BatteryCurrent(void);
extern UINT8 Get_BatteryRSOC(void);
extern UINT8 Get_BatteryFCC(void);
extern UINT8 Get_BatteryRMC(void);
extern UINT8 Get_BatteryDSC(void);
extern UINT8 Get_BatteryCycle(void);
extern UINT8 Get_BatteryTemp(void);
extern UINT8 Get_AdapterWatt(void);
extern UINT8 Get_PowerSource(void);
extern UINT8 Get_BatteryName(void);
extern UINT8 Send_Control_Cmd(void);
extern UINT8 Set_BatteryStorage(void);
extern UINT8 Clear_Battery_FUD(void);
extern UINT8 Get_Battery_FUD(void);
extern UINT8 Get_BAT_FW_Ver(void);
extern UINT8 Set_Charge_Current(void);

extern UINT8 One_key_test(void);
extern UINT8 Get_KB_Status(void);
extern UINT8 Get_IT8296_FW_Ver(void);
extern UINT8 Set_KB_ID(void);

extern UINT8 Get_Temperature(void);
extern UINT8 Get_Fan_RPM(void);
extern UINT8 Set_Fan_Debug_RPM(void);
extern UINT8 Set_Fan_Debug_PWM(void);

extern UINT8 LED_Enter_Test(void);
extern UINT8 LED_Exit_Test(void);
extern UINT8 Get_LID_Status(void);

extern UINT8 Get_USBC_Status(void);
extern UINT8 Get_PD_FW_Ver(void);
extern UINT8 Write_EC_RAM_Cmd(void);
extern UINT8 Read_EC_RAM_Cmd(void);
extern UINT8 Enable_PowerOn_WDT(void);
extern UINT8 W_EC_RAM_4E4F_Cmd(void);
extern UINT8 R_EC_RAM_4E4F_Cmd(void);

UINT8  EC_Tool_Cmd=0;
typedef UINT8 (*Cmd_FUN_P)(void);
char **ToolArgv;
char ToolArgc;

typedef struct Tool_Cmd_info
{
    char       cmd_name[64];
    char       cmd_number[64];
    Cmd_FUN_P  cmd_fun_pointer;
    UINT8      main_cmd;
    UINT8      sub_cmd;
}TOOL_INFO_STRUCT;


TOOL_INFO_STRUCT Tool_Cmd_Array[] =
{
//   cmd meaning                            cmd number  cmd function         main  sub
    {"First_cmd",                               "/A0000",  NULL_Cmd,           0x00, 0x00},
    
    {"(Hello)",                                 "/A1000",  Hello_Cmd,          0x00, 0x00},

    {"(Set Battery ShipMode after S5)",         "/A1001",  Send_Control_Cmd,   0x10, 0x01},
    {"(Get Battery Voltage)",                   "/A1002",  Get_BatteryVoltage, 0x10, 0x02},
    {"(Get Battery Current)",                   "/A1003",  Get_BatteryCurrent, 0x10, 0x03},
    {"(Get Battery RSOC)",                      "/A1004",  Get_BatteryRSOC,    0x10, 0x04},
    {"(Get Battery FCC)",                       "/A1005",  Get_BatteryFCC,     0x10, 0x05},
    {"(Get Battery RMC)",                       "/A1006",  Get_BatteryRMC,     0x10, 0x06},
    {"(Get Battery DSC)",                       "/A1007",  Get_BatteryDSC,     0x10, 0x07},
    {"(Get Battery Cycle)",                     "/A1008",  Get_BatteryCycle,   0x10, 0x08},
    {"(Get Battery Temp)",                      "/A1009",  Get_BatteryTemp,    0x10, 0x09},
    {"(Get Adapter Watt)",                      "/A100A",  Get_AdapterWatt,    0x10, 0x0A},
    {"(Get Battery Name)",                      "/A100B",  Get_BatteryName,    0x10, 0x0B},
    {"(Set Battery Discharge)",                 "/A100C",  Send_Control_Cmd,   0x10, 0x0C},
    {"(Set Battery Storage, /A100D H L)",       "/A100D",  Set_BatteryStorage, 0x10, 0x0D},
    {"(Set Battery Stopcharge)",                "/A100E",  Send_Control_Cmd,   0x10, 0x0E},
    {"(Cancel battery Special Control)",        "/A100F",  Send_Control_Cmd,   0x10, 0x0F},
    {"(Get Power Source)",                      "/A1010",  Get_PowerSource,    0x10, 0x10},
    {"(Clear Battery FUD)",                     "/A1011",  Clear_Battery_FUD,  0x10, 0x11},
    {"(Get Battery FUD)",                       "/A1012",  Get_Battery_FUD,    0x10, 0x12},
    {"(Get Battery FW Version)",                "/A1013",  Get_BAT_FW_Ver,     0x10, 0x13},
    {"(Set Charge Current)",                    "/A1014",  Set_Charge_Current, 0x10, 0x14},
    
    {"                 ",                       "      ",  NULL_Cmd,           0x00, 0x00},
    {"(Get EC sensor temperature)",             "/B2001",  Get_Temperature,    0x20, 0x01},
    {"(Get Fan1 RPM)",                          "/B2010",  Get_Fan_RPM,        0x20, 0x10},
    {"(Set Fan1 Debug RPM, /B2011 RPM)",        "/B2011",  Set_Fan_Debug_RPM,  0x20, 0x11},
    {"(Clear Fan1 Debug RPM)",                  "/B2012",  Send_Control_Cmd,   0x20, 0x12},
    {"(Set Fan1 PWM Duty Cycle, /B2023 n)",     "/B2013",  Set_Fan_Debug_PWM,  0x20, 0x13},
    {"(Clear Fan1 PWM Duty Cycle)",             "/B2014",  Send_Control_Cmd,   0x20, 0x14},
    {"(Get Fan2 RPM)",                          "/B2020",  Get_Fan_RPM,        0x20, 0x20},
    {"(Set Fan2 Debug RPM, /B2021 RPM)",        "/B2021",  Set_Fan_Debug_RPM,  0x20, 0x21},
    {"(Clear Fan2 Debug RPM)",                  "/B2022",  Send_Control_Cmd,   0x20, 0x22},
    {"(Set Fan2 PWM Duty Cycle, /B2023 n)",     "/B2023",  Set_Fan_Debug_PWM,  0x20, 0x23},
    {"(Clear Fan2 PWM Duty Cycle)",             "/B2024",  Send_Control_Cmd,   0x20, 0x24},
    {"                 ",                       "      ",  NULL_Cmd,           0x00, 0x00},
    {"(LED Enter Test, /C3001 LED_ID[0-10])",   "/C3001",  LED_Enter_Test,     0x30, 0x01},
    {"(LED Exit Test)",                         "/C3002",  LED_Exit_Test,      0x30, 0x02},
    {"                 ",                       "      ",  NULL_Cmd,           0x00, 0x00},
    {"(Keyboard enter test mode)",              "/D4001",  Send_Control_Cmd,   0x40, 0x01},
    {"(Keyboard exit test mode)",               "/D4002",  Send_Control_Cmd,   0x40, 0x02},
    {"(Power Button Test)",                     "/D4003",  One_key_test,       0x40, 0x03},
    {"(Novo Button Test)",                      "/D4004",  One_key_test,       0x40, 0x04},
    {"(Get Keyboard status)",                   "/D4005",  Get_KB_Status,      0x40, 0x05},
    {"(Vol+ Button Test)",                      "/D4006",  One_key_test,       0x40, 0x06},
    {"(Vol- Button Test)",                      "/D4007",  One_key_test,       0x40, 0x07},
    {"(Set KB ID, /D4008 n)",                   "/D4008",  Set_KB_ID,          0x40, 0x08},
    {"(Get IT8296 FW Version)",                 "/D4010",  Get_IT8296_FW_Ver,  0x40, 0x10},
    {"                 ",                       "      ",  NULL_Cmd,           0x00, 0x00},
    {"(Get USBC Port_A Status)",                "/E5001",  Get_USBC_Status,    0x50, 0x01},
    {"(Get USBC Port_B Status)",                "/E5010",  Get_USBC_Status,    0x50, 0x10},
    {"(Get PD FW Version)",                     "/E5030",  Get_PD_FW_Ver,      0x50, 0x30},
    {"                 ",                       "      ",  NULL_Cmd,           0x00, 0x00},
    {"(Get LID Status)",                        "/F6001",  Get_LID_Status,     0x60, 0x01},
    {"(Set LID function Disable)",              "/F6002",  Send_Control_Cmd,   0x60, 0x02},
    {"(Set LID function Enable)",               "/F6003",  Send_Control_Cmd,   0x60, 0x03},
    {"                 ",                       "      ",  NULL_Cmd,           0x00, 0x00},
    {"(Get EC Version)",                        "/GF001",  Get_EC_Version,     0xF0, 0x01},
    {"(Set S5 Auto Power On, /GF002 sleeptime)","/GF002",  S5_AutoPowerOn,     0xF0, 0x02},
    {"(Disable G3)",                            "/GF003",  Send_Control_Cmd,   0xF0, 0x03},
    {"(Enable G3)",                             "/GF004",  Send_Control_Cmd,   0xF0, 0x04},
    {"(Enable Mirror enter S5)",                "/GF005",  Send_Control_Cmd,   0xF0, 0x05},
    {"(No-override TXE/ME, can't flash ME)",    "/GF006",  Send_Control_Cmd,   0xF0, 0x06},
    {"(Override TXE/ME, can flash ME)",         "/GF007",  Send_Control_Cmd,   0xF0, 0x07},
    {"(Write EC RAM by EC Interface)",          "/GF008",  Write_EC_RAM_Cmd,   0xF0, 0x08},
    {"(Read EC RAM by EC Interface)",           "/GF009",  Read_EC_RAM_Cmd,    0xF0, 0x09},
	{"(Enable Power On WDT, /GF00A WDT_timer)", "/GF00A",  Enable_PowerOn_WDT, 0xF0, 0x0A},
    {"(Disable Power On WDT)",                  "/GF00B",  Send_Control_Cmd,   0xF0, 0x0B},
	{"(Write EC RAM by 4E4F)",                  "/GF0F0",  W_EC_RAM_4E4F_Cmd,  0xF0, 0xF0},
    {"(Read EC RAM by 4E4F)",                   "/GF0F1",  R_EC_RAM_4E4F_Cmd,  0xF0, 0xF1},
    
    {"Last_Cmd",                                "/FFFFF",  NULL_Cmd,           0x00, 0x00},
};


void Help(void)
{
    UINT8 i;
    printf("======================================================================\n");
    printf("=                ITE EC Test Tool Version : %s                     =\n",TOOLS_VER);
    printf("======================================================================\n");
    printf("=     %s All Rights Reserved.        =\n", CopyRight);
    printf("=                                                                    =\n");
    printf("=                                            --%s           =\n", __DATE__);
    printf("======================================================================\n");
    i=1;
    while(1)
    {
        if(!strcmp(Tool_Cmd_Array[i].cmd_name, "Last_Cmd"))
        {
            break;
        }
        printf("=   %-10s %-53s =\n", Tool_Cmd_Array[i].cmd_number, Tool_Cmd_Array[i].cmd_name);
        i++;
    }
    printf("=                                                                    =\n");
    printf("=        Example : EC_Test_Tool.exe  /A1000                          =\n");
    printf("======================================================================\n");
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
#define MBox_Data6      0xC158
#define MBox_Data7      0xC159
#define MBox_Data8      0xC15A
#define MBox_Data9      0xC15B
#define MBox_Data10     0xC15C

UINT8 Hello_Cmd(void)
{
    printf("Hello, This EC test tool, who are you?\n");
    return 0;
}

UINT8 NULL_Cmd(void)
{
    printf("This command is not support\n");
    return 1;
}

UINT8 LED_Enter_Test(void)
{
    UINT8 wait_count;
    UINT8 led_id=0;
    UINT8 led_blink=0x07;
    char *endptr;
    
    if(NULL!=ToolArgv[2])
    {
        led_id = strtol(ToolArgv[2], &endptr, 10);
    }
    
    led_blink = 0x03; // LED On
    
    EC_RAM_WRITE(MBox_Data1, led_id);
    EC_RAM_WRITE(MBox_Data2, led_blink);
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
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
        printf("LED Test mode enter OK\n");
        return 0;
    }
    
    printf("LED Test mode enter fail\n");
    return 1;
}

UINT8 LED_Exit_Test(void)
{
    UINT8 wait_count;
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
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
        printf("LED Test mode exit OK\n");
        return 0;
    }
    
    printf("LED Test mode exit fail\n");
    return 1;
}

// This is for Power Button and Lenovo Novo Button Test
UINT8 One_key_test(void)
{
    UINT8 button_state;
    UINT16 wait_count;
    
    printf("%s\n", Tool_Cmd_Array[EC_Tool_Cmd].cmd_name);
    
    // Step1, check button release
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
    _sleep(50); // ms
    button_state = EC_RAM_READ(MBox_Data1);
    
    if(0x55==button_state) // button release
    {
        printf("Button Release\n");
    }
    else if(0xAA==button_state) // button press
    {
        SetTextColor(EFI_LIGHTRED, EFI_BLACK);
        printf("Button short circuit\n");
        SetTextColor(EFI_WHITE, EFI_BLACK);
        return 1;
    }
    else
    {
        SetTextColor(EFI_LIGHTRED, EFI_BLACK);
        printf("Tool read button state error\n");
        SetTextColor(EFI_WHITE, EFI_BLACK);
        return 1;
    }
    
    wait_count = 0;
button_step2:
    // Step2, check button press
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
    _sleep(50); // ms
    button_state = EC_RAM_READ(MBox_Data1);
    
    if(0x55==button_state) // button release
    {
        wait_count++;
        if(wait_count>120) // wait 60s
        {
            SetTextColor(EFI_LIGHTRED, EFI_BLACK);
            printf("Button open circuit\n");
            SetTextColor(EFI_WHITE, EFI_BLACK);
            return 1;
        }
        _sleep(500); // ms
        goto button_step2;
    }
    else if(0xAA==button_state) // button press
    {
        printf("Button press\n");
    }
    else
    {
        SetTextColor(EFI_LIGHTRED, EFI_BLACK);
        printf("Tool read button state error\n");
        SetTextColor(EFI_WHITE, EFI_BLACK);
        return 1;
    }

    wait_count = 0;
button_step3:
    // Step3, check button release
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
    _sleep(50); // ms
    button_state = EC_RAM_READ(MBox_Data1);
    
    if(0x55==button_state) // button release
    {
        printf("Button Release\n");
    }
    else if(0xAA==button_state) // button press
    {
        wait_count++;
        if(wait_count>20) // wait 10s
        {
            SetTextColor(EFI_LIGHTRED, EFI_BLACK);
            printf("Button short circuit\n");
            SetTextColor(EFI_WHITE, EFI_BLACK);
            return 1;
        }
        _sleep(500); // ms
        goto button_step3;
        return 1;
    }
    else
    {
        SetTextColor(EFI_LIGHTRED, EFI_BLACK);
        printf("Tool read button state error\n");
        SetTextColor(EFI_WHITE, EFI_BLACK);
        return 1;
    }
    
    return 0;
}

UINT8 Get_KB_Status(void)
{
    UINT8 wait_count;
    UINT8 kb_status;
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
    _sleep(100); // ms
    
    for(wait_count=0; wait_count<10; wait_count++)
    {
        if(0x55==EC_RAM_READ(MBox_CmdState))
        {
            kb_status = EC_RAM_READ(MBox_Data1);
            if(0xAA==kb_status)
            {
                printf("Keyboard status : Disable\n");
            }
            else if(0x55==kb_status)
            {
                printf("Keyboard status : Enable\n");
            }
            else
            {
                printf("Keyboard status : Unknown\n");
            }
            
            break;
        }
        _sleep(100); // ms
    }
    
    if(wait_count<10)
    {
        return 0;
    }
    
    printf("%s fail\n", Tool_Cmd_Array[EC_Tool_Cmd].cmd_name);
    return 1;
}

UINT8 Set_KB_ID(void)
{
    UINT8 wait_count;
    char *endptr;
    UINT8 KB_CountryCode = 0x00; 
    
    if(NULL!=ToolArgv[2])
    {
        KB_CountryCode = strtol(ToolArgv[2], &endptr, 10);
    }
    
    if(KB_CountryCode == 'B')
    {
        printf("Set Brazil Country KB layout OK \n");
    }
    else 
    {
        printf("Set US/UK Country KB layout OK \n");
    }
    
    EC_RAM_WRITE(MBox_Data1,  KB_CountryCode);   
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
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
    
    printf("Set KB Country layout fail\n");
    return 1;
}

UINT8 Get_IT8296_FW_Ver(void)
{
    UINT8 wait_count;
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
    _sleep(100); // ms
    
    for(wait_count=0; wait_count<10; wait_count++)
    {
        if(0x55==EC_RAM_READ(MBox_CmdState))
        {
            printf("IT8296_KB_FW_VER=%02X%02X\n", EC_RAM_READ(MBox_Data1),
                                                  EC_RAM_READ(MBox_Data2));
                                                          
            break;
        }
        _sleep(100); // ms
    }
    
    if(wait_count<10)
    {
        return 0;
    }
    
    printf("%s fail\n", Tool_Cmd_Array[EC_Tool_Cmd].cmd_name);
    return 1;
}


UINT8 S5_AutoPowerOn(void)
{
    UINT8 wait_count;
    char *endptr;
    UINT8 sleep_time=30;
    
    if(NULL!=ToolArgv[2])
    {
        sleep_time = strtol(ToolArgv[2], &endptr, 10);
    }
    
    EC_RAM_WRITE(MBox_Data1,  sleep_time);    // 30s
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
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
        printf("Set auto cold boot after enter S5 %d(s)\n", sleep_time);
        return 0;
    }
    
    printf("Set auto cold boot fail\n");
    return 1;
}

UINT8 Send_Control_Cmd(void)
{
    UINT8 wait_count;
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
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
         printf("%s OK\n", Tool_Cmd_Array[EC_Tool_Cmd].cmd_name);
        return 0;
    }
    
    printf("%s fail\n", Tool_Cmd_Array[EC_Tool_Cmd].cmd_name);
    return 1;
}

UINT8 Get_EC_Version(void)
{
    UINT8 wait_count;
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
    _sleep(100); // ms
    
    for(wait_count=0; wait_count<10; wait_count++)
    {
        if(0x55==EC_RAM_READ(MBox_CmdState))
        {
            printf("set EC_Version=%02d%02d\n", EC_RAM_READ(MBox_Data1),EC_RAM_READ(MBox_Data2));
            break;
        }
        _sleep(100); // ms
    }
    
    if(wait_count<10)
    {
        return 0;
    }
    
    printf("Get EC version fail\n");
    return 1;
}

UINT8 Write_EC_RAM_Cmd(void)
{
    char *endptr;
    UINT16 RAM_Addr=0;
    UINT8 W_Data;
    UINT8 wait_count;

    if(NULL!=ToolArgv[2])
    {
        RAM_Addr = strtol(ToolArgv[2], &endptr, 16);
    }
    else
    {
        printf("Please input write address and write data\n");
        printf("Example : EC_Test_Tool.exe  /GF008  0xC410 0x55");
        return 1;
    }
    
    if(NULL!=ToolArgv[3])
    {
        W_Data = strtol(ToolArgv[3], &endptr, 16);
    }
    else
    {
        printf("Please input write address and write data\n");
        printf("Example : EC_Test_Tool.exe  /GF008  0xC410 0x55");
        return 1;
    }
    
    EC_RAM_WRITE(MBox_Data1,  RAM_Addr>>8);
    EC_RAM_WRITE(MBox_Data2,  RAM_Addr&0xFF);
    EC_RAM_WRITE(MBox_Data3,  W_Data&0xFF);
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
    _sleep(100); // ms
    
    for(wait_count=0; wait_count<10; wait_count++)
    {
        if(0x55==EC_RAM_READ(MBox_CmdState))
        {
            printf("Write %#X -> %#X\n", W_Data, RAM_Addr);
            break;
        }
        _sleep(100); // ms
    }
    
    if(wait_count<10)
    {
        return 0;
    }
    
    printf("Write data to EC fail\n");
    return 1;
}

UINT8 Read_EC_RAM_Cmd(void)
{
    char *endptr;
    UINT16 RAM_Addr=0;
    UINT8 wait_count;

    if(NULL!=ToolArgv[2])
    {
        RAM_Addr = strtol(ToolArgv[2], &endptr, 16);
    }
    else
    {
        printf("Please input read address\n");
        printf("Example : EC_Test_Tool.exe  /GF008  0xC410");
        return 1;
    }
    
    EC_RAM_WRITE(MBox_Data1,  RAM_Addr>>8);
    EC_RAM_WRITE(MBox_Data2,  RAM_Addr&0xFF);
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
    _sleep(100); // ms
    
    for(wait_count=0; wait_count<10; wait_count++)
    {
        if(0x55==EC_RAM_READ(MBox_CmdState))
        {
            printf("RAM_Addr[%#X] = %#X\n",RAM_Addr, EC_RAM_READ(MBox_Data3));
            break;
        }
        _sleep(100); // ms
    }
    
    if(wait_count<10)
    {
        return 0;
    }
    
    printf("Read data form EC fail\n");
    return 1;
}

UINT8 Enable_PowerOn_WDT(void)
{
	UINT8 wait_count;
    char *endptr;
    UINT16 wdt_time=120;
    
    if(NULL!=ToolArgv[2])
    {
        wdt_time = strtol(ToolArgv[2], &endptr, 10);
    }
    
    EC_RAM_WRITE(MBox_Data1,  (wdt_time>>8)&0xFF);    // default 120s
	EC_RAM_WRITE(MBox_Data2,  wdt_time&0xFF);         // default 120s
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
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
        printf("Enable power on WDT OK, system will restart after %d(s)\n", wdt_time);
        return 0;
    }
    
    printf("Enable power on WDT Fail\n");
    return 1;
}

UINT8 W_EC_RAM_4E4F_Cmd(void)
{
	char *endptr;
    UINT16 RAM_Addr=0;
    UINT8 W_Data;

    if(NULL!=ToolArgv[2])
    {
        RAM_Addr = strtol(ToolArgv[2], &endptr, 16);
    }
    else
    {
        printf("Please input write address and write data\n");
        printf("Example : EC_Test_Tool.exe  /GF0F0  0xC410 0x55");
        return 1;
    }
    
    if(NULL!=ToolArgv[3])
    {
        W_Data = strtol(ToolArgv[3], &endptr, 16);
    }
    else
    {
        printf("Please input write address and write data\n");
        printf("Example : EC_Test_Tool.exe  /GF0F0  0xC410 0x55");
        return 1;
    }
	
	EC_RAM_WRITE(RAM_Addr, W_Data);
	printf("Write %#X -> %#X\n", W_Data, RAM_Addr);
	
	return 0;
}

UINT8 R_EC_RAM_4E4F_Cmd(void)
{
	char *endptr;
    UINT16 RAM_Addr=0;
    UINT8 R_data;

    if(NULL!=ToolArgv[2])
    {
        RAM_Addr = strtol(ToolArgv[2], &endptr, 16);
    }
    else
    {
        printf("Please input read address\n");
        printf("Example : EC_Test_Tool.exe  /GF008  0xC410");
        return 1;
    }
    
    R_data = EC_RAM_READ(RAM_Addr);
	printf("RAM_Addr[%#X] = %#X\n",RAM_Addr, R_data);
	return 0;
}


UINT8 Get_BatteryVoltage(void)
{
    UINT8 wait_count;
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
    _sleep(100); // ms
    
    for(wait_count=0; wait_count<10; wait_count++)
    {
        if(0x55==EC_RAM_READ(MBox_CmdState))
        {
            printf("set Battery_Voltage=%d\n", (EC_RAM_READ(MBox_Data1)+EC_RAM_READ(MBox_Data2)*256));
            break;
        }
        _sleep(100); // ms
    }
    
    if(wait_count<10)
    {
        return 0;
    }
    
    printf("Get Battery voltage fail\n");
    return 1;
}

UINT8 Get_BatteryCurrent(void)
{
    UINT8 wait_count;
    short bat_current;
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
    _sleep(100); // ms
    
    for(wait_count=0; wait_count<10; wait_count++)
    {
        if(0x55==EC_RAM_READ(MBox_CmdState))
        {
            bat_current = (EC_RAM_READ(MBox_Data1)+EC_RAM_READ(MBox_Data2)*256);
            printf("set Battery_Current=%d\n", bat_current);
            #if 0
            if(bat_current&0x8000)
            {
                bat_current ^= 0xFFFF;
                bat_current++;
                printf("BAT_Current:-%d mA\n", bat_current);
            }
            else
            {
                printf("BAT_Current:%d mA\n", bat_current);
            }
            #endif
            break;
        }
        _sleep(100); // ms
    }
    
    if(wait_count<10)
    {
        return 0;
    }
    
    printf("Get Battery current fail\n");
    return 1;
}

UINT8 Get_BatteryRSOC(void)
{
    UINT8 wait_count;
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
    _sleep(100); // ms
    
    for(wait_count=0; wait_count<10; wait_count++)
    {
        if(0x55==EC_RAM_READ(MBox_CmdState))
        {
            printf("set Battery_RSOC=%d\n", EC_RAM_READ(MBox_Data1));
            break;
        }
        _sleep(100); // ms
    }
    
    if(wait_count<10)
    {
        return 0;
    }
    
    printf("Get Battery RSOC fail\n");
    return 1;
}

UINT8 Get_BatteryFCC(void)
{
    UINT8 wait_count;
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
    _sleep(100); // ms
    
    for(wait_count=0; wait_count<10; wait_count++)
    {
        if(0x55==EC_RAM_READ(MBox_CmdState))
        {
            printf("set Battery_FCC=%d\n", (EC_RAM_READ(MBox_Data1)+EC_RAM_READ(MBox_Data2)*256));
            break;
        }
        _sleep(100); // ms
    }
    
    if(wait_count<10)
    {
        return 0;
    }
    
    printf("Get Battery Full charge capacity fail\n");
    return 1;
}

UINT8 Get_BatteryRMC(void)
{
    UINT8 wait_count;
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
    _sleep(100); // ms
    
    for(wait_count=0; wait_count<10; wait_count++)
    {
        if(0x55==EC_RAM_READ(MBox_CmdState))
        {
            printf("set Battery_RMC=%d\n", (EC_RAM_READ(MBox_Data1)+EC_RAM_READ(MBox_Data2)*256));
            break;
        }
        _sleep(100); // ms
    }
    
    if(wait_count<10)
    {
        return 0;
    }
    
    printf("Get Battery remain capacity fail\n");
    return 1;
}

UINT8 Get_BatteryDSC(void)
{
    UINT8 wait_count;
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
    _sleep(100); // ms
    
    for(wait_count=0; wait_count<10; wait_count++)
    {
        if(0x55==EC_RAM_READ(MBox_CmdState))
        {
            printf("set Battery_DSC=%d\n", (EC_RAM_READ(MBox_Data1)+EC_RAM_READ(MBox_Data2)*256));
            break;
        }
        _sleep(100); // ms
    }
    
    if(wait_count<10)
    {
        return 0;
    }
    
    printf("Get Battery design capacity fail\n");
    return 1;
}

UINT8 Get_BatteryCycle(void)
{
    UINT8 wait_count;
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
    _sleep(100); // ms
    
    for(wait_count=0; wait_count<10; wait_count++)
    {
        if(0x55==EC_RAM_READ(MBox_CmdState))
        {
            printf("set Battery_Cycle=%d\n", (EC_RAM_READ(MBox_Data1)+EC_RAM_READ(MBox_Data2)*256));
            break;
        }
        _sleep(100); // ms
    }
    
    if(wait_count<10)
    {
        return 0;
    }
    
    printf("Get Battery cycle count fail\n");
    return 1;
}

UINT8 Get_BatteryTemp(void)
{
    UINT8 wait_count;
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
    _sleep(100); // ms
    
    for(wait_count=0; wait_count<10; wait_count++)
    {
        if(0x55==EC_RAM_READ(MBox_CmdState))
        {
            printf("set Battery_Temp=%d\n", EC_RAM_READ(MBox_Data1));
            break;
        }
        _sleep(100); // ms
    }
    
    if(wait_count<10)
    {
        return 0;
    }
    
    printf("Get Battery Temperature fail\n");
    return 1;
}

UINT8 Get_AdapterWatt(void)
{
    UINT8 wait_count;
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
    _sleep(100); // ms
    
    for(wait_count=0; wait_count<10; wait_count++)
    {
        if(0x55==EC_RAM_READ(MBox_CmdState))
        {
            printf("set Adapter_Watt=%d\n", EC_RAM_READ(MBox_Data1));
            break;
        }
        _sleep(100); // ms
    }
    
    if(wait_count<10)
    {
        return 0;
    }
    
    printf("Get Adapter watt fail\n");
    return 1;
}

UINT8 Get_PowerSource(void)
{
    UINT8 wait_count;
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
    _sleep(100); // ms
    
    for(wait_count=0; wait_count<10; wait_count++)
    {
        if(0x55==EC_RAM_READ(MBox_CmdState))
        {
            printf("set Adapter_State=%s\n", (0x01&EC_RAM_READ(MBox_Data1))?"In":"Out");
            printf("set Battery_State=%s\n", (0x02&EC_RAM_READ(MBox_Data1))?"In":"Out");
            break;
        }
        _sleep(100); // ms
    }
    
    if(wait_count<10)
    {
        return 0;
    }
    
    printf("Get power source fail\n");
    return 1;
}

UINT8 Get_BatteryName(void)
{
    UINT8 wait_count;
    UINT8 i;
    UINT8 device_name[32];
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
    _sleep(100); // ms
    
    for(wait_count=0; wait_count<10; wait_count++)
    {
        if(0x55==EC_RAM_READ(MBox_CmdState))
        {
            for(i=0; i<29; i++)
            {
                device_name[i] = EC_RAM_READ(MBox_Data1+i);
                
                if(0==device_name[i])
                {
                    break;
                }
            }
            device_name[28] = 0;
            printf("set Battery_Name=%s\n", device_name);
            break;
        }
        _sleep(100); // ms
    }
    
    if(wait_count<10)
    {
        return 0;
    }
    
    printf("Get battery device name fail\n");
    return 1;
}

UINT8 Set_BatteryStorage(void)
{
    UINT8 wait_count;
    char *endptr;
    UINT8 High_level=80;
    UINT8 Low_level=30;
    
    if(NULL!=ToolArgv[2])
    {
        High_level = strtol(ToolArgv[2], &endptr, 10);
    }
    
    if(NULL!=ToolArgv[3])
    {
        Low_level = strtol(ToolArgv[3], &endptr, 10);
    }
    
    EC_RAM_WRITE(MBox_Data1,  High_level);
    EC_RAM_WRITE(MBox_Data2,  Low_level);
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
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
        printf("Set battery storage OK, High=%d, Low=%d\n", High_level, Low_level);
        return 0;
    }
    
    printf("Set battery storage fail\n");
    return 1;
}

UINT8 Clear_Battery_FUD(void)
{
    UINT8 wait_count;
    char *endptr;
    UINT8 date_h=0;
    UINT8 date_l=0;
    UINT32 tmpvalue;
    UINT32 year=0;
    UINT32 month=0;
    UINT32 day=0;

    if(ToolArgc>3)
    {
        if(NULL!=ToolArgv[2])
        {
            year = strtol(ToolArgv[2], &endptr, 10);
        }
        
        if(NULL!=ToolArgv[3])
        {
            month = strtol(ToolArgv[3], &endptr, 10);
        }
        
        if(NULL!=ToolArgv[4])
        {
            day = strtol(ToolArgv[4], &endptr, 10);
        }
    }
    
    if((year>0) && (month>0) && (month<13) && (day>0) && (day<32))
    {
        tmpvalue = (day&0x1F) | ((month&0x0F)<<5) | (((year-1980)&0x7F)<<9);
        date_l = tmpvalue&0xFF;
        date_h = (tmpvalue>>8)&0xFF;
    }
    else
    {
        date_l =0;
        date_h =0;
    }

    EC_RAM_WRITE(MBox_Data1,  date_l);
    EC_RAM_WRITE(MBox_Data2,  date_h);
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
    _sleep(100); // ms
    
    for(wait_count=0; wait_count<10; wait_count++)
    {
        if(0x55==EC_RAM_READ(MBox_CmdState))
        {
            tmpvalue = date_l+date_h*256;
            
            if(tmpvalue)
            {
                printf("Battery first use date set to: %d-%d-%d \n",
                (((tmpvalue>>9)&0x7F)+1980),
                    ((tmpvalue>>5)&0x0F),
                    tmpvalue&0x1F);
                printf("Set battery FUD OK\n");
            }
            else
            {
                printf("Clear battery FUD OK\n");
            }
            break;
        }
        _sleep(100); // ms
    }
    
    if(wait_count<10)
    {
        return 0;
    }
    
    printf("Set battery FUD fail\n");
    return 1;
}

UINT8 Get_Battery_FUD(void)
{
    UINT8 wait_count;
    UINT32 tmpvalue;
    
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
    _sleep(100); // ms
    
    for(wait_count=0; wait_count<10; wait_count++)
    {
        if(0x55==EC_RAM_READ(MBox_CmdState))
        {
            tmpvalue = (EC_RAM_READ(MBox_Data1)+EC_RAM_READ(MBox_Data2)*256);
            
            if(tmpvalue)
            {
                printf("Battery first use date is : %d-%d-%d \n",
                        (((tmpvalue>>9)&0x7F)+1980),
                        ((tmpvalue>>5)&0x0F),
                        tmpvalue&0x1F);

                if(((tmpvalue>>5)&0x0F)>0  &&
                   ((tmpvalue>>5)&0x0F)<13 &&  // 0<month<13
                   (tmpvalue&0x1F)>0  &&
                   (tmpvalue&0x1F)<32)         // 0<day<31
                {
                }
                else
                {
                    printf("Date format error\n");
                }
            }
            else
            {
                printf("Battery FUD is empty\n");
            }
            
            break;
        }
        _sleep(100); // ms
    }
    
    if(wait_count<10)
    {
        return 0;
    }
    
    printf("Get Battery first use date fail\n");
    return 1;
}

UINT8 Get_BAT_FW_Ver(void)
{
    UINT8 wait_count;
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
    _sleep(100); // ms
    
    for(wait_count=0; wait_count<10; wait_count++)
    {
        if(0x55==EC_RAM_READ(MBox_CmdState))
        {
            printf("Battery_FW_VER=%02X%02X%02X%02X%02X%02X%02X%02X\n",
                                                  EC_RAM_READ(MBox_Data2),
                                                  EC_RAM_READ(MBox_Data1),
                                                  
                                                  EC_RAM_READ(MBox_Data4),
                                                  EC_RAM_READ(MBox_Data3),
                                                  
                                                  EC_RAM_READ(MBox_Data6),
                                                  EC_RAM_READ(MBox_Data5),
                                                  
                                                  EC_RAM_READ(MBox_Data8),
                                                  EC_RAM_READ(MBox_Data7));
                                                  
                                                          
            break;
        }
        _sleep(100); // ms
    }
    
    if(wait_count<10)
    {
        return 0;
    }
    
    printf("%s fail\n", Tool_Cmd_Array[EC_Tool_Cmd].cmd_name);
    return 1;
}

UINT8 Set_Charge_Current(void)
{
    UINT8 wait_count;
    char *endptr;
    UINT16 Charger_current;
    UINT8 Charger_currentl;
    UINT8 Charger_currenth;

    if (NULL != ToolArgv[2])
    {
        Charger_current = strtol(ToolArgv[2], &endptr, 10);
        Charger_currentl = Charger_current & 0xFF;
        Charger_currenth = Charger_current >> 8;
    }
    EC_RAM_WRITE(MBox_Data1, Charger_currentl);
    EC_RAM_WRITE(MBox_Data2, Charger_currenth);
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd, Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
    _sleep(100); // ms

    for (wait_count = 0; wait_count<10; wait_count++)
    {
        if (0x55 == EC_RAM_READ(MBox_CmdState))
        {
            break;
        }
        _sleep(100); // ms
    }

    if (wait_count<10)
    {
        printf("Set charge current to %d \n", Charger_current);
        return 0;
    }

    printf("Set charg currnt fail\n");
    return 1;
}

typedef struct Sensor_Info_struct
{
    UINT8 Sensor_number;
    UINT8 Sensor_name[128];
    UINT8 Sensor_enable;
}Sensor_Info_Struct;

enum InfoNameEnum
{
    Thermal_Sensor_1=0,
    Thermal_Sensor_2,
    Thermal_Sensor_3,
    Thermal_Sensor_4,
    Thermal_Sensor_5,
    Thermal_Sensor_6,
    Thermal_Sensor_7,
    Thermal_Sensor_8,
    Thermal_Sensor_9,
    INFONAMECOUNT
};

Sensor_Info_Struct Sensor_Info_Table[] =
{
    {Thermal_Sensor_1, "Sensor Temperature 1", 0},
    {Thermal_Sensor_2, "Sensor Temperature 2", 0},
    {Thermal_Sensor_3, "Sensor Temperature 3", 0},
    {Thermal_Sensor_4, "Sensor Temperature 4", 0},
    {Thermal_Sensor_5, "Sensor Temperature 5", 0},
    {Thermal_Sensor_6, "Sensor Temperature 6", 0},
    {Thermal_Sensor_7, "Sensor Temperature 7", 0},
    {Thermal_Sensor_8, "Sensor Temperature 8", 0},
    {Thermal_Sensor_9, "Sensor Temperature 9", 0},
};

UINT8 Read_Cfg_File(void)
{
    FILE *pTestToolCfgFile = NULL;
    int  HexNum;
    int  i;
    char *str;
    char *pStrLine;
    char StrLine[1024];
    char StrNum[16];
    int  InfoIndex=0;
    
    if((pTestToolCfgFile = fopen("EC_Test_Tool.cfg","r")) == NULL)
    {
        printf("EC_Test_Tool.cfg not exist\n");
        return 1;
    }
    
    while (!feof(pTestToolCfgFile))
    {   
        // Read one line data
        fgets(StrLine,1024,pTestToolCfgFile);
        //printf("%s", StrLine);
        
        pStrLine = StrLine;
        if(!strcmp(StrLine, "[Thermal_Sensor]\n"))
        {
            while ((!feof(pTestToolCfgFile)) && (InfoIndex<INFONAMECOUNT))
            {
                fgets(StrLine,1024,pTestToolCfgFile);
                pStrLine = StrLine;
                
                // Get name
                while(('#' != (*pStrLine++)));
                i=0;
                while(('#' != (*pStrLine)))
                {
                    Sensor_Info_Table[InfoIndex].Sensor_name[i] = *pStrLine++;
                    i++;
                }
                Sensor_Info_Table[InfoIndex].Sensor_name[i]=0;
                
                // Get enable
                pStrLine++;
                while(('#' != (*pStrLine++)));
                HexNum = (int)strtol(pStrLine, &str, 10);
                if(0x00==HexNum)
                {
                    Sensor_Info_Table[InfoIndex].Sensor_enable = 0;
                }
                else if(0x01==HexNum)
                {
                    Sensor_Info_Table[InfoIndex].Sensor_enable = 1;
                }
                
                //printf("Name:%s  [Enable:%d]\n", Sensor_Info_Table[InfoIndex].Sensor_name, HexNum);
                InfoIndex++;
            }
        }
    }
    
    fclose(pTestToolCfgFile);
    
    return 0;
}

UINT8 Get_Temperature(void)
{
    UINT8 wait_count;
    UINT8 sensor_index;
    
    if(Read_Cfg_File())
    {
        return 1;
    }
    
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
    _sleep(100); // ms
    
    for(wait_count=0; wait_count<10; wait_count++)
    {
        if(0x55==EC_RAM_READ(MBox_CmdState))
        {
            for(sensor_index=0; sensor_index<INFONAMECOUNT; sensor_index++)
            {
                if(Sensor_Info_Table[sensor_index].Sensor_enable)
                {
                    printf("set %s=%d\n",
                        Sensor_Info_Table[sensor_index].Sensor_name,
                        EC_RAM_READ(MBox_Data1+sensor_index));
                }
            }
            break;
        }
        _sleep(100); // ms
    }
    
    if(wait_count<10)
    {
        return 0;
    }
    
    printf("%s fail\n", Tool_Cmd_Array[EC_Tool_Cmd].cmd_name);
    return 1;
}

UINT8 Get_Fan_RPM(void)
{
    UINT8 wait_count;
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
    _sleep(100); // ms
    
    for(wait_count=0; wait_count<10; wait_count++)
    {
        if(0x55==EC_RAM_READ(MBox_CmdState))
        {
            if(0x10==Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd)
            {
                printf("set Fan1_Current_RPM=%d\n", EC_RAM_READ(MBox_Data1) + EC_RAM_READ(MBox_Data2)*256);
            }
            else if(0x20==Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd)
            {
                printf("set Fan2_Current_RPM=%d\n", EC_RAM_READ(MBox_Data1) + EC_RAM_READ(MBox_Data2)*256);
            }
            break;
        }
        _sleep(100); // ms
    }
    
    if(wait_count<10)
    {
        printf("%s OK\n", Tool_Cmd_Array[EC_Tool_Cmd].cmd_name);
        return 0;
    }
    
    printf("%s fail\n", Tool_Cmd_Array[EC_Tool_Cmd].cmd_name);
    return 1;
}

UINT8 Set_Fan_Debug_RPM(void)
{
    UINT8 wait_count;
    char *endptr;
    UINT8 fan_rpm=30; //default 30*100 RPM
    
    if(NULL!=ToolArgv[2])
    {
        fan_rpm = strtol(ToolArgv[2], &endptr, 10);
    }
    
    EC_RAM_WRITE(MBox_Data1,  fan_rpm);    // fan_rpm*100 RPM
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
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
        printf("Set Fan RPM fixed to %d\n", fan_rpm);
        return 0;
    }
    
    printf("Set Fan RPM fail\n");
    return 1;
}

UINT8 Set_Fan_Debug_PWM(void)
{
    UINT8 wait_count;
    char *endptr;
    UINT8 pwm_duty_cycle=30; //default 30%
    
    if(NULL!=ToolArgv[2])
    {
        pwm_duty_cycle = strtol(ToolArgv[2], &endptr, 10);
    }
    
    if((pwm_duty_cycle<10) || (pwm_duty_cycle>100))
    {
        pwm_duty_cycle = 30;
        printf("Set Fan PWM Duty Cycle must be within range 10--100\n");
    }
    
    printf("Set Fan PWM Duty Cycle fixed to %d\n", pwm_duty_cycle);
    
    EC_RAM_WRITE(MBox_Data1,  pwm_duty_cycle);    // PWM Duty Cycle 10--100
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
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
    
    printf("Set PWM Duty Cycle fail\n");
    return 1;
}

UINT8 Get_USBC_Status(void)
{
    UINT8 wait_count;
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
    _sleep(100); // ms
    
    for(wait_count=0; wait_count<10; wait_count++)
    {
        if(0x55==EC_RAM_READ(MBox_CmdState))
        {
            if(0x01 == Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd)
            {
                printf("USBC_Port_A : %s\n", (0x01&EC_RAM_READ(MBox_Data1))?"Connect":"Disconnect");
            }
            if(0x10 == Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd)
            {
                printf("USBC_Port_B : %s\n", (0x01&EC_RAM_READ(MBox_Data1))?"Connect":"Disconnect");
            }
            if(0x01&EC_RAM_READ(MBox_Data1))
            {
                printf("Direction : %s\n", (0x02&EC_RAM_READ(MBox_Data1))?"Positive":"Negative");
            }
            else
            {
                printf("Direction : NA\n");
            }
            break;
        }
        _sleep(100); // ms
    }
    
    if(wait_count<10)
    {
        return 0;
    }
    
    printf("%s fail\n", Tool_Cmd_Array[EC_Tool_Cmd].cmd_name);
    return 1;
}

UINT8 Get_PD_FW_Ver(void)
{
    UINT8 wait_count;
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
    _sleep(100); // ms
    
    for(wait_count=0; wait_count<10; wait_count++)
    {
        if(0x55==EC_RAM_READ(MBox_CmdState))
        {
            printf("PD Firmware Version : %02X%02X%02X%02X\n", EC_RAM_READ(MBox_Data1),
                                                          EC_RAM_READ(MBox_Data2),
                                                          EC_RAM_READ(MBox_Data3),
                                                          EC_RAM_READ(MBox_Data4));
                                                          
            break;
        }
        _sleep(100); // ms
    }
    
    if(wait_count<10)
    {
        return 0;
    }
    
    printf("%s fail\n", Tool_Cmd_Array[EC_Tool_Cmd].cmd_name);
    return 1;
}

UINT8 Get_LID_Status(void)
{
    UINT8 wait_count;
    EC_RAM_WRITE(MBox_SubCmd, Tool_Cmd_Array[EC_Tool_Cmd].sub_cmd);
    EC_RAM_WRITE(MBox_Cmd,    Tool_Cmd_Array[EC_Tool_Cmd].main_cmd);
    _sleep(100); // ms
    
    for(wait_count=0; wait_count<10; wait_count++)
    {
        if(0x55==EC_RAM_READ(MBox_CmdState))
        {
            printf("LID   0: %s\n", (0x01&EC_RAM_READ(MBox_Data1))?"Close":"Open");
            printf("LID 360: %s\n", (0x02&EC_RAM_READ(MBox_Data1))?"Close":"Open");
            printf("LID    : %s\n", (0x04&EC_RAM_READ(MBox_Data1))?"Disable":"Enable");
            break;
        }
        _sleep(100); // ms
    }
    
    if(wait_count<10)
    {
        return 0;
    }
    
    printf("%s fail\n", Tool_Cmd_Array[EC_Tool_Cmd].cmd_name);
    return 1;
}


int main(int Argc, char *Argv[])
{
    char IOInitOK=0;
    int i;

    if(1==Argc)
    {
        goto ArgcError;
    }
    
    ToolArgc = Argc;
    
    // check command
    EC_Tool_Cmd = 0;
    i=0;
    while(1)
    {
        if(!strcmp(Tool_Cmd_Array[i].cmd_number, Argv[1]))
        {
            EC_Tool_Cmd = i;
            ToolArgv = &Argv[0];
            break;
        }
        
        if(!strcmp(Tool_Cmd_Array[i].cmd_name, "Last_Cmd"))
        {
            break;
        }
        i++;
    }
    
    // Init IO
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
    
    // Send command
    if(EC_Tool_Cmd)
    {
        if(0!=Tool_Cmd_Array[EC_Tool_Cmd].cmd_fun_pointer())
        {
            goto CmdError;
        }
    }
    else
    {
        printf("This command is not found\n");
        goto ArgcError;
    }
    
    goto end;

ArgcError:
    //printf("Argc Number is = %d\n", Argc);
    Help();
    ShutdownWinIo();
    return 1;
    
IOError:
    ShutdownWinIo();
    return 1;
    
CmdError:
    ShutdownWinIo();
    return 1;
    
end:
ShutdownWinIo();
    return 0;
}