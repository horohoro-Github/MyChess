// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OnlineBeaconClient.h"
#include "Interfaces/IHttpRequest.h"
#include "../Player/MyPlayerState.h"
#include "Containers/UnrealString.h"
#include "../Player/MyPlayerController.h"
#include "MyOnlineBeaconClient.generated.h"

class AMyPlayerController;
class AMyPlayerState;
class UMyGameInstance;

UCLASS()
class MYCHESS__API AMyOnlineBeaconClient : public AOnlineBeaconClient
{
	GENERATED_BODY()
public:
	AMyOnlineBeaconClient();

	enum ENextAction : uint8
	{
		NONEACTION,
		FRIENDSLIST,
		INVITE
	};

	FUsersData userdata;
	FFriendsData friendsdata;

	UFUNCTION(Client, Reliable)
		void GetFriendsComplete(FFriendsData friendsdb, const TArray<FUsersData>& username);
	void GetFriendsComplete_Implementation(FFriendsData friendsdb, const TArray<FUsersData>& username);

	UFUNCTION(Server, Reliable)
		void GetFriends();
	void GetFriends_Implementation();

	UFUNCTION(Server, Reliable)
		void AddFriends(const FString& name, const FString& Type);
	void AddFriends_Implementation(const FString& name, const FString& Type);

	UFUNCTION(Server, Reliable)
		void TakeOnChessWithFriend(FUsersData user, const FString& Type);
	void TakeOnChessWithFriend_Implementation(FUsersData user, const FString& Type);

	UFUNCTION(Client, Reliable)
		void Invite_Complete(FUsersData user, const TArray<FUsersData>& username, FUsersData otherUser, bool alarm, bool Send);
	void Invite_Complete_Implementation(FUsersData user, const TArray<FUsersData>& username, FUsersData otherUser, bool alarm, bool Send);

	UFUNCTION(Client, Reliable)
		void StopInvite(bool LoadLevel);
	void StopInvite_Implementation(bool LoadLevel);

	uint64 token = 0;
	FTimerHandle h_GetFriends;

	UFUNCTION(BlueprintCallable)
		bool ConnectToServer(const FString& address);

	AMyPlayerState* PS = NULL;

	virtual void OnFailure()override;
	virtual void OnConnected()override;

	UMyGameInstance* gameins = NULL;

	UFUNCTION(BlueprintCallable)
		EBeaconConnectionState GetConnectionState();

	UFUNCTION(Server, Reliable)
		void RenamePlayer(const FString& id, const FString& name);
	void RenamePlayer_Implementation(const FString& id, const FString& name);

	UFUNCTION(Client, Reliable)
		void RenamePlayerComplete(const FString& name, bool Succeded);
	void RenamePlayerComplete_Implementation(const FString& name, bool Succeded);

	UFUNCTION(Server, Reliable)
		void AccountRegistration(const FString& id, const FString& pass);
	void AccountRegistration_Implementation(const FString& id, const FString& pass);

	UFUNCTION(Server, Reliable)
		void LoginAttempt(const FString& id, const FString& pass, bool login, bool leveltravel);
	void LoginAttempt_Implementation(const FString& id, const FString& pass, bool login, bool leveltravel);

	UFUNCTION(Client, Reliable)
		void UpdateData(FUsersData userdb);
	void UpdateData_Implementation(FUsersData userdb);

	UFUNCTION(Client, Reliable)
		void LoginComplete(bool Succeded, bool leveltravel);
	void LoginComplete_Implementation(bool Succeded, bool leveltravel);

	UFUNCTION(Client, Reliable)
		void AddUserComplete(bool Succeded, const FString& id);
	void AddUserComplete_Implementation(bool Succeded, const FString& id);

	UFUNCTION(Server, Reliable)
		void MatchMaking(bool Cancel);
	void MatchMaking_Implementation(bool Cancel);

	UFUNCTION(Client, Reliable)
		void MatchComplete(uint64 t, bool Succeded, const FString& url, const FString& optionString);
	void MatchComplete_Implementation(uint64 t, bool Succeded, const FString& url, const FString& optionString);

	UFUNCTION(Server, Reliable)
		void LoginWithToken(uint64 t);
	void LoginWithToken_Implementation(uint64 t);

	UFUNCTION(Server, Reliable)
		void Relogin(const FString& url, FUsersData userdb);
	void Relogin_Implementation(const FString& url, FUsersData userdb);

	UFUNCTION(Client, Reliable)
		void Relogin_OpenLevel(const FString& url, uint64 tkn);
	void Relogin_OpenLevel_Implementation(const FString& url, uint64 tkn);

	FString GetLevelName();

	UFUNCTION(Client, Reliable)
		void PrintSystemMessage(const FString& message);
	void PrintSystemMessage_Implementation(const FString& message);

	UPROPERTY()
		FString LevelName = "";

	UFUNCTION(Server, Reliable)
		void SetLevelName(const FString& name);
	void SetLevelName_Implementation(const FString& name);

};
