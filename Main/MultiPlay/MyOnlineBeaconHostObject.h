// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OnlineBeaconHostObject.h"
#include "../MyChess_.h"
#include "MyOnlineBeaconHostObject.generated.h"

class  AMyOnlineBeaconClient;
class FHttpModule;
/**
 * 
 */
UCLASS()
class MYCHESS__API AMyOnlineBeaconHostObject : public AOnlineBeaconHostObject
{
	GENERATED_BODY()
	
public:

	AMyOnlineBeaconHostObject();

	virtual void BeginPlay()override;

	virtual void EndPlay(EEndPlayReason::Type type) override;

	virtual void OnClientConnected(AOnlineBeaconClient* NewClientActor, UNetConnection* ClientConnection) override;

	virtual void NotifyClientDisconnected(AOnlineBeaconClient* LeavingClientActor) override;


	TArray<AMyOnlineBeaconClient*> AllPlayerBeacons;

	TArray<AOnlineBeaconClient*> GetBeaconClients();

	void ServerActivate(bool Activate);

	AMyOnlineBeaconClient* GetBeaconsClient(FString bcid);

	FHttpModule* Http;

	FServerData serverData;
};
