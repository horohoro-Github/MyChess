// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAI.h"
#include "../Chess/MyChessPiece.h"
#include "../Chess/MyTile.h"
#include "Kismet/GameplayStatics.h"
#include "../Player/MyPlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"
#include "MyAIThread.h"
#include "../Chess/MyMove.h"
#include "../Game/MyGameStateBase.h"
#include "../Chess/MyBoard.h"
#include "../Chess/MyMoveCalculator.h"
#include "MyBook.h"
#include "../Chess/MyFen.h"
#include "MyTranspositionTable.h"
#include "MyMoveOrdering.h"
#include "MyEvaluation.h"
#include "../Chess/MyPreComputedMoveData.h"

AMyAI::AMyAI()
{
	PrimaryActorTick.bCanEverTick = true;	
}

void AMyAI::BeginPlay()
{
	Super::BeginPlay();


	book = NewObject<UMyBook>(this, UMyBook::StaticClass());

	book->CreateBook("");
}

void AMyAI::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
	if (!ThreadQueue.IsEmpty() && ThreadQueue.Dequeue(ProcessedCalculation))
	{
		
		int startSquare = ProcessedCalculation & 0b111111;
		int targetSquare = (ProcessedCalculation >> 6) & 0b111111;
		int flag = (ProcessedCalculation >> 12);
		MyMove* newMove = new MyMove(startSquare, targetSquare, flag);
		MoveChessPieces(newMove);
 	}
}

void AMyAI::StartSearch()
{
	gameState->playState = EPlayState::AIPlaying;

	MyMove* bookMove = new MyMove(0);
	if (board->playCount <= 10)
	{
		MyFen* fen = new MyFen();
		
		FString fenString = fen->CurrentFEN(board, false);

		if (book->HasPosition(fenString))
		{
			
			bookMove = book->GetRandomBookMoveWeighted(fenString, board);
		}
	}
	
	if (bookMove->MoveValue() != 0)
	{
		FTimerHandle threadHandle;
		FTimerDelegate threadDelegate = FTimerDelegate::CreateLambda([bookMove, this]()
			{
				this->MoveChessPieces(bookMove);
			});

		GetWorld()->GetTimerManager().SetTimer(threadHandle, threadDelegate, 0.3f, false);
	}
	else
	{
		if (aiThread && runningThread)
		{
			runningThread->Suspend(true);
			aiThread->bStop = true;
			runningThread->Suspend(false);
			runningThread->Kill(false);
			runningThread->WaitForCompletion();
			delete aiThread;
		}
		aiThread = new FMyAIThread(this, board, board->moveCal);
		runningThread = FRunnableThread::Create(aiThread, TEXT("Runnable Thread"), 0, EThreadPriority::TPri_Highest, FPlatformAffinity::GetAsyncLoadingThreadMask());

		FTimerHandle threadHandle;
		FTimerDelegate threadDelegate = FTimerDelegate::CreateLambda([&]()
			{
				aiThread->bStop = true;
			});
			
		GetWorld()->GetTimerManager().SetTimer(threadHandle, threadDelegate, 1.15f, false);
	}
}

void AMyAI::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (runningThread && aiThread)
	{
		runningThread->Suspend(true);
		aiThread->bStop = true;
		runningThread->Suspend(false);
		runningThread->Kill(false);
		runningThread->WaitForCompletion();
		delete aiThread;
	}
}

void AMyAI::MoveChessPieces(MyMove* movePiece)
{
	gameState->playState = EPlayState::Playing;
	int startSquare = movePiece->StartSquare();
	int targetSquare = movePiece->TargetSquare();
	int flag = movePiece->Flags();

	int newFile = targetSquare & 0b111;
	int newRank = targetSquare >> 3;

	if (board->PieceArray[targetSquare] != NULL)
	{
		board->Capture(board->PieceArray[movePiece->TargetSquare()]);
	}

	if (board->PieceArray[startSquare] != NULL)
	{
		for (int i = 0; i < 64; i++) board->UpdateRoute(i, EDecalState::Nomal);
		board->UpdateRoute(startSquare, EDecalState::StartPoint);
		board->PieceArray[startSquare]->File = newFile;
		board->PieceArray[startSquare]->Rank = newRank;
		board->PieceArray[targetSquare] = board->PieceArray[startSquare];
		board->PieceArray[targetSquare]->MoveLocation(board->TileArray[targetSquare]->GetActorLocation() + FVector(0.f, 0.f, 50.f), true);
		board->PieceArray[startSquare] = NULL;
	}

	bool isPromotion = false;
	if ((board->Square[startSquare & 0b111][startSquare >> 3] & 0b111) == FChessPiece::Pawn)
	{
		if (flag == Flag::EnPassantCapture)
		{
			int enPassantOffset = AITeamColor == FTeamColor::White ? -8 : 8;
			board->Capture(board->PieceArray[targetSquare + enPassantOffset]);
		}
		if (flag == Flag::PromoteToQueen)
		{
			isPromotion = true;
			board->promotionTarget.startSquare = startSquare; board->promotionTarget.targetSquare = targetSquare;
			board->ChessPromotion(FChessPiece::Queen, true);
		}
		if (flag == Flag::PromoteToRook)
		{
			isPromotion = true;
			board->promotionTarget.startSquare = startSquare; board->promotionTarget.targetSquare = targetSquare;
			board->ChessPromotion(FChessPiece::Rook, true);
		}
		if (flag == Flag::PromoteToBishop)
		{
			isPromotion = true;
			board->promotionTarget.startSquare = startSquare; board->promotionTarget.targetSquare = targetSquare;
			board->ChessPromotion(FChessPiece::Bishop, true);
		}
		if (flag == Flag::PromoteToKnight)
		{
			isPromotion = true;
			board->promotionTarget.startSquare = startSquare; board->promotionTarget.targetSquare = targetSquare;
			board->ChessPromotion(FChessPiece::Knight, true);
		}
	}

	if ((board->Square[startSquare & 0b111][startSquare >> 3] & 0b111) == FChessPiece::King)
	{
		if (flag == Flag::Castling)
		{
			if (newFile == 2) //queenside 
			{
				board->PieceArray[newFile - 2 + newRank * 8]->File = newFile + 1;
				board->PieceArray[newFile - 2 + newRank * 8]->Rank = newRank;
				board->PieceArray[newFile - 2 + newRank * 8]->MoveLocation(board->TileArray[newFile + 1 + newRank * 8]->GetActorLocation() + FVector(0.f, 0.f, 50.f), false);
				board->PieceArray[newFile + 1 + newRank * 8] = board->PieceArray[newFile - 2 + newRank * 8];
				board->PieceArray[newFile - 2 + newRank * 8] = NULL;
			}
			if (newFile == 6) //kingside
			{
				board->PieceArray[newFile + 1 + newRank * 8]->File = newFile - 1;
				board->PieceArray[newFile + 1 + newRank * 8]->Rank = newRank;
				board->PieceArray[newFile + 1 + newRank * 8]->MoveLocation(board->TileArray[newFile - 1 + newRank * 8]->GetActorLocation() + FVector(0.f, 0.f, 50.f), false);
				board->PieceArray[newFile - 1 + newRank * 8] = board->PieceArray[newFile + 1 + newRank * 8];
				board->PieceArray[newFile + 1 + newRank * 8] = NULL;

			}
		}
	}

	if (isPromotion == false) board->MakeMoves(movePiece, false);
}
