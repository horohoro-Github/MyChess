// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyChess_GameModeBase.h"
#include "MyChess_GameModeBase_AI.generated.h"

/**
 * 
 */
UCLASS()
class MYCHESS__API AMyChess_GameModeBase_AI : public AMyChess_GameModeBase
{
	GENERATED_BODY()
public:

	virtual void BeginPlay()override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	AMyBoard* board = NULL;
private:
	TSubclassOf<UUserWidget> AIModeWidgetClass;
};
