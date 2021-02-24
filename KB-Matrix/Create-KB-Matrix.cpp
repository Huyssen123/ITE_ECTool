#define  TOOLS_VER   "V2.2"

//*****************************************
// BatteryTool Version : 1.0
//*****************************************


/* Copyright (C)Copyright 2005-2020 ZXQ Telecom. All rights reserved.

   Author:
*/


// Using VS2012 X86 cmd tool to compilation
// For windows-32/64bit

//============================Include file =====================================
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


//======================Console control interface===============================
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


//============================Tool info=========================================
#define  TOOLS_NAME  "KB Matrix Create"
#define  ITE_IC      "ITE8987"
#define  CopyRight   "(C)Copyright 2020 Bitland Telecom."
#define  DEBUG       0
#define  ESC         0x1B
#define  KEY_UP      0x48
#define  KEY_DOWN    0x50

#define KEY_W        0x57
#define KEY_S        0x53
#define KEY_w        0x77
#define KEY_s        0x73
//==============================================================================

typedef struct TableInfoStruct
{
    unsigned char Code_Index;
    unsigned char Code_Value;
    unsigned char Key_str[12];
}TABLE_INFO_STRUCT;

TABLE_INFO_STRUCT Location_Code_Table[]=
{
    { 0,  0x00, 0},
    { 1,  0x0E, "~`"},    // ~`
    { 2,  0x16, "!1"},    // !1
    { 3,  0x1E, "@2"},    // @2
    { 4,  0x26, "#3"},    // #3
    { 5,  0x25, "$4"},    // $4
    { 6,  0x2E, "%5"},    // %5
    { 7,  0x36, "^6"},    // ^6
    { 8,  0x3D, "&7"},    // &7
    { 9,  0x3E, "*8"},    // *8
    {10,  0x46, "(9"},    // (9
    {11,  0x45, ")0"},    // )0
    {12,  0x4E, "_-"},    // _-
    {13,  0x55, "+="},    // +=
    {14,  0x6A, "|(JP)"}, // NA
    {15,  0x66, "Back"},  // Backspace
    
    {16,  0x0D, "Tab"},   // Tab
    {17,  0x15, "Q"},     // Q
    {18,  0x1D, "W"},     // W
    {19,  0x24, "E"},     // E
    {20,  0x2D, "R"},     // R
    {21,  0x2C, "T"},     // T
    {22,  0x35, "Y"},     // Y
    {23,  0x3C, "U"},     // U
    {24,  0x43, "I"},     // I
    {25,  0x44, "O"},     // O
    {26,  0x4D, "P"},     // P
    {27,  0x54, "{["},    // {[
    {28,  0x5B, "}]"},    // }]
    {29,  0x5D, "|(US)"}, // |  Key 29 is available on the US and not on the International Keyboard
    {30,  0x58, "CapsLk"},// Caps Lock
    {31,  0x1C, "A"},     // A
    
    {32,  0x1B, "S"},     // S
    {33,  0x23, "D"},     // D
    {34,  0x2B, "F"},     // F
    {35,  0x34, "G"},     // G
    {36,  0x33, "H"},     // H
    {37,  0x3B, "J"},     // J
    {38,  0x42, "K"},     // K
    {39,  0x4B, "L"},     // L
    {40,  0x4C, ":;"},    // :;
    {41,  0x52, "'"},     // "'
    {42,  0x5D, "42(UK)"},// NA
    {43,  0x5A, "Enter"}, // Enter
    {44,  0x88, "L-Sft"}, // L-Shift, 12
    {45,  0x61, "45(UK)"},// |  UK keyboard
    {46,  0x1A, "Z"},     // Z
    {47,  0x22, "X"},     // X
    
    {48,  0x21, "C"},    // C
    {49,  0x2A, "V"},    // V
    {50,  0x32, "B"},    // B
    {51,  0x31, "N"},    // N
    {52,  0x3A, "M"},    // M
    {53,  0x41, "<,"},   // <,
    {54,  0x49, ">."},   // >.
    {55,  0x4A, "?/"},   // ?/
    {56,  0x51, "56(JP)"},// JP 56
    {57,  0x89, "R-Sft"},// R-Shift, 59
    {58,  0x8C, "L-Ctl"},// L-Ctrl, 14
    {59,  0x8E, "Fn"},   // Fn
    {60,  0x8A, "L-Alt"},// L-Alt, 11
    {61,  0x29, "Space"},// Space Bar
    {62,  0x8B, "R-Alt"},// R-Alt, E0 11
    {63,  0x00, 0},      // NA
    
    {64,  0x8D, "R-Ctl"},// R-Ctrl, E0 14
    {65,  0x00, 0},      //
    {66,  0x00, 0},      //
    {67,  0x00, 0},      //
    {68,  0x00, 0},      //
    {69,  0x00, 0},      //
    {70,  0x00, 0},      //
    {71,  0x00, 0},      //
    {72,  0x00, 0},      //
    {73,  0x00, 0},      //
    {74,  0x00, 0},      //
    {75,  0xC2, "Ins"},  // Insert
    {76,  0xC0, "Del"},  // Delete
    {77,  0x00, 0} ,     //
    {78,  0x00, 0},      //
    {79,  0x9A, "Left"}, // Left-Arrow
    
    {80,  0x94, "Home"}, // Home
    {81,  0x95, "End"},  // End
    {82,  0x00, 0},      // 
    {83,  0x98, "Up"},   // Up-Arrow
    {84,  0x99, "Down"}, // Down-Arrow
    {85,  0x96, "PgUp"}, // Page_Up
    {86,  0x97, "PgDn"}, // Page_Down
    {87,  0x00, 0},      //
    {88,  0x00, 0},      //
    {89,  0x9B, "Right"},// Right-Arrow
    {90,  0x77, "NumLk"},// Num Lock
    {91,  0x9C, "Num7"}, // Num 7
    {92,  0xA0, "Num4"}, // Num 4
    {93,  0xA4, "Num1"}, // Num 1
    {94,  0x00, 0},      //
    {95,  0xAA, "Num/"}, // Num-/
    
    {96,  0x9D, "Num8"}, // Num 8
    {97,  0xA1, "Num5"}, // Num 5
    {98,  0xA5, "Num2"}, // Num 2
    {99,  0xA8, "Num0"}, // Num 0
    {100, 0x9F, "Num*"}, // Num *
    {101, 0x9E, "Num9"}, // Num 9
    {102, 0xA2, "Num6"}, // Num 6
    {103, 0xA6, "Num3"}, // Num 3
    {104, 0xA9, "Num."}, // Num .
    {105, 0xA3, "Num-"}, // Num -
    {106, 0xA7, "Num+"}, // Num +
    {107, 0x00, 0},      //
    {108, 0x81, "NumEn"},// Num Enter
    {109, 0x00, 0},      //
    {110, 0x76, "ESC"},  // ESC
    {111, 0x00, 0},      //
    
    {112, 0xE0, "F1"},   // F1
    {113, 0xE1, "F2"},   // F2
    {114, 0xE2, "F3"},   // F3
    {115, 0xE3, "F4"},   // F4
    {116, 0xE4, "F5"},   // F5
    {117, 0xE5, "F6"},   // F6
    {118, 0xE6, "F7"},   // F7
    {119, 0xE7, "F8"},   // F8
    {120, 0xE8, "F9"},   // F9
    {121, 0xE9, "F10"},  // F10
    {122, 0xEA, "F11"},  // F11
    {123, 0xEB, "F12"},  // F12
    {124, 0xC3, "PrtSc"},// Print Screen
    {125, 0x7E, "ScrLk"},// Scroll Lock
    {126, 0x91, "Pause"},// Pause
    
    {127, 0x82, "L-Win"},// L-Win
    {128, 0x83, "R-Win"},// R-Win
    {129, 0x84, "App"},     //
    {130, 0x00, 0},         //
    {131, 0x67, "131(JP)"}, //
    {132, 0x64, "132(JP)"}, //
    {133, 0x13, "133(JP)"}, //
    {134, 0x00, 0},    //
    {135, 0x00, 0},    //
    {136, 0x00, 0},    //
    {137, 0x00, 0},    //
    {138, 0x00, 0},    //
    {139, 0x00, 0},    //
    {140, 0x00, 0},    //
    {141, 0x00, 0},    //
    {142, 0x00, 0},    //
};

unsigned char KB_Key_Location_Code[8][19]={0};
unsigned char KB_Matrix_Code[8][19]={0};
unsigned char EC_Matrix_Code[8][19]={0};
unsigned char KB_KSI_Index[8]={0};
unsigned char KB_KSO_Index[19]={0};

FILE *CfgFile = NULL;

void Read_Matrix_File(void)
{
    char StrLine[1024];
    char *pStrLine;
    char *str;
    int  HexNum;
    int i=0,j=0;
    
    if((CfgFile = fopen("KB_Matrix.cfg","r")) == NULL)
    {
        printf("KB_Matrix.cfg not exist\n");
        return ;
    }
    
    i=0;
    while (!feof(CfgFile))
    {   
        // Read one line data
        fgets(StrLine,1024,CfgFile);
        #if DEBUG
        printf("%s", StrLine);
        #endif
        
        pStrLine = StrLine;
        if(('$'==StrLine[0]) && (('1'==StrLine[1])))
        {
            for(j=0; j<19; j++)
            {
                while((',' != (*pStrLine++)));
                KB_Key_Location_Code[i][j] = (int)strtol(pStrLine, &str, 10);
            }
            i++;
        }
        
        if(('$'==StrLine[0]) && (('2'==StrLine[1])))
        {
            for(j=0; j<8; j++)
            {
                while((',' != (*pStrLine++)));
                KB_KSI_Index[j] = (int)strtol(pStrLine, &str, 10);
            }
        }
        
        if(('$'==StrLine[0]) && (('3'==StrLine[1])))
        {
            for(j=0; j<19; j++)
            {
                while((',' != (*pStrLine++)));
                KB_KSO_Index[j] = (int)strtol(pStrLine, &str, 10);
            }
        }
    }

    fclose(CfgFile);
    
    printf("\nLocation Code list\n");
    for(i=0; i<8; i++)
    {
        for(j=0; j<19; j++)
        {
            printf("%03u  ", KB_Key_Location_Code[i][j]);
        }
        printf("\n");
    }
    
    printf("KSI Sequence\n");
    for(j=0; j<8; j++)
    {
        printf("%03u  ", KB_KSI_Index[j]);
    }
    printf("\n");
    
    printf("KSO Sequence\n");
    for(j=0; j<19; j++)
    {
        printf("%03u  ", KB_KSO_Index[j]);
    }
    printf("\n");
}

void Select_Matrix_Code(void)
{
    int i,j;
    
    for(i=0; i<8; i++)
    {
        for(j=0; j<19; j++)
        {
            KB_Matrix_Code[i][j] = 
                    Location_Code_Table[KB_Key_Location_Code[i][j]].Code_Index;
        }
    }
    
    #if DEBUG
    printf("\n\n\nMatrix Code list\n");
    for(i=0; i<8; i++)
    {
        for(j=0; j<16; j++)
        {
            printf("0x%02X,", KB_Matrix_Code[i][j]);
        }
        printf("\n");
    }
    
    printf("\n");
    for(i=0; i<8; i++)
    {
        for(j=16; j<19; j++)
        {
            printf("0x%02X,", KB_Matrix_Code[i][j]);
        }
        printf("\n");
    }
    #endif
}

void Adjust_Matrix_Code(void)
{
    int i;
    int j;
    
    for(i=0; i<8; i++)
    {
        for(j=0; j<19; j++)
        {
            EC_Matrix_Code[KB_KSI_Index[i]][KB_KSO_Index[j]] = KB_Matrix_Code[i][j];
        }
    }
    
    SetPosition_X_Y(3, 14);
    printf("\n\n\nEC Matrix Code list\n");
    for(i=0; i<8; i++)
    {
        SetPosition_X_Y(0,20+(2*i));
        printf("//");
        for(j=0; j<16; j++)
        {
            SetPosition_X_Y(3+7*j,20+(2*i));
            printf("%s", Location_Code_Table[EC_Matrix_Code[i][j]].Key_str);
            SetPosition_X_Y(3+7*j,21+(2*i));
            printf("0x%02X,", Location_Code_Table[EC_Matrix_Code[i][j]].Code_Value);
        }
    }
    
    printf("\n");
    for(i=0; i<8; i++)
    {
        SetPosition_X_Y(0,38+(2*i));
        printf("//");
        for(j=0; j<3; j++)
        {
            SetPosition_X_Y(3+7*j,38+(2*i));
            printf("%s", Location_Code_Table[EC_Matrix_Code[i][j+16]].Key_str);
            SetPosition_X_Y(3+7*j,39+(2*i));
            printf("0x%02X,", Location_Code_Table[EC_Matrix_Code[i][j+16]].Code_Value);
        }
    }
}

int main(int argc, char *argv[])
{
    int i;    int j;
    
    SetTextColor(EFI_WHITE, EFI_BLACK);
    system("mode con cols=150 lines=100");
    system("cls");
    
    Read_Matrix_File();
    Select_Matrix_Code();
    Adjust_Matrix_Code();

    goto end;

end:
    return 0;
}