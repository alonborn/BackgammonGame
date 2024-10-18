
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <Windows.h>
#include <tchar.h>

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
	ConsoleWriteLine("\nOptions\n-------\nt/test\np/play\npos w2 0 0 0 b5... (game string)\ns/selected tests\ng/game\nh/help\nq/quit");
}

void PrintCurrentDir() {
	TCHAR c[500];
	GetCurrentDirectory(500, c);
	_tprintf(TEXT("\nDir: %s\n"), c);
}


void PrintBest() {
	if (Searching) {
		printf("AI busy searcing\n");
		fflush(stdout);
		return;
	}
	MoveSet set;	
	set.Length = 0;
	Searching = true;
	InitAiManual(&AIs[0]);
	InitAiManual(&AIs[1]);
	int depth = G.Dice[0] == G.Dice[1] ? 1 : 1;
	int setIdx = FindBestMoveSet(&G, &set, depth);

	fflush(stdout);	
	PrintSet(set);	
	Searching = false;
}

void ChangePlayer()
{
	G.CurrentPlayer = OtherColor(G.CurrentPlayer);
	G.Turns++;
	RollDice(&G);
	PrintGame(&G);
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
	int numOfMoves = 2;
	if (G.Dice[0] == G.Dice[1])
		numOfMoves = 4;

	int movements[4];

	for (int i = 0; i < 4; i++)
	{
		movements[i] = 0;
	}

	if (numOfMoves == 2)
	{
		movements[0] = G.Dice[0];
		movements[1] = G.Dice[1];
		movements[2] = 0;
		movements[3] = 0;
	}
	else
	{
		movements[0] = G.Dice[0];
		movements[1] = G.Dice[0];
		movements[2] = G.Dice[1];
		movements[3] = G.Dice[1];
	}

	for (int i = 0; i < numOfMoves; i++)
	{
		set.Length = 0;
		char moveStr[400];
		
		printf("From: ");
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
		set.Moves[0].color = nonAIPlayer;
		set.Length = 1;
		

		
		if (set.Moves[0].from != nonAIBar && set.Moves[0].from != nonAIHome)
		{
			int found = -1;
			for (int j = 0; j < 4; j++)
			{
				int dist = abs(set.Moves[0].from - set.Moves[0].to);
				if (movements[j] == dist)
				{
					found = j;
					break;
				}
			}
			bool moveValid = IsMoveValid(set.Moves[0], &G);
			if (found >= 0 && moveValid)
			{
				DoMove(set.Moves[0], &G);
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
			if (IsMoveValid(set.Moves[0], &G))
			{
				DoMove(set.Moves[0], &G);
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
		PrintGame(&G);
	}
	
	ChangePlayer();
	
}
void DoBestMoveAI() {
	if (Searching) {
		printf("AI busy searcing\n");
		fflush(stdout);
		return;
	}
	MoveSet set;
	set.Length = 0;
	Searching = true;
	InitAiManual(&AIs[0]);
	InitAiManual(&AIs[1]);
	int depth = G.Dice[0] == G.Dice[1] ? 1 : 1;

	int setIdx = FindBestMoveSet(&G, &set, depth);

	fflush(stdout);
	PrintSet(set);

	char moveStr[400];
	GetSetAsString(moveStr, &set);
	SetLastMove(moveStr);
	

	for (int i = 0 ; i < set.Length ; i++)
		DoMove(set.Moves[i], &G);

	Searching = false;
	G.CurrentPlayer = OtherColor(G.CurrentPlayer);
	G.Turns++;
	RollDice(&G);
	PrintGame(&G);
}
int main() {
	system("cls");

	//seedRand(1234, &g_Rand);
	seedRand(time(NULL), &g_Rand);
	srand(time(NULL));

	//Default values
	Settings.DiceQuads = 4;
	Settings.MaxTurns = 2000;
	Settings.PausePlay = false;
	Settings.SearchDepth = 1;
	Searching = false;
	
	printf("Welcome to backgammon\n: ");
	//TestTraining();
	/*for (int c = 170; c < 255; c++)
		printf("%d %c\n\n", c, c);*/
	CheckerCountAssert = true; // Dont change this here. Do it in the tests and switch back after test is done.	
	// Changing to a nice code page.
	system("chcp 437");
	SetDiceCombinations();
	InitSeed(&G, 100);
	InitAi(&AIs[0], true);
	InitAi(&AIs[1], true);
	InitHashes();
	StartPosition(&G);
	G.Dice[0] = 3;
	G.Dice[1] = 5;
	//PrintGame(&G);
	//PrintCurrentDir();
	printf("ready\n");
	printf("\n: ");
	fflush(stdout);

	char buf[BUF_SIZE];
	//fgets(buf, BUF_SIZE, stdin);
	PrintGame(&G);
	while (!Streq(buf, "quit\n") && !Streq(buf, "q\n"))
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
		if (G.CurrentPlayer == nonAIPlayer)
			DoManualMoves();
		else
			DoBestMoveAI();
			//DoBestMoveAI();
		//Pause(&G);
	}
	char heart = 3;
	printf("Bye %c", heart);
}
