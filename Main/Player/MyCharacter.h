// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "MyCharacter.generated.h"


class AMyObserver;
class AMyReferee;
class AMyPlayerController;
class UMyGameInstance;
class AMyPlayerState;

UCLASS()
class MYCHESS__API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMyCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void UnPossessed();

	UPROPERTY(EditAnywhere)
		UCameraComponent* CameraComp;
	UPROPERTY(EditAnywhere)
		USpringArmComponent* SpringArm;

	UPROPERTY()
		bool bDoOnce = true;
	UPROPERTY()
		AMyObserver* Observer;
	UPROPERTY(EditAnywhere)
		TSubclassOf<AActor>ObserverClass;
	UPROPERTY()
		AMyPlayerController* PlayerController;
	void MoveRight(float value);
	void MoveForward(float value);
	void RotationTurn(float value);
	void RotationLookUp(float value);
	void RotationTurnAtRate(float value);
	void RotationLookupAtRate(float value);
	
	void InteractionMode();
	void MovableMode();
	void GetPromotion();

	UFUNCTION(BlueprintCallable)
		void ChangeObserverMode();

	UPROPERTY()
		UMyGameInstance* gameins = NULL;
	UPROPERTY()
		AMyPlayerState* PlayerStates;
	UPROPERTY()
		FString PlayerName = "None";

	UPROPERTY()
		FVector2D TouchStartLoc = FVector2D(0.f, 0.f);

	UPROPERTY()
		bool bSpectator = false;
	UFUNCTION(Server, Reliable)
		void SetSpectator();
	void SetSpectator_Implementation();

	UFUNCTION()
		void TouchBegin(const ETouchIndex::Type FingerIndex, const FVector TouchedLoc);
	UFUNCTION()
		void TouchMoved(const ETouchIndex::Type FingerIndex, const FVector TouchedLoc);
	UFUNCTION()
		void TouchEnd(const ETouchIndex::Type FingerIndex, const FVector TouchedLoc);
private:

	float forwardAxis = 0.f;
	float rightAxis = 0.f;
	class AMyGameStateBase* gameState = NULL;
};
