// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../MyChess_.h"
#include "MyDecal.generated.h"


UCLASS()
class MYCHESS__API AMyDecal : public AActor
{
	GENERATED_BODY()
	
public:	
	AMyDecal();

public:	
	
	UStaticMeshComponent* plane;

	TArray<UMaterialInstance*> decalMaterials;

	UPROPERTY(ReplicatedUsing = OnRep_SetMaterial)
		UMaterialInstance* currentMaterial;
	UFUNCTION()
		virtual void OnRep_SetMaterial();

	void ApplyDecal(EDecalState decalState);

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const;


private:
	EDecalState currentDecalState = EDecalState::Nomal;

};
