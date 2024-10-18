#include "pch.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "Ai.h"
#include "Game.h"
#include "Utils.h"
#include "Hash.h"
#include "mtwister.h"

// The global game variable
Game G;
GameSettings Settings;

GameSettings* GetSettings() {
	return &Settings;
}

Game* GetGame() {
	return &G;
}

void InitSeed(Game* g, int seed) {
	/*g->rnd_seed = 1070372;
	for (int i = 0; i < seed; i++)
		LlrandShift(g);*/
	seedRand(seed, &g->rand);
}

//U64 LlrandShift(Game* g) {
//	U64 r = g->rnd_seed;
//	r ^= r >> 12, r ^= r << 25, r ^= r >> 27;
//	g->rnd_seed = r;
//	return r * 2685821657736338717LL;
//}

void Reset(Game* g) {
	for (int i = 0; i < 26; i++)
		g->Position[i] = 0;
	g->WhiteHome = 0;
	g->BlackHome = 0;
	g->CurrentPlayer = Black;

	g->Dice[0] = 0;
	g->Dice[1] = 0;
	g->Turns = 0;
}

int CountAllCheckers(PlayerSide side, Game* game) {
	int count = 0;
	for (int i = 0; i < 26; i++)
	{
		if (game->Position[i] & side)
			count += CheckerCount(game->Position[i]);
	}
	if (side == Black)
		count += game->BlackHome;
	if (side == White)
		count += game->WhiteHome;
	return count;
}



void StartPosition(Game* g) {
	Reset(g);
	g->Position[1] = 2 | Black;
	g->Position[6] = 5 | White;
	g->Position[8] = 3 | White;
	g->Position[12] = 5 | Black;
	g->Position[13] = 5 | White;
	g->Position[17] = 3 | Black;
	g->Position[19] = 5 | Black;
	g->Position[24] = 2 | White;
	g->BlackLeft = 167;
	g->WhiteLeft = 167;
	StartHash(g);
}

void WriteGameString(char* s, Game* g) {

	int idx = 0;
	for (size_t i = 0; i < 26; i++)
	{
		if (g->Position[i] > 0) {
			ushort black = g->Position[i] & Black;
			if (black) {
				s[idx++] = 'b';
			}
			else {
				s[idx++] = 'w';
			}
		}

		char* n = (char*) malloc(5);
		int length = sprintf_s(n, 4, "%d", CheckerCount(g->Position[i]));
		memcpy(&s[idx], n, length);
		idx += length;
		//s[idx++] = '0' + CheckerCount(g->Position[i]);
		s[idx++] = ' ';
	}
	s[idx++] = '0' + g->WhiteHome;
	s[idx++] = ' ';
	s[idx++] = '0' + g->BlackHome;
	s[idx++] = ' ';
	s[idx++] = g->CurrentPlayer == Black ? 'b' : 'w';
	s[idx++] = ' ';
	s[idx++] = '0' + g->Dice[0];
	s[idx++] = ' ';
	s[idx++] = '0' + g->Dice[1];
	s[idx] = '\0';
}

void SetPointsLeft(Game* g) {
	ushort blackLeft = 0;
	ushort whiteLeft = 0;
	for (int i = 0; i < 25; i++)
	{
		if (g->Position[i] & Black)
			blackLeft += (25 - i) * CheckerCount(g->Position[i]);
		if (g->Position[i] & White)
			whiteLeft += i * CheckerCount(g->Position[i]);
	}
	g->BlackLeft = blackLeft;
	g->WhiteLeft = whiteLeft;
}

void ReadGameString(const char* s, Game* g) {
	Reset(g);
	// 31 tokens
	// BlackBar, pos 1 - 24, WhiteBar, BlackHome, WhiteHome, turn, dice1, dice2
	// "0 b2 0 0 0 0 w5 0 w3 0 0 0 b5 w5 0 0 0 b3 0 b5 0 0 0 0 w2 0 0 0 b 5 6"

	size_t len = strlen(s);
	char copy[100];
	memcpy(copy, s, len + 1);

	char* context = NULL;
	char* token = strtok_s(copy, " ", &context);
	VERIFY(token != NULL);

	for (size_t i = 0; i < 26; i++)
	{
		char n[100];

		if (StartsWith(token, "b")) {
			SubString(token, n, 2, 3);
			g->Position[i] = atoi(n);
			if (g->Position[i] > 0)
				g->Position[i] |= Black;
		}
		else if (StartsWith(token, "w")) {
			SubString(token, n, 2, 3);
			g->Position[i] = atoi(n);
			if (g->Position[i] > 0)
				g->Position[i] |= White;
		}

		token = strtok_s(NULL, " ", &context);
		VERIFY(token != NULL);
	}
	g->WhiteHome = atoi(token);
	token = strtok_s(NULL, " ", &context);
	VERIFY(token != NULL);

	g->BlackHome = atoi(token);
	token = strtok_s(NULL, " ", &context);
	VERIFY(token != NULL);

	g->CurrentPlayer = Streq("b", token) ? Black : White;
	token = strtok_s(NULL, " ", &context);
	VERIFY(token != NULL);

	g->Dice[0] = atoi(token);
	token = strtok_s(NULL, " ", &context);
	VERIFY(token != NULL);

	g->Dice[1] = atoi(token);

	SetPointsLeft(&G);
	StartHash(g);
}

char LastMove[400];

void SetLastMove(const char* s) {
	strcpy_s(LastMove, 400, s);
}

void PrintGame(Game* g) {
	// ╔═══╤═══╤═══╤═══╤═══╤═══╤═══╤═══╗
	char top[] = { 201,205,205,205,209,205,205,205,209,205,205,205,209,205,205,205,209,205,205,205,209,205,205,205,203,205,205,205,209,205,205,205,209,205,205,205,209,205,205,205,209,205,205,205,209,205,205,205,187,0 };
	// ╟───┼───┼───┼───┼───┼───┼───┼───╢
	char rowLine[] = { 199,196,196,196,197,196,196,196,197,196,196,196,197,196,196,196,197,196,196,196,197,196,196,196,215,196,196,196,197,196,196,196,197,196,196,196,197,196,196,196,197,196,196,196,197,196,196,196,182,0 };
	// ╚═══╧═══╧═══╧═══╧═══╧═══╧═══╧═══╝
	char lastLine[] = { 200,205,205,205, 207,205,205,205, 207,205,205,205, 207,205,205,205, 207,205,205,205, 207,205,205,205, 202,205,205,205, 207,205,205,205, 207,205,205,205, 207,205,205,205, 207,205,205,205, 207,205,205,205, 188, 0 };
	// ║

	// https://en.wikipedia.org/wiki/Code_page_437
	char vBorder = 186; //║
	char vLine = 179; // │
	//char smile[] = { 32 , 2, 32, 0 }; // ☻
	system("cls");

	printf("  12   11  10  9   8   7   6   5   4   3   2   1\n");
	printf(" %s\n", top);
	for (int y = 0; y < 10; y++)
	{
		printf(" %c", vBorder);
		for (int x = 0; x < 12; x++)
		{
			PlayerSide color = (PlayerSide)(g->Position[12 - x] & 48);
			int count = CheckerCount(g->Position[12 - x]);
			if (count > y)
				color == White ? PrintYellow(" O ") : PrintBlue(" X ");
			else
				printf("   ");
			if (x < 11)
				x == 5 ? printf("%c", vBorder) : printf("%c", vLine);
		}
		printf("%c", vBorder);
		printf("\n");
	}

	printf(" %s\n", rowLine);

	for (int y = 0; y < 10; y++)
	{
		printf(" %c", vBorder);
		for (int x = 13; x <= 24; x++)
		{
			PlayerSide color = (PlayerSide) (g->Position[x] & 48);
			int count = CheckerCount(g->Position[x]);
			if (count >= 10 - y)
				color == White ? PrintYellow(" O ") : PrintBlue(" X ");
			else
				printf("   ");
			if (x < 24)
				x == 18 ? printf("%c", vBorder) : printf("%c", vLine);
		}
		printf("%c", vBorder);
		printf("\n");
	}
	printf(" %s\n", lastLine);

	printf("  13   14  15 16  17  18  19  20  21  22  23  24\n");
	printf("Black X: %d   Bar: %d   Home: %d\n", g->BlackLeft, CheckerCount(g->Position[0]), g->BlackHome);
	printf("White O: %d   Bar: %d   Home: %d\n", g->WhiteLeft, CheckerCount(g->Position[25]), g->WhiteHome);
	if (g->CurrentPlayer == Black)
		PrintBlue("Blacks turn. ");
	else if (g->CurrentPlayer == White)
		PrintYellow("Whites turn. ");
	printf("Dice: %d, %d\n", g->Dice[0], g->Dice[1]);
	char gs[500];
	WriteGameString(gs, g);
	printf("LastMove: %s\n", LastMove);
	printf("%s\n", gs);
	printf("Last eval count: %d\n", g->EvalCounts);
	fflush(stdout);
}

