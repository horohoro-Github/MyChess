// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyTile.generated.h"

class AMyChessPiece;
class AMyPlayerController;
UCLASS()
class MYCHESS__API AMyTile : public AActor
{
	GENERATED_BODY()
	
public:	
	AMyTile();

public:	
	UPROPERTY()
		bool bDoOnce = true;
	UPROPERTY()
		bool bDoOnce_B = true;
	UPROPERTY()
		AMyChessPiece* ChessPiece = NULL;

	UPROPERTY(Replicated)
		int32 ColorIndex = 0;
	UPROPERTY(EditAnywhere, Replicated)
		UStaticMeshComponent* MeshComp;
	UPROPERTY(EditAnywhere, Category = "State")
		bool bDeadZone = false;
	UPROPERTY(EditAnywhere, Category = "State")
		bool bWhiteSquare = true;
	UPROPERTY(EditAnywhere, Category = "Class")
		UClass* SpawnClass_Black[9];
	UPROPERTY(EditAnywhere, Category = "Class")
		UClass* SpawnClass_White[9];
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_SetMaterial)
		UMaterialInterface* MaterialColors[6];
	UPROPERTY(EditAnywhere)
		UMaterialInterface* EmphasizeColors[3];
	UPROPERTY(ReplicatedUsing = OnRep_SetMaterial)
		UMaterialInterface* CurrentMaterial;
	UPROPERTY(ReplicatedUsing = OnRep_SetStaticMesh)
		UStaticMesh* CurrentMesh;
	UPROPERTY(Replicated)
		float colorBrightness = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_SetEPIntensity)
		float EPIntensity = 0.f;

	UPROPERTY(Replicated)
		AMyChessPiece* CurrentPiece = NULL;

	UPROPERTY(Replicated)
		int TileTeamColor = 2;
	UPROPERTY(Replicated)
		int TileNumber = -1;

	int EndLine = 2;
	UPROPERTY(Replicated)
		int TileDeadZoneNumber = 0;
	UPROPERTY(Replicated)
		int TileState = 0;

	int File = 0;
	int Rank = 0;

	void SetupMaterial();

	UFUNCTION(NetMulticast, Reliable)
		void SetScalarParameter(FName paramName, float value);
	void SetScalarParameter_Implementation(FName paramName, float value);

	UFUNCTION()
		virtual void OnRep_SetMaterial();
	UFUNCTION()
		virtual void OnRep_SetStaticMesh();
	UFUNCTION()
		virtual void OnRep_SetEPIntensity();

	UFUNCTION(NetMulticast, Reliable)
		void Highlight(bool bHighlight);
	void Highlight_Implementation(bool bHighlight);

	UFUNCTION(NetMulticast, Reliable)
		void ChangeMaterial(UMaterialInstance* mat);
	void ChangeMaterial_Implementation(UMaterialInstance* mat);

	UFUNCTION(Server, Reliable)
		void ChangeMaterial_Server(UMaterialInstance* mat);
	void ChangeMaterial_Server_Implementation(UMaterialInstance* mat);
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const;
};
