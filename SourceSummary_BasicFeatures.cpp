//오브젝트를 클릭하면 MyBoard클래스에서 처리하며 경로 표기 및 진행이 가능하게 합니다.
//MyMove클래스는 이동할 위치와 이동한 위치의 값을 가지며 주로 이동 계산을 끝냈을때 배열의 형태로 사용하게 됩니다.

class MyMove
{
	uint32 moveValue = 0;

	MyMove(int startSquare, int targetSquare)
	{
		moveValue = (uint32)(startSquare | targetSquare << 6);
	}

	uint32 MoveValue()
	{
		return moveValue;
	}
}


//

class AMyBoard
{
	TArray<TArray<int>> Square;
	TArray<MyMove*> canMoves;
	TArray<UMaterialInstance*> materials;
	TSharedPtr<UMyMoveCalculator> moveCal;
	void Init()
	{
		//보드판 생성
		for (int rank = 0; rank < 8; rank++)
		{
			for (int file = 0; file < 8; file++)
			{
				AMyTile* tile = Cast<AMyTile>(GetWorld()->SpawnActor<AActor>(AMyTile::StaticClass(), FVector(file * -100.f, rank * 100.f, 0.f), FRotator(0.f, 0.f, 0.f), FieldSpawnParams));
				tile->File = file;
				tile->Rank = rank;
				float odd = (file + rank) % 2;
				tile->colorBrightness = odd;
				TileArray[file + rank * 8] = tile;


				AMyDecal* decal = Cast<AMyDecal>(GetWorld()->SpawnActor<AActor>(AMyDecal::StaticClass(), FVector(file * -100.f, rank * 100.f, 50.f), FRotator(0.f, 0.f, 0.f), FieldSpawnParams));
				decals.Add(decal);
			}
		}

		//문자열로 나타낸 기물 위치표기를 정수형으로 변화하여 SpawningPosition에 추가 하였습니다.
		TArray<int> SpawningPosition;

		for (int i = 0; i < SpawningPosition.Num(); i++)
		{
			if (SpawningPosition[i] != 0)
			{
				int file = i & 0b111;
				int rank = i >> 3;
				Square[file][rank] = SpawningPosition[i];
				int color = (Square[file][rank] >> 3) & 1;
				FRotator rotation = color == 0 ? FRotator() : FRotator(0.f, 180.f, 0.f);
				AMyChessPiece* piece = Cast<AMyChessPiece>(GetWorld()->SpawnActor<AActor>(AMyChessPiece::StaticClass(), FVector(file * -100.f, rank * 100.f, 50.f), rotation, pieceSpawnParams));
			}
		}
		moveCal = MakeSharable(NewObject<UMyMoveCalculator>(GetWorld(), UMyMoveCalculator::StaticClass());
	}

	void SelectedPiece(AMyChessPiece* piece)
	{
		if (piece)
		{
			int file = piece->File;
			int rank = piece->Rank;
			int square = file + rank * 8;
			int type = Square[file][rank] & 0b111;

			TArray<MyMove*> moves = moveCal->GeneratedMove(this, true, square, type, true); //기물의 종류에 해당하는 이동계산 결과를 가져옵니다.
			for (int i = 0; i < moves.Num(); i++)
			{
				if (moves[i]->StartSquare() == square)
				{
					canMoves.Add(moves[i]);
				}
			}
			WayPoint(canMoves);
		}
	}

	void SelectedSquare(AMyTile* tile)
	{
		if (tile)
		{
			int SquareIndex = tile->File + tile->Rank * 8;
			MyMove* selectedMove = new MyMove(0);
			for (int i = 0; i < canMoves.Num(); i++)
			{
				if (SquareIndex == canMoves[i]->TargetSquare())
				{
					selectedMove = canMoves[i];
					break;
				}
			}
			int file = selectedMove->StartSquare() & 0b111;
			int rank = selectedMove->StartSquare() >> 3;

			if (selectedMove->GetPieceType() != 0)
				if ((Square[file][rank] & 0b111) != 0 && selectedMove->MoveValue() != 0 && selectedMove->Flags() != Flag::CantMove)
				{
					int newFile = selectedMove->TargetSquare() & 0b111;
					int newRank = selectedMove->TargetSquare() >> 3;
					int flag = selectedMove->Flags();

					if (PieceArray[selectedMove->StartSquare()] != NULL)
					{

						PieceArray[selectedMove->StartSquare()]->File = newFile;
						PieceArray[selectedMove->StartSquare()]->Rank = newRank;
						if (PieceArray[selectedMove->TargetSquare()] != NULL)
						{
							Capture(PieceArray[selectedMove->TargetSquare()]);
						}

						PieceArray[selectedMove->StartSquare()]->MoveLocation(TileArray[selectedMove->TargetSquare()]->GetActorLocation() + FVector(0.f, 0.f, 50.f), true);
						PieceArray[selectedMove->TargetSquare()] = PieceArray[selectedMove->StartSquare()];
						PieceArray[selectedMove->StartSquare()] = NULL;

						MakeMoves(selectedMove, false);
					}
					canMoves.Empty();
				}
		}
	}

	void WayPoint(TArray<MyMove*> target)
	{
		for (int i = 0; i < target.Num(); i++)
		{
			int startSquare = moves[i]->StartSquare();
			int targetSquare = moves[i]->TargetSquare();

			int file = targetSquare & 0b111;
			int rank = targetSquare >> 3;

			int squareIndex = ((file % 2 + rank % 2) % 2) == 0 ? 10 : 2;
			float squareColorParam = ((file % 2 + rank % 2) % 2) == 0 ? 0.f : 1.f;
			if (moves[i]->Flags() == Flag::Castling)
			{
				TileArray[targetSquare]->MeshComp->SetMaterial(0, Materials[EChessMaterialAssets::CASTLINGSQUARE]);
				TileArray[targetSquare]->CurrentMaterial = Materials[EChessMaterialAssets::CASTLINGSQUARE];
			}
			else
			{
				int tileState = Square[file][rank] == FChessPiece::None ? EChessMaterialAssets::MOVABLESQUARE : EChessMaterialAssets::CAPTABLESQUARE;

				if (moves[i]->Flags() == Flag::EnPassantCapture) tileState = EChessMaterialAssets::CAPTABLESQUARE;

				if (moves[i]->Flags() == Flag::CantMove) tileState = EChessMaterialAssets::IMMOVABLESQUARE;

				TileArray[targetSquare]->MeshComp->SetMaterial(0, Materials[tileState]);
				TileArray[targetSquare]->CurrentMaterial = Materials[tileState];
			}
			TileArray[targetSquare]->MeshComp->SetScalarParameterValueOnMaterials(FName(TEXT("ColorBrightness")), squareColorParam);
			TileArray[targetSquare]->colorBrightness = squareColorParam;
		}
	}
}


