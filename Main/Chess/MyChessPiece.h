// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../MyChess_.h"
#include "MyChessPiece.generated.h"


class AMyGameStateBase;
class AMyPlayerController;
class AMyChessPiece;
class AMyBoard;
UCLASS()
class MYCHESS__API AMyChessPiece : public AActor
{
	GENERATED_BODY()
	
public:	
	AMyChessPiece();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(Replicated)
		FVector newLocation;
	UPROPERTY(Replicated)
		FVector recloc;
	UPROPERTY(Replicated)
		bool bMove = false;
	UPROPERTY(Replicated)
		bool bFirstAction = true;
	UPROPERTY(Replicated)
		bool bDead = false;
	UPROPERTY(Replicated)
		bool bCanMove = false;
	UPROPERTY(Replicated)
		bool bWhiteSquareBishop = true;
	UPROPERTY(Replicated)
		int PieceNumbering = 0;
	UPROPERTY(EditAnywhere, Replicated)
		UStaticMeshComponent* MeshComp;

	AMyGameStateBase* gameState = NULL;

	UPROPERTY(EditAnywhere)
		bool bMajorPieceOrPawn = false;
	UPROPERTY(EditAnywhere, Category = "State", Replicated)
		int TeamColor = 0;
	UPROPERTY(EditAnywhere, Category = "State", Replicated)
		uint8 ChessType;
	UPROPERTY(EditAnywhere, Category = "State")
		UStaticMesh* OriginMesh;
	UPROPERTY(EditAnywhere, Category = "State")
		uint8 OriginChessType;
	UPROPERTY(Replicated)
		float teamColorParameter = 0.f;
	UPROPERTY(Replicated)
		int CurrentPosition = -1;
	bool route = false;

	UPROPERTY(ReplicatedUsing = OnRep_SetStaticMesh)
		UStaticMesh* CurrentMesh;

	UPROPERTY(ReplicatedUsing = OnRep_SetMaterial)
		UMaterialInterface* CurrentMaterial;

	int File = -1;
	int Rank = -1;

	void MoveLocation(FVector location, bool showRoute);

	UFUNCTION(NetMulticast, Reliable)
		void Highlight(bool bHighlight);
	void Highlight_Implementation(bool bHighlight);

	UFUNCTION()
		virtual void OnRep_SetMaterial();
	UFUNCTION()
		virtual void OnRep_SetStaticMesh();
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const;
};
