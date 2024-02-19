// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "../Player/MyPlayerController.h"
#include "Http.h"
#include "../MyChess_.h"
#include "MyChess_GameModeBase.generated.h"


class AMyPlayerState;
class AMyPlayerController;
class AMyOnlineBeaconClient;
class UMyUserWidget;
class AMyBoard;
class FHttpModule;


USTRUCT()
struct FOptions
{
	GENERATED_BODY()
public:
	UPROPERTY()
		FString Options = "";
	UPROPERTY()
		FUniqueNetIdRepl uniqueid;
};

USTRUCT(BlueprintType)
struct FLoginData
{
	GENERATED_BODY()
public:
	UPROPERTY()
		FString UID = "-1";
	UPROPERTY()
		FString userid = "";
	UPROPERTY()
		FString userpassword = "";
};

UCLASS()
class MYCHESS__API AMyChess_GameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	TArray<FUsersData> AllUsers;

	AMyChess_GameModeBase(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay()override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable)
		bool CreateHostBeacon();

	class AMyOnlineBeaconHostObject* HostObject = NULL;

	TArray<AMyOnlineBeaconClient*> MatchPlayers;
	int MatchPlayerNumber = 0;

	TMap<uint64, FVerifyToken*> verifyToken;
	TArray<FVerifyToken*> TokenArray;

	FRandomStream randomStream;

	UPROPERTY(EditAnywhere)
		TSubclassOf<APawn> CharacterClass;
	UPROPERTY(EditAnywhere)
		TSubclassOf<APawn> ObserverClass;

	FServerData serverdata;

	//ConnectedControllers : 플레이어 + 관전자, ConnectedPlayers : 플레이어
	//
	virtual FString BoardPositionStrings();
	virtual FString ZobristHash();
	virtual uint64 RandomNumber(int seed);

	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId,	FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

	FString APP_ID = RAW_APP_ID;

	void QuitServer();
	
	void ServerActivate(bool Activate);

	UPROPERTY()
		float QuitTimer = 15.f;

	void RenamePlayer(AMyOnlineBeaconClient* BC, const FString& id, const FString& name);

	UPROPERTY()
		FTimerHandle movehandle;

	void UserConnection(AMyOnlineBeaconClient* BC, const FString& id, const FString& pass, bool login, bool leveltravel);

	void AddUserData(AMyOnlineBeaconClient* BC, FString id, const FString& pass);

	void MatchingTask(AMyOnlineBeaconClient* BC);

	void CancelMatching(AMyOnlineBeaconClient* BC);

	void MatchManagement();

	void CreateGameServer(AMyOnlineBeaconClient* BC1, AMyOnlineBeaconClient* BC2);

	void LoginWithToken(AMyOnlineBeaconClient* BC, uint64 t);

	void CheckServerTraveler();

	void GetAllUserData();

	TArray<FString> GetSplitStringArray(FString LongString);

	TSubclassOf<UMyUserWidget> LobbyWidgetClass;

	TSubclassOf<UMyUserWidget> LoginWidgetClass;

	TSubclassOf<UMyUserWidget> InGameWidgetClass;

	TSubclassOf<APawn> RefereeClass;

	TSubclassOf<AMyBoard> boardClass;

	void GetFriends(AMyOnlineBeaconClient* BC);
	void AddFriend(AMyOnlineBeaconClient* BC, FString name, FString Type);

	void InviteTheGame(AMyOnlineBeaconClient* BC, FUsersData user, FString Type);
protected:
	FHttpModule* Http;

private:

};
