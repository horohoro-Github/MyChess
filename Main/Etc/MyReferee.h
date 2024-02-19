// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Animation/AnimMontage.h"
#include "MyReferee.generated.h"

class AMyGameStateBase;
UCLASS()
class MYCHESS__API AMyReferee : public ACharacter
{
	GENERATED_BODY()

public:
	AMyReferee();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(Replicated)
		bool bDead = false;
	UPROPERTY()
		int handup = 0;

	UPROPERTY(EditAnywhere)
		UAnimMontage* MontageIdle;
	UPROPERTY(EditAnywhere)
		UAnimMontage* MontageLeftHand;
	UPROPERTY(EditAnywhere)
		UAnimMontage* MontageRightHand;
	UPROPERTY(EditAnywhere)
		UAnimMontage* MontageHandShake;

	UFUNCTION(NetMulticast, Reliable)
		void HandShake_Server();
	void HandShake_Server_Implementation();
	
	UFUNCTION(Server, Reliable)
		void HandShake_Client();
	void HandShake_Client_Implementation();

	void HandUp(int TeamColor);
	UFUNCTION(NetMulticast, Reliable)
		void HandUp_Server(int TeamColor);
	void HandUp_Server_Implementation(int TeamColor);
	
	UFUNCTION(Server, Reliable)
		void HandUp_Client(int TeamColor);
	void HandUp_Client_Implementation(int TeamColor);

	UFUNCTION(NetMulticast, Reliable)
		void ReturnToIdle_Server();
	void ReturnToIdle_Server_Implementation();

	UFUNCTION(Server, Reliable)
		void ReturnToIdle_Client();
	void ReturnToIdle_Client_Implementation();

	AMyGameStateBase* gameState = NULL;
	int currentTeamColor = 2;
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const;
};
