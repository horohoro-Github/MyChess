// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAIThread.h"
#include "MyAI.h"
#include "../Chess/MyChessPiece.h"
#include "../Chess/MyMove.h"
#include "../Chess/MyTile.h"
#include "../Game/MyGameStateBase.h"
#include "../Chess/MyBoard.h"
#include "../Chess/MyPieceList.h"
#include "../Chess/MyMoveCalculator.h"
#include "MyTranspositionTable.h"
#include "MyEvaluation.h"
#include "MyMoveOrdering.h"

FMyAIThread::FMyAIThread(AMyAI* funActor, AMyBoard* MyBoard, TSharedPtr<UMyMoveCalculator> calculatorObject)
{
	ai = funActor;
	board = MyBoard;
	calculator = calculatorObject;

	moveScores.Init(0, 218);
	evaluation = new MyEvaluation();

	tt = new MyTranspositionTable(board, 64000);
}

bool FMyAIThread::Init()
{
	return true;
}

uint32 FMyAIThread::Run()
{
	bestMoveThisIteration = bestMove = new MyMove(0);

	tt->enabled = true;
	tt->Clear();

	currentIterativeSearchDepth = 0;
	bStop = false;
	searchColor = board->GetTeamColor();
	opponentSearchColor = 1 - searchColor;
	int targetDepth = INT_MAX;

	for (int searchDepth = 1; searchDepth <= targetDepth; searchDepth++)
	{
		
		SearchMoves(searchDepth, 0, negative, positive, searchColor);
		
		if (bStop)
		{
			break;
		}
		else
		{
			currentIterativeSearchDepth = searchDepth;
			bestMove = bestMoveThisIteration;
			bestEval = bestEvalThisIteration;

			if (IsMateScore(bestEval))
			{
				break;
			}
		}
		
	}
	uint64 datas = 0;
	datas = bestMove->MoveValue();

	tt->freeTT();
	delete tt;
	delete evaluation;
	moveScores.Empty();
	
	ai->ThreadQueue.Enqueue(datas);
	
	return 0;
}

void FMyAIThread::Stop()
{
	UE_LOG(LogTemp, Warning, TEXT("Stop"));
}

FMyAIThread::~FMyAIThread()
{
	if (runningThread != nullptr)
	{
		bStop = true;
		runningThread->Suspend(false);
		runningThread->Kill(true);
		delete runningThread;
	}
}

int FMyAIThread::SearchMoves(int depth, int plyFromRoot, int alpha, int beta, int color)
{
	if (bStop)
	{
		return 0;
	}
	if (plyFromRoot > 0)
	{
		if (board->repetitionHistory.Contains(board->Zobrist))
		{
			return 0;
		}

		alpha = FMath::Max(alpha, (-immediatematescore + plyFromRoot));
		beta = FMath::Min(beta, (immediatematescore - plyFromRoot));
		if (alpha >= beta)
		{
			return alpha;
		}
	}

	int ttVal = tt->LookupEvaluation(depth, plyFromRoot, alpha, beta);
	if (ttVal != tt->lookupFailed)
	{
		numTransposition++;
		if (plyFromRoot == 0)
		{
			bestMoveThisIteration = tt->GetStoredMove();
			bestEvalThisIteration = tt->entries[tt->Index()]->value;
		}
		return ttVal;
	}

	if (depth == 0)
	{
		int evaluations = QuiescenceSearch(alpha, beta);

		return evaluations;
	}

	TArray<MyMove*> moves = calculator->GenerateMove(board, true, -1, FChessPiece::All, true);
	
	OrderMoves(moves, true);

	if (moves.Num() == 0)
	{
		if (calculator->check)
		{
			int mateScore = immediatematescore - plyFromRoot;
			return -mateScore;
		}
		else
		{
			return 0;
		}
	}

	int evalType = tt->upperBound;
	MyMove* bestMoveInThisPosition = new MyMove(0);

	
	for (int i = 0; i < moves.Num(); i++)
	{
		board->MakeMoves(moves[i], true);
		int eval = -SearchMoves(depth - 1, plyFromRoot + 1, -beta, -alpha, board->GetTeamColor());
		board->UnMakeMoves(moves[i], true);
		if (eval >= beta)
		{
			tt->StoreEvaluation(depth, plyFromRoot, beta, tt->lowerBound, moves[i]);
			return beta;
		}

		if (eval > alpha)
		{
			evalType = tt->exact;
			bestMoveInThisPosition = moves[i];

			alpha = eval;
			if (plyFromRoot == 0)
			{
				bestMoveThisIteration = moves[i];
				bestEvalThisIteration = eval;
			}
		}
	}

	tt->StoreEvaluation(depth, plyFromRoot, alpha, evalType, bestMoveInThisPosition);
	
	return alpha;
}

int FMyAIThread::QuiescenceSearch(int alpha, int beta)
{
	int eval = evaluation->Evaluate(board);
	if (eval >= beta)
	{
		return beta;
	}
	if (eval > alpha)
	{
		alpha = eval;
	}
	TArray<MyMove*> moves = calculator->GenerateMove(board, false, -1, FChessPiece::All);

	OrderMoves(moves, false);
	for (int i = 0; i < moves.Num(); i++)
	{
		board->MakeMoves(moves[i], true);
		eval = -QuiescenceSearch(-beta, -alpha);
		board->UnMakeMoves(moves[i], true);
		if (eval >= beta)
		{
			return beta;
		}
		if (eval > alpha)
		{
			alpha = eval;
		}
	}
	return alpha;
}

void FMyAIThread::OrderMoves(TArray<class MyMove*>& moves, bool UseTT)
{
	moveScores.Init(0, 218);
	int16 hashValue = 0;

	if (UseTT)
	{
		if (MyMove* newMove = tt->GetStoredMove())
		{
			if (newMove != nullptr) hashValue = newMove->MoveValue();
		}
	}

	if (moves.Num() <= moveScores.Num())
	{
		for (int i = 0; i < moves.Num(); i++)
		{
			int16 score = 0;
			int moveValue = moves[i]->MoveValue();
			
			int movePieceType = board->Square[moves[i]->StartSquare() & 0b111][moves[i]->StartSquare() >> 3] & 0b111;
			int capturedPieceType = board->Square[moves[i]->TargetSquare() & 0b111][moves[i]->TargetSquare() >> 3] & 0b111;

			int flag = moves[i]->Flags();

			if (capturedPieceType != FChessPiece::None)
			{
				score = (int16)(10 * GetPieceValue(capturedPieceType) - GetPieceValue(movePieceType));
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
				if (((calculator->opponentPawnAttackMap >> moves[i]->TargetSquare()) & 1) != 0)
				{
					score -= 350;
				}
			}
			if (moveValue == hashValue)
			{
				score += 10000;
			}
			moveScores[i] = score;
		}

		Sort(moves);
	}
}

int FMyAIThread::GetPieceValue(int pieceType)
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
void FMyAIThread::Sort(TArray<class MyMove*>& moves)
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

