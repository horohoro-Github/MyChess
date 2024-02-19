// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "MyAI.generated.h"

class FRunnableThread;
class FMyAIThread;
class AMyGameStateBase;
class UMyBook;
class MyMove;
class AMyBoard;

USTRUCT(BlueprintType)
struct FPossibleMove
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY()
		TArray<int> pm;

};

USTRUCT(BlueprintType)
struct FCopyBoard
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY()
		bool FirstAction = true;
	UPROPERTY()
		int PiecesCurrentPosition = 99;
	UPROPERTY()
		int TilesTeamColor = 2;
	UPROPERTY()
		int ChessPiece = -1;
	UPROPERTY()
		int EnpassantB = -1;
	UPROPERTY()
		int EnpassantW = -1;
	UPROPERTY()
		int ChessType = 0;

};

USTRUCT(BlueprintType)
struct FBestMove
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY()
		int PieceIndex = -1;
	UPROPERTY()
		int TileIndex = -1;

};

UCLASS()
class MYCHESS__API AMyAI : public AActor
{
	GENERATED_BODY()

public:

	AMyAI();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY()
		UMyBook* book;
	FMyAIThread* aiThread = NULL;
	AMyGameStateBase* gameState = NULL;
	FRunnableThread* runningThread = NULL;

	TArray<class MyMove*> moves;
	int AITeamColor = 2;
	UFUNCTION()
		void StartSearch();
	int ProcessedCalculation = 0;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	void MoveChessPieces(MyMove* movePiece);
	TQueue<int> ThreadQueue;


	AMyBoard* board = NULL;
};