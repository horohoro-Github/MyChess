
// AI���϶� MyAI Ŭ������ StartSearch�Լ��� ������ �����մϴ�.
// �����忡�� ��ġ���� ã�Ƴ��� ThreadQueue������ ���� �־��ݴϴ�.
// Tick �Լ����� ThreadQueue�� ���� ������ ü���̵��� ����մϴ�.
enum class EPlayState : uint8
{
	Play_None,
	GameOver,
	Preparing,
	Playing,
	Promotion,
	AIPlaying

};
class AMyAI
{
	AMyGameStateBase* gameState;
	FMyAIThread* aiThread = NULL;
	FRunnableThread* runningThread = NULL;
	int ProcessedCalculation = 0;
	TQueue<int> ThreadQueue;
	AMyBoard* board = NULL;
	TArray<class MyMove*> moves;
	int AITeamColor = 2;

	void StartSearch()
	{
		gameState->playState = EPlayState::AIPlaying;

		aiThread = new FMyAIThread(this, board, board->moveCal);
		runningThread = FRunnableThread::Create(aiThread, TEXT("Runnable Thread"), 0, EThreadPriority::TPri_Highest, FPlatformAffinity::GetAsyncLoadingThreadMask());

		FTimerHandle threadHandle;
		FTimerDelegate threadDelegate = FTimerDelegate::CreateLambda([&]()
			{
				aiThread->abortSearch = true;
			});

		GetWorld()->GetTimerManager().SetTimer(threadHandle, threadDelegate, 1.15f, false);

	}

	void Tick(float DeltaTime)
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

}


class FMyAIThread
{
	bool abortSearch = false;
	int bestEvalThisIteration = 0;
	int bestEval = 0;
	MyMove* bestMoveThisIteration = NULL;
	int currentIterativeSearchDepth = 0;
	int SearchMoves(int depth, int plyFromRoot, int alpha, int beta, int color)
	{
		if (abortSearch)
		{
			return 0;
		}
		if (plyFromRoot > 0)
		{

			alpha = FMath::Max(alpha, (-100000 + plyFromRoot));
			beta = FMath::Min(beta, (100000 - plyFromRoot));
			if (alpha >= beta)
			{
				return alpha;
			}
		}
		if (depth == 0)
		{
			int evaluations = QuiescenceSearch(alpha, beta);

			return evaluations;
		}

		TArray<MyMove*> moves = calculator->GenerateMove(board, true, -1, FChessPiece::All, true);
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
				if (plyFromRoot == 0)
				{
					bestMoveThisIteration = moves[i];
					bestEvalThisIteration = eval;
				}
			}
		}
		return alpha;
	}

	int QuiescenceSearch(int alpha, int beta)
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

}