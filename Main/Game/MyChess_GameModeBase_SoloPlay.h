// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyChess_GameModeBase.h"
#include "MyChess_GameModeBase_SoloPlay.generated.h"

/**
 * 
 */
class AMyChessRule;

UCLASS()
class MYCHESS__API AMyChess_GameModeBase_SoloPlay : public AMyChess_GameModeBase
{
	GENERATED_BODY()
	
public:

	virtual void BeginPlay()override;

	virtual void PostLogin(APlayerController* NewPlayer)override;
	AMyBoard* board = NULL;
};
