// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyUserWidget.generated.h"

/**
 * 
 */
enum EMenuWidgetIndex
{
	Loading,
	Login,
	Register,
	PlayerData,
	Rename,
	WorkedSuccessfully
};

UCLASS(Abstract)
class MYCHESS__API UMyUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadWrite)
		class AMyPlayerController* PC = NULL;

	UPROPERTY(BlueprintReadWrite)
		class AMyPlayerState* PS = NULL;
	UPROPERTY(BlueprintReadWrite)
		TArray<class AMyPlayerState*> OtherPSArray;

	UPROPERTY(BlueprintReadWrite)
		class AMyGameStateBase* GameState = NULL;

	UPROPERTY(BlueprintReadWrite)
		class UMyGameInstance* gameIns = NULL;

	UPROPERTY(BlueprintReadWrite)
		FString WidgetMessage = "";

	UPROPERTY(BlueprintReadWrite)
		bool DisableAllWidgets = false;

	UFUNCTION(BlueprintImplementableEvent)
		void SetActiveSwitch(int index);

	UFUNCTION(BlueprintImplementableEvent)
		void SetFriendsList();

	UFUNCTION(BlueprintImplementableEvent)
		void SetMessage(FUsersData db, FUsersData otherUser, bool alarm, bool Send);

	UFUNCTION(BlueprintImplementableEvent)
		void SystemMessage(const FString& message);

	UFUNCTION(BlueprintImplementableEvent)
		void RemoveChildWidget();

	UFUNCTION(BlueprintImplementableEvent)
		void AddChildWidget();

	UFUNCTION(BlueprintImplementableEvent)
		void UpdateHistory(int causeGiver);

	UFUNCTION(BlueprintCallable)
		TArray<FUsersData> ExtractMessages(FUsersData db);

	UFUNCTION(BlueprintCallable)
		bool EqualFriendsList(FUsersData user1, FUsersData user2);

	UFUNCTION(BlueprintCallable)
		FUserName SetFriends();

	UFUNCTION(BlueprintCallable)
		void Promotion(int index);

	UFUNCTION(BlueprintCallable)
		TArray<FString> GetHistory();

	UFUNCTION(BlueprintCallable)
		TArray<FAlgebraicNotation> GetNotation(int causeGiver);

	UFUNCTION(BlueprintCallable)
		void ResetThisGame();
};
