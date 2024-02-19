// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "../Player/MyPlayerController.h"
#include "../Player/MyPlayerState.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameMapsSettings.h"
#include "Blueprint/UserWidget.h"
#include "../Etc/MyTransparencyActor.h"
#include "../MultiPlay/MyOnlineBeaconClient.h"
#include "Engine/LevelStreaming.h"
#include "MoviePlayer/Public/MoviePlayer.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SThrobber.h"
#include "GameFramework/OnlineReplStructs.h"
#include "Kismet/KismetStringLibrary.h"
#include "../Widget/MyUserWidget.h"

UMyGameInstance::UMyGameInstance()
{
	static ConstructorHelpers::FClassFinder<AActor> SpotLightClass(TEXT("Blueprint'/Game/Blueprints/SpotLight.SpotLight_C'"));
	if (SpotLightClass.Succeeded()) LightClass = SpotLightClass.Class;

}

void UMyGameInstance::Init()
{
	Super::Init();

	spawnbeacon.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UMyGameInstance::BeginLoadingScreen);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UMyGameInstance::EndLoadingScreen);
}

void UMyGameInstance::BeginLoadingScreen(const FString& MapName)
{
	bTraveling = false;
}

void UMyGameInstance::EndLoadingScreen(UWorld* InLoadedWorld)
{
	FString levelName = UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
	if (!IsRunningDedicatedServer())
	{
		int port = GetWorld()->URL.Port;
		FString portstr = FString::FromInt(port);

		if (UGameplayStatics::GetCurrentLevelName(GetWorld(), true) == FString("OffLineMap") || UGameplayStatics::GetCurrentLevelName(GetWorld(), true) == FString("MainMap") || UGameplayStatics::GetCurrentLevelName(GetWorld(), true) == FString("VSAIMap"))
		{
			FActorSpawnParameters sp;
			wall = GetWorld()->SpawnActor<AMyTransparencyActor>(AMyTransparencyActor::StaticClass(), FVector(-350.f, 350.f, 720.f), FRotator(0.f, 0.f, 0.f), sp);

			if (UGameplayStatics::GetPlatformName() == "Android") AActor* spotlight = GetWorld()->SpawnActor<AActor>(LightClass, FVector(-350.f, 350.f, 480.f), FRotator(0.f, 0.f, 0.f), sp);
		}
		if (levelName != FString("LoadingMap") && levelName != FString("MainMap"))
		{
			if (BC == NULL)
			{
				BC = GetWorld()->SpawnActor<AMyOnlineBeaconClient>(AMyOnlineBeaconClient::StaticClass(), FTransform(FRotator(0.f, 0.f, 0.f), FVector(0.f, 0.f, 0.f)), spawnbeacon);

				if (BC)
				{
					BC->gameins = this;
					BC->LevelName = UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
					BC->ConnectToServer(SERVER_IPADDR);
				}
			}
		}
		MyPC = Cast<AMyPlayerController>(GetPrimaryPlayerController(true));
	}
}

void UMyGameInstance::RegisterID(FString id, FString password)
{
	if (MyPC)
	{
		UMyUserWidget* widget = Cast<UMyUserWidget>(MyPC->CurrentWidget);

		widget->DisableAllWidgets = true;
		if (widget)
		{
			if (BC)
			{
				if (BC->GetConnectionState() == EBeaconConnectionState::Open)
				{
					BC->AccountRegistration(id, password);
				}
				else widget->DisableAllWidgets = false;
			}
			else widget->DisableAllWidgets = false;
			
		}
	}
}

void UMyGameInstance::LoginAttempt(FString id, FString pass, bool login)
{
	if (MyPC)
	{
		UMyUserWidget* widget = Cast<UMyUserWidget>(MyPC->CurrentWidget);

		widget->DisableAllWidgets = true;
		if (widget)
		{
			if (BC)
			{
				if (BC->GetConnectionState() == EBeaconConnectionState::Open)
				{
					BC->LoginAttempt(id, pass, login, false);
				}
				else widget->DisableAllWidgets = false;
			}
			else widget->DisableAllWidgets = false;
		}
	}
}

void UMyGameInstance::PlayerRename(FString name)
{
	if (MyPC)
	{
		UMyUserWidget* widget = Cast<UMyUserWidget>(MyPC->CurrentWidget);

		widget->DisableAllWidgets = true;
		if (widget)
		{
			if (BC)
			{
				if (BC->GetConnectionState() == EBeaconConnectionState::Open)
				{
					BC->RenamePlayer(userdb.userid, name);
				}
				else widget->DisableAllWidgets = false;
			}
			else widget->DisableAllWidgets = false;
		}
	}
}

void UMyGameInstance::AddToFriends(FString name, FString Type)
{
	if (BC->GetConnectionState() == EBeaconConnectionState::Open)
	{
		BC->AddFriends(name, Type);
	}
}

void UMyGameInstance::TakeOnChessWithFriend(FUsersData user, FString Type)
{
	if (BC->GetConnectionState() == EBeaconConnectionState::Open)
	{
		BC->TakeOnChessWithFriend(user, Type);
	}
}

void UMyGameInstance::CreateLoadingWidget()
{
	if (LoadingWidget == NULL)
	{
		LoadingWidget = CreateWidget<UUserWidget>(GetWorld(), LoadingWidgetClass);
		LoadingWidget->AddToViewport();
		//	LoadSteam
	}
}

void UMyGameInstance::ReConnectToServer()
{
	FString levelName = UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
	if (levelName != FString("LoadingMap") && levelName != FString("MainMap"))
	{
		if (BC)
		{
			BC->ConnectToServer(SERVER_IPADDR);
		}
	}
}

void UMyGameInstance::OpenLevel(FString url, FString Optionstr)
{
	if (MyPC)
	{
		MyPC->FadeScreen(2);
	}
	else
	{
		MyPC = Cast<AMyPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		if (MyPC)
		{
			MyPC->FadeScreen(2);
		}
	}
	
	GetWorld()->GetTimerManager().ClearTimer(openLevelHandle);

	GetWorld()->GetTimerManager().SetTimer(openLevelHandle, FTimerDelegate::CreateLambda([this,url,Optionstr]()
		{
			
			UGameplayStatics::UnloadStreamLevel(GetWorld(), FName("LoadingMap"), latentinfo, false);
			MyPC = NULL;
			MyPS = NULL;
			WorldName = "";
			if (BC != NULL)
			{
				UGameplayStatics::OpenLevel(this, FName(url), true, Optionstr);
			}
			else
			{
				UGameplayStatics::OpenLevel(this, FName(url), true);
			}
		}), 1.f, false);
}

void UMyGameInstance::FindMatching(bool Cancel)
{
	if (BC != NULL)
	{
		if (BC->GetConnectionState() == EBeaconConnectionState::Open)
		{
			BC->MatchMaking(Cancel);
		}
	}
}

void UMyGameInstance::ReLogin(FString url)
{
	if (userdb.userid == "" && userdb.userpass == "")
	{
		OpenLevel(url, "");
	}
	else
	{
		if (bTraveling == false)
		{
			
			if (BC != NULL)
			{
				if (BC->GetConnectionState() == EBeaconConnectionState::Open)
				{
					GetWorld()->GetTimerManager().ClearTimer(openLevelHandle);
					bTraveling = true;
					BC->Relogin(url, userdb);
				}
				if (BC->GetConnectionState() == EBeaconConnectionState::Closed)
				{
					userdb = FUsersData();
					bTraveling = true;
					OpenLevel(url, "");
				}
			}
		}
	}
}

void UMyGameInstance::SetUserdb(FUsersData db)
{

	MyPS = Cast<AMyPlayerState>(GetPrimaryPlayerController(true)->PlayerState);
	
	if (MyPS)
	{
		MyPS->SetUserDB(db);
	}
}