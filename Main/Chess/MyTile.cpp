
#include "MyTile.h"
#include "Kismet/GameplayStatics.h"
#include "MyChessPiece.h"
#include "../Player/MyPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "../AI/MyAI.h"
#include "../Game/MyChess_GameModeBase.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"

AMyTile::AMyTile()
{
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tile"));
	RootComponent = MeshComp;
	bReplicates = true;
}

void AMyTile::SetupMaterial()
{
	MeshComp->SetMaterial(0, CurrentMaterial);
}

void AMyTile::SetScalarParameter_Implementation(FName paramName, float value)
{
	this->MeshComp->SetScalarParameterValueOnMaterials(paramName, value);
}

void AMyTile::OnRep_SetMaterial()
{
	if (!HasAuthority())
	{
		MeshComp->SetMaterial(0, CurrentMaterial);
		MeshComp->SetScalarParameterValueOnMaterials(FName(TEXT("ColorBrightness")), colorBrightness);
	}
}

void AMyTile::OnRep_SetStaticMesh()
{
	if (!HasAuthority())
	{
		this->MeshComp->SetStaticMesh(CurrentMesh);
		MeshComp->SetScalarParameterValueOnMaterials(FName(TEXT("ColorBrightness")), colorBrightness);
	}
}

void AMyTile::OnRep_SetEPIntensity()
{
	if(!HasAuthority())
	MeshComp->SetScalarParameterValueOnMaterials(FName(TEXT("Emphasize")), EPIntensity);
}

void AMyTile::Highlight_Implementation(bool bHighlight)
{
	MeshComp->SetRenderCustomDepth(bHighlight);
	if (bHighlight == false)
	{
		MeshComp->SetCustomDepthStencilValue(0);
	}
}

void AMyTile::ChangeMaterial_Implementation(UMaterialInstance* mat)
{
	this->MeshComp->SetMaterial(0, mat);
}

void AMyTile::ChangeMaterial_Server_Implementation(UMaterialInstance* mat)
{
	ChangeMaterial(mat);
}

void AMyTile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyTile, ColorIndex);
	DOREPLIFETIME(AMyTile, TileNumber);
	DOREPLIFETIME(AMyTile, TileTeamColor);
	DOREPLIFETIME(AMyTile, CurrentPiece);
	DOREPLIFETIME(AMyTile, CurrentMaterial);
	DOREPLIFETIME(AMyTile, CurrentMesh);
	DOREPLIFETIME(AMyTile, TileDeadZoneNumber);
	DOREPLIFETIME(AMyTile, TileState);
	DOREPLIFETIME(AMyTile, MeshComp);
	DOREPLIFETIME(AMyTile, MaterialColors);
	DOREPLIFETIME(AMyTile, colorBrightness);
	DOREPLIFETIME(AMyTile, EPIntensity);
}