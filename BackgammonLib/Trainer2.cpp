#include "pch.h"
#include <stdlib.h>
#include <Windows.h>
#include <stdio.h>
#include <stdio.h>

#include "Utils.h"
#include "Game.h"
#include "Trainer.h"
#include "Ai.h"

TrainerStruct Trainer;
Game* ThreadGames;
Game* GetThreadGame() {
	return ThreadGames;
}
TrainerStruct* GetTrainer() {
	return &Trainer;
}

DWORD WINAPI  TrainParaThreadStart(LPVOID lpParam) {
	int threadNo = *((int*)lpParam);
	PlayGame(&ThreadGames[threadNo]);
	return 0;
}

// Comparing two AIs.
void PlayBatchMatch(AiConfig ai0, AiConfig ai1, int gameCount, int* score) {

	int batches = gameCount / ThreadCount; // batches * threads == no games.

	SetAi(0,&ai0);
	SetAi(1,&ai1);

	for (int i = 0; i < batches; i++)
	{
		HANDLE* handles = (HANDLE*) malloc(sizeof(HANDLE) * ThreadCount);
		for (int t = 0; t < ThreadCount; t++)
			handles[t] = CreateThread(NULL, 0, TrainParaThreadStart, &t, 0, 0);
		printf("WaitForMultipleObjects");
		WaitForMultipleObjects(ThreadCount, handles, TRUE, INFINITE);
		for (int t = 0; t < ThreadCount; t++)
			CloseHandle(handles[t]);

		free(handles);

		// Evaluate the games.
		for (int t = 0; t < ThreadCount; t++)
		{
			if (ThreadGames[t].BlackLeft == 0) {
				score[0] ++;
			}
			else if (ThreadGames[t].WhiteLeft == 0) {
				score[1] ++;
			}
			//else {
			//	// Added a limit for number of moves. So there can be a draw.
			//	//ASSERT_DBG(false); // There is no draw in backgammon
			//}
		}
	}
}

void NewGeneration() {
	MTRand* g_Rand = GetRand();
	Trainer.Generation++;
	// Select two best, and make 10 new children, randomize split pos of lists. Add a random mutation to two of them.
	double bf0[26];// = TrainedSet[0].BlotFactors;
	double bf1[26];// = TrainedSet[1].BlotFactors;
	//double cf0[26];// = TrainedSet[0].ConnectedBlocksFactor;
	//double cf1[26];// = TrainedSet[1].ConnectedBlocksFactor;
	memcpy(&bf0, &Trainer.Set[0].BlotFactors, 26 * sizeof(double));
	memcpy(&bf1, &Trainer.Set[1].BlotFactors, 26 * sizeof(double));
	//memcpy(&cf0, &Trainer.Set[0].ConnectedBlocksFactor, 26 * sizeof(double));
	//memcpy(&cf1, &Trainer.Set[1].ConnectedBlocksFactor, 26 * sizeof(double));

	//Keep the best
	for (int i = 1; i < TrainedSetCount; i++)
	{
		//combining values from 0 and 1.
		int split = RandomInt(g_Rand, 0, 26);
		if (split > 0)
			memcpy(&Trainer.Set[i].BlotFactors, &bf0, split * sizeof(double));
		if (split < 26)
			memcpy(&Trainer.Set[i].BlotFactors[split], &bf1[split], (26ll - split) * sizeof(double));

		/*split = RandomInt(&g_Rand, 0, 26);
		if (split > 0)
			memcpy(&Trainer.Set[i].ConnectedBlocksFactor, &cf0, split * sizeof(double));
		if (split < 26)
			memcpy(&Trainer.Set[i].ConnectedBlocksFactor[split], &cf1[split], (26ll - split) * sizeof(double));*/

		double f = RandomDouble(g_Rand, 0.96, 1.04);
		for (int x = 0; x < 26; x++)
		{
			Trainer.Set[i].BlotFactors[x] *= f;
			Trainer.Set[i].ConnectedBlocksFactor[x] *= f;
		}
	}

	// Mutations
	int set = RandomInt(g_Rand, 0, TrainedSetCount);
	int rndIdx = RandomInt(g_Rand, 0, 25);
	double val = RandomDouble(g_Rand, 0, 1);
	Trainer.Set[set].BlotFactors[rndIdx] = val;

	set = RandomInt(g_Rand, 0, TrainedSetCount);
	rndIdx = RandomInt(g_Rand, 0, 25);
	val = RandomDouble(g_Rand, 0, 1);
	Trainer.Set[set].ConnectedBlocksFactor[rndIdx] = val;
}

void InitTrainer() {
	if (!LoadTrainedSet("TrainedSet"))
	{
		Trainer.Generation = 0;
		// 1. Make Random sets of Ais
		for (int i = 0; i < TrainedSetCount; i++)
		{
			InitAi(&Trainer.Set[i], false);
			Trainer.Set[i].Id = i + 1;
		}
	}

	MTRand* g_Rand = GetRand();

	ThreadGames = (Game*) malloc(sizeof(Game) * ThreadCount);
	if (ThreadGames != NULL) {
		for (int t = 0; t < ThreadCount; t++)
			InitSeed(&ThreadGames[t], RandomInt(g_Rand, 1, 100));
	}
}

void CheckDataIntegrity() {
	for (int i = 0; i < TrainedSetCount; i++)
	{
		if (Trainer.Set[i].Id < 1 || Trainer.Set[i].Id > TrainedSetCount) {
			printf("\nTrainedSet Id invalid");
			exit(100);
		}

		if (Trainer.Set[i].SearchDepth < 0 || Trainer.Set[i].SearchDepth > 2) {
			printf("\nTrainedSet SearchDepth invalid");
			exit(101);
		}

		if (Trainer.Set[i].Score < 0) {
			printf("\nTrainedSet Score invalid");
			exit(102);
		}

		for (int f = 0; f < 26; f++)
		{
			if (Trainer.Set[i].BlotFactors[f] < 0) {
				printf("\nTrainedSet Blotfactor invalid");
				exit(103);
			}
			if (Trainer.Set[i].ConnectedBlocksFactor[f] < 0) {
				printf("\nTrainedSet ConnectedBlocksFactor invalid");
				exit(104);
			}
		}
	}
}

void SaveTrainedSet(int generation, const char* name) {
	CheckDataIntegrity();
	char fName1[100];
	sprintf_s(fName1, sizeof(fName1), "%s.bin", name);
	FILE* file1;
	fopen_s(&file1, fName1, "wb");
	if (file1 != NULL) {
		fwrite(&Trainer, sizeof(Trainer), 1, file1);
		fclose(file1);
	}
	else {
		printf("\nError. Failed to open file %s in SaveTrainedSet\n", fName1);
	}

	//Also saving generation.
	char fName2[100];
	sprintf_s(fName2, sizeof(fName2), "%s_Gen_%d.bin", name, Trainer.Generation);
	FILE* file2;
	fopen_s(&file2, fName2, "wb");
	if (file2 != NULL) {
		fwrite(&Trainer, sizeof(Trainer), 1, file2);
		fclose(file2);
	}
	else {
		printf("\nError. Failed to open file %s in SaveTrainedSet\n", fName2);
	}
}

bool LoadTrainedSet(const char* name) {
	char fName[100];
	sprintf_s(fName, sizeof(fName), "%s.bin", name);

	FILE* file;
	fopen_s(&file, fName, "rb");

	if (file != NULL) {
		fread(&Trainer, sizeof(Trainer), 1, file);
		fclose(file);
		return true;
	}
	else {
		printf("\nFailed to open file in LoadTrainedSet\n");
		return false;
	}
}

int CompareScores(const void* a, const void* b) {
	AiConfig* aConfig = (AiConfig*)a;
	AiConfig* bConfig = (AiConfig*)b;
	
	return bConfig->Score - aConfig->Score;
}

double CompareAIs(AiConfig trained, AiConfig untrained) {
	untrained.Score = 0;
	trained.Score = 0;

	int score1[2] = { 0, 0 };
	int scrUntrained = 0;
	int scrTrained = 0;
	int n = 1500;
	PlayBatchMatch(trained, untrained, n, score1);
	scrTrained += score1[0];
	scrUntrained += score1[1];

	int score2[2] = { 0, 0 };
	PlayBatchMatch(untrained, trained, n, score2);
	scrTrained += score2[1];
	scrUntrained += score2[0];

	double pct = ((double)100 * (double)scrTrained) / ((double)scrTrained + (double)scrUntrained);
	printf("\nScore for trained vs untrained: %d-%d (%.2f%s)", scrTrained, scrUntrained, pct, "%");
	return pct;
}

void SaveProgress(double progress, int fullGames) {
	char text[100];
	snprintf(text, sizeof(text), "%d;%.1f;%d\n", Trainer.Generation, progress, fullGames);

	char fileName[] = "Progress.csv";
	FILE* stream;
	fopen_s(&stream, fileName, "a");
	if (stream != NULL)
	{
		fwrite(text, strlen(text), 1, stream);
		fclose(stream);
	}
}

void SaveFactors(const char* fileName, double* factors) {

	char text[100];
	snprintf(text, sizeof(text), "%d;", Trainer.Generation);

	FILE* stream;
	fopen_s(&stream, fileName, "a");
	if (stream != NULL)
	{
		fwrite(text, strlen(text), 1, stream);
		for (int i = 0; i < 26; i++)
		{
			char buf[10];
			snprintf(buf, sizeof(buf), ";%.2f", factors[i]);
			fwrite(buf, strlen(buf), 1, stream);
		}
		char nl[] = "\n\0";
		fwrite(nl, strlen(nl), 1, stream);
		fclose(stream);
	}
}

void Train() {
	GetSettings()->DiceQuads = 4;
	// When untrained many games goes on for very long because the AIs are so bad at leaving blots.
	// But after some generations it gets better and it is very rare with long games.
	GetSettings()->MaxTurns = 400;
	GetSettings()->SearchDepth = 0;
	GetSettings()->PausePlay = false;

	InitTrainer();
	AiConfig untrained;
	InitAi(&untrained, true);
	int genCount = 500;
	for (int gen = 0; gen < genCount; gen++)
	{
		printf("\nGeneration %d\n", Trainer.Generation);
		printf("==============\n");
		for (int i = 0; i < TrainedSetCount; i++)
			Trainer.Set[i].Score = 0;

		// Combine all configs
		for (int i = 0; i < TrainedSetCount; i++)
		{
			for (int j = i + 1; j < TrainedSetCount; j++)
			{
				// Let them compete, n games for each color.
				int n = 300;
				int score1[2] = { 0, 0 };
				PlayBatchMatch(Trainer.Set[i], Trainer.Set[j], n, score1);
				Trainer.Set[i].Score += score1[0];
				Trainer.Set[j].Score += score1[1];

				int score2[2] = { 0, 0 };
				//Switching sides
				PlayBatchMatch(Trainer.Set[j], Trainer.Set[i], n, score2);
				Trainer.Set[j].Score += score2[0];
				Trainer.Set[i].Score += score2[1];
				printf("\nScore for %d vs %d: %d-%d", Trainer.Set[i].Id, Trainer.Set[j].Id, score1[0] + score2[1], score1[1] + score2[0]);
			}
		}

		//free(ThreadGames);
		//sorting out best 2
		qsort(Trainer.Set, TrainedSetCount, sizeof(AiConfig), CompareScores);

		double tot = 0;
		for (int i = 0; i < TrainedSetCount; i++)
			tot += Trainer.Set[i].Score;

		printf("\n\nTotals\n");
		for (int i = 0; i < TrainedSetCount; i++)
			printf("Wins for %d: %d (%.2f%s)\n", Trainer.Set[i].Id, Trainer.Set[i].Score, (Trainer.Set[i].Score / tot) * 100, "%");

		NewGeneration();

		if (gen > 0 && gen % 5 == 0) {
			double progress = CompareAIs(Trainer.Set[0], untrained);
			SaveProgress(progress, (int)tot);
			SaveTrainedSet(Trainer.Generation, "TrainedSet");
			SaveFactors("BlotFactors.csv", Trainer.Set[0].BlotFactors);
			SaveFactors("ConnectedBlocksFactor.csv", Trainer.Set[0].ConnectedBlocksFactor);
		}
	}
}
