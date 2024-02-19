// Copyright Epic Games, Inc. All Rights Reserved.


#include "MyChess_GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "../Chess/MyTile.h"
#include "../Chess/MyChessPiece.h"
#include "../Etc/MyReferee.h"
#include "../Etc/MyObserver.h"
#include "MyGameInstance.h"
#include "../Player/MyPlayerState.h"
#include "../Player/MyCharacter.h"
#include "JsonObjectConverter.h"
#include "GameFramework/GameSession.h"
#include "Engine/NetDriver.h"
#include "../MultiPlay/MyOnlineBeaconClient.h"
#include "OnlineBeaconHost.h"
#include "../MultiPlay/MyOnlineBeaconHostObject.h"
#include "MyGameStateBase.h"
#include "Containers/UnrealString.h"
#include "Kismet/KismetStringLibrary.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Components/WidgetComponent.h"
#include "../Widget/MyUserWidget.h"
#include "Net/UnrealNetwork.h"
#include "../Chess/MyBoard.h"

class ULevelStreaming* mainstreaming = NULL;

AMyChess_GameModeBase::AMyChess_GameModeBase(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<APawn> character(TEXT("Blueprint'/Game/Blueprints/BP_MyCharacter.BP_MyCharacter_C'"));
	if(character.Succeeded() && character.Class != NULL) CharacterClass = character.Class;
	static ConstructorHelpers::FClassFinder<APawn> observer(TEXT("Blueprint'/Game/Blueprints/BP_MyObserver.BP_MyObserver_C'"));
	if(observer.Succeeded() && observer.Class != NULL) ObserverClass = observer.Class;

	PlayerControllerClass = AMyPlayerController::StaticClass();
	PlayerStateClass = AMyPlayerState::StaticClass();
	GameStateClass = AMyGameStateBase::StaticClass();
	DefaultPawnClass = NULL;
	Http = &FHttpModule::Get();

	boardClass = AMyBoard::StaticClass();

	static ConstructorHelpers::FClassFinder<APawn> referee(TEXT("Blueprint'/Game/Blueprints/BP_MyReferee.BP_MyReferee_C'"));
	if (referee.Succeeded()) RefereeClass = referee.Class;
}

void AMyChess_GameModeBase::BeginPlay()
{	
	Super::BeginPlay();

	int p = GetWorld()->URL.Port;
	
	serverdata.serverip = SERVER_IPADDR;
	serverdata.serverport = p;
	
	FString p_str = FString::FromInt(p);
	if (IsRunningDedicatedServer())
	{
		if (p_str == "7777")
		{
			serverdata.servertype = "MainServer";
			FTimerHandle MatchHandle;
			GetWorld()->GetTimerManager().SetTimer<AMyChess_GameModeBase>(MatchHandle, this, &AMyChess_GameModeBase::MatchManagement, 1.f, true, 1.f);

			FTimerHandle TravelerHandle;
			GetWorld()->GetTimerManager().SetTimer<AMyChess_GameModeBase>(TravelerHandle, this, &AMyChess_GameModeBase::CheckServerTraveler, 0.5f, true, 1.f);
			CreateHostBeacon();

			FTimerHandle GetUserHandle;
			GetWorld()->GetTimerManager().SetTimer<AMyChess_GameModeBase>(GetUserHandle, this, &AMyChess_GameModeBase::GetAllUserData, 1.f, true, 3.f);
			
		}
		else
		{
			serverdata.servertype = "GameServer";
		}
	}
}

void AMyChess_GameModeBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if(IsRunningDedicatedServer()) ServerActivate(false);
}

bool AMyChess_GameModeBase::CreateHostBeacon()
{
	if (AOnlineBeaconHost* Host = GetWorld()->SpawnActor<AOnlineBeaconHost>(AOnlineBeaconHost::StaticClass()))
	{
		if (Host->InitHost())
		{
			Host->PauseBeaconRequests(false);
			HostObject = GetWorld()->SpawnActor<AMyOnlineBeaconHostObject>(AMyOnlineBeaconHostObject::StaticClass());
			if (HostObject)
			{
				Host->RegisterHost(HostObject);
				return true;
			}
		}
	}
	return false;
}

FString AMyChess_GameModeBase::BoardPositionStrings()
{
	FString path = FPaths::ProjectContentDir() + TEXT("PositionNumbers.txt");

	FString NumberString = "";
	FString PositionString = "RNBQKBNRPPPPPPPP////pppppppprnbqkbnr";
	if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*path))
	{
		FFileHelper::LoadFileToString(NumberString, *path, FFileHelper::EHashOptions::EnableVerify);
	}
	else
	{
		for (int i = 0; i < PositionString.Len(); i++)
		{
			unsigned long long position = 0;
			
			int firstaction = 1;
			int location = 0;
			int type = 0;
			int color = 0;

			if (toupper(PositionString[i]) == 'P') type = 1;
			if (toupper(PositionString[i]) == 'N') type = 2;
			if (toupper(PositionString[i]) == 'B') type = 3;
			if (toupper(PositionString[i]) == 'R') type = 4;
			if (toupper(PositionString[i]) == 'Q') type = 5;
			if (toupper(PositionString[i]) == 'K') type = 6;

			if (i < 16) location = i;
			else location = i + 32;

			if (PositionString[i] != '/')
			{
				color = isupper(PositionString[i]) ? 0 : 1;

				//position = location;
				position |= (unsigned long long) type;
				position |= (unsigned long long) color << 3;

				NumberString += FString::Printf(TEXT("%llu"), position);

				if (i != PositionString.Len() - 1) NumberString += ',';
			}
			else
			{
				NumberString += "////////";
			}
			
		}
		FFileHelper::SaveStringToFile(NumberString, *path);
	}
	return NumberString;
}

FString AMyChess_GameModeBase::ZobristHash()
{
	FString path = FPaths::ProjectContentDir() + TEXT("Zobrist.txt");

	FString RandomString = "";
	FString PositionString = "RNBQKBNRPPPPPPPPpppppppprnbqkbnr";
	if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*path))
	{
		FFileHelper::LoadFileToString(RandomString, *path, FFileHelper::EHashOptions::EnableVerify);
	}
	else
	{
		int numString = 64 * 8 * 2 + 9 + 16 + 1;//스퀘어 * 기물 타입 * 색상 + 앙파상 + 캐슬링
		
		srand(2361912);
		for (int i = 0; i < numString; i++)
		{
			RandomString += FString::Printf(TEXT("%llu"), RandomNumber(i * 8));
			if(i != numString - 1) RandomString += ',';
		}
		
		FFileHelper::SaveStringToFile(RandomString, *path);
	}

	return RandomString;
}
uint64 AMyChess_GameModeBase::RandomNumber(int seed)
{
	uint64 returnNumber =0;

	for (int i = 0; i < 8; i++)
	{
		int randNum = rand() % 255;
		returnNumber |= (uint64)randNum << 8 * i;
	}
//	FRandomStream test(25000 + seed);
	//int8 buffer[8];
//	uint64 rand = test.FRandRange((double)MIN_uint64, (double)MAX_uint64);
	//uint64 rand = FMath::FRandRange(MIN_uint64, MAX_uint64);
	return returnNumber;
}

void AMyChess_GameModeBase::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
}

void AMyChess_GameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AMyPlayerController* PC = Cast<AMyPlayerController>(NewPlayer);

	if (PC)
	{
		AMyPlayerState* playerState = Cast<AMyPlayerState>(PC->PlayerState);

		if (playerState) playerState->playerState = EPlayerState::NONE_PLAYERSTATE;
		PC->FadeScreen(0);
		PC->PlayerControllerSetup(false, true);
		PC->AddWidget(EWidgetState::MainMenu, true);
	}
}

void AMyChess_GameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
}

void AMyChess_GameModeBase::QuitServer()
{

	FGenericPlatformMisc::RequestExit(false);
}

void AMyChess_GameModeBase::ServerActivate(bool Activate)
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
	servd.serverip = serverdata.serverip;
	servd.serverport = serverdata.serverport;
	servd.servertype = serverdata.servertype;

	FJsonObjectConverter::UStructToJsonObjectString(servd, JsonString);
	Requests->SetContentAsString(JsonString);
	Requests->ProcessRequest();
}

void AMyChess_GameModeBase::RenamePlayer(AMyOnlineBeaconClient* BC, const FString& id, const FString& name)
{
	if (IsRunningDedicatedServer())
	{
		TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
		FHttpRequestCompleteDelegate& Delegate = Request->OnProcessRequestComplete();


		Request->SetURL("http://127.0.0.1:8080/Player/Rename");
		Request->SetVerb("Patch");
		Request->SetHeader(TEXT("Content-type"), TEXT("application/json"));


		Delegate.BindLambda([BC, id, name](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool Succeeded)
			{
				TArray<FUsersData> userdb;
				if (!HttpResponse->GetContentAsString().Contains("timestamp")) FJsonObjectConverter::JsonArrayStringToUStruct(HttpResponse->GetContentAsString(), &userdb, 0, 0);
				if (userdb.Num() > 0)
				{
					BC->userdata.login = userdb[0].login;
					BC->RenamePlayerComplete(name, true);
				}
				else
				{
					BC->RenamePlayerComplete(name, false);
				}
			});

		FString JsonString;
		FUsersData userD;

		userD.userid = id;
		userD.player_name = name;

		FJsonObjectConverter::UStructToJsonObjectString(userD, JsonString);
		Request->SetContentAsString(JsonString);
		Request->ProcessRequest();
	}
}

void AMyChess_GameModeBase::AddUserData(AMyOnlineBeaconClient* BC, FString id, const FString& pass)
{
	if (IsRunningDedicatedServer())
	{
		TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
		FHttpRequestCompleteDelegate& Delegate = Request->OnProcessRequestComplete();

		Request->SetURL("http://127.0.0.1:8080/Player/AddUser");
		Request->SetVerb("Post");
		Request->SetHeader(TEXT("Content-type"), TEXT("application/json"));
		FUsersData userdb;

		userdb.userid = id;
		userdb.userpass = pass;
		FString JsonString;
		
		Delegate.BindLambda([BC, id, pass](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool Succeeded)
			{
				TArray<FUsersData> userdb;
				if (!HttpResponse->GetContentAsString().Contains("timestamp")) FJsonObjectConverter::JsonArrayStringToUStruct(HttpResponse->GetContentAsString(), &userdb, 0, 0);
				if (userdb.Num() > 0)
				{
					BC->AddUserComplete(true, id);
				}
				else
				{
					BC->AddUserComplete(false, id);
				}
			});
		
		FJsonObjectConverter::UStructToJsonObjectString(userdb, JsonString);
		Request->SetContentAsString(JsonString);
		Request->ProcessRequest();
	}
}

void AMyChess_GameModeBase::UserConnection(AMyOnlineBeaconClient* BC, const FString& id, const FString& pass, bool login, bool leveltravel)
{
	if (IsRunningDedicatedServer())
	{
		TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
		FHttpRequestCompleteDelegate& Delegate = Request->OnProcessRequestComplete();


		Request->SetURL("http://127.0.0.1:8080/Player/UserConnection");
		Request->SetVerb("Patch");
		Request->SetHeader(TEXT("Content-type"), TEXT("application/json"));

		Delegate.BindLambda([BC, id, pass, leveltravel, this](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool Succeeded)
			{ 
				if (BC)
				{
					FUsersData defaultdata;
					TArray<FUsersData> userdb;
					if (!HttpResponse->GetContentAsString().Contains("timestamp")) FJsonObjectConverter::JsonArrayStringToUStruct(HttpResponse->GetContentAsString(), &userdb, 0, 0);
					if (userdb.Num() > 0)
					{
						if (userdb[0].login == true)
						{
							BC->userdata = userdb[0];
							BC->UpdateData(userdb[0]);
							BC->LoginComplete(true, leveltravel);
							GetFriends(BC);
						}
						else
						{
							if (BC->userdata.userid != "") InviteTheGame(BC, BC->userdata, "Remove");
							BC->userdata = defaultdata;
							BC->UpdateData(defaultdata);
							BC->LoginComplete(true, leveltravel);
						}
					}
					else
					{
						BC->LoginComplete(false, leveltravel);
					}
				}
			});

		FString JsonString;
		FUsersData userD;

		userD.userid = id;
		userD.userpass = pass;
		userD.login = login;

		FJsonObjectConverter::UStructToJsonObjectString(userD, JsonString);
		Request->SetContentAsString(JsonString);
		Request->ProcessRequest();
	}
}

void AMyChess_GameModeBase::MatchingTask(AMyOnlineBeaconClient* BC)
{
	if (IsRunningDedicatedServer())
	{
		MatchPlayers.Add(BC);
	}
}

void AMyChess_GameModeBase::CancelMatching(AMyOnlineBeaconClient* BC)
{
	if (IsRunningDedicatedServer())
	{
		MatchPlayers.Remove(BC);
		BC->PrintSystemMessage(TEXT("매치를 취소함"));
	}
}

void AMyChess_GameModeBase::MatchManagement()
{
	if (IsRunningDedicatedServer())
	{
		if (MatchPlayers.Num() >= 2)
		{
			AMyOnlineBeaconClient* Player1 = NULL;
		
			Player1 = MatchPlayers[0];
			if (Player1) Player1->PrintSystemMessage(TEXT("매치를 찾는 중"));
			AMyOnlineBeaconClient* Player2 = NULL;
			Player2 = MatchPlayers[1];
			if (Player2) Player2->PrintSystemMessage(TEXT("매치를 찾는 중"));
			MatchPlayers.Remove(Player1);
			MatchPlayers.Remove(Player2);

			if (Player1 && Player2)
			{
				CreateGameServer(Player1, Player2);
			}
		}
	}
}

TArray<FString> AMyChess_GameModeBase::GetSplitStringArray(FString LongString)
{
	TArray<FString> strarray;
	while (true)
	{
		FString substr = ",";
		FString temp;
		FString temp2;

		if (UKismetStringLibrary::Split(LongString, substr, temp, temp2, ESearchCase::IgnoreCase, ESearchDir::FromStart) == true)
		{
			strarray.Add(temp);
			LongString = temp2;
		}
		else
		{
			strarray.Add(LongString);
			break;
		}
	}
	return strarray;
}

void AMyChess_GameModeBase::CreateGameServer(AMyOnlineBeaconClient* BC1, AMyOnlineBeaconClient* BC2)
{
	if (IsRunningDedicatedServer())
	{
		if (BC1 && BC2)
		{
			FDateTime time = FDateTime::Now();
			int32 t = 0;
			int32 now = time.GetSecond();
			if (now + 30 > 59) t = now + 30 - 59;
			else t = now + 30;
			
			uint64 returnNumber = 0;
			uint64 returnNumber2 = 0;

			returnNumber = randomStream.FRandRange(0, (double)_I64_MAX);
			returnNumber2 = randomStream.FRandRange(0, (double)_I64_MAX);
			FVerifyToken* p1 = new FVerifyToken(BC1->userdata, t);
			FVerifyToken* p2 = new FVerifyToken(BC2->userdata, t);
			verifyToken.Add(returnNumber, p1);
			verifyToken.Add(returnNumber2, p2);
			BC1->token = returnNumber;
			BC2->token = returnNumber2;
			FString id1 = BC1->userdata.userid;
			FString id2 = BC2->userdata.userid;
			
			TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
			FHttpRequestCompleteDelegate& Delegate = Request->OnProcessRequestComplete();
			

			Request->SetURL("http://127.0.0.1:8080/Server/GameServer");
			Request->SetVerb("Post");
			Request->SetHeader(TEXT("Content-type"), TEXT("application/json"));

			Delegate.BindLambda([&, BC1, BC2, returnNumber, returnNumber2, id1, id2](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool Succeeded)
				{
					TArray<FServerData> serverdb;
					if (!HttpResponse->GetContentAsString().Contains("timestamp")) FJsonObjectConverter::JsonArrayStringToUStruct(HttpResponse->GetContentAsString(), &serverdb, 0, 0);
					if (serverdb.Num() > 0)
					{
						FString dir = TEXT("C:\\MyChess_Packaged\\develp\\GameServer\\WindowsServer\\MyChess_Server.exe");
						const TCHAR* dir_char = *dir;
						FString port = TEXT("-port=" + FString::FromInt(serverdb[0].serverport) + " -token1=" + FString::Printf(TEXT("%llu"), returnNumber) + " -token2=" + FString::Printf(TEXT("%llu"), returnNumber2) + " -id1=" + id1 + " -id2=" + id2);
						const TCHAR* port_char = *port;
						FPlatformProcess::CreateProc(dir_char, port_char, true, false, false, nullptr, 0, nullptr, nullptr);

						FString url = TEXT(SERVER_IPADDR + FString(":") + FString::FromInt(serverdb[0].serverport));
						FString option1 = FString("?token=" + FString::Printf(TEXT("%llu"), returnNumber) + "?id=" + id1);
						FString option2 = FString("?token=" + FString::Printf(TEXT("%llu"), returnNumber2) + "?id=" + id2);

					    if(BC1)	BC1->MatchComplete(returnNumber, true, url, option1);
						if(BC2) BC2->MatchComplete(returnNumber2, true, url, option2);
					}
				});

			FString JsonString;
			FServerData serverdb;

			serverdb.serverip = SERVER_IPADDR;
			serverdb.servertype = "GameServer";

			FJsonObjectConverter::UStructToJsonObjectString(serverdb, JsonString);
			Request->SetContentAsString(JsonString);
			Request->ProcessRequest();
		}
	}
}

void AMyChess_GameModeBase::LoginWithToken(AMyOnlineBeaconClient* BC, uint64 t)
{
	if (verifyToken.Contains(t))
	{
		FUsersData userdb;
		userdb = verifyToken[t]->userdata;

		TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
		FHttpRequestCompleteDelegate& Delegate = Request->OnProcessRequestComplete();


		Request->SetURL("http://127.0.0.1:8080/Player/Check");
		Request->SetVerb("Post");
		Request->SetHeader(TEXT("Content-type"), TEXT("application/json"));

		Delegate.BindLambda([&, BC, userdb](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool Succeeded)
			{
				FUsersData defaultData = FUsersData();
				TArray<FUsersData> userdata;
				if (!HttpResponse->GetContentAsString().Contains("timestamp")) FJsonObjectConverter::JsonArrayStringToUStruct(HttpResponse->GetContentAsString(), &userdata, 0, 0);
				if (userdata.Num() > 0)
				{
					if (userdata[0].login == true)
					{
						if (BC)
						{
							BC->userdata = userdata[0];
							BC->UpdateData(userdata[0]);
							BC->LoginComplete(true, true);
							GetFriends(BC);
							if (userdata[0].message_box != "") BC->Invite_Complete(userdata[0], AllUsers, defaultData, true, false);
						}
					}
					else
					{		
						if (BC)
						{
							BC->userdata = defaultData;
							BC->UpdateData(defaultData);
							BC->LoginComplete(false, true);
						}
					}
				}
			});

		FString JsonString;

		verifyToken.Remove(t);
		
		FJsonObjectConverter::UStructToJsonObjectString(userdb, JsonString);
		Request->SetContentAsString(JsonString);
		Request->ProcessRequest();
	}
}

void AMyChess_GameModeBase::CheckServerTraveler()
{
	FDateTime datetime = FDateTime::Now();
	int32 aa = datetime.GetSecond();

	verifyToken.GenerateValueArray(TokenArray);

	for (int i = TokenArray.Num() - 1; i >= 0; i--)
	{
		if (TokenArray[i]->Time == aa)
		{
			uint64 key = *verifyToken.FindKey(TokenArray[i]);
			if (verifyToken.Contains(key))
			{
				FUsersData userdb = verifyToken[key]->userdata;
				UserConnection(NULL, userdb.userid, userdb.userpass, false, true);
				verifyToken.Remove(key);
			}

		}
	}
}

void AMyChess_GameModeBase::GetAllUserData()
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
	FHttpRequestCompleteDelegate& Delegate = Request->OnProcessRequestComplete();


	Request->SetURL("http://127.0.0.1:8080/Player/GetPlayerName");
	Request->SetVerb("Post");
	Request->SetHeader(TEXT("Content-type"), TEXT("application/json"));

	Delegate.BindLambda([&](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool Succeeded)
		{
			TArray<FUsersData> userdb;
			if (!HttpResponse->GetContentAsString().Contains("timestamp")) FJsonObjectConverter::JsonArrayStringToUStruct(HttpResponse->GetContentAsString(), &userdb, 0, 0);
			AllUsers = userdb;
		});

	Request->ProcessRequest();

}

void AMyChess_GameModeBase::GetFriends(AMyOnlineBeaconClient* BC)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
	FHttpRequestCompleteDelegate& Delegate = Request->OnProcessRequestComplete();


	Request->SetURL("http://127.0.0.1:8080/Friend/Get");
	Request->SetVerb("Post");
	Request->SetHeader(TEXT("Content-type"), TEXT("application/json"));

	Delegate.BindLambda([&, BC](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool Succeeded)
		{
			TArray<FFriendsData> friendsdb;
			if (!HttpResponse->GetContentAsString().Contains("timestamp")) FJsonObjectConverter::JsonArrayStringToUStruct(HttpResponse->GetContentAsString(), &friendsdb, 0, 0);
			if (friendsdb.Num() > 0)
			{
				if (BC->userdata.userid == friendsdb[0].userid)
				{
					BC->friendsdata = friendsdb[0];
					BC->GetFriendsComplete(friendsdb[0], AllUsers);
				}
				
			}
		});

	FString JsonString = "";
	FFriendsData friendsdb;
	friendsdb.userid = BC->userdata.userid;

	FJsonObjectConverter::UStructToJsonObjectString(friendsdb, JsonString);
	Request->SetContentAsString(JsonString);
	Request->ProcessRequest();

}

void AMyChess_GameModeBase::AddFriend(AMyOnlineBeaconClient* BC, FString name, FString Type)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
	FHttpRequestCompleteDelegate& Delegate = Request->OnProcessRequestComplete();


	FString appendUrl = FString(Type + '/' + name);
	Request->SetURL("http://127.0.0.1:8080/Friend/" + appendUrl);
	Request->SetVerb("Post");
	Request->SetHeader(TEXT("Content-type"), TEXT("application/json"));

	Delegate.BindLambda([&, BC](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool Succeeded)
		{
			TArray<FFriendsData> friendsdb;
			if (!HttpResponse->GetContentAsString().Contains("timestamp")) FJsonObjectConverter::JsonArrayStringToUStruct(HttpResponse->GetContentAsString(), &friendsdb, 0, 0);
			if (friendsdb.Num() > 0)
			{
				for (int i = 0; i < friendsdb.Num(); i++)
				{
					for (int beacon = 0; beacon < HostObject->GetBeaconClients().Num(); beacon++)
					{
						AMyOnlineBeaconClient* beaconClient = Cast<AMyOnlineBeaconClient>(HostObject->GetBeaconClients()[beacon]);
						if (beaconClient->friendsdata.userid == friendsdb[i].userid)
						{
							beaconClient->friendsdata = friendsdb[i];
							beaconClient->GetFriendsComplete(friendsdb[i], AllUsers);
						}
					}
				}
			}
			else
			{
				BC->PrintSystemMessage(FString("요청 실패"));

			}
		});

	FString JsonString = "";
	FFriendsData friendsdb;
	friendsdb.userid = BC->userdata.userid;

	FJsonObjectConverter::UStructToJsonObjectString(friendsdb, JsonString);
	Request->SetContentAsString(JsonString);
	Request->ProcessRequest();


}

void AMyChess_GameModeBase::InviteTheGame(AMyOnlineBeaconClient* BC, FUsersData user, FString Type)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();

	FString appendUrl = FString(Type + '/' + user.userid);
	Request->SetURL("http://127.0.0.1:8080/Player/Invite/" + appendUrl);
	Request->SetVerb("Post");
	Request->SetHeader(TEXT("Content-type"), TEXT("application/json"));

	FHttpRequestCompleteDelegate& Delegate = Request->OnProcessRequestComplete();

	Delegate.BindLambda([&, BC, Type, user](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool Succeeded)
		{
			TArray<FUsersData> userdata;
			if (!HttpResponse->GetContentAsString().Contains("timestamp")) FJsonObjectConverter::JsonArrayStringToUStruct(HttpResponse->GetContentAsString(), &userdata, 0, 0);
			if (userdata.Num() > 0)
			{
				TArray<AMyOnlineBeaconClient*> players;
				for (int beacon = 0; beacon < HostObject->GetBeaconClients().Num(); beacon++)
				{
					AMyOnlineBeaconClient* beaconClient = Cast<AMyOnlineBeaconClient>(HostObject->GetBeaconClients()[beacon]);

					if (Type != "Remove")
					{
						for (int i = 0; i < userdata.Num(); i++)
						{
							if (beaconClient->userdata.userid == userdata[i].userid)
							{
								bool alarm = false;
								if (BC->userdata.userid != userdata[i].userid && Type == "Send") alarm = true;
								if (BC->userdata.userid != userdata[i].userid && Type == "Reject")
								{
									beaconClient->PrintSystemMessage(TEXT("초대 거절됨"));
									beaconClient->StopInvite(false);
								}
								beaconClient->userdata = userdata[i];
								beaconClient->Invite_Complete(userdata[i], AllUsers, user, alarm, Type == "Send");
								
								if (Type == "Receive" && beaconClient->LevelName != "MainMap") players.Add(beaconClient);
								break;
							}
						}
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("MessageBox %s"), *userdata[0].message_box);
						TArray<FString> id;
						userdata[0].message_box.ParseIntoArray(id, TEXT("?"), true);
						for (int r = 0; r < id.Num(); r++)
						{
							if (id[r] == beaconClient->userdata.userid)
							{
								beaconClient->PrintSystemMessage(TEXT("초대 거절됨"));
								beaconClient->StopInvite(false);
								break;
							}
						}
					}
				}

				if (Type == "Receive")
				{
					if (players.Num() == 2)
					{
						players[0]->PrintSystemMessage(TEXT("경기 준비중"));
						players[1]->PrintSystemMessage(TEXT("경기 준비중"));
						players[0]->StopInvite(true);
						players[1]->StopInvite(true);
						CreateGameServer(players[0], players[1]);
					}
					else
					{
						players[0]->PrintSystemMessage(TEXT("취소됨"));
					}
				}
			}
			else
			{
				if (Type == "Send") BC->PrintSystemMessage(TEXT("초대 실패"));
				if (Type == "Receive") BC->PrintSystemMessage(TEXT("상대가 오프라인중"));

			}
		});
	
	FString JsonString = "";
	FUsersData userdb;
	userdb.userid = BC->userdata.userid;

	FJsonObjectConverter::UStructToJsonObjectString(userdb, JsonString);
	Request->SetContentAsString(JsonString);
	Request->ProcessRequest();
}