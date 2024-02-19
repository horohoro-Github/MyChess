// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "MyPlayerController.h"
#include "../Etc/MyReferee.h"
#include "../Chess/MyTile.h"
#include "../Etc/MyObserver.h"
#include "Net/UnrealNetwork.h"
#include "../Game/MyChess_GameModeBase.h"
#include "MyPlayerState.h"
#include "../Game/MyGameInstance.h"
#include "Components/CapsuleComponent.h"
#include "../Etc/MyTransparencyActor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "../Game/MyGameStateBase.h"

AMyCharacter::AMyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetMesh(), FName("Head"));
	SpringArm->TargetArmLength = 0.f;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArm);

	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 30.f;
	SpringArm->ProbeChannel = ECC_Visibility;
	static ConstructorHelpers::FClassFinder<UAnimInstance>playerAnim(TEXT("AnimBlueprint'/Game/Animations/Player/Player_AnimBP.Player_AnimBP_C'"));

	if (playerAnim.Succeeded()) GetMesh()->AnimClass = playerAnim.Class;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> skeletalMesh(TEXT("SkeletalMesh'/Game/Meshes/ybot.ybot'"));

	if (skeletalMesh.Succeeded()) GetMesh()->SetSkeletalMesh(skeletalMesh.Object);

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -90.f), FRotator(0.f, -90.f, 0.f));

	GetCapsuleComponent()->SetRelativeScale3D(FVector(1.8f, 1.8f, 1.8f));

	GetMesh()->bOwnerNoSee = true;
	
}

void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	gameState = Cast<AMyGameStateBase>(GetWorld()->GetGameState());
	PlayerController = Cast<AMyPlayerController>(GetController());
	
	gameins = Cast<UMyGameInstance>(GetGameInstance());
}

void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	FVector moveVector = CameraComp->GetForwardVector() * forwardAxis + CameraComp->GetRightVector() * rightAxis;
	AddMovementInput(moveVector);

}

void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveRight", this, &AMyCharacter::MoveRight);
	PlayerInputComponent->BindAxis("MoveForward", this, &AMyCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Turn", this, &AMyCharacter::RotationTurn);
	PlayerInputComponent->BindAxis("Lookup", this, &AMyCharacter::RotationLookUp);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMyCharacter::RotationTurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMyCharacter::RotationLookupAtRate);

	PlayerInputComponent->BindAction("InteractionMode", IE_Pressed, this, &AMyCharacter::InteractionMode);
	PlayerInputComponent->BindAction("MovableMode", IE_Pressed, this, &AMyCharacter::MovableMode);
	PlayerInputComponent->BindAction("ObserverMode", IE_Pressed, this, &AMyCharacter::ChangeObserverMode);

	PlayerInputComponent->BindTouch(IE_Pressed, this, &AMyCharacter::TouchBegin);
	PlayerInputComponent->BindTouch(IE_Repeat, this, &AMyCharacter::TouchMoved);
	PlayerInputComponent->BindTouch(IE_Released, this, &AMyCharacter::TouchEnd);

}

void AMyCharacter::MoveRight(float value)
{
	
	if (value != 0 && gameState)
	{
		if (gameState->playState != EPlayState::Preparing)
		{
			rightAxis = value;
		}
		else rightAxis = 0.f;
	}
	else rightAxis = 0.f;
}

void AMyCharacter::MoveForward(float value)
{
	if (value != 0 && gameState)
	{
		if (gameState->playState != EPlayState::Preparing)
		{
			forwardAxis = value;
		}
		else forwardAxis = 0.f;
	}
	else forwardAxis = 0.f;
}

void AMyCharacter::RotationTurn(float value)
{
	if (gameState)
	{
		if (gameState->playState != EPlayState::Preparing) AddControllerYawInput(value);
	}
}

void AMyCharacter::RotationLookUp(float value)
{
	if (gameState)
	{
		if (gameState->playState != EPlayState::Preparing) AddControllerPitchInput(value);
	}
}

void AMyCharacter::RotationTurnAtRate(float value)
{
	if (gameState)
	{
		if (gameState->playState != EPlayState::Preparing) AddControllerYawInput(value * 45.f * GetWorld()->GetDeltaSeconds());
	}
}

void AMyCharacter::RotationLookupAtRate(float value)
{
	if (gameState)
	{
		if (gameState->playState != EPlayState::Preparing) AddControllerPitchInput(value * 45.f * GetWorld()->GetDeltaSeconds());
	}
}

void AMyCharacter::InteractionMode()
{
	if(PlayerController) PlayerController->InteractionMode();
}

void AMyCharacter::MovableMode()
{
	if (PlayerController) PlayerController->MovableMode();
}

void AMyCharacter::GetPromotion()
{
	if (PlayerController) PlayerController->bPromotion = true;
}

void AMyCharacter::ChangeObserverMode()
{
	AMyPlayerState* PS = Cast<AMyPlayerState>(GetPlayerState());
	gameState = Cast<AMyGameStateBase>(GetWorld()->GetGameState());
	if (PS->playerState == EPlayerState::PLAYER)
	{
		if (gameState->playState == EPlayState::Playing || gameState->playState == EPlayState::Promotion || gameState->playState == EPlayState::AIPlaying)
		{
			if (Observer)
			{
				gameins = Cast<UMyGameInstance>(GetGameInstance());
				gameins->wall->SetActorHiddenInGame(true);
				Controller->Possess(Observer);
			}
		}
	}
}

void AMyCharacter::SetSpectator_Implementation()
{
	if (PlayerController) bSpectator = PlayerController->bSpectator;
}

void AMyCharacter::TouchBegin(const ETouchIndex::Type FingerIndex,const FVector TouchedLoc)
{
	
	if (PlayerController && gameState)
	{
		TouchStartLoc = FVector2D(TouchedLoc.X, TouchedLoc.Y);

		
		if (gameState->playState == EPlayState::Playing)
		{

			FHitResult hr;
			if (PlayerController->GetHitResultUnderFingerByChannel(ETouchIndex::Touch1, ETraceTypeQuery::TraceTypeQuery1, true, hr))
			{
				AMyChessPiece* piece = Cast<AMyChessPiece>(hr.GetActor());
				if (piece)
				{
				//	PlayerController->GetPiece(piece, false);
				}
				AMyTile* tile = Cast<AMyTile>(hr.GetActor());
				if (tile)
				{
					//PlayerController->GetTile(tile);
				}
			}
		}
		
	}
	
}

void AMyCharacter::TouchMoved(const ETouchIndex::Type FingerIndex, const FVector TouchedLoc)
{
	if (PlayerController)
	{
		FVector2D tempvector2d = FVector2D(TouchedLoc.X - TouchStartLoc.X, TouchedLoc.Y - TouchStartLoc.Y);
		tempvector2d /= 8.f;

		PlayerController->AddYawInput(tempvector2d.X);
		PlayerController->AddPitchInput(tempvector2d.Y);
		TouchStartLoc = FVector2D(TouchedLoc.X, TouchedLoc.Y);
	}
}

void AMyCharacter::TouchEnd(const ETouchIndex::Type FingerIndex, const FVector TouchedLoc)
{
	if (PlayerController)
	{
		TouchStartLoc = FVector2D(TouchedLoc.X, TouchedLoc.Y);
	}
}

void AMyCharacter::UnPossessed()
{
	Super::UnPossessed();

	GetCharacterMovement()->StopMovementImmediately();
}