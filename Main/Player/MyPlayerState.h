// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "../MyChess_.h"
#include "MyPlayerState.generated.h"

#define RAW_APP_ID "480"

class AMyBoard;
class AMyPlayerController;
class AMyGameStateBase;
class UMyGameInstance;
class AMyChessPiece;
class AMyChess_GameModeBase;

UCLASS()
class MYCHESS__API AMyPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	
	AMyPlayerState();
	virtual void BeginPlay()override;
	virtual void Tick(float DeltaTime)override;

	UPROPERTY(BlueprintReadWrite, Replicated)
		FUsersData userdb;
	UPROPERTY(Replicated)
		AMyBoard* board = NULL;
	UPROPERTY()
		UMyGameInstance* Gameins = NULL;

	AMyGameStateBase* gameState = NULL;
	AMyPlayerController* PlayerControllers = NULL;

	UFUNCTION(Server, Reliable)
		void PromotionExecution(class AMyBoard* boards, int index);
	void PromotionExecution_Implementation(class AMyBoard* boards, int index);

	/*
	UFUNCTION(Server, Reliable)
		void SetWhitePlayer();
	void SetWhitePlayer_Implementation();
	*/
	EPlayerState playerState = EPlayerState::NONE_PLAYERSTATE;

	UPROPERTY(BlueprintReadWrite, Replicated)
		int MyTeamColor = 2;

	UFUNCTION(BlueprintCallable, Server, Reliable)
		void SelectTeam(int colorIndex);
	void SelectTeam_Implementation(int colorIndex);

	UFUNCTION(Client, Reliable)
		void GameHistory(int disconnectedTeamColor);
	void GameHistory_Implementation(int disconnectedTeamColor);

	UFUNCTION(Server, Reliable)
		void RematchVote();
	void RematchVote_Implementation();
	
	UFUNCTION(Client, Reliable)
		void SetPlayerData(FUsersData data, const FString& url, bool onlyServerTravel);
	void SetPlayerData_Implementation(FUsersData data, const FString& url, bool onlyServerTravel);

	UFUNCTION(Server, Reliable)
		void SetUserDB(FUsersData data);
	void SetUserDB_Implementation(FUsersData data);

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const;
};
