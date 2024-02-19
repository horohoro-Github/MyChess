// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPreComputedMoveData.h"

MyPreComputedMoveData::MyPreComputedMoveData()
{
	pawnAttacksWhite.Init(TArray<int>{0,}, 64);
	pawnAttacksBlack.Init(TArray<int>{0, }, 64);
	knightMoves.Init(TArray<int>{0, }, 64);
	kingMoves.Init(TArray<int>{0, }, 64);
	numSquaresToEdge.Init(TArray<int>{0, }, 64);

	rookMoves.Init(0, 64);
	bishopMoves.Init(0, 64);
	queenMoves.Init(0, 64);

	TArray<int> allKnightJumps = { 15,17,-17,-15,10,-6,6,-10 };

	knightAttackBitboards.Init(0, 64);
	kingAttackBitboards.Init(0, 64);
	pawnAttackBitboards.Init(TArray<uint64>{0,}, 64);

	for (int squareIndex = 0; squareIndex < 64; squareIndex++)
	{

		int y = squareIndex / 8;
		int x = squareIndex - y * 8;

		int north = 7 - y;
		int south = y;
		int west = x;
		int east = 7 - x;
		numSquaresToEdge[squareIndex].Init(0, 8);
		numSquaresToEdge[squareIndex][0] = north;
		numSquaresToEdge[squareIndex][1] = south;
		numSquaresToEdge[squareIndex][2] = west;
		numSquaresToEdge[squareIndex][3] = east;
		numSquaresToEdge[squareIndex][4] = FMath::Min(north, west);
		numSquaresToEdge[squareIndex][5] = FMath::Min(south, east);
		numSquaresToEdge[squareIndex][6] = FMath::Min(north, east);
		numSquaresToEdge[squareIndex][7] = FMath::Min(south, west);

		TArray<int> legalKnightJumps;
		uint64 knightBitboard = 0;

		for (int i=0, knightJumpDelta = 0; i < allKnightJumps.Num(); i++)
		{
			knightJumpDelta = allKnightJumps[i];
			int knightJumpSquare = squareIndex + knightJumpDelta;
			if (knightJumpSquare >= 0 && knightJumpSquare < 64)
			{
				int knightSquareY = knightJumpSquare / 8;
				int knightSquareX = knightJumpSquare - knightSquareY * 8;
				int maxCoordMoveDst = FMath::Max(abs(x - knightSquareX), abs(y - knightSquareY));
				if (maxCoordMoveDst == 2)
				{
					legalKnightJumps.Add(knightJumpSquare);
					knightBitboard |= 1ull << knightJumpSquare;
				}
			}
		}
		knightMoves[squareIndex] = legalKnightJumps;
		knightAttackBitboards[squareIndex] = knightBitboard;

		TArray<int> legalKingMoves;

		for(int i=0, kingMoveDelta=0; i < directionOffsets.Num(); i++)
		{
			kingMoveDelta = directionOffsets[i];
			int kingMoveSquare = squareIndex + kingMoveDelta;
			if (kingMoveSquare >= 0 && kingMoveSquare < 64)
			{
				int kingSquareY = kingMoveSquare / 8;
				int kingSquareX = kingMoveSquare - kingSquareY * 8;
				int maxCoordMoveDst = FMath::Max(abs(x - kingSquareX), abs(y - kingSquareY));
				if (maxCoordMoveDst == 1)
				{
					legalKingMoves.Add(kingMoveSquare);
					kingAttackBitboards[squareIndex] |= 1ull << kingMoveSquare;
				}
			}
		}
		kingMoves[squareIndex] = legalKingMoves;
		TArray<int> pawnCapturesWhite;
		TArray<int> pawnCapturesBlack;

		pawnAttackBitboards[squareIndex].Init(0, 2);
		if (x > 0)
		{
			if (y < 7)
			{
				pawnCapturesWhite.Add(squareIndex + 7);
				pawnAttackBitboards[squareIndex][0] |= 1ull << (squareIndex + 7);
			}
			if (y > 0)
			{
				pawnCapturesBlack.Add(squareIndex - 9);
				pawnAttackBitboards[squareIndex][1] |= 1ull << (squareIndex - 9);
			}
		}
		if (x < 7)
		{
			if (y < 7)
			{
				pawnCapturesWhite.Add(squareIndex + 9);
				pawnAttackBitboards[squareIndex][0] |= 1ull << (squareIndex + 9);
			}
			if (y > 0)
			{
				pawnCapturesBlack.Add(squareIndex - 7);
				pawnAttackBitboards[squareIndex][1] |= 1ull << (squareIndex - 7);
			}
		}
		pawnAttacksWhite[squareIndex] = pawnCapturesWhite;
		pawnAttacksBlack[squareIndex] = pawnCapturesBlack;


		for (int directionIndex = 0; directionIndex < 4; directionIndex++)
		{
			int currentDirOffset = directionOffsets[directionIndex];
			for (int n = 0; n < numSquaresToEdge[squareIndex][directionIndex]; n++)
			{
				int targetSquare = squareIndex + currentDirOffset * (n + 1);
				rookMoves[squareIndex] |= 1ull << targetSquare;
			}
		}

		for (int directionIndex = 4; directionIndex < 8; directionIndex++)
		{
			int currentDirOffset = directionOffsets[directionIndex];
			for (int n = 0; n < numSquaresToEdge[squareIndex][directionIndex]; n++)
			{
				int targetSquare = squareIndex + currentDirOffset * (n + 1);
				bishopMoves[squareIndex] |= 1ull << targetSquare;
			}
		}
		queenMoves[squareIndex] = rookMoves[squareIndex] | bishopMoves[squareIndex];
	}
	directionLookup.Init(0,127);

	for (int i = 0; i < 127; i++)
	{
		int offset = i - 63;
		int absOffset = abs(offset);
		int absDir = 1;
		if (absOffset % 9 == 0)
		{
			absDir = 9;
		}
		else if (absOffset % 8 == 0)
		{
			absDir = 8;
		}
		else if (absOffset % 7 == 0)
		{
			absDir = 7;
		}

		directionLookup[i] = absDir * FMath::Sign(offset);
	}

	TArray<int> orthogonal;
	orthogonal.Init(0, 64);
	orthogonalDistance.Init(orthogonal, 64);

	TArray<int> kingdis;
	kingdis.Init(0, 64);
	kingDistance.Init(kingdis, 64);
	centreManhattanDistance.Init(0, 64);

	for (int squareA = 0; squareA < 64; squareA++)
	{
		int fileA = squareA & 0b111;
		int rankA = squareA >> 3;
		int fileDstFromCentre = FMath::Max(3 - fileA, fileA - 4);
		int rankDstFromCentre = FMath::Max(3 - rankA, rankA - 4);
		centreManhattanDistance[squareA] = fileDstFromCentre + rankDstFromCentre;

		for (int squareB = 0; squareB < 64; squareB++)
		{
			int fileB = squareB & 0b111;
			int rankB = squareB >> 3;
			int rankDistance = abs(rankA - rankB);
			int fileDistance = abs(fileA - fileB);
			orthogonalDistance[squareA][squareB] = fileDistance + rankDistance;
			kingDistance[squareA][squareB] = FMath::Max(fileDistance, rankDistance);
		}
	}
}

MyPreComputedMoveData::~MyPreComputedMoveData()
{

}
