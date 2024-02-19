// Fill out your copyright notice in the Description page of Project Settings.


#include "MyOnlineBeaconHostObject.h"
#include "MyOnlineBeaconClient.h"
#include "../Game/MyChess_GameModeBase.h"
#include "Http.h"
#include "JsonObjectConverter.h"
#include "Kismet/GameplayStatics.h"

AMyOnlineBeaconHostObject::AMyOnlineBeaconHostObject()
{
	ClientBeaconActorClass = AMyOnlineBeaconClient::StaticClass();
	BeaconTypeName = ClientBeaconActorClass->GetName();

	Http = &FHttpModule::Get();
	bReplicates = true;
	
}

void AMyOnlineBeaconHostObject::BeginPlay()
{
	Super::BeginPlay();

	int a = GetWorld()->URL.Port;

	serverData.serverport = a;
	serverData.serverip = SERVER_IPADDR;
	UE_LOG(LogTemp, Warning, TEXT("beacon host port %d"), a);
	if (a == 7777)
	{
		serverData.servertype = "MainServer";
		AMyChess_GameModeBase* gamemode = Cast<AMyChess_GameModeBase>(GetWorld()->GetAuthGameMode());
		gamemode->ServerActivate(true);
	}
	else
	{
		serverData.servertype = "GameServer";
	}
}

void AMyOnlineBeaconHostObject::EndPlay(EEndPlayReason::Type type)
{
	Super::EndPlay(type);

}

void AMyOnlineBeaconHostObject::OnClientConnected(AOnlineBeaconClient* NewClientActor, UNetConnection* ClientConnection)
{
	Super::OnClientConnected(NewClientActor, ClientConnection);

}

void AMyOnlineBeaconHostObject::NotifyClientDisconnected(AOnlineBeaconClient* LeavingClientActor)
{
	Super::NotifyClientDisconnected(LeavingClientActor);

	AMyOnlineBeaconClient* BC = Cast<AMyOnlineBeaconClient>(LeavingClientActor);

	AMyChess_GameModeBase* gamemode = Cast<AMyChess_GameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

	
	if (BC->userdata.userid != "" && BC->token == 0)
	{
		if (gamemode)
		{
			gamemode->UserConnection(BC, BC->userdata.userid, BC->userdata.userpass, false, true);
		}
	}
	else if (BC->userdata.userid != "" && BC->token != 0)
	{
		
		UE_LOG(LogTemp, Warning, TEXT("Remaining %d Tokens"), gamemode->verifyToken.Num());
	}
	
	
}

TArray<AOnlineBeaconClient*> AMyOnlineBeaconHostObject::GetBeaconClients()
{
	return ClientActors;
}

void AMyOnlineBeaconHostObject::ServerActivate(bool Activate)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Requests = Http->CreateRequest();
	
	if (Activate)
	{
		Requests->SetURL("http://127.0.0.1:8080/Server/AddServer");
		Requests->SetVerb("Post");
	}
	else
	{
		Requests->SetURL("http://127.0.0.1:8080/Server/DropServer");
		Requests->SetVerb("Delete");
	}
	Requests->SetHeader(TEXT("Content-type"), TEXT("application/json"));

	FString JsonString;
	FServerData servd;
	servd.serverip = serverData.serverip;
	servd.serverport = serverData.serverport;
	servd.servertype = serverData.servertype;
	
	FJsonObjectConverter::UStructToJsonObjectString(servd, JsonString);
	Requests->SetContentAsString(JsonString);
	Requests->ProcessRequest();
}

AMyOnlineBeaconClient* AMyOnlineBeaconHostObject::GetBeaconsClient(FString bcid)
{
	for (int i = 0; i < ClientActors.Num(); i++)
	{
		AMyOnlineBeaconClient* BeaconClinet = Cast<AMyOnlineBeaconClient>(ClientActors[i]);

		if (BeaconClinet->userdata.userid == bcid)
		{
			return BeaconClinet;
		}
	}
	return nullptr;
}