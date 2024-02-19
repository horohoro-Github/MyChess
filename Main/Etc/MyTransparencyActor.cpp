// Fill out your copyright notice in the Description page of Project Settings.


#include "MyTransparencyActor.h"

AMyTransparencyActor::AMyTransparencyActor()
{
	SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	RootComponent = SceneComp;

	Ceiling = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ceiling"));
	Lamp1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Lamp1"));
	Lamp2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Lamp2"));
	Ceiling->SetupAttachment(SceneComp);
	Lamp1->SetupAttachment(SceneComp);
	Lamp2->SetupAttachment(SceneComp);
	Ceiling->SetCollisionProfileName(FName("NoCollision"));
	Lamp1->SetCollisionProfileName(FName("NoCollision"));
	Lamp2->SetCollisionProfileName(FName("NoCollision"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CeilingMesh(TEXT("StaticMesh'/Game/Meshes/CubeDefault.CubeDefault'"));
	if (CeilingMesh.Succeeded()) Ceiling->SetStaticMesh(CeilingMesh.Object);
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> LampMesh(TEXT("StaticMesh'/Game/StarterContent/Props/SM_Lamp_Wall.SM_Lamp_Wall'"));
	if (LampMesh.Succeeded())
	{
		Lamp1->SetStaticMesh(LampMesh.Object);
		Lamp2->SetStaticMesh(LampMesh.Object);
	}
	
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> CeilingMat(TEXT("MaterialInstanceConstant'/Game/Materials/MaterialsInstance/Cube_Material_Inst.Cube_Material_Inst'"));
	if (CeilingMat.Succeeded()) Ceiling->SetMaterial(0, CeilingMat.Object);

	Ceiling->SetRelativeScale3D(FVector(30.f, 30.f, 0.2f));
	Lamp1->SetRelativeRotation(FRotator(180.f, 0.f, 0.f));
	Lamp2->SetRelativeRotation(FRotator(180.f, 180.f, 0.f));

	Tags.Add("WallTag");
}
