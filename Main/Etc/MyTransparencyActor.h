// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "MyTransparencyActor.generated.h"

UCLASS()
class MYCHESS__API AMyTransparencyActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AMyTransparencyActor();
public:	
	UPROPERTY(EditAnywhere)
		USceneComponent* SceneComp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* Ceiling;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* Lamp1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* Lamp2;
};
