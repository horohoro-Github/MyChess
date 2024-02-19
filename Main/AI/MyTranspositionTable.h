// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class MyMove;
class AMyBoard;
class FMyAIThread;
struct Entry
{
public:
	uint64 key=0;
	int value=0;
	MyMove* move = NULL;
	int depth=0;
	int nodeType=0;

	Entry() {}
	~Entry()
	{
		delete move;
	}
	Entry(uint64 k, int v, int d, int n, MyMove* m)
	{
		key = k;
		value = v;
		move = m;
		
		depth = d;
		nodeType = n;
	}
};

class MYCHESS__API MyTranspositionTable
{
public:

	const int lookupFailed = INT_MIN;
	const int exact = 0;
	const int lowerBound = 1;
	const int upperBound = 2;

	TArray<Entry*> entries;

	uint64 size = 0;
	bool enabled = true;
	AMyBoard* board;

	MyTranspositionTable(AMyBoard* chessBoard, int arraySize);
	void Clear();

	uint64 Index();

	MyMove* GetStoredMove();


	void StoreEvaluation(int depth, int numPlySearched, int eval, int evalType, MyMove* move);

	int LookupEvaluation(int depth, int plyFromRoot, int alpha, int beta);

	int CorrectMateScoreForStorage(int score, int numPlySearched);


	int CorrectRetrievedMateScore(int score, int numPlySearched);


	void freeTT();
};
