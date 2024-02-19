// Fill out your copyright notice in the Description page of Project Settings.


#include "MyChess_GameModeBase_SoloPlay.h"
#include "../Player/MyPlayerController.h"
#include "MyGameStateBase.h"
#include "../Player/MyPlayerState.h"
#include "../Chess/MyTile.h"
#include "../Chess/MyChessPiece.h"
#include "../Chess/MyBoard.h"

void AMyChess_GameModeBase_SoloPlay::BeginPlay()
{
	if (board == NULL)
	{
		FActorSpawnParameters spawnparam;
		board = GetWorld()->SpawnActor<AMyBoard>(boardClass, FVector(), FRotator(), spawnparam);
		if (board)
		{
			AMyGameStateBase* stateBase = Cast<AMyGameStateBase>(GetWorld()->GetGameState());
			if (stateBase)
			{
				stateBase->playState = EPlayState::Playing;
				stateBase->playMode = EPlayMode::SoloPlay;
				stateBase->TeamColor = 0;
				stateBase->board = board;
				stateBase->TimeLimit[0] = -1.f;
				stateBase->TimeLimit[1] = -1.f;
				board->gameState = stateBase;
				board->SpawnBoard();
			}
		
			
		}
	}
	
}

void AMyChess_GameModeBase_SoloPlay::PostLogin(APlayerController* NewPlayer)
{
	AMyPlayerController* PC = Cast<AMyPlayerController>(NewPlayer);

	if (PC)
	{
		PC->FadeScreen(0);
		PC->PlayerControllerSetup(false, false);
		PC->AddWidget(EWidgetState::Ingame, false);
		AMyPlayerState* PS = Cast<AMyPlayerState>(PC->PlayerState);
		if (PS)
		{
			PS->playerState = EPlayerState::PLAYER;
			PS->MyTeamColor = 0;
		}
	}
}