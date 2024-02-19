// Fill out your copyright notice in the Description page of Project Settings.


#include "MyMoveOrdering.h"
#include "../Chess/MyMoveCalculator.h"
#include "MyTranspositionTable.h"
#include "../Chess/MyMove.h"
#include "../Chess/MyBoard.h"
#include "MyEvaluation.h"

UMyMoveOrdering::UMyMoveOrdering()
{
}

UMyMoveOrdering::~UMyMoveOrdering()
{
	moveScores.Empty();
	delete move;
	delete evaluation;
	delete transpositionTable;
}
/*
UMyMoveOrdering::UMyMoveOrdering(UMyMoveCalculator* movecalculator, MyTranspositionTable* tt)
{
	moveScores.SetNum(218);
	moveCal = movecalculator;
	transpositionTable = tt;
	move = new MyMove(0);
	evaluation = new MyEvaluation();
}
*/

void UMyMoveOrdering::Init(UMyMoveCalculator* movecalculator, MyTranspositionTable* tt)
{
	moveScores.Init(0, 218);
	moveCal = movecalculator;
	transpositionTable = tt;
	move = new MyMove(0);
	evaluation = new MyEvaluation();
}

void UMyMoveOrdering::OrderMoves(AMyBoard* board, TArray<MyMove*>& moves, bool UseTT)
{
	MyMove* hasMove = move;
	if (UseTT)
	{
		if (MyMove* newMove = transpositionTable->GetStoredMove())
		{
			if (newMove != NULL) hasMove = newMove;
		}
	}
	if (moves.Num() <= moveScores.Num())
	{
		//UE_LOG(LogTemp, Warning, TEXT("order moves %d"), moves.Num());
		for (int i = 0; i < moves.Num(); i++)
		{
			int16 score = 0;

			int movePieceType = board->Square[moves[i]->StartSquare() & 0b111][moves[i]->StartSquare() >> 3] & 0b111;
			int capturedPieceType = board->Square[moves[i]->TargetSquare() & 0b111][moves[i]->TargetSquare() >> 3] & 0b111;

			int flag = moves[i]->Flags();

			if (capturedPieceType != FChessPiece::None)
			{
				score = (int16)(capturedPieceValueMultiplier * GetPieceValue(capturedPieceType) - GetPieceValue(movePieceType));
			}

			if (movePieceType == FChessPiece::Pawn)
			{
				if (flag == Flag::PromoteToQueen)
				{
					score += evaluation->queenValue;
				}
				else if (flag == Flag::PromoteToKnight)
				{
					score += evaluation->knightValue;
				}
				else if (flag == Flag::PromoteToRook)
				{
					score += evaluation->rookValue;
				}
				else if (flag == Flag::PromoteToBishop)
				{
					score += evaluation->bishopValue;
				}
			}
			else
			{
				if (((moveCal->opponentPawnAttackMap >> moves[i]->TargetSquare()) & 1) != 0)
				{
					score -= squareControlledByOpponentPawnPenalty;
				}
			}
			if (moves[i]->MoveValue() == hasMove->MoveValue())
			{
				score += 10000;
			}

			//	UE_LOG(LogTemp, Warning, TEXT("score %d"), score);
			moveScores[i] = score;
		}

		Sort(moves);
	}
}

int UMyMoveOrdering::GetPieceValue(int pieceType)
{
	switch (pieceType)
	{
	case FChessPiece::Pawn:
		return evaluation->pawnValue;
	case FChessPiece::Knight:
		return evaluation->knightValue;
	case FChessPiece::Bishop:
		return evaluation->bishopValue;
	case FChessPiece::Rook:
		return evaluation->rookValue;
	case FChessPiece::Queen:
		return evaluation->queenValue;
	default:
		return 0;
	}
}

void UMyMoveOrdering::Sort(TArray<MyMove*>& moves)
{
	for (int i = 0; i < moves.Num() - 1; i++)
	{
		for (int j = i + 1; j > 0; j--)
		{
			int swapIndex = j - 1;
			if (moveScores[swapIndex] < moveScores[j])
			{
				MyMove* tempMove = moves[j];
				moves[j] = moves[swapIndex];
				moves[swapIndex] = tempMove;


				int temp = moveScores[j];
				moveScores[j] = moveScores[swapIndex];
				moveScores[swapIndex] = temp;
			}
		}
	}
}

int UMyMoveOrdering::TestFunc()
{

	for (int i = 0; i < 20; i++) moveScores[i] = i;
	return 0;
}

