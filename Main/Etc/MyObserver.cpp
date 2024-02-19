// Fill out your copyright notice in the Description page of Project Settings.


#include "MyObserver.h"
#include "../Player/MyCharacter.h"
#include "../Player/MyPlayerController.h"
#include "MyReferee.h"
#include "Kismet/GameplayStatics.h"
#include "../Player/MyPlayerState.h"
#include "MyTransparencyActor.h"
#include "../Game/MyGameInstance.h"
#include "Net/UnrealNetwork.h"
#include "../Game/MyChess_GameModeBase.h"
#include "../Player/MyPlayerState.h"
#include "../Game/MyGameStateBase.h"

AMyObserver::AMyObserver()
{
	BaseEyeHeight = 0.f;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComp->SetCollisionProfileName(FName("OverlapAllDynamic"));
	RootComponent = CollisionComp;
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(CollisionComp);
	SpringArm->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArm);
	
	MovementComp = CreateDefaultSubobject<UPawnMovementComponent, UFloatingPawnMovement>(TEXT("Movement"));

}
void AMyObserver::BeginPlay()
{
	Super::BeginPlay();
	gameins = Cast<UMyGameInstance>(GetGameInstance());
}

void AMyObserver::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveRight", this, &AMyObserver::MoveRight);
	PlayerInputComponent->BindAxis("MoveForward", this, &AMyObserver::MoveForward);
	PlayerInputComponent->BindAxis("ZoomIn", this, &AMyObserver::ZoomInCamera);

	PlayerInputComponent->BindAction("InteractionMode", IE_Pressed, this, &AMyObserver::InteractionMode);
	PlayerInputComponent->BindAction("MovableMode", IE_Pressed, this, &AMyObserver::MovableMode);
	PlayerInputComponent->BindAction("ObserverMode", IE_Pressed, this, &AMyObserver::ReturnToPlayer);
	PlayerInputComponent->BindAction("ReverseCamera", IE_Pressed, this, &AMyObserver::ReverseCamera);
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AMyObserver::TouchBegin);
}

void AMyObserver::UnPossessed()
{
	Super::UnPossessed();
}

void AMyObserver::MoveRight(float value)
{
	if (TopView == true)
	{
		if (bWhite == true)
		{
			if (value > 0 && GetActorLocation().X >= -700.f) AddMovementInput(GetActorRightVector(), value);
			else if (value < 0 && GetActorLocation().X <= 0.f) AddMovementInput(GetActorRightVector(), value);
		}
		else
		{
			if (value < 0 && GetActorLocation().X >= -700.f) AddMovementInput(GetActorRightVector(), value);
			else if (value > 0 && GetActorLocation().X <= 0.f) AddMovementInput(GetActorRightVector(), value);
		}
	}
	else
	{
		if (bWhite == true)
		{
			if (value > 0 && GetActorLocation().X >= -700.f) AddMovementInput(GetActorRightVector(), value);
			else if (value < 0 && GetActorLocation().X <= 0.f) AddMovementInput(GetActorRightVector(), value);
		}
		else
		{
			if (value < 0 && GetActorLocation().X >= -700.f) AddMovementInput(GetActorRightVector(), value);
			else if (value > 0 && GetActorLocation().X <= 0.f) AddMovementInput(GetActorRightVector(), value);
		}
	}
}

void AMyObserver::MoveForward(float value)
{
	if (TopView == true)
	{
		if (bWhite == true)
		{
			if (value < 0 && GetActorLocation().Y >= 0.f) AddMovementInput(GetActorForwardVector(), value);
			else if (value > 0 && GetActorLocation().Y <= 700.f) AddMovementInput(GetActorForwardVector(), value);
		}
		else
		{
			if (value > 0 && GetActorLocation().Y >= 0.f) AddMovementInput(GetActorForwardVector(), value);
			else if (value < 0 && GetActorLocation().Y <= 700.f) AddMovementInput(GetActorForwardVector(), value);
		}
	}
	else
	{
		if (bWhite == true)
		{
			if (value < 0 && GetActorLocation().Y >= -500.f) AddMovementInput(GetActorForwardVector(), value);
			else if (value > 0 && GetActorLocation().Y <= 150.f) AddMovementInput(GetActorForwardVector(), value);
		}
		else
		{
			if (value > 0 && GetActorLocation().Y >= 550.f) AddMovementInput(GetActorForwardVector(), value);
			else if (value < 0 && GetActorLocation().Y <= 1200.f) AddMovementInput(GetActorForwardVector(), value);
		}
	}
}

void AMyObserver::ZoomInCamera(float value)
{
	if (TopView == true)
	{
		if (value > 0.f)
		{
			if (SpringArm->TargetArmLength - value * 8.f > 0.f) SpringArm->TargetArmLength -= value * 8.f;
		}
		else if (value < 0.f)
		{
			if (SpringArm->TargetArmLength - value * 8.f < 1000.f) SpringArm->TargetArmLength -= value * 8.f;
		}
	}
	else
	{
		if (value > 0.f)
		{
			if (SpringArm->TargetArmLength - value * 8.f > 100.f) SpringArm->TargetArmLength -= value * 8.f;
		}
		else if (value < 0.f)
		{
			if (SpringArm->TargetArmLength - value * 8.f < 500.f) SpringArm->TargetArmLength -= value * 8.f;
		}
	}
}

void AMyObserver::InteractionMode()
{
	if (PlayerController->bSpectator == false) PlayerController->InteractionMode();
	else PlayerController->bShowMouseCursor = true;
}

void AMyObserver::MovableMode()
{
	if (PlayerController->bSpectator == false) PlayerController->MovableMode();
	else PlayerController->bShowMouseCursor = true;
}

void AMyObserver::ReverseCamera()
{
	AMyGameStateBase* gameState = Cast<AMyGameStateBase>(GetWorld()->GetGameState());
	AMyPlayerState* PS = Cast<AMyPlayerState>(GetPlayerState());
	if (PS->playerState == EPlayerState::PLAYER)
	{
		if (gameState->playMode == EPlayMode::MultiPlay || gameState->playMode == EPlayMode::MultiPlay)
		{
			if (TopView == false)
			{
				TopView = true;
				SpringArm->TargetArmLength = 300.f;
				CameraComp->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
				SetActorLocation(FVector(-350.f, 350.f, 350.f));
			}
			else
			{
				TopView = false;
//				if (PlayerController->PlayerStates->MyTeamColor == 1)
				{
					SpringArm->TargetArmLength = 300.f;
					CameraComp->SetRelativeRotation(FRotator(45.f, 0.f, 0.f));
					SetActorLocation(FVector(-350.f, 1050.f, 350.f));
				}
			//	else
				{
					SpringArm->TargetArmLength = 300.f;
					CameraComp->SetRelativeRotation(FRotator(45.f, 0.f, 0.f));
					SetActorLocation(FVector(-350.f, -350.f, 350.f));
				}
			}
		}
		else
		{
			if (bWhite == true)
			{
				bWhite = false;
				SetActorRotation(FRotator(0.f, -90.f, 0.f));
				SetActorLocation(FVector(-350.f, 350.f, 350.f));
			}
			else
			{
				bWhite = true;
				SetActorRotation(FRotator(0.f, 90.f, 0.f));
				SetActorLocation(FVector(-350.f, 350.f, 350.f));
			}
		}
	}
	else
	{
		if (bWhite == true)
		{
			bWhite = false;
			SetActorRotation(FRotator(0.f, -90.f, 0.f));
			SetActorLocation(FVector(-350.f, 350.f, 350.f));
		}
		else
		{
			bWhite = true;
			SetActorRotation(FRotator(0.f, 90.f, 0.f));
			SetActorLocation(FVector(-350.f, 350.f, 350.f));
		}
	}
}

void AMyObserver::ReturnToPlayer()
{
	AMyPlayerState* playerState = Cast<AMyPlayerState>(PlayerController->PlayerState);
	AMyGameStateBase* gameState = Cast<AMyGameStateBase>(GetWorld()->GetGameState());
	if (playerState->playerState == EPlayerState::PLAYER)
	{
		if (gameState->playState == EPlayState::Playing || gameState->playState == EPlayState::Promotion || gameState->playState == EPlayState::AIPlaying)
		{
			if (gameins != NULL)
			{
				if (gameins->wall != NULL) gameins->wall->SetActorHiddenInGame(false);
			}
			Controller->Possess(Player);
		}
	}
}

void AMyObserver::ReturnToPlayer_Server_Implementation()
{
	if (gameins != NULL)
	{
		if (gameins->wall != NULL) gameins->wall->SetActorHiddenInGame(false);
	}
}

void AMyObserver::ReturnToPlayer_Client_Implementation()
{
	if (gameins != NULL)
	{
		if (gameins->wall != NULL) gameins->wall->SetActorHiddenInGame(false);
	}
}

void AMyObserver::EscapeTheGame()
{
	if (PlayerController->bSpectator == false)
	{
		
	}
		
}

void AMyObserver::SetSpectator_Implementation()
{
	bSpectator = PlayerController->bSpectator;
}

void AMyObserver::SetWallTransparency_Implementation(AActor* otheractor, bool bhiddens)
{
	otheractor->SetActorHiddenInGame(bhiddens);
}

void AMyObserver::SetCameraRotation_Implementation()
{
	if (PlayerController == NULL)
	{
		PlayerController = Cast<AMyPlayerController>(GetController());
		if (PlayerStates == NULL)
		{
			PlayerStates = Cast<AMyPlayerState>(PlayerController->PlayerState);
		}
	}

	AMyGameStateBase* gameState = Cast<AMyGameStateBase>(GetWorld()->GetGameState());

	if (gameState->playState == EPlayState::AIPlaying || gameState->playState == EPlayState::Playing || gameState->playState == EPlayState::Promotion)
	{
		if (PlayerController->bSpectator == false)
		{
			if (gameState->playMode == EPlayMode::MultiPlay)
			{
				TopView = false;
				if (PlayerStates->MyTeamColor == 0)
				{
					bWhite = true;
					SetActorRotation(FRotator(0.f, 90.f, 0.f));
					SetActorLocation(FVector(-350.f, -350.f, 350.f));
					CameraComp->SetRelativeRotation(FRotator(45.f, 0.f, 0.f));
				}
				else
				{
					bWhite = false;
					SetActorRotation(FRotator(0.f, -90.f, 0.f));
					SetActorLocation(FVector(-350.f, 1050.f, 350.f));
					CameraComp->SetRelativeRotation(FRotator(45.f, 0.f, 0.f));
				}
			}
			else if (gameState->playMode == EPlayMode::AIPlay)
			{

				AMyPlayerController* PC = Cast<AMyPlayerController>(GetController());
				if (PC)
				{
					if (PC->MyTeamColor == 0)
					{
						bWhite = true;
						SetActorRotation(FRotator(0.f, 90.f, 0.f));
						SetActorLocation(FVector(-350.f, -350.f, 350.f));
						CameraComp->SetRelativeRotation(FRotator(45.f, 0.f, 0.f));
					}
					else
					{
						bWhite = false;
						SetActorRotation(FRotator(0.f, -90.f, 0.f));
						SetActorLocation(FVector(-350.f, 1050.f, 350.f));
						CameraComp->SetRelativeRotation(FRotator(45.f, 0.f, 0.f));
					}
				}

			}
			else
			{
				TopView = true;
				SetActorRotation(FRotator(0.f, 90.f, 0.f));
				SetActorLocation(FVector(-350.f, 350.f, 350.f));
			}
		}
		else
		{
			if (gameins != NULL)
			{
				if (gameins->wall != NULL) gameins->wall->SetActorHiddenInGame(true);
			}
			else
			{
				gameins = Cast<UMyGameInstance>(GetGameInstance());
				if (gameins->wall != NULL) gameins->wall->SetActorHiddenInGame(true);
			}

			TopView = true;
			SetActorRotation(FRotator(0.f, 90.f, 0.f));
			SetActorLocation(FVector(-350.f, 350.f, 350.f));
		}
	}
}

void AMyObserver::TouchBegin(const ETouchIndex::Type FingerIndex, const FVector TouchedLoc)
{
	if (PlayerController)
	{
		if (PlayerController->bSpectator == false)
		{
			TouchStartLoc = FVector2D(TouchedLoc.X, TouchedLoc.Y);

			//if (PlayerController->ChessRule)
			{
			//	if (PlayerController->ChessRule->bCanStartGame == true)
				{

					FHitResult hr;
					if (PlayerController->GetHitResultUnderFingerByChannel(ETouchIndex::Touch1, ETraceTypeQuery::TraceTypeQuery1, true, hr))
					{
						AMyChessPiece* piece = Cast<AMyChessPiece>(hr.GetActor());
						if (piece)
						{
//							PlayerController->GetPiece(piece, false);
						}
						AMyTile* tile = Cast<AMyTile>(hr.GetActor());
						if (tile)
						{
						//	PlayerController->GetTile(tile);
						}
					}
				}
			}
		}
	}
}

void AMyObserver::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AMyObserver, bWhite);
	DOREPLIFETIME(AMyObserver, TopView);
}

