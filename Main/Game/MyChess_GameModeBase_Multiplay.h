// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyChess_GameModeBase.h"
#include "MyChess_GameModeBase_Multiplay.generated.h"

/**
 * 
 */

UCLASS()
class MYCHESS__API AMyChess_GameModeBase_Multiplay : public AMyChess_GameModeBase
{
	GENERATED_BODY()
public:


	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime)override;
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	virtual void PostLogin(APlayerController* NewPlayer)override;


	virtual void Logout(AController* Exiting);

	TMap<FString, FString> userinfo;
	//TMap<FString, UPlayer*> playerinfo;

	TArray<FString> playerID;

	void CheckValidPlayer(FString id);


	void GameResult(AMyPlayerState* PS, FString id, EGameResult result, bool leaved);
	AMyBoard* board = NULL;

	//bool bFirstLoad = true;
	FHttpModule* Http;
};
