// Fill out your copyright notice in the Description page of Project Settings.


#include "MyReferee.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "../Game/MyGameStateBase.h"

AMyReferee::AMyReferee()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	GetCapsuleComponent()->SetRelativeScale3D(FVector(1.5f, 1.5f, 1.5f));
	GetMesh()->SetRelativeLocation(FVector(0, 0, -90.f));
	GetMesh()->SetRelativeRotation(FRotator(0, -90.f, 0));

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> skMesh(TEXT("SkeletalMesh'/Game/Meshes/whiteclown_n_hallin.whiteclown_n_hallin'"));
	if (skMesh.Succeeded()) GetMesh()->SetSkeletalMesh(skMesh.Object);

	static ConstructorHelpers::FObjectFinder<UAnimBlueprintGeneratedClass> animBP(TEXT("AnimBlueprint'/Game/Animations/Referee/Referee_AnimBP.Referee_AnimBP_C'"));
	if (animBP.Succeeded()) GetMesh()->AnimClass = animBP.Object;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> montageIdle(TEXT("AnimMontage'/Game/Animations/Referee/hallin_idle_Montage.hallin_idle_Montage'"));
	if (montageIdle.Succeeded()) MontageIdle = montageIdle.Object;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> leftHand(TEXT("AnimMontage'/Game/Animations/Referee/Referee_Left_2_Montage.Referee_Left_2_Montage'"));
	if (leftHand.Succeeded()) MontageLeftHand = leftHand.Object;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> rightHand(TEXT("AnimMontage'/Game/Animations/Referee/RightHand_Montage.RightHand_Montage'"));
	if (rightHand.Succeeded()) MontageRightHand = rightHand.Object;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> shake(TEXT("AnimMontage'/Game/Animations/Referee/Hallin_HandShakes_Montage.Hallin_HandShakes_Montage'"));
	if (shake.Succeeded()) MontageHandShake = shake.Object;

}

void AMyReferee::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle delayHande;
	FTimerDelegate delayDelegate = FTimerDelegate::CreateLambda([&]()
		{
			gameState = Cast<AMyGameStateBase>(GetWorld()->GetGameState());
		});

	GetWorld()->GetTimerManager().SetTimer(delayHande, delayDelegate, 0.2f, false);
}

void AMyReferee::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	if (gameState)
	{
		if (gameState->playState == EPlayState::Playing)
		{
			if (gameState->TeamColor != currentTeamColor)
			{
				currentTeamColor = gameState->TeamColor;
				if (currentTeamColor == 0)
				{
					UAnimInstance* AnimIns = GetMesh()->GetAnimInstance();
					AnimIns->Montage_Play(MontageRightHand, 0.01f);
				}
				if (currentTeamColor == 1)
				{
					UAnimInstance* AnimIns = GetMesh()->GetAnimInstance();
					AnimIns->Montage_Play(MontageLeftHand, 0.01f);
				}
			
			}
		}
		else
		{
			if (currentTeamColor != 2)
			{
				currentTeamColor = 2;
				UAnimInstance* AnimIns = GetMesh()->GetAnimInstance();
				AnimIns->Montage_Play(MontageIdle, 0.01f);
			}
		}
	}
}

void AMyReferee::HandShake_Server_Implementation()
{
	if (bDead == false)
	{
		UAnimInstance* AnimIns = GetMesh()->GetAnimInstance();
		AnimIns->Montage_Play(MontageHandShake, 1.f);
	}
}

void AMyReferee::HandShake_Client_Implementation()
{
	if (bDead == false)
	{
		UAnimInstance* AnimIns = GetMesh()->GetAnimInstance();
		AnimIns->Montage_Play(MontageHandShake, 1.f);
	}
}

void AMyReferee::HandUp(int TeamColor)
{
	UAnimInstance* AnimIns = GetMesh()->GetAnimInstance();
	if (TeamColor == 0) AnimIns->Montage_Play(MontageRightHand, 0.01f); else AnimIns->Montage_Play(MontageLeftHand, 0.01f);
}

void AMyReferee::HandUp_Server_Implementation(int TeamColor)
{
	if (bDead == false)
	{
		UAnimInstance* AnimIns = GetMesh()->GetAnimInstance();
		if (TeamColor == 0) AnimIns->Montage_Play(MontageRightHand, 0.01f); else AnimIns->Montage_Play(MontageLeftHand, 0.01f);
	}
}


void AMyReferee::HandUp_Client_Implementation(int TeamColor)
{
	if (bDead == false)
	{
		UAnimInstance* AnimIns = GetMesh()->GetAnimInstance();
		if (TeamColor == 0) AnimIns->Montage_Play(MontageRightHand, 0.01f); else AnimIns->Montage_Play(MontageLeftHand, 0.01f);
	}
}

void AMyReferee::ReturnToIdle_Server_Implementation()
{
	UAnimInstance* AnimIns = GetMesh()->GetAnimInstance();
	AnimIns->Montage_Play(MontageIdle, 0.01f);
}

void AMyReferee::ReturnToIdle_Client_Implementation()
{
	UAnimInstance* AnimIns = GetMesh()->GetAnimInstance();
	AnimIns->Montage_Play(MontageIdle, 0.01f);
}

void AMyReferee::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyReferee, bDead);
}

