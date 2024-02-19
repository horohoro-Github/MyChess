// Fill out your copyright notice in the Description page of Project Settings.


#include "MyDecal.h"
#include "Net/UnrealNetwork.h"

AMyDecal::AMyDecal()
{
	bReplicates = true;
	plane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Plane"));
	RootComponent = plane;

	decalMaterials.SetNum(3);
	
	
	struct FConstructAsset
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> planeMesh;

		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> nomalMat;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> startPointMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> targetPointMaterial;
		
		FConstructAsset()
			: planeMesh(TEXT("StaticMesh'/Engine/BasicShapes/Plane.Plane'"))
			, nomalMat(TEXT("MaterialInstanceConstant'/Game/Materials/Decal/NoMaterial_Inst.NoMaterial_Inst'"))
			, startPointMaterial(TEXT("MaterialInstanceConstant'/Game/Materials/Decal/startCircle_Mat_Inst.startCircle_Mat_Inst'"))
			, targetPointMaterial(TEXT("MaterialInstanceConstant'/Game/Materials/Decal/targetCircle_Mat_Inst.targetCircle_Mat_Inst'"))
		{}
	};

	static FConstructAsset ConstructAssets;

	plane->SetStaticMesh(ConstructAssets.planeMesh.Get());
	decalMaterials[0] = ConstructAssets.nomalMat.Get();	decalMaterials[1] = ConstructAssets.startPointMaterial.Get(); decalMaterials[2] = ConstructAssets.targetPointMaterial.Get();

	plane->SetMaterial(0, decalMaterials[0]);
	plane->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Ignore);
	
}

void AMyDecal::OnRep_SetMaterial()
{
	plane->SetMaterial(0, currentMaterial);
}

void AMyDecal::ApplyDecal(EDecalState decalState)
{
	if (currentDecalState != decalState)
	{
		currentDecalState = decalState;
		plane->SetMaterial(0, decalMaterials[decalState]);
		currentMaterial = decalMaterials[decalState];
	}
}

void AMyDecal::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyDecal, currentMaterial);
}

