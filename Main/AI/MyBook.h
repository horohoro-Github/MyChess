// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MyBookPosition.h"
#include "MyBook.generated.h"

class MyMove;
class AMyBoard;

struct MyBookMove
{
public:
  
	FString moveString;
	int playedNum;

	MyBookMove(FString moveStrings, int numPlayed)
	{
		moveString = moveStrings;
		playedNum = numPlayed;
	}
};
UCLASS()
class MYCHESS__API UMyBook : public UObject
{
	GENERATED_BODY()
	
public:

	UMyBook();

	void CreateBook(FString fileString);

	uint64 GetPositionKey(FString key);
	TArray<FString> GetMoveInfo(FString key);

	TMap<uint64, MyBookPosition> bookPositions;

	TMap<FString, TArray<MyBookMove*>> movesByPosition;
	MyMove* GetRandomBookMoveWeighted(FString key, AMyBoard* board);

	bool HasPosition(FString positionKey);

	void Add(uint64 positionKey, MyMove* move, int numTimesPlayed);
};
