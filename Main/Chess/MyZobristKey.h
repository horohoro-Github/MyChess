// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MyZobristKey.generated.h"

/**
 * 
 */
class AMyBoard;
UCLASS()
class MYCHESS__API UMyZobristKey : public UObject
{
	GENERATED_BODY()
	
public:

	UMyZobristKey();
	TArray<TArray<TArray<uint64>>> ALLPieces;
	TArray<uint64> Enpassants;
	TArray<uint64> Castlings;

	uint64 move;
	void SetupZobristHash(FString zobrist);

	uint64 CalculateZobristKey(AMyBoard* board);
};
