// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
/*
#pragma warning(push)
#pragma warning(disable: 4996)
#include "Steam/steam_api.h"
#pragma warning(pop)
*/
#include "MyGameInstance.generated.h"
/**
 * 
 */
class AMyTransparencyActor;
class AMyOnlineBeaconClient;
class AMyPlayerController;
class AMyPlayerState;

UCLASS()
class MYCHESS__API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:

	TArray<FUsersData> FindUserName;

	UPROPERTY(BlueprintReadWrite)
		FUsersData userdb;

	UPROPERTY(BlueprintReadWrite)
		FFriendsData friendsdb;

	UMyGameInstance();

	virtual void Init()override;

	UFUNCTION()
		virtual void BeginLoadingScreen(const FString& MapName);
	UFUNCTION()
		virtual void EndLoadingScreen(UWorld* InLoadedWorld);

	UPROPERTY(EditAnywhere)
		TSubclassOf<AActor> WallClass;

	AMyTransparencyActor* wall;

	FTimerHandle reloginHandle;

	TSubclassOf<AActor> LightClass; //모바일
	
	UPROPERTY(EditAnywhere, Category = "Widget")
		TSubclassOf<UUserWidget> LoadingWidgetClass;
	UPROPERTY()
		UUserWidget* LoadingWidget = NULL;
	uint64 token = 0;

	UFUNCTION(BlueprintCallable)
		void RegisterID(FString id, FString password);

	UFUNCTION(BlueprintCallable)
		void LoginAttempt(FString id, FString pass, bool login);

	UFUNCTION(BlueprintCallable)
		void PlayerRename(FString name);

	UFUNCTION(BlueprintCallable)
		void AddToFriends(FString name, FString Type);

	UFUNCTION(BlueprintCallable)
		void TakeOnChessWithFriend(FUsersData user, FString Type);

	UFUNCTION(BlueprintCallable)
		void CreateLoadingWidget();

	UFUNCTION(BlueprintCallable)
		void ReConnectToServer();

	UFUNCTION(BlueprintCallable)
		void OpenLevel(FString url, FString Optionstr);

	UFUNCTION(BlueprintCallable)
		void FindMatching(bool Cancel);

	UFUNCTION(BlueprintCallable)
		void ReLogin(FString url);

	UPROPERTY(BlueprintReadWrite)
		FString WorldName = "";

	UPROPERTY()
		FLatentActionInfo latentinfo;
	FActorSpawnParameters spawnbeacon;

	UPROPERTY()
		bool bTraveling = false;

	FTimerHandle loadtimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		AMyOnlineBeaconClient* BC = NULL;

	AMyPlayerController* MyPC = NULL;
	UPROPERTY()
		AMyPlayerState* MyPS = NULL;

	void SetUserdb(FUsersData db);
	
	FTimerHandle openLevelHandle;
private:
	
};
