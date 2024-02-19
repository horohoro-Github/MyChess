// Fill out your copyright notice in the Description page of Project Settings.


#include "MyChess_GameModeBase_Multiplay.h"
#include "Kismet/GameplayStatics.h"
#include "../MultiPlay/MyOnlineBeaconClient.h"
#include "../Game/MyGameStateBase.h"
#include "../Chess/MyBoard.h"
#include "../Etc/MyReferee.h"
#include "JsonObjectConverter.h"
#include "GameFramework/GameSession.h"


void AMyChess_GameModeBase_Multiplay::BeginPlay()
{
	if(IsRunningDedicatedServer())
	{
		Http = &FHttpModule::Get();
	}

	if (board == NULL)
	{
		FActorSpawnParameters spawnparam;
		board = GetWorld()->SpawnActor<AMyBoard>(boardClass, FVector(), FRotator(), spawnparam);
		if (board)
		{
			AMyGameStateBase* stateBase = Cast<AMyGameStateBase>(GetWorld()->GetGameState());
			if (stateBase)
			{
				stateBase->playState = EPlayState::Preparing;
				stateBase->playMode = EPlayMode::MultiPlay;
				stateBase->TeamColor = 0;
				stateBase->board = board;
				stateBase->TimeLimit[0] = 600.f;
				stateBase->TimeLimit[1] = 600.f;
				stateBase->selectTeamTimer = 30.f;
			    AMyReferee* referee = GetWorld()->SpawnActor<AMyReferee>(AMyReferee::StaticClass(), FVector(200.f, 350.f, 82.f), FRotator(0.f, -180.f, 0.f));
				stateBase->referee = referee;
				board->gameState = stateBase;
				board->SpawnBoard();
			}
		}
	}

///	bFirstLoad = true;

	
	Super::BeginPlay();

	FString id1 = "";
	FString id2 = "";

	FParse::Value(FCommandLine::Get(), TEXT("id1="), id1);
	FParse::Value(FCommandLine::Get(), TEXT("id2="), id2);

	playerID.Add(id1);
	playerID.Add(id2);
}

void AMyChess_GameModeBase_Multiplay::Tick(float DeltaTime)
{
	if (IsRunningDedicatedServer() && serverdata.servertype == "GameServer")
	{
		if (GetNumPlayers() == 0)
		{
			QuitTimer -= 1.f * DeltaTime;
			if (QuitTimer <= 0.f)
			{
				QuitServer();
			}
		}
		else QuitTimer = 5.f;
	}
}

void AMyChess_GameModeBase_Multiplay::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	uint64 tokenValue = 0;
	uint64 tokenValue2 = 0;
	FParse::Value(FCommandLine::Get(), TEXT("token1="), tokenValue);
	FParse::Value(FCommandLine::Get(), TEXT("token2="), tokenValue2);
	FString id1 = "";
	FString id2 = "";

	FParse::Value(FCommandLine::Get(), TEXT("id1="), id1);
	FParse::Value(FCommandLine::Get(), TEXT("id2="), id2);

	FString eee = FString("token invalid");
	uint64 playerValue = 0;
	FParse::Value(*Options, TEXT("?token="), playerValue);
	FString id = "";
	FParse::Value(*Options, TEXT("id="), id);
	id.Split(TEXT("?"), &id, NULL);

	if ((tokenValue == playerValue && id1 == id) || (tokenValue2 == playerValue && id2 == id))
	{
		FString key = UniqueId.ToString();

		userinfo.Add(key, id);
	}
	else
	{
		FGameModeEvents::GameModePreLoginEvent.Broadcast(this, UniqueId, eee);
	}
}

void AMyChess_GameModeBase_Multiplay::PostLogin(APlayerController* NewPlayer)
{
	AMyPlayerController* PC = Cast<AMyPlayerController>(NewPlayer);	
	if (PC)
	{
		PC->AddBeaconClient();
		PC->FadeScreen(0);
		PC->PlayerControllerSetup(false, false);
		PC->AddWidget(EWidgetState::Ingame, false);
		AMyPlayerState* PS = Cast<AMyPlayerState>(PC->PlayerState);
		if (PS)
		{
			PS->playerState = EPlayerState::PLAYER;
			PS->MyTeamColor = 0;
		}
	}
}

void AMyChess_GameModeBase_Multiplay::Logout(AController* Exiting)
{
	AMyPlayerState* ps = Cast<AMyPlayerState>(Exiting->PlayerState);

	AMyGameStateBase* statebase = Cast<AMyGameStateBase>(GetWorld()->GetGameState());

	if (ps->playerState == EPlayerState::PLAYER)
	{
		if (statebase->playState == EPlayState::Promotion || statebase->playState == EPlayState::Playing || statebase->playState == EPlayState::Preparing)
		{
			for (int i = 0; i < statebase->PlayerArray.Num(); i++)
			{
				AMyPlayerState* winner = Cast<AMyPlayerState>(statebase->PlayerArray[i]);
				if (winner != ps)
				{
					for (int j = 0; j < playerID.Num(); j++)
					{
						if (winner->userdb.userid == playerID[j])
						{
							CheckValidPlayer(winner->userdb.userid);
							break;
						}
					}
				}
				else
				{
					statebase->UpdateHistory(ps->MyTeamColor);
				}
			}
		}

		if (statebase->playState == EPlayState::GameOver)
		{
			for (int i = 0; i < statebase->PlayerArray.Num(); i++)
			{
				AMyPlayerState* otherplayer = Cast<AMyPlayerState>(statebase->PlayerArray[i]);
				if (otherplayer != ps)
				{
					otherplayer->SetPlayerData(FUsersData(), "LobbyLevel", true);
				}
			}
		}
	}
}

void AMyChess_GameModeBase_Multiplay::CheckValidPlayer(FString id)
{
	AMyGameStateBase* statebase = Cast<AMyGameStateBase>(GetWorld()->GetGameState());

	if (statebase->playState == EPlayState::Promotion || statebase->playState == EPlayState::Playing || statebase->playState == EPlayState::Preparing)
	{
		statebase->playState = EPlayState::Play_None;
		for (int player = 0; player < statebase->PlayerArray.Num(); player++)
		{
			AMyPlayerState* PlayerState = Cast<AMyPlayerState>(statebase->PlayerArray[player]);
			for (int i = playerID.Num() - 1; i >= 0; i--)
			{
				if (PlayerState->userdb.userid == playerID[i])
				{
					EGameResult result;
					if (playerID[i] == id) result = EGameResult::Win;
					else result = EGameResult::Lose;
					GameResult(PlayerState, playerID[i], result, true);

					playerID.RemoveAt(i);
					break;
				}
			}
		}

		for (int i = 0; i < playerID.Num(); i++)
		{
			GameResult(NULL, playerID[i], EGameResult::Lose, true);
		}
	}
}

void AMyChess_GameModeBase_Multiplay::GameResult(AMyPlayerState* PS, FString id, EGameResult result, bool leaved)
{

	if (IsRunningDedicatedServer())
	{
		TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
		FHttpRequestCompleteDelegate& Delegate = Request->OnProcessRequestComplete();

		FString matchResult = "";
		if (result == EGameResult::Win) matchResult = TEXT("Win");
		if (result == EGameResult::Draw) matchResult = TEXT("Draw");
		if (result == EGameResult::Lose) matchResult = TEXT("Lose");

		Request->SetURL("http://127.0.0.1:8080/Player/MatchUpdate/" + matchResult);
		Request->SetVerb("Patch");
		Request->SetHeader(TEXT("Content-type"), TEXT("application/json"));


		Delegate.BindLambda([&, PS, leaved](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool Succeeded)
			{
				TArray<FUsersData> userdb;
				if (!HttpResponse->GetContentAsString().Contains("timestamp")) FJsonObjectConverter::JsonArrayStringToUStruct(HttpResponse->GetContentAsString(), &userdb, 0, 0);
				if (userdb.Num() > 0)
				{
					FString url = "";

					if (leaved) url = "LobbyLevel";
					if (PS)
					{
						PS->userdb = userdb[0];
						PS->SetPlayerData(userdb[0], url, false);
					}
				}
			});

		FString JsonString;
		FUsersData userD;

		userD.userid = id;
		
		FJsonObjectConverter::UStructToJsonObjectString(userD, JsonString);
		Request->SetContentAsString(JsonString);
		Request->ProcessRequest();
	}
}
