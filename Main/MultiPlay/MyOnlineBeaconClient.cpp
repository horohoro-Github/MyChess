// Fill out your copyright notice in the Description pge of Project Settings.


#include "MyOnlineBeaconClient.h"
#include "../Player/MyPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "../Game/MyChess_GameModeBase.h"
#include "JsonObjectConverter.h"
#include "../Game/MyGameInstance.h"
#include "MyOnlineBeaconHostObject.h"
#include "Net/UnrealNetwork.h"
#include "Engine/LevelStreaming.h"
#include "../Widget/MyUserWidget.h"
#include "Engine/NetConnection.h"

AMyOnlineBeaconClient::AMyOnlineBeaconClient()
{
	bReplicates = true;
}

void AMyOnlineBeaconClient::GetFriendsComplete_Implementation(FFriendsData friendsdb, const TArray<FUsersData>& username)
{
	gameins->friendsdb = friendsdb;
	gameins->FindUserName = username;
	if (LevelName == "LobbyLevel")
	{

		AMyPlayerController* PC = Cast<AMyPlayerController>(gameins->MyPC);
		if (PC)
		{
			UMyUserWidget* widget = Cast<UMyUserWidget>(PC->CurrentWidget);
			if (widget)
			{
				widget->SetFriendsList();
			}
		}
	}
}

bool AMyOnlineBeaconClient::ConnectToServer(const FString& address)
{
	gameins = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
	FURL Destination = FURL(nullptr, *address, ETravelType::TRAVEL_Absolute);
	Destination.Port = 7787;
	return InitClient(Destination);
	
}

void AMyOnlineBeaconClient::GetFriends_Implementation()
{
	AMyChess_GameModeBase* gamemode = Cast<AMyChess_GameModeBase>(GetWorld()->GetAuthGameMode());
	gamemode->GetFriends(this);
}

void AMyOnlineBeaconClient::AddFriends_Implementation(const FString& name, const FString& Type)
{
	AMyChess_GameModeBase* gamemode = Cast<AMyChess_GameModeBase>(GetWorld()->GetAuthGameMode());
	gamemode->AddFriend(this, name, Type);
}

void AMyOnlineBeaconClient::TakeOnChessWithFriend_Implementation(FUsersData user, const FString& Type)
{
	AMyChess_GameModeBase* gamemode = Cast<AMyChess_GameModeBase>(GetWorld()->GetAuthGameMode());
	gamemode->InviteTheGame(this, user, Type);
}


void AMyOnlineBeaconClient::Invite_Complete_Implementation(FUsersData user, const TArray<FUsersData>& username, FUsersData otherUser, bool alarm, bool Send)
{
	if (gameins->MyPC)
	{
		gameins->FindUserName = username;
		UMyUserWidget* widget = Cast<UMyUserWidget>(gameins->MyPC->SlideWidget);
		if (widget)
		{
			widget->SetMessage(user, otherUser, alarm, Send);
		}
	}
}

void AMyOnlineBeaconClient::OnFailure()
{
	UE_LOG(LogTemp, Warning, TEXT("Connected Fail"));
	DestroyBeacon();
}

void AMyOnlineBeaconClient::OnConnected()
{

	UE_LOG(LogTemp, Warning, TEXT("Connected Success"));
	if (IsRunningClientOnly())
	{
	    LevelName =	UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
		SetLevelName(LevelName);
		if (gameins->token != 0)
		{
			LoginWithToken(gameins->token);
		}
		else
		{
			LoginAttempt(gameins->userdb.userid, gameins->userdb.userpass, true, true);
		}

		GetWorld()->GetTimerManager().SetTimer<AMyOnlineBeaconClient>(h_GetFriends, this, &AMyOnlineBeaconClient::GetFriends, 3.f, true, 1.f);

	}

	
}

EBeaconConnectionState AMyOnlineBeaconClient::GetConnectionState()
{
	return ConnectionState;
}

void AMyOnlineBeaconClient::RenamePlayer_Implementation(const FString& id, const FString& name)
{
	AMyChess_GameModeBase* gamemode = Cast<AMyChess_GameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	
	if (gamemode) gamemode->RenamePlayer(this, id, name);
}

void AMyOnlineBeaconClient::StopInvite_Implementation(bool LoadLevel)
{
	if (gameins->MyPC)
	{
		UMyUserWidget* widget = Cast<UMyUserWidget>(gameins->MyPC->SlideWidget);
		if (widget)
		{
			if (LoadLevel) widget->AddChildWidget();
			else widget->RemoveChildWidget();
		}
	}
}

void AMyOnlineBeaconClient::RenamePlayerComplete_Implementation(const FString& name, bool Succeded)
{
	if (gameins)
	{
		if (gameins->MyPC)
		{
			UMyUserWidget* widget = Cast<UMyUserWidget>(gameins->MyPC->CurrentWidget);
			if (Succeded)
			{
				gameins->userdb.player_name = name;
				widget->SetActiveSwitch(EMenuWidgetIndex::WorkedSuccessfully);
			}
			else
			{
				PrintSystemMessage(TEXT("이름 변경 실패"));
			}
			widget->DisableAllWidgets = false;
		}
	}
}

void AMyOnlineBeaconClient::AccountRegistration_Implementation(const FString& id, const FString& pass)
{
	AMyChess_GameModeBase* gamemode = Cast<AMyChess_GameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (gamemode)
	{
		gamemode->AddUserData(this, id, pass);
	}
}



void AMyOnlineBeaconClient::LoginAttempt_Implementation(const FString& id, const FString& pass, bool login, bool leveltravel)
{
	AMyChess_GameModeBase* gamemode = Cast<AMyChess_GameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (gamemode)
	{
		gamemode->UserConnection(this, id, pass, login, leveltravel);
	}
}

void AMyOnlineBeaconClient::UpdateData_Implementation(FUsersData userdb)
{
	if (gameins)
	{
		gameins->userdb = userdb;
		gameins->SetUserdb(userdb);
	}
}

void AMyOnlineBeaconClient::LoginComplete_Implementation(bool Succeded, bool leveltravel)
{
	FString levelName = UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
	if (levelName == FString("LobbyLevel"))
	{
		AMyPlayerController* PC = Cast<AMyPlayerController>(gameins->MyPC);
		if (PC)
		{
			UMyUserWidget* widget = Cast<UMyUserWidget>(PC->CurrentWidget);
			if (widget)
			{
				if (Succeded)
				{
					if (gameins->userdb.userid != "" && gameins->userdb.userpass != "")
					{
						if (gameins->userdb.player_name == "")
						{
							widget->SetActiveSwitch(EMenuWidgetIndex::Rename);
						}
						else
						{
							widget->SetActiveSwitch(EMenuWidgetIndex::PlayerData);
						}
					}
					else
					{
						widget->SetActiveSwitch(EMenuWidgetIndex::Login);
					}
				}
				else
				{
					if (leveltravel == false) PrintSystemMessage(TEXT("로그인 실패"));
					else widget->SetActiveSwitch(EMenuWidgetIndex::Login);
				}

				widget->DisableAllWidgets = false;
			}
		}
	}
}

void AMyOnlineBeaconClient::AddUserComplete_Implementation(bool Succeded, const FString& id)
{
	if (gameins)
	{
		AMyPlayerController* PC = Cast<AMyPlayerController>(gameins->MyPC);
		if (PC)
		{
			UMyUserWidget* widget = Cast<UMyUserWidget>(PC->CurrentWidget);
			if (widget)
			{
				if (Succeded)
				{
					widget->SetActiveSwitch(EMenuWidgetIndex::WorkedSuccessfully);
				}
				else
				{
					PrintSystemMessage(TEXT("이미 존재하는 ID"));
				}
				widget->DisableAllWidgets = false;
			}
		}
	}
}

void AMyOnlineBeaconClient::MatchMaking_Implementation(bool Cancel)
{
	AMyChess_GameModeBase* gamemode = Cast<AMyChess_GameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (gamemode)
	{
		if (userdata.userid != "")
		{
			if (Cancel)
			{
				gamemode->CancelMatching(this);
			}
			else
			{
				gamemode->MatchingTask(this);
			}
		}
	}
}

void AMyOnlineBeaconClient::MatchComplete_Implementation(uint64 t, bool Succeded, const FString& url, const FString& optionString)
{
	if (Succeded)
	{
		gameins->token = t;
		
		FTimerDelegate matchDelegate = FTimerDelegate::CreateLambda([&, url, optionString]()
			{
				gameins->OpenLevel(url, optionString);
			});
		
		GetWorld()->GetTimerManager().SetTimer(gameins->openLevelHandle, matchDelegate, 3.f, false);
	}
}

void AMyOnlineBeaconClient::LoginWithToken_Implementation(uint64 t)
{
	AMyChess_GameModeBase* gamemode = Cast<AMyChess_GameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (gamemode) gamemode->LoginWithToken(this, t);
}

void AMyOnlineBeaconClient::Relogin_Implementation(const FString& url, FUsersData userdb)
{
	AMyChess_GameModeBase* gamemode = Cast<AMyChess_GameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (gamemode)
	{
		uint64 returnNumber = 0;

		returnNumber = gamemode->randomStream.FRandRange(0, (double)_I64_MAX);

		FDateTime time = FDateTime::Now();
		int32 t = 0;
		int32 now = time.GetSecond();
		if (now + 30 > 59) t = now + 30 - 59;
		else t = now + 30;

		token = returnNumber;
		FVerifyToken* verfy = new FVerifyToken(userdb, t);
		gamemode->verifyToken.Add(returnNumber, verfy);
		Relogin_OpenLevel(url, returnNumber);
	}
}

void AMyOnlineBeaconClient::Relogin_OpenLevel_Implementation(const FString& url, uint64 tkn)
{
	if (gameins)
	{
		gameins->token = tkn;
		gameins->OpenLevel(url, "");

	}
}

void AMyOnlineBeaconClient::SetLevelName_Implementation(const FString& name)
{
	LevelName = name;
}

FString AMyOnlineBeaconClient::GetLevelName()
{
	return UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
}

void AMyOnlineBeaconClient::PrintSystemMessage_Implementation(const FString& message)
{
	UMyUserWidget* widget = gameins->MyPC->CurrentWidget;

	if(widget) widget->SystemMessage(message);
}
