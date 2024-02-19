// Fill out your copyright notice in the Description page of Project Settings.


#include "MyChess_GameModeBase_AI.h"
#include "../Player/MyPlayerState.h"
#include "MyGameStateBase.h"
#include "../Chess/MyChessPiece.h"
#include "../Chess/MyTile.h"
#include "../AI/MyAI.h"
#include "../Chess/MyBoard.h"
void AMyChess_GameModeBase_AI::BeginPlay()
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
				stateBase->playState = EPlayState::Preparing;
				stateBase->playMode = EPlayMode::AIPlay;
				stateBase->TeamColor = 0;
				stateBase->board = board;
				stateBase->TimeLimit[0] = -1.f;
				stateBase->TimeLimit[1] = -1.f;
				stateBase->selectTeamTimer = 0.f;
				board->gameState = stateBase;
				board->playMode = EGamePlayMode::VS_AI;
				board->SpawnBoard();
				FActorSpawnParameters param;
				board->aiPlayer = GetWorld()->SpawnActor<AMyAI>(AMyAI::StaticClass(), FVector(), FRotator(), param);
				board->aiPlayer->board = board;
				board->aiPlayer->gameState = stateBase;
			}
		}
	}
}

void AMyChess_GameModeBase_AI::PostLogin(APlayerController* NewPlayer)
{
	AMyPlayerController* PC = Cast<AMyPlayerController>(NewPlayer);

	if (PC)
	{
		PC->FadeScreen(0);
		PC->PlayerControllerSetup(false, false);
		PC->AddWidget(EWidgetState::Ingame, true);
		AMyPlayerState* PS = Cast<AMyPlayerState>(PC->PlayerState);
		if (PS)
		{
			PS->playerState = EPlayerState::PLAYER;
		}
	}
}