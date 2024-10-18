// BackgammonCPPApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Game.h"

extern "C"  __declspec(dllexport) void SetBoard(char* board);
extern "C"  __declspec(dllexport) void SetDice(int dice1, int dice2);
extern "C"  __declspec(dllexport) char* GetNextMove(const char* board);
extern "C"  __declspec(dllexport) void InitGame();
extern "C"  __declspec(dllexport) void RunGame();


void RunGameInApp();

int main()
{
    RunGameInApp();
    //char board[200] = "0 0 0 b1 0 0 w4 w1 w2 0 0 b1 b5 w5 0 0 0 b3 0 b5 0 0 0 0 w2 0 0 0 w 1 3 0";
    InitGame();
    //SetBoard(board);
    char retVal[300];
    //strcpy_s(retVal,300, GetNextMove(board));
    char board2[200] = "b0 0 w5 w6 w2 0 0 0 0 0 0 0 b1 w1 b1 b1 0 b1 0 b1 b2 b1 b3 b2 b2 w1 0 0 w 1 4 0";
                       
       
    char retVal2[300];
    strcpy_s(retVal2, 300, GetNextMove(board2));
    
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
