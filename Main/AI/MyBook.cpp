// Fill out your copyright notice in the Description page of Project Settings.


#include "MyBook.h"
#include "../Chess/MyMove.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "../Chess/MyBoard.h"
UMyBook::UMyBook()
{
}

void UMyBook::CreateBook(FString fileString)
{
	FString path = FPaths::ProjectContentDir() + TEXT("Book.txt");
	FString bookString = "";
	if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*path))
	{
		FFileHelper::LoadFileToString(bookString, *path, FFileHelper::EHashOptions::EnableVerify);
	}

	FString tempString = bookString;
	TArray<FString> books;

	tempString.ParseIntoArray(books, TEXT("pos "), true);

	for (int i = 0; i < books.Num(); i++)
	{
		TArray<FString> bookValue;

		books[i].ParseIntoArray(bookValue, TEXT("\n"), true);
		bookValue[0].TrimEndInline();

		TArray<MyBookMove*> bookMove;
		bookMove.SetNum(bookValue.Num() - 1);
		for (int j = 1; j < bookValue.Num(); j++)
		{
			FString moveStr = "";
			FString moveVal = "";
			UKismetStringLibrary::Split(bookValue[j], " ", moveStr, moveVal, ESearchCase::IgnoreCase, ESearchDir::FromStart);

			bookMove[j - 1] = new MyBookMove(moveStr, FCString::Atoi(*moveVal));
		}
		movesByPosition.Add(bookValue[0], bookMove);
	}
}

uint64 UMyBook::GetPositionKey(FString key)
{
	FString tempString = "";
	for (int i = 0; i < key.Len(); i++)
	{
		if (key[i] == ':')
		{
			return FCString::Strtoui64(*tempString, NULL, 10);
		}
		else
		{
			tempString += key[i];
		}
	}
	return uint64();
}

TArray<FString> UMyBook::GetMoveInfo(FString key)
{
	TArray<FString> tempArray;
	FString tempString = "";

	for (int i = 0; i < key.Len(); i++)
	{
		if (key[i] == ',')
		{
			tempArray.Add(tempString);
			tempString = "";
		}
		else
		{
			tempString += key[i];
		}
		if (i == key.Len() - 1)
		{
			tempArray.Add(tempString);
			return tempArray;
		}
	}
	return TArray<FString>();
}

MyMove* UMyBook::GetRandomBookMoveWeighted(FString key, AMyBoard* board)
{

	//auto p = bookPosition[key];
	FRandomStream random;
	TArray<MyBookMove*> bookMove = movesByPosition[key];
	int bookIndex = random.RandRange(0, bookMove.Num() - 1);

	TArray<float> moveWeights;
	for (int i = 0; i < bookMove.Num(); i++)
	{
		moveWeights.Add(bookMove[i]->playedNum);
	}

	
	float sumsub = 0;
	for (int i = 0; i < moveWeights.Num(); i++)
	{
		sumsub += moveWeights[i];
	}
	
	float avg = sumsub / moveWeights.Num();

	for (int i = 0; i < moveWeights.Num(); i++)
	{
		float offsetFromAvg = avg - moveWeights[i];
		moveWeights[i] += offsetFromAvg * 0.5f;
	}

	float sum = 0;
	for (int i = 0; i < moveWeights.Num(); i++)
	{
		sum += moveWeights[i];
	}

	TArray<float> moveProbabilitiesCumul;
	moveProbabilitiesCumul.Init(0, moveWeights.Num());
	float previousProbability = 0;
	for (int i = 0; i < moveWeights.Num(); i++)
	{
		moveProbabilitiesCumul[i] = previousProbability + moveWeights[i] / sum;
		previousProbability = moveProbabilitiesCumul[i];
	}

	int64 DateInSeconds = FDateTime::Now().ToUnixTimestamp();
	FRandomStream SRand = FRandomStream();
	SRand.Initialize(DateInSeconds);
	
	float t = SRand.FRandRange(0.f, 1.f);
	
	for (int i = 0; i < bookMove.Num(); i++) 
	{
		if (t <= moveProbabilitiesCumul[i])
		{
			FString moves = bookMove[i]->moveString;

			int f = moves[0] - 'a';
			int r = moves[1] - '1';

		
			int f2 = moves[2] - 'a';
			int r2 = moves[3] - '1';


			int StartSquare = f + r * 8;
			int TargetSquare = f2 + r2 * 8;

			int type = board->Square[f][r] & 0b111;
			int color = board->Square[f][r] >> 3;

			int checkFlag = 0;
			if (type == FChessPiece::Pawn)
			{
				checkFlag = abs(StartSquare - TargetSquare) == 16 ? Flag::PawnTwoForward : Flag::None;
			}
			if (type == FChessPiece::King)
			{
				checkFlag = abs(f - f2) == 2 ? Flag::Castling : Flag::None;
			}

			MyMove* move = new MyMove(StartSquare, TargetSquare, checkFlag);
			return move;
		}
	}
	return new MyMove(0);
}

bool UMyBook::HasPosition(FString positionKey)
{
	return movesByPosition.Contains(positionKey);
}

void UMyBook::Add(uint64 positionKey, MyMove* move, int numTimesPlayed)
{
	MyBookPosition* bookposition = new MyBookPosition();
	if (!bookPositions.Contains(positionKey))
	{
		bookPositions.Add(positionKey, MyBookPosition::MyBookPosition());
	}

	bookPositions[positionKey].AddMove(move, numTimesPlayed);
}
