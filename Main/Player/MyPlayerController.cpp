// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "../Chess/MyChessPiece.h"
#include "../Chess/MyTile.h"
#include "../Player/MyCharacter.h"
#include "../Player/MyPlayerState.h"
#include "Blueprint/UserWidget.h"
#include "../Etc/MyObserver.h"
#include "../Game/MyChess_GameModeBase.h"
#include "../Game/MyGameInstance.h"
#include "Components/EditableTextBox.h"
#include "MyPlayerState.h"
#include "Components/ListView.h"
#include "../Etc/MyTransparencyActor.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/LevelStreaming.h"
#include "Components/Border.h"
#include "Kismet/KismetStringLibrary.h"
#include "../MultiPlay/MyOnlineBeaconClient.h"
#include "../AI/MyAI.h"
#include "../Widget/MyUserWidget.h"
#include "MyPlayerCameraManager.h"
#include "GameFramework/TouchInterface.h"
#include "../Game/MyGameStateBase.h"
#include "Blueprint/UserWidget.h"
#include "../Chess/MyBoard.h"

AMyPlayerController::AMyPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	//모바일

	if (!IsRunningDedicatedServer())
	{
		static ConstructorHelpers::FClassFinder<UUserWidget> lobby(TEXT("WidgetBlueprint'/Game/Widget/UserWidget/MainMenu.MainMenu_C'"));
		if (lobby.Succeeded()) LobbyWidgetClass = lobby.Class;

		static ConstructorHelpers::FClassFinder<UUserWidget> ingame(TEXT("WidgetBlueprint'/Game/Widget/UserWidget/Ingame.Ingame_C'"));
		if (ingame.Succeeded()) InGameWidgetClass = ingame.Class;
		
		static ConstructorHelpers::FClassFinder<UUserWidget> exit(TEXT("WidgetBlueprint'/Game/Widget/UserWidget/Exit.Exit_C'"));
		if (exit.Succeeded()) ExitClass = exit.Class;
	}

	PlayerCameraManagerClass = AMyPlayerCameraManager::StaticClass();
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (UGameplayStatics::GetPlatformName() == "Android")
	{
		bAndroidController = true;
	}
	GameIns = Cast<UMyGameInstance>(GetGameInstance());
	CameraManager = Cast<AMyPlayerCameraManager>(PlayerCameraManager);
}

void AMyPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (board == NULL) board = Cast<AMyBoard>(UGameplayStatics::GetActorOfClass(GetWorld(), AMyBoard::StaticClass()));
	if (PlayerStates == NULL) PlayerStates = Cast<AMyPlayerState>(PlayerState);
	if(gameState == NULL) gameState = Cast<AMyGameStateBase>(GetWorld()->GetGameState());
	
	if (gameState && PlayerStates)
	{
		if (bAndroidController == false)
		{
			if (bSpectator == false && bInteraction == true)
			{
				if (PlayerStates->MyTeamColor == gameState->TeamColor)
				{
					FVector Start, Dir, End;
					DeprojectMousePositionToWorld(Start, Dir);
					End = Start + (Dir * 8000.f);
					TraceForBlock(Start, End);
				}
			}
		}
	}
}

void AMyPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (InPawn == Observer)
	{
		if (Observer) Observer->SetCameraRotation();
	}
	
}

void AMyPlayerController::InteractionMode()
{
	if (bSpectator == false && gameState)
	{
		if (bInteraction == false)
		{
			bInteraction = true;
			bShowMouseCursor = true;
			bEnableClickEvents = true;
			bEnableTouchEvents = true;
			FInputModeGameAndUI GameUI;
			GameUI.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
			SetInputMode(GameUI);
		}
		else
		{
			if (gameState->playState == EPlayState::Playing)
			{
				if (ChessPiece)
				{
					if (PlayerStates->MyTeamColor == gameState->TeamColor)
					{
						GetPieces(ChessPiece, false);
					}
				}
				if (ChessTile)
				{
					if (PlayerStates->MyTeamColor == gameState->TeamColor)
					{
						GetTiles(ChessTile);
					}
				}
			}
			
		}
	}
}

void AMyPlayerController::MovableMode()
{
	if (board && gameState)
	{
		if (bSpectator == false)
		{
			if (gameState->playState == EPlayState::Playing)
			{
				if (PlayerStates->MyTeamColor == gameState->TeamColor)
				{
					CallTileEmphasize_Client(ChessTile, false);
					ChessPiece = NULL;
					ChessTile = NULL;
				}
			}

			bInteraction = false;
			bShowMouseCursor = false;
			FInputModeGameOnly GameMode;
			SetInputMode(GameMode);
		}
	}
	else
	{
		bInteraction = false;
		bShowMouseCursor = false;
		FInputModeGameOnly GameMode;
		SetInputMode(GameMode);
	}
}

void AMyPlayerController::TraceForBlock(const FVector& Start, const FVector& End)
{
	FHitResult hitresult;
	GetWorld()->LineTraceSingleByChannel(hitresult, Start, End, ECC_Visibility);
	if (hitresult.GetActor()!=nullptr)
	{
		AMyChessPiece* chesspiece = Cast<AMyChessPiece>(hitresult.GetActor());
		AMyTile* chesstile = Cast<AMyTile>(hitresult.GetActor());
		
		if (chesspiece != ChessPiece) ChessPiece = chesspiece;
		if (chesstile != ChessTile)
		{
			if(ChessTile) CallTileEmphasize_Client(ChessTile, false);
			ChessTile = chesstile;
			CallTileEmphasize_Client(ChessTile, true);
		}
	}
	else
	{
		if(ChessTile) CallTileEmphasize_Client(ChessTile, false);
		ChessPiece = NULL;
		ChessTile = NULL;
	}
}

void AMyPlayerController::CallTileEmphasize_Client_Implementation(AMyTile* tiles, bool emphasized)
{
	if (PlayerStates && board && gameState)
	{
		if (gameState->TeamColor == PlayerStates->MyTeamColor && gameState->playState == EPlayState::Playing)
		{
			board->EmphasizeSquare(tiles, emphasized);
		}
	}
}

void AMyPlayerController::GetPieces_Implementation(AMyChessPiece* piece, bool bReset)
{
	if (board && PlayerStates && gameState)
	{
		if(PlayerStates->MyTeamColor == gameState->TeamColor && gameState->playState == EPlayState::Playing) board->SelectedPiece(piece);
	}
}

void AMyPlayerController::GetTiles_Implementation(AMyTile* tiles)
{
	if (board && PlayerStates && gameState)
	{
		if (PlayerStates->MyTeamColor == gameState->TeamColor && gameState->playState == EPlayState::Playing) board->SelectedSquare(tiles);
	}
}

void AMyPlayerController::PossessActor_Implementation(bool bPlayer)
{
	if (bPlayer == true)
	{
		if (bSpectator == false)
		{
			UnPossess();
		
			Possess(Observer);
		}
		else
		{
			FActorSpawnParameters spawnparam;
			spawnparam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			Observer = Cast<AMyObserver>(GetWorld()->SpawnActor<AActor>(ObserverClass, FVector(-350.f, 350.f, 350.f), FRotator(0.f, 0.f, 0.f), spawnparam));
			Observer->SetActorRotation(FRotator(0.f, 90.f, 0.f));
			Observer->Player = MyPlayer;
			Possess(Observer);
		}
	}
	else
	{
		UnPossess();
		Possess(MyPlayer);
	}
}

void AMyPlayerController::ResetPossessActor_Implementation()
{
	UnPossess();
	Possess(MyPlayer);
}

void AMyPlayerController::SetSpectator_Implementation(bool bSpectators)
{
	bSpectator = bSpectators;
}

void AMyPlayerController::HiddenCeiling_Implementation()
{
	UMyGameInstance* gameins = Cast<UMyGameInstance>(GetGameInstance());
	if (gameins)
	{
		if (gameins->wall != NULL) gameins->wall->SetActorHiddenInGame(false);
	}
}

void AMyPlayerController::FadeScreen_Implementation(int FadeState) // 0: 초기 설정, 1: 원래 화면, 2: 검은 화면  
{
	if (CameraManager)
	{
		if (FadeState == 0)
		{
			CameraManager->SetManualCameraFade(1.f, FLinearColor::Black, false);
		}
		if (FadeState == 1)
		{
			bWidgetVisibility = true;
			CameraManager->StartCameraFade(1.f, 0.f, 0.5f, FLinearColor::Black, false, true);
		}
		if (FadeState == 2)
		{
			bWidgetVisibility = false;
			CameraManager->StartCameraFade(0.f, 1.f, 0.5f, FLinearColor::Black, false,true);
			UMyGameInstance* gameins = Cast<UMyGameInstance>(GetGameInstance());
			GetWorld()->GetTimerManager().SetTimer(gameins->loadtimer, FTimerDelegate::CreateLambda([this]()
				{
					UGameplayStatics::LoadStreamLevel(this, FName("LoadingMap"), true,false, latentinfo);
				}), 0.5f, false);
		}
	}
}

void AMyPlayerController::PlayerControllerSetup_Implementation(bool Spec, bool bIsLobbyLevel)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (bIsLobbyLevel == false)
	{
		if (AMyChess_GameModeBase* GameMode = Cast<AMyChess_GameModeBase>(GetWorld()->GetAuthGameMode()))
		{
			FVector SpawnLoc = FVector(-1400.f, 300.f, 100.f);
			SpawnLocation.Z = 100.f;

			if (Spec == false)
			{
				if (MyPlayer == NULL)
				{
					if (APawn* PlayerPawn = GetWorld()->SpawnActor<APawn>(AMyCharacter::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams))
					{
						MyPlayer = Cast<AMyCharacter>(PlayerPawn);
						SetSpectator(Spec);
						Possess(MyPlayer);
						
					}
				}
				if (APawn* Observ = GetWorld()->SpawnActor<APawn>(AMyObserver::StaticClass(), FVector(-350.f, 350.f, 350.f), FRotator(0.f, 90.f, 0.f), SpawnParams))
				{
					Observer = Cast<AMyObserver>(Observ);
					Observer->Player = MyPlayer;
					if(MyPlayer) MyPlayer->Observer = Observer;
				}
			}
			else
			{
				if (APawn* Observ = GetWorld()->SpawnActor<APawn>(AMyObserver::StaticClass(), FVector(-350.f, 350.f, 350.f), FRotator(0.f, 90.f, 0.f), SpawnParams))
				{
					Observer = Cast<AMyObserver>(Observ);
					Observer->SetActorRelativeRotation(FRotator(0.f, 90.f, 0.f));
					Observer->TopView = true;
					SetSpectator(Spec);
					Possess(Observer);
				}
			}
		}
	}
	else
	{
		if (APawn* nonec = GetWorld()->SpawnActor<APawn>(APawn::StaticClass(), FVector(0.f, 0.f, 0.f), FRotator(0.f, 0.f, 0.f), SpawnParams))
		{
			Possess(nonec);
		}
	}
}

void AMyPlayerController::AddWidget_Implementation(int widState, bool showCursor)
{
	bShowMouseCursor = showCursor;
	AMyPlayerState* PS = Cast<AMyPlayerState>(PlayerState);
	switch (widState)
	{
	case EWidgetState::None_Widget:
	{
		break;
	}
	case EWidgetState::MainMenu:
	{
		CurrentWidget = CreateWidget<UMyUserWidget>(GetWorld(), LobbyWidgetClass);
		break;
	}
	case EWidgetState::Ingame:
	{
		CurrentWidget = CreateWidget<UMyUserWidget>(GetWorld(), InGameWidgetClass);
		break;
	}
	}
	SlideWidget = CreateWidget<UMyUserWidget>(GetWorld(), ExitClass);
	SlideWidget->AddToViewport(1);
	if(CurrentWidget != NULL) CurrentWidget->AddToViewport();

}

void AMyPlayerController::ClearWidget()
{
	if (CurrentWidget != NULL)
	{
		CurrentWidget->RemoveFromParent();
		CurrentWidget = NULL;
	}
}

void AMyPlayerController::KickPlayer_Implementation(bool bMainServer)
{
	UMyGameInstance* gameins = Cast<UMyGameInstance>(GetGameInstance());
	if (gameins)
	{
		gameins->ReLogin("LobbyLevel");
	}
}

float AMyPlayerController::GetFadeValue()
{
	if (CameraManager)
	{
		return CameraManager->FadeAmount;
	}
	else
	{
		return 0.f;
	}
}

void AMyPlayerController::AddBeaconClient_Implementation()
{
	UMyGameInstance* gameins = Cast<UMyGameInstance>(GetGameInstance());

	FActorSpawnParameters spawnbeacon;
	gameins->BC = GetWorld()->SpawnActor<AMyOnlineBeaconClient>(AMyOnlineBeaconClient::StaticClass(), FTransform(FRotator(0.f, 0.f, 0.f), FVector(0.f, 0.f, 0.f)), spawnbeacon);

	if (gameins->BC)
	{
		gameins->BC->gameins = gameins;
		gameins->BC->LevelName = UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
		gameins->BC->ConnectToServer(SERVER_IPADDR);
	}
}
