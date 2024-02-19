// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MyMoveOrdering.generated.h"
/**
 * 
 */

class UMyMoveCalculator;
class MyTranspositionTable;
class AMyBoard;
class MyMove;
class MyEvaluation;

UCLASS()
class MYCHESS__API UMyMoveOrdering : public UObject
{
	GENERATED_BODY()
public:
	UMyMoveOrdering();
	~UMyMoveOrdering();

	UPROPERTY()
		TArray<int16> moveScores;
	const int maxMoveCount = 218;

	const int squareControlledByOpponentPawnPenalty = 350;
	const int capturedPieceValueMultiplier = 10;

	UMyMoveCalculator* moveCal;
	MyTranspositionTable* transpositionTable;

	MyMove* move;
	MyEvaluation* evaluation;

	void Init(UMyMoveCalculator* movecalculator, MyTranspositionTable* tt);


	void OrderMoves(AMyBoard* board, TArray<MyMove*>& moves, bool UseTT);


	int GetPieceValue(int pieceType);

	void Sort(TArray<MyMove*>& moves);

	int TestFunc();

};
