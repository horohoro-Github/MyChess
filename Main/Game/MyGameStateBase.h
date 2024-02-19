// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../MyChess_.h"
#include "GameFramework/GameStateBase.h"
#include "MyGameStateBase.generated.h"

class AMyBoard;
class AMyPlayerState;
class AMyReferee;

UENUM(BlueprintType)
enum class EPlayState : uint8
{
	Play_None,
	GameOver,
	Preparing,
	Playing,
	Promotion,
	AIPlaying
	
};
UENUM(BlueprintType)
enum class EPlayMode : uint8
{
	SoloPlay,
	MultiPlay,
	AIPlay
};

UCLASS()
class MYCHESS__API AMyGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
public:


	AMyGameStateBase();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite, Replicated)
		EPlayMode playMode;
	UPROPERTY(BlueprintReadWrite, Replicated)
		EPlayState playState = EPlayState::Play_None;

	UPROPERTY(BlueprintReadWrite, Replicated)
		int TeamColor = 2;
	UPROPERTY(BlueprintReadWrite, Replicated)
		int ReadyPlayer = 0;

	UPROPERTY(BlueprintReadWrite, Replicated)
		FString GameEndingText = "";
	UPROPERTY(BlueprintReadWrite, Replicated)
		FString WinnerText = "";

	UPROPERTY(BlueprintReadWrite, Replicated)
		TArray<float> TimeLimit;

	UPROPERTY(BlueprintReadWrite, Replicated)
		bool bChecked = false;

	int FiftyMoveRule = 100;

	UPROPERTY(BlueprintReadWrite, Replicated)
		float selectTeamTimer = 0.f;

	UPROPERTY(Replicated)
		AMyBoard* board = NULL;
	UPROPERTY(BlueprintReadWrite, Replicated)
		bool Check = false;

	UPROPERTY(BlueprintReadWrite, Replicated)
		TArray<int> Turn;
	UPROPERTY(BlueprintReadWrite, Replicated)
		TArray<AMyPlayerState*> MatchPlayer;

	TArray<TArray<unsigned long long>> CurrentGameState;
	TArray<TArray<unsigned long long>> RepetitionGameState;
	FTimerHandle regameTimer;

	AMyReferee* referee;
	UPROPERTY(Replicated)
		TArray<uint64> gameHistory;

	void CallStateBase();

	void SelectedTeam(AMyPlayerState* PS, int color);
	TArray<uint64> GetHistory();
	void UpdateHistory(int disconnectedTeamColor);
	void RegameVoted(AMyPlayerState* PS);

	UFUNCTION(Server, Reliable)
		void UpdateGameResult(AMyPlayerState* PS, const FString& id, EGameResult result);
	void UpdateGameResult_Implementation(AMyPlayerState* PS, const FString& id, EGameResult result);

//	UFUNCTION(NetMulticast, Reliable)
//		void SetState(EPlayState state);
//	void SetState_Implementation(EPlayState state);
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const;
};
