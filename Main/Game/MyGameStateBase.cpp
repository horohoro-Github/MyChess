// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameStateBase.h"
#include "MyChess_GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "../Player/MyPlayerState.h"
#include "../Player/MyPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "../AI/MyAI.h"
#include "../Chess/MyBoard.h"
#include "../Etc/MyReferee.h"
#include "MyChess_GameModeBase_Multiplay.h"
AMyGameStateBase::AMyGameStateBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bNetLoadOnClient = true;

	MatchPlayer.SetNum(2);
	TimeLimit.SetNum(2);
	Turn.SetNum(2);


}

void AMyGameStateBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (playMode == EPlayMode::MultiPlay && playState == EPlayState::Preparing)
	{
		if (MatchPlayer[FTeamColor::White] || MatchPlayer[FTeamColor::Black])
		{
			selectTeamTimer -= 1.f * DeltaTime;

			if (selectTeamTimer < 0.f)
			{
				
				if (HasAuthority())
				{
					AMyChess_GameModeBase_Multiplay* gamemode = Cast<AMyChess_GameModeBase_Multiplay>(GetWorld()->GetAuthGameMode());

					
					FString id = "";
					if (MatchPlayer[FTeamColor::White]) id = MatchPlayer[FTeamColor::White]->userdb.userid;
					if (MatchPlayer[FTeamColor::Black]) id = MatchPlayer[FTeamColor::Black]->userdb.userid;

					if (MatchPlayer[FTeamColor::White] && MatchPlayer[FTeamColor::Black])
					{
						playState = EPlayState::Playing;
						ReadyPlayer = 0;
					}
					else
					{
						gamemode->CheckValidPlayer(id);
					}
						
					
					
				}

			}
		}
		else
		{
			selectTeamTimer = 30.f;
		}
	}

	if (playMode == EPlayMode::MultiPlay && playState == EPlayState::Playing)
	{
		TimeLimit[TeamColor] -= 1.f * DeltaTime;
		if (TimeLimit[TeamColor] <= 0.f)
		{
			playState = EPlayState::GameOver;
			GameEndingText = "TimeOut";
			if (TeamColor == FTeamColor::White)
			{
				UpdateHistory(FTeamColor::White);
				UpdateGameResult(MatchPlayer[FTeamColor::White], MatchPlayer[FTeamColor::White]->userdb.userid, EGameResult::Lose);
				UpdateGameResult(MatchPlayer[FTeamColor::Black], MatchPlayer[FTeamColor::Black]->userdb.userid, EGameResult::Win);
				WinnerText = "Black Win";
				
			}
			else
			{
				UpdateHistory(FTeamColor::Black);
				UpdateGameResult(MatchPlayer[FTeamColor::White], MatchPlayer[FTeamColor::White]->userdb.userid, EGameResult::Win);
				UpdateGameResult(MatchPlayer[FTeamColor::Black], MatchPlayer[FTeamColor::Black]->userdb.userid, EGameResult::Lose);
				WinnerText = "White Win";
			}
			
			UE_LOG(LogTemp, Warning, TEXT("Time out"));
		}
	}
}

void AMyGameStateBase::CallStateBase()
{
	UE_LOG(LogTemp, Warning, TEXT("Check"));
}

void AMyGameStateBase::SelectedTeam(AMyPlayerState* PS, int color)
{
	if (board)
	{
	
		if (playMode == EPlayMode::AIPlay)
		{
			playState = EPlayState::Playing;
			TeamColor = 0;
			board->aiPlayer->AITeamColor = 1 - color;
			PS->MyTeamColor = color;

			if (board->aiPlayer->AITeamColor == 0)
			{
				FTimerHandle aiHandle;
				GetWorld()->GetTimerManager().SetTimer(aiHandle, board->aiPlayer, &AMyAI::StartSearch, 0.2f, false);
				//board->aiPlayer->StartSearch();
			}
		}

		if (playMode == EPlayMode::MultiPlay)
		{
			if (ReadyPlayer < 2)
			{
				if (MatchPlayer[color] == NULL)
				{
					if (MatchPlayer[1 - color] != PS)
					{
						MatchPlayer[color] = PS;
						ReadyPlayer++;
					}
				}
				else if (MatchPlayer[color] == PS)
				{
					MatchPlayer[color] = NULL;
					ReadyPlayer--;
				}

				if (ReadyPlayer == 2)
				{
					if (MatchPlayer[FTeamColor::White] != NULL && MatchPlayer[FTeamColor::Black] != NULL)
					{
						TeamColor = FTeamColor::White;
						TimeLimit[FTeamColor::White] = 70.f;
						TimeLimit[FTeamColor::Black] = 70.f;
						MatchPlayer[FTeamColor::White]->MyTeamColor = FTeamColor::White;
						MatchPlayer[FTeamColor::Black]->MyTeamColor = FTeamColor::Black;
						selectTeamTimer = 5.f;

					}
				}
			}
		}
		
	}
}

TArray<uint64> AMyGameStateBase::GetHistory()
{
	
	return gameHistory;
}

void AMyGameStateBase::UpdateHistory(int disconnectedTeamColor)
{
	for (int i = 0; i < PlayerArray.Num(); i++)
	{
		AMyPlayerState* PS = Cast<AMyPlayerState>(PlayerArray[i]);
		if (PS->playerState == EPlayerState::PLAYER)
		{
			PS->GameHistory(disconnectedTeamColor);
		}
	}
}

void AMyGameStateBase::RegameVoted(AMyPlayerState* PS)
{
	if (board)
	{
		if (playMode == EPlayMode::SoloPlay)
		{
			board->teamColors = FTeamColor::White;
			TeamColor = FTeamColor::White;
			board->ResetPiece();
			Check = false;
			UpdateHistory(FTeamColor::Neutral);

			playState = EPlayState::Playing;
		}
		if (playMode == EPlayMode::AIPlay)
		{
			board->teamColors = FTeamColor::White;
			TeamColor = FTeamColor::White;
			board->ResetPiece();
			Check = false;
			UpdateHistory(FTeamColor::Neutral);
			playState = EPlayState::Preparing;
		}
		if (playMode == EPlayMode::MultiPlay)
		{
			
			
			if (MatchPlayer[FTeamColor::White] == PS)
			{
				MatchPlayer[FTeamColor::White] = NULL;
				ReadyPlayer++;
			}
			if (MatchPlayer[FTeamColor::Black] == PS)
			{
				MatchPlayer[FTeamColor::Black] = NULL;
				ReadyPlayer++;
			}

			if (ReadyPlayer == 2)
			{
				ReadyPlayer = 0;
				if (MatchPlayer[FTeamColor::Black] == NULL && MatchPlayer[FTeamColor::White] == NULL)
				{
					board->teamColors = FTeamColor::White;
					TeamColor = FTeamColor::White;
					MatchPlayer.Empty();
					MatchPlayer.SetNum(2);
					board->ResetPiece();
					Check = false;
					UpdateHistory(FTeamColor::Neutral);
					playState = EPlayState::Preparing;
				}
			}
			
		}
	}
}

void AMyGameStateBase::UpdateGameResult_Implementation(AMyPlayerState* PS, const FString& id, EGameResult result)
{
	AMyChess_GameModeBase_Multiplay* gamemode = Cast<AMyChess_GameModeBase_Multiplay>(GetWorld()->GetAuthGameMode());
	
	if (gamemode)
	{
		gamemode->GameResult(PS, id, result, false);
	}
}

/*
void AMyGameStateBase::SetState_Implementation(EPlayState state)
{
	playState = state;
}

*/

void AMyGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyGameStateBase, playState);
	DOREPLIFETIME(AMyGameStateBase, playMode);
	DOREPLIFETIME(AMyGameStateBase, TeamColor);
	DOREPLIFETIME(AMyGameStateBase, GameEndingText);
	DOREPLIFETIME(AMyGameStateBase, TimeLimit);
	DOREPLIFETIME(AMyGameStateBase, bChecked);
	DOREPLIFETIME(AMyGameStateBase, Turn);
	DOREPLIFETIME(AMyGameStateBase, MatchPlayer);
	DOREPLIFETIME(AMyGameStateBase, board);
	DOREPLIFETIME(AMyGameStateBase, WinnerText);
	DOREPLIFETIME(AMyGameStateBase, gameHistory);
	DOREPLIFETIME(AMyGameStateBase, Check);
	DOREPLIFETIME(AMyGameStateBase, selectTeamTimer);
	DOREPLIFETIME(AMyGameStateBase, ReadyPlayer);
}