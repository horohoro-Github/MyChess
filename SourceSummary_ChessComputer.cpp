
// AI턴일때 MyAI 클래스의 StartSearch함수로 스레드 실행합니다.
// 스레드에서 위치값을 찾아내고 ThreadQueue변수에 값을 넣어줍니다.
// Tick 함수에서 ThreadQueue의 값을 가져와 체스이동에 사용합니다.
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