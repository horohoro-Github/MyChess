// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/SphereComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Components/SceneCaptureComponent2D.h"
#include "MyObserver.generated.h"

class AMyReferee;
class AMyCharacter;
class AMyPlayerController;
class AMyPlayerState;
class UMyGameInstance;

UCLASS()
class MYCHESS__API AMyObserver : public APawn
{
	GENERATED_BODY()

public:
	AMyObserver();

protected:
	virtual void BeginPlay() override;

public:	

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void UnPossessed() override;

	UPROPERTY()
		bool bDoOnce = true;
	UPROPERTY(Replicated)
		bool bWhite = true;
	UPROPERTY()
		int PlayerTeamColor = 2;
	UPROPERTY(Replicated)
		bool TopView = false;
	UPROPERTY(EditAnywhere)
		USphereComponent* CollisionComp;
	
	UPROPERTY(EditAnywhere)
		USpringArmComponent* SpringArm;
	UPROPERTY(EditAnywhere)
		UCameraComponent* CameraComp;

	UPROPERTY(EditAnywhere)
		UPawnMovementComponent* MovementComp;


	UPROPERTY()
		AMyReferee* Referee;
	UPROPERTY(EditAnywhere, Category = "Class")
		TSubclassOf<AActor> RefereeClass;
	UPROPERTY()
		AMyCharacter* Player;
	UPROPERTY()
		AMyPlayerController* PlayerController;
	UPROPERTY()
		AMyPlayerState* PlayerStates;
	UPROPERTY()
		UMyGameInstance* gameins = NULL;

	void MoveRight(float value);
	
	void MoveForward(float value);
	UFUNCTION(BlueprintCallable)
		void ZoomInCamera(float value);
	

	void InteractionMode();
	void MovableMode();
	
	UFUNCTION(BlueprintCallable)
		void ReverseCamera();
	UFUNCTION(BlueprintCallable)
		void ReturnToPlayer();
	UFUNCTION(Server, Reliable)
		void ReturnToPlayer_Server();
	void ReturnToPlayer_Server_Implementation();
	UFUNCTION(Client, Reliable)
		void ReturnToPlayer_Client();
	void ReturnToPlayer_Client_Implementation();
	void EscapeTheGame();

	UPROPERTY()
		bool bSpectator = false;
	UFUNCTION(Server, Reliable)
		void SetSpectator();
	void SetSpectator_Implementation();

	UFUNCTION(Client, Reliable)
		void SetWallTransparency(AActor* otheractor, bool bhiddens);
	void SetWallTransparency_Implementation(AActor* otheractor, bool bhiddens);

	UFUNCTION(Client, Reliable)
		void SetCameraRotation();
	void SetCameraRotation_Implementation();
	UPROPERTY()
		FVector2D TouchStartLoc = FVector2D(0.f, 0.f);

	UFUNCTION()
		void TouchBegin(const ETouchIndex::Type FingerIndex, const FVector TouchedLoc);
	
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const;
};
