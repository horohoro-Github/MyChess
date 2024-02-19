// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
static class MYCHESS__API MyPreComputedMoveData
{
public:
	MyPreComputedMoveData();
	~MyPreComputedMoveData();


	TArray<int> directionOffsets = { 8,-8,-1,1,7,-7,9,-9 };

	TArray<TArray<int>> numSquaresToEdge;
	TArray<TArray<int>> knightMoves;
	TArray<TArray<int>> kingMoves;

	TArray<TArray<int>> pawnAttackDirections = {
		TArray<int> {4,6},
		TArray<int> {7,5}
	};
	TArray<TArray<int>>pawnAttacksWhite;

	TArray<TArray<int>>pawnAttacksBlack;
	TArray<int>directionLookup;
	TArray<uint64> kingAttackBitboards;
	TArray<uint64>knightAttackBitboards;

	TArray<TArray<uint64>> pawnAttackBitboards;
	TArray<uint64> rookMoves;
	TArray<uint64> bishopMoves;
	TArray<uint64> queenMoves;

	TArray<TArray<int>> orthogonalDistance;
	TArray<TArray<int>>kingDistance;
	TArray<int>centreManhattanDistance;
};
