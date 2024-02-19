// Fill out your copyright notice in the Description page of Project Settings.


#include "MyUserWidget.h"
#include "../Game/MyGameStateBase.h"
#include "../Player/MyPlayerController.h"
#include "../Chess/MyBoard.h"
#include "../Chess/MyMove.h"
#include "../Player/MyPlayerState.h"
#include "../Game/MyGameInstance.h"
void UMyUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	FTimerHandle constructTimer;
	FTimerDelegate constructDelegate = FTimerDelegate::CreateLambda([&]()
		{
			gameIns = Cast<UMyGameInstance>(GetGameInstance());
			PC = Cast<AMyPlayerController>(GetOwningPlayer());
			GameState = Cast<AMyGameStateBase>(GetWorld()->GetGameState());
			if(PC) PS = Cast<AMyPlayerState>(PC->PlayerState);
			if (GameState)
			{
				for (int i = 0; i < GameState->PlayerArray.Num(); i++)
				{
					AMyPlayerState* OtherPS = Cast<AMyPlayerState>(GameState->PlayerArray[i]);
					if (OtherPS != PS)
					{
						OtherPSArray.Add(OtherPS);
					}
				}
			}
		}
	);

	GetWorld()->GetTimerManager().SetTimer(constructTimer, constructDelegate, 0.5f, false);
	
}

TArray<FUsersData> UMyUserWidget::ExtractMessages(FUsersData db)
{
	TArray<FUsersData> users;
	TArray<FString> allMessages;

	db.message_box.ParseIntoArray(allMessages, TEXT("?"), true);
	
	for (int i = 0; i < gameIns->FindUserName.Num(); i++)
	{
		for (int j = 0; j < allMessages.Num(); j++)
		{
			
			if (gameIns->FindUserName[i].userid == allMessages[j])
			{
				users.Add(gameIns->FindUserName[i]);
				break;
			}
		}
	}
	return users;
}

bool UMyUserWidget::EqualFriendsList(FUsersData user1, FUsersData user2)
{
	if (user1.player_name == user2.player_name && user1.win == user2.win && user1.draw == user2.draw && user1.lose == user2.lose
		&& user1.login == user2.login)
		return true;
	else
	return false;
}

FUserName UMyUserWidget::SetFriends()
{
	
	FUserName username;
	
	TArray<FString> friends_id;
	gameIns->friendsdb.friends_id.ParseIntoArray(friends_id, TEXT("?"), true);
	TArray<FString> friends_req;
	gameIns->friendsdb.friends_req.ParseIntoArray(friends_req, TEXT("?"), true);
	TArray<FString> friends_res;
	gameIns->friendsdb.friends_res.ParseIntoArray(friends_res, TEXT("?"), true);

	for (int i = 0; i < gameIns->FindUserName.Num(); i++)
	{
		if (gameIns->userdb.userid != gameIns->FindUserName[i].userid)
		{
			for (int id = 0; id < friends_id.Num(); id++)
			{
				if (gameIns->FindUserName[i].userid == friends_id[id])
				{
					username.username.Add(gameIns->FindUserName[i]);
				}
			}
			for (int req = 0; req < friends_req.Num(); req++)
			{
				if (gameIns->FindUserName[i].userid == friends_req[req])
				{
					username.name_req.Add(gameIns->FindUserName[i]);
				}
			}
			for (int res = 0; res < friends_res.Num(); res++)
			{
				if (gameIns->FindUserName[i].userid == friends_res[res])
				{
					username.name_res.Add(gameIns->FindUserName[i]);
				}
			}
		}
	}
	return username;
}

void UMyUserWidget::Promotion(int index)
{
	if (PS)
	{
		PS->PromotionExecution(GameState->board, index);
	}
}

TArray<FString> UMyUserWidget::GetHistory()
{
	TArray<FString> gameHistory;
	
	if (GameState->playState == EPlayState::AIPlaying)
	{
		gameHistory.Add("AIPlaying");
		return gameHistory;
	}
	
	if (GameState->playState != EPlayState::Play_None)
	{
		TArray<uint64> historys = GameState->GetHistory();

		for (int i = 1; i < historys.Num(); i++)
		{
			FString ReturnString = "";
			uint64 history = historys[i] >> 18;

			int startSquare = history & 0b111111;
			int targetSquare = (history >> 6) & 0b111111;

			int flag = (history >> 12) & 0b1111;

			int piece = (historys[i] >> 34) & 0b1111;

			int teamColor = piece >> 3;
			int type = piece & 0b111;

			int startFile = startSquare & 0b111;
			int startRank = startSquare >> 3;

			int targetFile = targetSquare & 0b111;
			int targetRank = targetSquare >> 3;

			int capturedType = (historys[i] >> 8) & 0b111;

			int SameSquare = (historys[i] >> 39) & 0b11;
			int check = (historys[i] >> 41) & 1;
			int checkMate = (historys[i] >> 42) & 1;

			FString Files = "abcdefgh";
			FString Ranks = "12345678";

			FString Pieces = "PNBRQK";
			int numberOfTurns = ((i - 1) / 2) + 1;
			if (teamColor == FTeamColor::White) ReturnString += FString::FromInt(numberOfTurns) + ". ";
			
			if (type == FChessPiece::Pawn)
			{
				if (capturedType != 0)
				{
					ReturnString += Files[startFile];
					ReturnString += "x";
				}
				ReturnString += Files[targetFile];
				ReturnString += Ranks[targetRank];
				if (flag >= Flag::PromoteToBishop && flag <= Flag::PromoteToQueen)
				{
					ReturnString += "=";
					ReturnString += Pieces[flag - 1];
				}
			}
			else
			{
				if (flag == Flag::Castling)
				{
					if (targetFile == 2) ReturnString += "0-0-0";
					if (targetFile == 6) ReturnString += "0-0";
				}
				else
				{
					ReturnString += Pieces[type - 1];

					if ((SameSquare & 0b10) != 0) ReturnString += Files[startFile];
					if ((SameSquare & 0b01) != 0) ReturnString += Ranks[startRank];
					if (capturedType != 0) ReturnString += "x";

					ReturnString += Files[targetFile];

					ReturnString += Ranks[targetRank];
				}
			}
			
			if (checkMate) ReturnString += "#";
			else if (check) ReturnString += "+";
			gameHistory.Add(ReturnString);

			if (i == historys.Num() - 1)
			{
				if (GameState->playState == EPlayState::GameOver)
				{
					if (ReturnString[ReturnString.Len() - 1] == '#')
					{
						if (GameState->TeamColor == 0)
						{
							gameHistory.Add("   0-1");
						}
						else
						{
							gameHistory.Add("1-0");
						}
					}
					else
					{
						if (GameState->TeamColor == 0)
						{
							gameHistory.Add("   1/2-1/2");
						}
						else
						{
							gameHistory.Add("1/2-1/2");
						}
					}
				}
			}
		}
		return gameHistory;
	}
	
	gameHistory.Add("NoneHistory");
	return gameHistory;
}

TArray<FAlgebraicNotation> UMyUserWidget::GetNotation(int causeGiver)
{
	TArray<FAlgebraicNotation> notation;
	FString Files = "abcdefgh";
	FString Ranks = "12345678";

	FString Pieces = "PNBRQK";
	if (GameState->playState != EPlayState::Play_None)
	{
		TArray<uint64> historys = GameState->GetHistory();
		for (int i = 1; i < historys.Num(); i++)
		{
			FString white = "";
			FString black = "";
			FAlgebraicNotation algebraicNotation;

			int a = i;
			for (int j = a; j < a + 2 && j < historys.Num(); j++)
			{
				uint64 history = historys[j] >> 18;

				int startSquare = history & 0b111111;
				int targetSquare = (history >> 6) & 0b111111;

				int flag = (history >> 12) & 0b1111;

				int piece = (historys[j] >> 34) & 0b1111;

				int teamColor = piece >> 3;
				int type = piece & 0b111;

				int startFile = startSquare & 0b111;
				int startRank = startSquare >> 3;

				int targetFile = targetSquare & 0b111;
				int targetRank = targetSquare >> 3;

				int capturedType = (historys[j] >> 8) & 0b111;

				int SameSquare = (historys[j] >> 39) & 0b11;
				int check = (historys[j] >> 41) & 1;
				int checkMate = (historys[j] >> 42) & 1;
				i = j;

				FString returnString = "";

				if (type == FChessPiece::Pawn)
				{
					if (capturedType != 0)
					{
						returnString += Files[startFile];
						returnString += "x";
					}
					returnString += Files[targetFile];
					returnString += Ranks[targetRank];
					if (flag >= Flag::PromoteToBishop && flag <= Flag::PromoteToQueen)
					{
						returnString += "=";
						returnString += Pieces[flag - 1];
					}
				}
				else
				{
					if (flag == Flag::Castling)
					{
						if (targetFile == 2) returnString += "0-0-0";
						if (targetFile == 6) returnString += "0-0";
					}
					else
					{
						returnString += Pieces[type - 1];

						if ((SameSquare & 0b10) != 0) returnString += Files[startFile];
						if ((SameSquare & 0b01) != 0) returnString += Ranks[startRank];
						if (capturedType != 0) returnString += "x";

						returnString += Files[targetFile];

						returnString += Ranks[targetRank];
					}
				}
				if (checkMate) returnString += "#";
				else if (check) returnString += "+";

				if (i % 2 == 1) white = returnString;
				else black = returnString;


			}
		
			algebraicNotation.Algebraic_Number = ((i - 1) / 2) + 1;
			algebraicNotation.Algebraic_White = white;
			algebraicNotation.Algebraic_Black = black;
			if ((i == historys.Num() - 1) && (GameState->playState == EPlayState::GameOver))
			{
				FAlgebraicNotation resNotation;

				if (i % 2 == 1)
				{
					if (white[white.Len() - 1] == '#')
					{
						algebraicNotation.Algebraic_Black = "1-0";
					}
					else
					{
						if (causeGiver == FTeamColor::White) algebraicNotation.Algebraic_Black = "0-1";
						
						if (causeGiver == FTeamColor::Black) algebraicNotation.Algebraic_Black = "1-0";
						
						if (causeGiver == FTeamColor::Neutral) algebraicNotation.Algebraic_Black = "1/2-1/2";
					
					}
					
					notation.Add(algebraicNotation);
				}
				else
				{
					if (black[black.Len() - 1] == '#')
					{
						resNotation.Algebraic_White = "0-1";
					}
					else if (GameState->Turn[FTeamColor::White] <= 0.f)
					{
						resNotation.Algebraic_White = "0-1";
					}
					else
					{
						if (causeGiver == FTeamColor::White) resNotation.Algebraic_White = "0-1";

						if (causeGiver == FTeamColor::Black) resNotation.Algebraic_White = "1-0";

						if (causeGiver == FTeamColor::Neutral) resNotation.Algebraic_White = "1/2-1/2";
					}
					notation.Add(algebraicNotation);
					notation.Add(resNotation);
				}
				
			}
			else
			{
				notation.Add(algebraicNotation);

			}
		}

	}
	return notation;
}

void UMyUserWidget::ResetThisGame()
{
	if (PS)
	{
		PS->RematchVote();
	}
}
