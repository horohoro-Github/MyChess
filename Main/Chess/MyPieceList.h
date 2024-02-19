// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class MYCHESS__API MyPieceList
{
public:

	int pieceNum = 0;
	
	TArray<int> map;

	TArray<int> PiecePosition;

	MyPieceList()
	{
		map.Init(0, 64);
		PiecePosition.Init(0, 16);
	}
	~MyPieceList();
	
	void AddPiece(int square)
	{
		PiecePosition[pieceNum] = square;
		map[square] = pieceNum;
		pieceNum++;
	}
	void MovePiece(int startSquare, int targetSquare)
	{
		int move = map[startSquare];
		PiecePosition[move] = targetSquare;
		map[targetSquare] = move;
	}
	void RemovePiece(int square)
	{
		if (pieceNum - 1 < 0)
		{
			return;
		}
		int move = map[square];
		PiecePosition[move] = PiecePosition[pieceNum - 1];
		map[PiecePosition[move]] = move;
		pieceNum--;
	}
};
