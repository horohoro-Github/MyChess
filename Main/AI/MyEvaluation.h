// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */

class AMyBoard;
class UMyPieceList;
class MySquareTable;

class MYCHESS__API MyEvaluation
{
public:
	MyEvaluation();
	~MyEvaluation();


	const int pawnValue = 100;
	const int knightValue = 320;
	const int bishopValue = 330;
	const int rookValue = 500;
	const int queenValue = 900;
	const int kingValue = 20000;

	const float endGameMaterialStart = rookValue * 2 + bishopValue + knightValue;
	
	AMyBoard* board = NULL;

	int Evaluate(AMyBoard* chessBoard);
	

	int MopupEval(int friendlyIndex, int opponentIndex, int  myMaterial, int opponentMaterial, float endgameWeight);


	int EndgamePhaseWeight(int materialCountWithoutPawns);


	int CountMaterial(int colorIndex);

	int EvaluatePieceSquareTables(int colorIndex, float endgamePhaseWeight);

	int EvaluatePieceSquareTable(TArray<int>& table, MyPieceList* pieceList, bool white);

};
