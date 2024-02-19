// Fill out your copyright notice in the Description page of Project Settings.


#include "MyTranspositionTable.h"
#include "MyAIThread.h"
#include "../Chess/MyBoard.h"
MyTranspositionTable::MyTranspositionTable(AMyBoard* chessBoard, int arraySize)
{
	board = chessBoard;
	size = (uint64)arraySize;
	entries.Init(new Entry(), arraySize);
}

void MyTranspositionTable::Clear()
{
	for (int i = 0; i < entries.Num(); i++)
	{
		entries[i] = new Entry();
	}
}

uint64 MyTranspositionTable::Index()
{
	return (board->Zobrist % size);
}

MyMove* MyTranspositionTable::GetStoredMove()
{
	return entries[Index()]->move;
}

void MyTranspositionTable::StoreEvaluation(int depth, int numPlySearched, int eval, int evalType, MyMove* move)
{
	if (!enabled)
	{
		return;
	}

	Entry* entry = new Entry(board->Zobrist, CorrectMateScoreForStorage(eval, numPlySearched), depth, evalType, move);
	entries[Index()] = entry;

}

int MyTranspositionTable::LookupEvaluation(int depth, int plyFromRoot, int alpha, int beta)
{
	if (!enabled)
	{
		return lookupFailed;
	}

	Entry* entry = entries[Index()];

	if (entry->key == board->Zobrist)
	{

		if (entry->depth >= depth)
		{
			int correctedScore = CorrectRetrievedMateScore(entry->value, plyFromRoot);

			if (entry->nodeType == exact)
			{
				return correctedScore;
			}
			if (entry->nodeType == upperBound && correctedScore <= alpha)
			{
				return correctedScore;
			}
			if (entry->nodeType == lowerBound && correctedScore >= beta)
			{
				return correctedScore;
			}
		}
	}
	return lookupFailed;
}

int MyTranspositionTable::CorrectMateScoreForStorage(int score, int numPlySearched)
{
	if (FMyAIThread::IsMateScore(score))
	{
		int sign = FMath::Sign(score);
		return (score * sign + numPlySearched) * sign;
	}
	return score;
}

int MyTranspositionTable::CorrectRetrievedMateScore(int score, int numPlySearched)
{
	if (FMyAIThread::IsMateScore(score))
	{
		int sign = FMath::Sign(score);
		return (score * sign - numPlySearched) * sign;
	}
	return score;
}

void MyTranspositionTable::freeTT()
{
	entries.Empty();
}
