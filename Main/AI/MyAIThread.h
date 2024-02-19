// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"

class FRunnableThread;
class AMyAI;
class AMyBoard;
class UMyMoveCalculator;
class MyMove;
class MyTranspositionTable;
class MyEvaluation;

class FMyAIThread : public FRunnable
{
public:	
	FMyAIThread(AMyAI* funActor, AMyBoard* MyBoard, TSharedPtr<UMyMoveCalculator> calculatorObject);
	
	virtual bool Init();

	virtual uint32 Run();

	virtual void Stop();
	virtual ~FMyAIThread();

	int SearchMoves(int depth, int plyFromRoot, int alpha, int beta, int color);
	bool bStop = false;
	const int positive = 9999999;
	const int negative = -9999999;
	static const int immediatematescore = 100000;
	int numTransposition = 0;
	AMyAI* ai;
	
	AMyBoard* board;

	FRunnableThread* runningThread = NULL;
	TSharedPtr<UMyMoveCalculator> calculator;

	int bestEvalThisIteration = 0;
	int bestEval = 0;

	MyMove* bestMoveThisIteration = NULL;
	int currentIterativeSearchDepth = 0;
	MyMove* bestMove = NULL;
	MyTranspositionTable* tt = NULL;
	MyEvaluation* evaluation = NULL;
	
	int searchColor = 0;
	int opponentSearchColor = 0;
	int QuiescenceSearch(int alpha, int beta);

	void OrderMoves(TArray<MyMove*>& moves, bool UseTT);
	int GetPieceValue(int pieceType);
	void Sort(TArray<MyMove*>& moves);

	TArray<int16> moveScores;
	static bool IsMateScore(int score)
	{
		const int maxMateDepth = 1000;
		return abs(score) > (immediatematescore - maxMateDepth);
	}
private:

	/*
	int Pawn_Table[64] =
	{
		 0,  0,  0,  0,  0,  0,  0,  0,
		50, 50, 50, 50, 50, 50, 50, 50,
		10, 10, 20, 30, 30, 20, 10, 10,
		 5,  5, 10, 25, 25, 10,  5,  5,
		 0,  0,  0, 20, 20,  0,  0,  0,
		 5, -5,-10,  0,  0,-10, -5,  5,
		 5, 10, 10,-20,-20, 10, 10,  5,
		 0,  0,  0,  0,  0,  0,  0,  0

	};
	int Knight_Table[64] =
	{
		 -50,-40,-30,-30,-30,-30,-40,-50,
		 -40,-20,  0,  0,  0,  0,-20,-40,
		 -30,  0, 10, 15, 15, 10,  0,-30,
		 -30,  5, 15, 20, 20, 15,  5,-30,
		 -30,  0, 15, 20, 20, 15,  0,-30,
		 -30,  5, 10, 15, 15, 10,  5,-30,
		 -40,-20,  0,  5,  5,  0,-20,-40,
		 -50,-40,-30,-30,-30,-30,-40,-50
	};
	int Bishop_Table[64] =
	{
		-20,-10,-10,-10,-10,-10,-10,-20,
		-10,  0,  0,  0,  0,  0,  0,-10,
		-10,  0,  5, 10, 10,  5,  0,-10,
		-10,  5,  5, 10, 10,  5,  5,-10,
		-10,  0, 10, 10, 10, 10,  0,-10,
		-10, 10, 10, 10, 10, 10, 10,-10,
		-10,  5,  0,  0,  0,  0,  5,-10,
		-20,-10,-10,-10,-10,-10,-10,-20
	};

	int Rook_Table[64] =
	{
		 0,  0,  0,  0,  0,  0,  0,  0,
		 5, 10, 10, 10, 10, 10, 10,  5,
		-5,  0,  0,  0,  0,  0,  0, -5,
		-5,  0,  0,  0,  0,  0,  0, -5,
		-5,  0,  0,  0,  0,  0,  0, -5,
		-5,  0,  0,  0,  0,  0,  0, -5,
		-5,  0,  0,  0,  0,  0,  0, -5,
		 0,  0,  0,  5,  5,  0,  0,  0
	};

	//	UPROPERTY()
	int Queen_Table[64] =
	{
		-20,-10,-10, -5, -5,-10,-10,-20,
		-10,  0,  0,  0,  0,  0,  0,-10,
		-10,  0,  5,  5,  5,  5,  0,-10,
		 -5,  0,  5,  5,  5,  5,  0, -5,
		  0,  0,  5,  5,  5,  5,  0, -5,
		-10,  5,  5,  5,  5,  5,  0,-10,
		-10,  0,  5,  0,  0,  0,  0,-10,
		-20,-10,-10, -5, -5,-10,-10,-20
	};
	*/
};
