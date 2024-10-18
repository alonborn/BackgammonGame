#include "pch.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <Windows.h>
#include <tchar.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "Ai.h"
#include "Game.h"
#include "utils.h"
#include "Tests.h"
#include "Ai.h"
#include "main.h"
#include "Hash.h"
#include "Trainer.h"

bool Searching;

int nonAIPlayer = Black;
int nonAIBar = 0;
int nonAIHome = 25;

void PrintHelp() {
	//ConsoleWriteLine("\nOptions\n-------\nt/test\np/play\npos w2 0 0 0 b5... (game string)\ns/selected tests\ng/game\nh/help\nq/quit");
}

void PrintCurrentDir() {
	TCHAR c[500];
	GetCurrentDirectory(500, c);
	_tprintf(TEXT("\nDir: %s\n"), c);
}


void PrintBest() {
	Game* g = GetGame();
	if (Searching) {
		printf("AI busy searcing\n");
		fflush(stdout);
		return;
	}
	MoveSet set;
	set.Length = 0;
	Searching = true;
	InitAiManual(GetAi(0));
	InitAiManual(GetAi(1));
	int depth = g->Dice[0] == g->Dice[1] ? 1 : 1;
	int setIdx = FindBestMoveSet(g, &set, depth);

	fflush(stdout);
	PrintSet(set);
	Searching = false;
}


// 11 1111, count & 15, color & 15
// 0 Black Bar, 1 - 24 common, 25 White Bar


void ChangePlayer()
{
	Game* g = GetGame();
	g->CurrentPlayer = (PlayerSide) OtherColor(g->CurrentPlayer);
	g->Turns++;
	RollDice(g);
	PrintGame(g);
}

void GetRemainingMoves(char* remaining, int* allMoves)
{
	strcpy_s(remaining, 100, "");
	for (int i = 0; i < 4; i++)
	{
		if (allMoves[i] != 0)
		{
			char strchar[100];
			sprintf_s(strchar, 100, "%d ", allMoves[i]);
			strcat_s(remaining, 100, strchar);
		}
	}
}
void DoManualMoves()
{
	MoveSet set;
	Game* g = GetGame();
	int numOfMoves = 2;
	if (g->Dice[0] == g->Dice[1])
		numOfMoves = 4;

	int movements[4];

	for (int i = 0; i < 4; i++)
	{
		movements[i] = 0;
	}

	if (numOfMoves == 2)
	{
		movements[0] = g->Dice[0];
		movements[1] = g->Dice[1];
		movements[2] = 0;
		movements[3] = 0;
	}
	else
	{
		movements[0] = g->Dice[0];
		movements[1] = g->Dice[0];
		movements[2] = g->Dice[1];
		movements[3] = g->Dice[1];
	}

	for (int i = 0; i < numOfMoves; i++)
	{
		set.Length = 0;
		Game* g = GetGame();
		printf("From: ");
		fflush(stdout);
		int from = 0;
		scanf_s("%d", &from);
		printf("To: ");
		int to = 0;
		scanf_s("%d", &to);
		if (from == 0 && to == 0)
			return;

		//user has no moves, give turn to other player
		if (from == -1)
		{
			ChangePlayer();
			return;
		}

		set.Moves[0].from = from;
		set.Moves[0].to = to;
		set.Moves[0].color = (PlayerSide) nonAIPlayer;
		set.Length = 1;



		if (set.Moves[0].from != nonAIBar && set.Moves[0].from != nonAIHome)
		{
			int found = -1;
			Game* g = GetGame();
			for (int j = 0; j < 4; j++)
			{
				int dist = abs(set.Moves[0].from - set.Moves[0].to);
				if (movements[j] == dist)
				{
					found = j;
					break;
				}
			}
			bool moveValid = IsMoveValid(set.Moves[0], g);
			if (found >= 0 && moveValid)
			{
				DoMove(set.Moves[0], g);
				movements[found] = 0;
				char remainingMoves[100] = "Remaining: ";
				char movementsStr[100] = "";
				GetRemainingMoves(movementsStr, movements);
				strcat_s(remainingMoves, 100, movementsStr);
				SetLastMove(remainingMoves);
			}
			else
			{
				SetLastMove("invalid move");
				i--;
			}
		}
		else
		{
			if (IsMoveValid(set.Moves[0], g))
			{
				DoMove(set.Moves[0], g);
				char remainingMoves[100] = "Remaining: ";
				char movementsStr[100] = "";
				GetRemainingMoves(movementsStr, movements);
				strcat_s(remainingMoves, 100, movementsStr);
				SetLastMove(remainingMoves);
			}
			else
			{
				SetLastMove("invalid move");
				i--;
			}
		}
		PrintGame(g);
	}

	ChangePlayer();

}

MoveSet CalculateBestMove(Game* g)
{
	MoveSet set;
	set.Length = 0;
	if (Searching) {
		printf("AI busy searcing\n");
		fflush(stdout);
		return set;
	}
	

	//Searching = true;
	InitAiManual(GetAi(0));
	InitAiManual(GetAi(1));

	//InitAiManual(&AIs[0]);
	//InitAiManual(&AIs[1]);
	int depth = g->Dice[0] == g->Dice[1] ? 1 : 1;

	int setIdx = FindBestMoveSet(g, &set, depth);

	fflush(stdout);
	PrintSet(set);
	return set;

}


void DoBestMoveAI() {
	Game* g= GetGame();
	MoveSet set = CalculateBestMove(g);
	char moveStr[400];
	GetSetAsString(moveStr, set);
	SetLastMove(moveStr);

	for (int i = 0; i < set.Length; i++)
		DoMove(set.Moves[i], g);

	Searching = false;
	g->CurrentPlayer = (PlayerSide) OtherColor(g->CurrentPlayer);
	g->Turns++;
	RollDice(g);
	PrintGame(g);
}



extern "C"  __declspec(dllexport) void SetBoard(char* board)
{
	Game* game = GetGame();

	std::string boardStr = board;

	std::vector<std::string> tokens;
	std::stringstream ss(boardStr);
	std::string token;

	while (std::getline(ss, token, ' ')) {
		tokens.push_back(token);
	}

	int pos = 0;
	for (const auto& t : tokens) {
		if (t.c_str() == "0")
			game->Position[pos++] = 0;
		else {
			char color = t.c_str()[0];
			int count = atoi(t.c_str() + 1);
			
			game->Position[pos++] = count + (color == 'W')?32:16;
		}
		
	}
}

extern "C"  __declspec(dllexport) void SetDice(int dice1, int dice2)
{
	Game* game = GetGame();
	game->Dice[0] = dice1;
	game->Dice[1] = dice2;
}

char nextMove[400];
extern "C"  __declspec(dllexport) char* GetNextMove(const char* gameString)
{
	printf("best move (DLL): ");
	Game* g = GetGame();
	ReadGameString(gameString, g);
	printf("2222 ");
	MoveSet set = CalculateBestMove(g);
	printf("3333");
	GetSetAsString(nextMove, set);
	SetLastMove(nextMove);
	printf("best move (DLL): ");
	printf("best move (DLL): %s", nextMove);
	return nextMove;
}

extern "C"  __declspec(dllexport) void InitGame()
{
	system("cls");
	Game* g = GetGame();
	//seedRand(1234, &g_Rand);
	seedRand(time(NULL), GetRand());
	srand(time(NULL));

	//Default values
	GetSettings()->DiceQuads = 4;
	GetSettings()->MaxTurns = 2000;
	GetSettings()->PausePlay = false;
	GetSettings()->SearchDepth = 1;
	Searching = false;

	printf("Welcome to backgammon\n: ");
	//TestTraining();
	/*for (int c = 170; c < 255; c++)
		printf("%d %c\n\n", c, c);*/
		//CheckerCountAssert = true; // Dont change this here. Do it in the tests and switch back after test is done.	
		// Changing to a nice code page.
	system("chcp 437");
	SetDiceCombinations();
	InitSeed(g, 100);
	InitAi(GetAi(0), true);
	InitAi(GetAi(1), true);
	InitHashes();
	StartPosition(g);
	g->Dice[0] = 3;
	g->Dice[1] = 5;
	//PrintGame(&G);
	//PrintCurrentDir();
	printf("ready\n");
	printf("\n: ");
	fflush(stdout);
}

extern "C"  __declspec(dllexport) void RunGame() 
{
	InitGame();
	Game* g = GetGame();
	//char buf[BUF_SIZE];
	//fgets(buf, BUF_SIZE, stdin);
	PrintGame(g);
	//while (!Streq(buf, "quit\n") && !Streq(buf, "q\n"))
	while (true)
	{/*
		if (Streq(buf, "test\n") || Streq(buf, "t\n")) {
			RunAllTests();
		}
		else if (Streq(buf, "s\n")) {
			RunSelectedTests();
		}
		else if (Streq(buf, "play\n") || Streq(buf, "p\n")) {
			PlayAndEvaluate();
		}
		else if (StartsWith(buf, "board ")) {
			ReadGameString(&buf[6], &G);
			PrintBest();
		}
		else if (StartsWith(buf, "bm")) {
			if (G.CurrentPlayer == Black)
				DoBestMoveAI();
			else
				DoManualMoves();

		}
		else if (Streq(buf, "game\n") || Streq(buf, "g\n") ) {
			PrintGame(&G);
		}
		else if (Streq(buf, "search\n")) {
			printf("searching...\n");
			fflush(stdout);
			PrintBest();
		}
		else if (Streq(buf, "w\n") || Streq(buf, "watch\n")) {
			WatchGame();
		}
		else if (Streq(buf, "help\n") || Streq(buf, "h\n")) {
			PrintHelp();
		}
		else {
			ConsoleWriteLine("Unknown command");
			PrintHelp();
		}
		printf(": ");
		fflush(stdout);
		fgets(buf, BUF_SIZE, stdin);*/
		if (g->CurrentPlayer == nonAIPlayer)
			DoManualMoves();
		else
			DoBestMoveAI();
		//DoBestMoveAI();
	//Pause(&G);
	}
	char heart = 3;
	printf("Bye %c", heart);
}

void RunGameInApp()
{
	RunGame();
}