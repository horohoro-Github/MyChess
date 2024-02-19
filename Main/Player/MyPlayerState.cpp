// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "../Chess/MyTile.h"
#include "../Chess/MyChessPiece.h"
#include "../Game/MyChess_GameModeBase.h"
#include "MyPlayerController.h"
#include "../Game/MyGameInstance.h"
#include "../MultiPlay/MyOnlineBeaconClient.h"
#include "../Game/MyGameStateBase.h"
#include "Kismet/KismetStringLibrary.h"
#include "../Chess/MyBoard.h"
#include "../Widget/MyUserWidget.h"

AMyPlayerState::AMyPlayerState()
{
	PrimaryActorTick.bCanEverTick = true;
	bNetLoadOnClient = true;
}

void AMyPlayerState::BeginPlay()
{
	Super::BeginPlay();
	Gameins = Cast<UMyGameInstance>(GetGameInstance());
	PlayerControllers = Cast<AMyPlayerController>(GetPlayerController());
	
	gameState = Cast<AMyGameStateBase>(GetWorld()->GetGameState());

}

void AMyPlayerState::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (gameState)
	{
	    if(gameState->playMode == EPlayMode::SoloPlay) MyTeamColor = gameState->TeamColor;
	}
}

void AMyPlayerState::PromotionExecution_Implementation(AMyBoard* boards, int index)
{
	boards->ChessPromotion(index, false);
}

void AMyPlayerState::SelectTeam_Implementation(int colorIndex)
{
	AMyGameStateBase* GameState = Cast<AMyGameStateBase>(GetWorld()->GetGameState());
	if (GameState) GameState->SelectedTeam(this, colorIndex);
}

void AMyPlayerState::GameHistory_Implementation(int disconnectedTeamColor)
{
	if (PlayerControllers->CurrentWidget)
	{
		PlayerControllers->CurrentWidget->UpdateHistory(disconnectedTeamColor);
	}
}

void AMyPlayerState::RematchVote_Implementation()
{
	if (gameState) gameState->RegameVoted(this);

}


void AMyPlayerState::SetPlayerData_Implementation(FUsersData data, const FString& url, bool onlyServerTravel)
{

	Gameins = Cast<UMyGameInstance>(GetGameInstance());
	if (Gameins)
	{
	    if(onlyServerTravel == false) Gameins->userdb = data;

		if (url != "")
		{
			AMyPlayerController* pc = Cast<AMyPlayerController>(GetPlayerController());
			UMyUserWidget* widget = Cast<UMyUserWidget>(pc->CurrentWidget);
			if (widget) widget->WidgetMessage = "OppositePlayer Disconnected";

			
			FTimerDelegate openlevelDelegate = FTimerDelegate::CreateLambda([&, url]()
				{
					
					if(Gameins) Gameins->ReLogin(url);
				});
			
			GetWorld()->GetTimerManager().SetTimer(Gameins->openLevelHandle, openlevelDelegate, 3.f, false);
		}
	}
}

void AMyPlayerState::SetUserDB_Implementation(FUsersData data)
{
	userdb = data;
}

void AMyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AMyPlayerState, MyTeamColor);
	DOREPLIFETIME(AMyPlayerState, board);
	DOREPLIFETIME(AMyPlayerState, userdb);
}