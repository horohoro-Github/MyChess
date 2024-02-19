// Fill out your copyright notice in the Description page of Project Settings.


#include "MyChessPiece.h"
#include "Kismet/GameplayStatics.h"
#include "../Player/MyPlayerController.h"
#include "MyTile.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
#include "../Player/MyPlayerState.h"
#include "../Game/MyGameStateBase.h"
#include "MyBoard.h"

AMyChessPiece::AMyChessPiece()
{
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Piece"));
	RootComponent = MeshComp;
	bReplicates = true;
	SetReplicateMovement(true);
	SetActorScale3D(FVector(20.f, 20.f, 20.f));
}

void AMyChessPiece::BeginPlay()
{
	Super::BeginPlay();
	gameState = Cast<AMyGameStateBase>(GetWorld()->GetGameState());
}

void AMyChessPiece::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (gameState)
	{
		if (gameState->TeamColor != TeamColor || gameState->playState == EPlayState::Promotion)
			MeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Ignore);
		else
			MeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);

		if (bMove == true && bDead == false && CurrentPosition != 99)
		{
			recloc = FMath::VInterpTo(GetActorLocation(), newLocation, GetWorld()->GetDeltaSeconds(), 6.f);

			float reclocX = GetActorLocation().X - recloc.X;
			float reclocY = GetActorLocation().Y - recloc.Y;
			float reclocZ = GetActorLocation().Z - recloc.Z;

			SetActorLocation(recloc);

			float maxloc = FMath::Max3(abs(reclocX), abs(reclocY), abs(reclocZ));

			if (maxloc < 0.8f && route)
			{
				if (TeamColor != gameState->TeamColor) gameState->board->UpdateRoute(File + Rank * 8, EDecalState::TargetPoint);

				route = false;
			}
			if (GetActorLocation() == newLocation)
			{
				bMove = false;
			}
		}
	}
}

void AMyChessPiece::MoveLocation(FVector location, bool showRoute)
{
	if (showRoute)
	{
		route = true;	
	}
	newLocation = location;
	bMove = true;
}

void AMyChessPiece::Highlight_Implementation(bool bHighlight)
{
	MeshComp->SetRenderCustomDepth(bHighlight);
	if (bHighlight == true)
	{
		MeshComp->SetCustomDepthStencilValue(2);
	}
	else
	{
		MeshComp->SetCustomDepthStencilValue(0);
	}

}

void AMyChessPiece::OnRep_SetMaterial()
{
	this->MeshComp->SetMaterial(0,CurrentMaterial);
	MeshComp->SetScalarParameterValueOnMaterials(FName("TeamColor"), teamColorParameter);
}

void AMyChessPiece::OnRep_SetStaticMesh()
{
	this->MeshComp->SetStaticMesh(CurrentMesh);
	MeshComp->SetScalarParameterValueOnMaterials(FName("TeamColor"), teamColorParameter);
}

void AMyChessPiece::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyChessPiece, CurrentPosition);
	DOREPLIFETIME(AMyChessPiece, bMove);
	DOREPLIFETIME(AMyChessPiece, bFirstAction);
	DOREPLIFETIME(AMyChessPiece, bDead);
	DOREPLIFETIME(AMyChessPiece, bCanMove);
	DOREPLIFETIME(AMyChessPiece, PieceNumbering);
	DOREPLIFETIME(AMyChessPiece, TeamColor);
	DOREPLIFETIME(AMyChessPiece, ChessType);
	DOREPLIFETIME(AMyChessPiece, bWhiteSquareBishop);
	DOREPLIFETIME(AMyChessPiece, recloc);
	DOREPLIFETIME(AMyChessPiece, newLocation);
	DOREPLIFETIME(AMyChessPiece, MeshComp);
	DOREPLIFETIME(AMyChessPiece, CurrentMaterial);
	DOREPLIFETIME(AMyChessPiece, CurrentMesh);
	DOREPLIFETIME(AMyChessPiece, teamColorParameter);
}