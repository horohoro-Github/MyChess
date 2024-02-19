// Fill out your copyright notice in the Description page of Project Settings.

#include "MyBoard.h"
#include "MyTile.h"
#include "../Game/MyChess_GameModeBase.h"
#include "MyChessPiece.h"
#include "MyPieceList.h"
#include "MyMoveCalculator.h"
#include "MyMove.h"
#include "../Game/MyGameStateBase.h"
#include "../Player/MyPlayerState.h"
#include "../AI/MyAI.h"
#include "../Chess/MyZobristKey.h"
#include "MyPreComputedMoveData.h"
#include "../AI/MyMoveOrdering.h"
#include "Net/UnrealNetwork.h"
#include "../Etc/MyDecal.h"
#include "../Etc/MyReferee.h"
#include "../Game/MyChess_GameModeBase_Multiplay.h"

AMyBoard::AMyBoard()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	bAlwaysRelevant = true;
	Meshes.SetNum(7);
	TileArray.SetNum(64);
	Materials.SetNum(6);

	struct FConstructAsset
	{
		//스퀘어
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> SM_Square;

		//체스피스
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> SM_Pawn;
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> SM_Knight;
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> SM_Bishop;
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> SM_Rook;
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> SM_Queen;
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> SM_King;
		//기물 머티리얼

		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> M_ChessPiece;

		//LightSquare
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> M_Square;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> M_MovableSquare;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> M_CaptableSquare;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> M_IMMovableSquare;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> M_CastlingSquare;

		FConstructAsset()
			
			: SM_Square(TEXT("StaticMesh'/Game/Meshes/CubeDefault.CubeDefault'"))
			, SM_Pawn(TEXT("StaticMesh'/Game/Meshes/ChessPieces/chess_pieces_Pawn.chess_pieces_Pawn'"))
			, SM_Knight(TEXT("StaticMesh'/Game/Meshes/ChessPieces/chess_pieces_Knight.chess_pieces_Knight'"))
			, SM_Bishop(TEXT("StaticMesh'/Game/Meshes/ChessPieces/chess_pieces_Bishop.chess_pieces_Bishop'"))
			, SM_Rook(TEXT("StaticMesh'/Game/Meshes/ChessPieces/chess_pieces_Rook.chess_pieces_Rook'"))
			, SM_Queen(TEXT("StaticMesh'/Game/Meshes/ChessPieces/chess_pieces_Queen.chess_pieces_Queen'"))
			, SM_King(TEXT("StaticMesh'/Game/Meshes/ChessPieces/chess_pieces_King.chess_pieces_King'"))

			, M_ChessPiece(TEXT("MaterialInstanceConstant'/Game/Materials/MaterialsInstance/M_ChessPiece_Inst.M_ChessPiece_Inst'"))

			, M_Square(TEXT("MaterialInstanceConstant'/Game/Materials/MaterialsInstance/M_Field_Inst.M_Field_Inst'"))
			, M_MovableSquare(TEXT("MaterialInstanceConstant'/Game/Materials/MaterialsInstance/M_Green_Inst.M_Green_Inst'"))
			, M_CaptableSquare(TEXT("MaterialInstanceConstant'/Game/Materials/MaterialsInstance/M_Yellow_Inst.M_Yellow_Inst'"))
			, M_IMMovableSquare(TEXT("MaterialInstanceConstant'/Game/Materials/MaterialsInstance/M_Red_Inst.M_Red_Inst'"))
			, M_CastlingSquare(TEXT("MaterialInstanceConstant'/Game/Materials/MaterialsInstance/M_Purple_Inst.M_Purple_Inst'"))
		{}
	};

	static FConstructAsset ConstructAssets;
	
	Meshes = { ConstructAssets.SM_Square.Get(), ConstructAssets.SM_Pawn.Get(), ConstructAssets.SM_Knight.Get(),
			   ConstructAssets.SM_Bishop.Get(), ConstructAssets.SM_Rook.Get(), ConstructAssets.SM_Queen.Get(), ConstructAssets.SM_King.Get() };
	Materials = { ConstructAssets.M_ChessPiece.Get(), ConstructAssets.M_Square.Get(), ConstructAssets.M_MovableSquare.Get(),
				  ConstructAssets.M_CaptableSquare.Get(), ConstructAssets.M_IMMovableSquare.Get(), ConstructAssets.M_CastlingSquare.Get() };
}

void AMyBoard::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	if (moveCal.IsValid())
	{
		if (moveCal.Get()->IsValidLowLevel())
		{
			moveCal.Get()->ConditionalBeginDestroy();
			moveCal = nullptr;
		}
	}
}

bool AMyBoard::SpawnBoard()
{
	TArray<int> PiecesPosition = BoardInit(false);
	
	FActorSpawnParameters FieldSpawnParams;
	
	FString fileName = "ABCDEFGH";
	FString rankName = "12345678";
	for (int rank=0; rank < 8; rank++)
	{
		for (int file = 0; file < 8; file++)
		{
			AMyTile* tile = Cast<AMyTile>(GetWorld()->SpawnActor<AActor>(AMyTile::StaticClass(), FVector(file * -100.f, rank * 100.f, 0.f), FRotator(0.f, 0.f, 0.f), FieldSpawnParams));
			tile->MeshComp->SetStaticMesh(Meshes[EChessMeshAssets::SQUARE]);
			tile->CurrentMesh = Meshes[EChessMeshAssets::SQUARE];
			tile->File = file;
			tile->Rank = rank;
			float odd = (file + rank) % 2;

			tile->MeshComp->SetMaterial(0, Materials[EChessMaterialAssets::EMPTYSQUARE]);
			tile->CurrentMaterial = Materials[EChessMaterialAssets::EMPTYSQUARE];
			tile->MeshComp->SetScalarParameterValueOnMaterials(FName(TEXT("ColorBrightness")), odd);
	
			tile->colorBrightness = odd;
		
			TileArray[file + rank * 8] = tile;


			AMyDecal* decal = Cast<AMyDecal>(GetWorld()->SpawnActor<AActor>(AMyDecal::StaticClass(), FVector(file * -100.f, rank * 100.f, 50.f), FRotator(0.f, 0.f, 0.f), FieldSpawnParams));
			decals.Add(decal);
		}
	}
	
	return SpawnPiece(PiecesPosition);
}

bool AMyBoard::SpawnPiece(TArray<int> SpawningPosition)
{
	FActorSpawnParameters pieceSpawnParams;
	FString zobristStr = "";
	AMyChess_GameModeBase* gameMode = Cast<AMyChess_GameModeBase>(GetWorld()->GetAuthGameMode());
	if (gameMode)
	{
		zobristStr = gameMode->ZobristHash();
	}
	ZobristKey->SetupZobristHash(zobristStr);

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
			PieceArray[i] = piece;
			OriginPiecePosition[i] = piece;
			piece->File = file;
			piece->Rank = rank;
			piece->TeamColor = color;
			int type = SpawningPosition[i] & 0b111;
			if (type != 0)
			{
				piece->MeshComp->SetStaticMesh(Meshes[type]);
				piece->CurrentMesh = Meshes[type];

				if (type == FChessPiece::King)
				{
					KingSquare1D[color] = file + rank * 8;
				}
				else
				{
					GetPieceList(type, color)->AddPiece(i);
				}

				piece->MeshComp->SetMaterial(0, Materials[EChessMaterialAssets::CHESSPIECE]);
				piece->CurrentMaterial = Materials[EChessMaterialAssets::CHESSPIECE];
				piece->MeshComp->SetScalarParameterValueOnMaterials(FName(TEXT("TeamColor")), (float)color);
				piece->teamColorParameter = color;
			}
		}
	}

	Zobrist = ZobristKey->CalculateZobristKey(this);

	preMove = new MyPreComputedMoveData();
	moveCal = MakeShareable(NewObject<UMyMoveCalculator>(GetWorld(), UMyMoveCalculator::StaticClass()));
	return true;
}

MyPieceList* AMyBoard::GetPieceList(int type, int color)
{
	return allPieces[type + 8 * color];
}

void AMyBoard::SelectedPiece(AMyChessPiece* piece)
{
	if (piece)
	{
		int file = piece->File;
		int rank = piece->Rank;
		int square = file + rank * 8;
		int type = Square[file][rank] & 0b111;
		if (canMoves.IsEmpty())
		{
			TArray<MyMove*> moves = moveCal->GenerateMove(this, true, file + rank * 8, type, true);
			for (int i = 0; i < moves.Num(); i++)
			{
				if (moves[i]->StartSquare() == square)
				{
					canMoves.Add(moves[i]);
				}
			}

			WayPointingSquare(canMoves);	
			if (!canMoves.IsEmpty()) piece->Highlight(true);
		}
		else
		{
			ClearSquare();
			
			if (canMoves[0]->StartSquare() != file + rank * 8)
			{
				canMoves.Empty();
				TArray<MyMove*> moves = moveCal->GenerateMove(this, true, file + rank * 8, type, true);
				for (int i = 0; i < moves.Num(); i++)
				{
					if (moves[i]->StartSquare() == square)
					{
						canMoves.Add(moves[i]);
					}
				}
				WayPointingSquare(canMoves);
				piece->Highlight(true);
			}
			else
			{
				canMoves.Empty();
			}
		}
		
	}
}

void AMyBoard::WayPointingSquare(TArray<MyMove*> moves)
{
	for (int i = 0; i < moves.Num(); i++)
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

void AMyBoard::SelectedSquare(AMyTile* tile)
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
		if((Square[file][rank] & 0b111) != 0 && selectedMove->MoveValue() != 0 && selectedMove->Flags() != Flag::CantMove)
		{
			ClearSquare();
			
			int newFile = selectedMove->TargetSquare() & 0b111;
			int newRank = selectedMove->TargetSquare() >> 3;
			int flag = selectedMove->Flags();

			if (PieceArray[selectedMove->StartSquare()] != NULL)
			{
				for (int i = 0; i < 64; i++)
				{
					UpdateRoute(i, EDecalState::Nomal);
				}
				UpdateRoute(selectedMove->StartSquare(), EDecalState::StartPoint);

				PieceArray[selectedMove->StartSquare()]->File = newFile;
				PieceArray[selectedMove->StartSquare()]->Rank = newRank;
				if (PieceArray[selectedMove->TargetSquare()] != NULL)
				{
					Capture(PieceArray[selectedMove->TargetSquare()]);
				}

				if (flag == Flag::EnPassantCapture)
				{
					int enpassantOffset = teamColors == FTeamColor::White ? -8 : 8;
					Capture(PieceArray[selectedMove->TargetSquare() + enpassantOffset]);

				}

				if (flag == Flag::Castling)
				{
					if (newFile == 2) // queenside
					{
						PieceArray[newFile - 2 + newRank * 8]->File = newFile + 1;
						PieceArray[newFile - 2 + newRank * 8]->Rank = newRank;
						PieceArray[newFile - 2 + newRank * 8]->MoveLocation(TileArray[newFile + 1 + newRank * 8]->GetActorLocation() + FVector(0.f, 0.f, 50.f), false);
						PieceArray[newFile + 1 + newRank * 8] = PieceArray[newFile - 2 + newRank * 8];
						PieceArray[newFile - 2 + newRank * 8] = 0;
					}
					if (newFile == 6) // kingside
					{
						PieceArray[newFile + 1 + newRank * 8]->File = newFile - 1;
						PieceArray[newFile + 1 + newRank * 8]->Rank = newRank;
						PieceArray[newFile + 1 + newRank * 8]->MoveLocation(TileArray[newFile - 1 + newRank * 8]->GetActorLocation() + FVector(0.f, 0.f, 50.f), false);
						PieceArray[newFile - 1 + newRank * 8] = PieceArray[newFile + 1 + newRank * 8];
						PieceArray[newFile + 1 + newRank * 8] = 0;
					}
				}
		
				PieceArray[selectedMove->StartSquare()]->MoveLocation(TileArray[selectedMove->TargetSquare()]->GetActorLocation() + FVector(0.f, 0.f, 50.f), true);
				PieceArray[selectedMove->TargetSquare()] = PieceArray[selectedMove->StartSquare()];
				PieceArray[selectedMove->StartSquare()] = NULL;
				if (flag >= Flag::PromoteToKnight && flag <= Flag::PromoteToQueen)
				{
					promotionTarget.startSquare = selectedMove->StartSquare();
					promotionTarget.targetSquare = selectedMove->TargetSquare();
					gameState->playState = EPlayState::Promotion;
				}
				else
				{
					MakeMoves(selectedMove, false);
				}
		
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Warning Error %d"), selectedMove->StartSquare());
			}

			canMoves.Empty();
		}
	}
}

void AMyBoard::EmphasizeSquare(AMyTile* tile, bool emphasized)
{
	if (tile)
	{
		float ScalarParam = emphasized ? 0.7f : 1.f;
		tile->EPIntensity = ScalarParam;
		tile->MeshComp->SetScalarParameterValueOnMaterials(FName("Emphasize"), ScalarParam);
	}
}

void AMyBoard::ClearSquare()
{
	for (int i = 0; i < 64; i++)
	{
		int file = i & 0b111;
		int rank = i >> 3;
		float index = (file % 2 + rank % 2) % 2 == 0 ? 0.f : 1.f;
		TileArray[i]->MeshComp->SetMaterial(0, Materials[EChessMaterialAssets::EMPTYSQUARE]);
		TileArray[i]->CurrentMaterial = Materials[EChessMaterialAssets::EMPTYSQUARE];
		TileArray[i]->MeshComp->SetScalarParameterValueOnMaterials(FName(TEXT("ColorBrightness")), index);
		TileArray[i]->colorBrightness = index;
		if(PieceArray[i] != NULL) PieceArray[i]->Highlight(false);
	}
}


void AMyBoard::Capture(AMyChessPiece* piece)
{
	int color = piece->TeamColor;
	int colorIndex = color == 0 ? 1 : -1;
	for (int i = 0; i < DeadPieceArray[color].Num(); i++)
	{
		if (DeadPieceArray[color][i] == NULL)
		{
			DeadPieceArray[color][i] = piece;
			piece->bDead = true;
			piece->route = false;
			piece->bMove = false;
			piece->SetActorLocation(FVector((i & 0b111) * -100.f, (-1000.f + 2700.f * color + 100.f * (i >> 3) * colorIndex), 50.f));
			break;
		}
	}
}

void AMyBoard::ResetPiece()
{
	TArray<int> PositionValue = BoardInit(true);
	for (int position = 0; position < PositionValue.Num(); position++)
	{
		if (PositionValue[position] != 0)
		{
			if (OriginPiecePosition[position] != NULL)
			{
				int file = position & 0b111;
				int rank = position >> 3;

				Square[file][rank] = PositionValue[position];
				int type = Square[file][rank] & 0b111;
				int color = Square[file][rank] >> 3;
				PieceArray[position] = OriginPiecePosition[position];
				PieceArray[position]->File = file;
				PieceArray[position]->Rank = rank;
				PieceArray[position]->bDead = false;
				PieceArray[position]->bMove = false;
				PieceArray[position]->route = false;
				FRotator rotation = color == 0 ? FRotator() : FRotator(0.f, 180.f, 0.f);
				PieceArray[position]->SetActorLocationAndRotation(FVector(-100.f * file, 100.f * rank, 50.f), rotation);
				PieceArray[position]->MeshComp->SetStaticMesh(Meshes[type]);
				PieceArray[position]->CurrentMesh = Meshes[type];
				PieceArray[position]->MeshComp->SetScalarParameterValueOnMaterials(FName(TEXT("TeamColor")), (float)color);
				PieceArray[position]->teamColorParameter = color;
				if (type == FChessPiece::King)
				{
					KingSquare1D[color] = position;
				}
				else
				{
					GetPieceList(type, color)->AddPiece(position);
				}
			}

		}
	}
			
}

void AMyBoard::ChessPromotion(int index, bool bAI)
{
	int StartSquare = promotionTarget.startSquare;
	int TargetSquare = promotionTarget.targetSquare;


	if (index != FChessPiece::Pawn && PieceArray[TargetSquare] != NULL)
	{
		if (index != FChessPiece::None)
		{
			PieceArray[TargetSquare]->MeshComp->SetStaticMesh(Meshes[index]);
			PieceArray[TargetSquare]->CurrentMesh = Meshes[index];
			PieceArray[TargetSquare]->MeshComp->SetScalarParameterValueOnMaterials(FName("TeamColor"), (float)teamColors);
			PieceArray[TargetSquare]->teamColorParameter = (float)teamColors;

		}
		MyMove* promotedMove = new MyMove(StartSquare, TargetSquare, index);
		gameState->playState = EPlayState::Playing;
		MakeMoves(promotedMove, false);
	}
}

TArray<int> AMyBoard::BoardInit(bool alreadyCreated)
{
	if (alreadyCreated)
	{
		allPieces.Empty();
		PieceArray.Empty();
		DeadPieceArray.Empty();
		KingSquare1D.Empty();
		for (int i = 0; i < 64; i++)
		{
			UpdateRoute(i, EDecalState::Nomal);
		}
	}
	else
	{
		ZobristKey = NewObject<UMyZobristKey>(this, UMyZobristKey::StaticClass());

		
	}
	KingSquare1D.SetNum(2);
	
	allPieces.SetNum(14);
	for (int i = 0; i < allPieces.Num(); i++)
	{
		allPieces[i] = new MyPieceList();
	}
	PieceArray.SetNum(64);
	OriginPiecePosition.SetNum(64);
	TArray<AMyChessPiece*> deadPieces;
	deadPieces.Init(NULL, 16);
	DeadPieceArray.Init(deadPieces, 2);
	TArray<int> SquareArr;
	SquareArr.SetNum(8);
	Square.Init(SquareArr, 8);

	playCount = 0;
	fiftyMoves = 0;
	ZobristHistory.Empty();
	gameStateHistory.Empty();
	currentState = 0b1111;
	gameStateHistory.Push(currentState);
	gameState->gameHistory.Empty();
	gameState->gameHistory.Push(currentState);
	Zobrist = ZobristKey->CalculateZobristKey(this);

	gameState->GameEndingText = "";
	gameState->WinnerText = "";


	FString returnString = "";
	TArray<int> PositionString;
	AMyChess_GameModeBase* gameMode = Cast<AMyChess_GameModeBase>(GetWorld()->GetAuthGameMode());
	if (gameMode)
	{
		returnString = gameMode->BoardPositionStrings();

	}
	FString tempString = "";
	for (int i = 0; i < returnString.Len(); i++)
	{
		if (returnString[i] == ',')
		{
			PositionString.Add(FCString::Atoi(*tempString));
			tempString = "";
		}
		else
		{
			if (returnString[i] == '/')
			{
				PositionString.Add(0);
				tempString = "";
			}
			else
			{
				tempString += returnString[i];
			}
		}

		if (i == returnString.Len() - 1)
		{
			PositionString.Add(FCString::Atoi(*tempString));
		}
	}

	return PositionString;
}

int AMyBoard::GetTeamColor()
{
	return teamColors;
}

void AMyBoard::UpdateRoute(int square, EDecalState decalState)
{
	if (decals.Num() > square)
	{
		decals[square]->ApplyDecal(decalState);
	}
}

void AMyBoard::MakeMoves(MyMove* move, bool search)
{

	uint64 oldEnPassantFile = (currentState >> 4) & 15;
	uint64 originalCastleState = currentState & 15;
	uint64 newCastleState = originalCastleState;
	currentState = 0;

	int opponentColourIndex = 1 - teamColors;
	int moveFrom = move->StartSquare();
	int moveTo = move->TargetSquare();

	int capturedPieceType = Square[moveTo % 8][moveTo / 8] & 0b111;
	int movePiece = Square[moveFrom % 8][moveFrom / 8];
	int movePieceType = movePiece & 0b111;

	int moveFlag = move->Flags();
	bool isPromotion = moveFlag >= Flag::PromoteToKnight && moveFlag <= Flag::PromoteToQueen;
	bool isEnPassant = moveFlag == Flag::EnPassantCapture;

	currentState |= ((uint64)capturedPieceType << 8);
	if (capturedPieceType != 0 && !isEnPassant)
	{
		Zobrist ^= ZobristKey->ALLPieces[capturedPieceType][opponentColourIndex][moveTo];
		GetPieceList(capturedPieceType, opponentColourIndex)->RemovePiece(moveTo);
	}

	if (movePieceType == FChessPiece::King)
	{
		KingSquare1D[teamColors] = moveTo;
		newCastleState &= (teamColors == FTeamColor::White) ? whiteCastleMask : blackCastleMask;
	}
	else
	{
		GetPieceList(movePieceType, teamColors)->MovePiece(moveFrom, moveTo);
	}

	int pieceOnTargetSquare = movePiece;

	if (isPromotion)
	{
		int promoteType = 0;
		switch (moveFlag)
		{
		case Flag::PromoteToQueen:
			promoteType = FChessPiece::Queen;
			GetPieceList(FChessPiece::Queen, teamColors)->AddPiece(moveTo);
			break;
		case Flag::PromoteToRook:
			promoteType = FChessPiece::Rook;
			GetPieceList(FChessPiece::Rook, teamColors)->AddPiece(moveTo);
			break;
		case Flag::PromoteToBishop:
			promoteType = FChessPiece::Bishop;
			GetPieceList(FChessPiece::Bishop, teamColors)->AddPiece(moveTo);
			break;
		case Flag::PromoteToKnight:
			promoteType = FChessPiece::Knight;
			GetPieceList(FChessPiece::Knight, teamColors)->AddPiece(moveTo);
			break;

		}
		pieceOnTargetSquare = promoteType | (teamColors << 3);
		GetPieceList(FChessPiece::Pawn, teamColors)->RemovePiece(moveTo);
	}
	else
	{
		switch (moveFlag)
		{
		case Flag::EnPassantCapture:
		{


			int epPawnSquare = moveTo + ((teamColors == FTeamColor::White) ? -8 : 8);
			currentState |= (((uint64)Square[epPawnSquare % 8][epPawnSquare / 8] & 0b111) << 8);
			Square[epPawnSquare % 8][epPawnSquare / 8] = 0;
			GetPieceList(FChessPiece::Pawn, 1 - teamColors)->RemovePiece(epPawnSquare);
			Zobrist ^= ZobristKey->ALLPieces[FChessPiece::Pawn][opponentColourIndex][epPawnSquare];
			break;
		}
		case Flag::Castling:
		{
			bool kingside = moveTo == 6 || moveTo == 62;
			int castlingRookFromIndex = (kingside) ? moveTo + 1 : moveTo - 2;
			int castlingRookToIndex = (kingside) ? moveTo - 1 : moveTo + 1;

			Square[castlingRookFromIndex % 8][castlingRookFromIndex / 8] = FChessPiece::None;
			Square[castlingRookToIndex % 8][castlingRookToIndex / 8] = FChessPiece::Rook | (teamColors << 3);

			GetPieceList(FChessPiece::Rook, teamColors)->MovePiece(castlingRookFromIndex, castlingRookToIndex);
			Zobrist ^= ZobristKey->ALLPieces[FChessPiece::Rook][teamColors][castlingRookFromIndex];
			Zobrist ^= ZobristKey->ALLPieces[FChessPiece::Rook][teamColors][castlingRookToIndex];
			break;
		}
		}
	}

	Square[moveTo % 8][moveTo / 8] = pieceOnTargetSquare;
	Square[moveFrom % 8][moveFrom / 8] = 0;

	if (moveFlag == Flag::PawnTwoForward)
	{
		int file = (moveFrom & 0b111) + 1;
		currentState |= ((uint64)file << 4);
		Zobrist ^= ZobristKey->Enpassants[file];
	}

	if (originalCastleState != 0)
	{
		if (moveTo == 7 || moveFrom == 7)
		{
			newCastleState &= whiteCastleKingsideMask;
		}
		else if (moveTo == 0 || moveFrom == 0)
		{
			newCastleState &= whiteCastleQueensideMask;
		}
		if (moveTo == 63 || moveFrom == 63)
		{
			newCastleState &= blackCastleKingsideMask;
		}
		else if (moveTo == 56 || moveFrom == 56)
		{
			newCastleState &= blackCastleQueensideMask;
		}
	}

	Zobrist ^= ZobristKey->move;
	Zobrist ^= ZobristKey->ALLPieces[movePieceType][teamColors][moveFrom];
	Zobrist ^= ZobristKey->ALLPieces[pieceOnTargetSquare & 0b111][teamColors][moveTo];

	if (oldEnPassantFile != 0)
		Zobrist ^= ZobristKey->Enpassants[oldEnPassantFile];

	if (newCastleState != originalCastleState)
	{
		Zobrist ^= ZobristKey->Castlings[originalCastleState]; 
		Zobrist ^= ZobristKey->Castlings[newCastleState];
	}
	currentState |= newCastleState;
	currentState |= (uint64)fiftyMoves << 12;

	currentState |= (uint64)move->MoveValue() << 18;
	currentState |= (uint64)movePiece << 34;

	teamColors = 1 - teamColors;
	gameState->TeamColor = 1 - gameState->TeamColor;
	playCount++;
	fiftyMoves++;
	if (!search)
	{
		TArray<MyMove*> moves = moveCal->GenerateMove(this, true, -1, FChessPiece::All);

		if (moveCal->check)
		{
			currentState |= 1ull << 41;
			gameState->Check = true;
			UE_LOG(LogTemp, Warning, TEXT("Check"));
		}
		else
		{
			gameState->Check = false;
		}
		if (moves.Num() == 0)
		{
			if (moveCal->check)
			{
				currentState |= 1ull << 42;
				UE_LOG(LogTemp, Warning, TEXT("CheckMate"));
				gameState->playState = EPlayState::GameOver;
				gameState->GameEndingText = "CheckMate";
				if (teamColors == FTeamColor::White)
					gameState->WinnerText = "Black Win";
				else
					gameState->WinnerText = "White Win";
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("StaleMate"));
				gameState->playState = EPlayState::GameOver;
				gameState->GameEndingText = "StaleMate";
				gameState->WinnerText = "Draw";
			}
		}

		int sameSquare = 0;
		bool existedOtherPieces = false;
		if (movePieceType >= FChessPiece::Knight && movePieceType <= FChessPiece::Queen)
		{
			for (int i = 0; i < moveCal->moveBitBoard[movePieceType][moveTo].Num(); i++)
			{
				int s = moveCal->moveBitBoard[movePieceType][moveTo][i];
				if (s != moveFrom)
				{
					int f = s & 0b111;
					int r = s >> 3;

					int existingFile = moveTo & 0b111;
					int existingRank = moveTo >> 3;

					if (f == existingFile) sameSquare |= 0b01;
					if (r == existingRank) sameSquare |= 0b10;

					existedOtherPieces = true;
				}

				if (i == moveCal->moveBitBoard[movePieceType][moveTo].Num() - 1 && sameSquare == 0 && existedOtherPieces)
				{
					sameSquare |= 0b01;
				}
			}
		}


		currentState |= (uint64)sameSquare << 39;
		if (movePieceType == FChessPiece::Pawn || capturedPieceType != FChessPiece::None)
		{
			repetitionHistory.Empty();

			fiftyMoves = 0;
		}
		else
		{
			repetitionHistory.Push(Zobrist);
		}

		if (gameState->playState != EPlayState::GameOver)
		{
			if (repetitionHistory.Contains(Zobrist))
			{
				int sum = 0;
				for (int i = 0; i < repetitionHistory.Num(); i++)
				{
					if (repetitionHistory[i] == Zobrist)
					{
						sum++;
						if (sum == 3)
						{
							gameState->playState = EPlayState::GameOver;
							gameState->GameEndingText = "Threefold Repetition";
							gameState->WinnerText = "Draw";
							UE_LOG(LogTemp, Warning, TEXT("Threefold Repetition"));

							break;
						}
					}
				}
			}
		}

		if (gameState->playState != EPlayState::GameOver)
		{
			if (fiftyMoves == 50)
			{
				gameState->playState = EPlayState::GameOver;
				gameState->GameEndingText = "FiftyMove Rules";
				gameState->WinnerText = "Draw";
				UE_LOG(LogTemp, Warning, TEXT("FiftyMove Rules"));
			}
		}
		if (gameState->playState != EPlayState::GameOver)
		{
			if (capturedPieceType != 0 || (moveFlag <= Flag::PromoteToBishop && moveFlag >= Flag::PromoteToKnight))
			{
				int MajorPieces = 0;
				int pawnMaterial = GetPieceList(FChessPiece::Pawn, FTeamColor::White)->pieceNum + GetPieceList(FChessPiece::Pawn, FTeamColor::Black)->pieceNum;
				int WhiteKnightMaterial = GetPieceList(FChessPiece::Knight, FTeamColor::White)->pieceNum;
				int BlackKnightMaterial = GetPieceList(FChessPiece::Knight, FTeamColor::Black)->pieceNum;

				MyPieceList* List_WhiteBishop = GetPieceList(FChessPiece::Bishop, FTeamColor::White);
				MyPieceList* List_BlackBishop = GetPieceList(FChessPiece::Bishop, FTeamColor::Black);

				int RookMaterial = GetPieceList(FChessPiece::Rook, FTeamColor::White)->pieceNum + GetPieceList(FChessPiece::Rook, FTeamColor::Black)->pieceNum;
				int QueenMaterial = GetPieceList(FChessPiece::Queen, FTeamColor::White)->pieceNum + GetPieceList(FChessPiece::Queen, FTeamColor::Black)->pieceNum;
				MajorPieces = pawnMaterial + RookMaterial + QueenMaterial;
				if (MajorPieces == 0)
				{
					int WhiteMinorPieces = 0;
					int BlackMinorPieces = 0;
					WhiteMinorPieces = List_WhiteBishop->pieceNum + WhiteKnightMaterial;
					BlackMinorPieces = List_BlackBishop->pieceNum + BlackKnightMaterial;

					bool InsufficientMaterial = false;
					if (WhiteMinorPieces < 2 && BlackMinorPieces < 2)
					{
						InsufficientMaterial = true;
					}
					else if((WhiteMinorPieces == 2 && BlackMinorPieces < 2) || (WhiteMinorPieces < 2 && BlackMinorPieces == 2))
					{
						if (WhiteKnightMaterial == 2 || BlackKnightMaterial == 2)
						{
							InsufficientMaterial = true;
						}
						if (List_WhiteBishop->pieceNum == 2)
						{
							bool WhiteslightSquare = false, WhitesDarkSquare = false;

							for (int i = 0; i < List_WhiteBishop->pieceNum; i++)
							{
								int file = List_WhiteBishop->PiecePosition[i] & 0b111;
								int rank = List_WhiteBishop->PiecePosition[i] >> 3;
								
								int odd = rank % 2;
								int lightSquare = (odd + file) % 2;
								if (lightSquare == 1) WhiteslightSquare = true; else WhitesDarkSquare = true;
							}
							if(!(WhiteslightSquare && WhitesDarkSquare)) InsufficientMaterial = true;;
						}
						if (List_BlackBishop->pieceNum == 2)
						{
							bool BlackslightSquare = false, BlacksDarkSquare = false;

							for (int i = 0; i < List_BlackBishop->pieceNum; i++)
							{
								int file = List_BlackBishop->PiecePosition[i] & 0b111;
								int rank = List_BlackBishop->PiecePosition[i] >> 3;

								int odd = rank % 2;
								int lightSquare = (odd + file) % 2;
								if (lightSquare == 1) BlackslightSquare = true; else BlacksDarkSquare = true;
							}
							if (!(BlackslightSquare && BlacksDarkSquare)) InsufficientMaterial = true;;
						}
					}

					if (InsufficientMaterial)
					{
						gameState->playState = EPlayState::GameOver;
						gameState->GameEndingText = "Insufficient Material";
						gameState->WinnerText = "Draw";
						UE_LOG(LogTemp, Warning, TEXT("Insufficient Material"));
						
					}
				}
			}
			
		}

		if (gameState->playMode == EPlayMode::MultiPlay && gameState->playState == EPlayState::GameOver)
		{
			AMyChess_GameModeBase_Multiplay* mode = Cast<AMyChess_GameModeBase_Multiplay>(GetWorld()->GetAuthGameMode());
			AMyPlayerState* PS1 = Cast<AMyPlayerState>(gameState->MatchPlayer[FTeamColor::White]);
			AMyPlayerState* PS2 = Cast<AMyPlayerState>(gameState->MatchPlayer[FTeamColor::Black]);


			if (gameState->WinnerText == "Draw")
			{
				if (PS1) mode->GameResult(PS1, PS1->userdb.userid, EGameResult::Draw, false);
				if (PS2) mode->GameResult(PS2, PS2->userdb.userid, EGameResult::Draw, false);
			}
			else
			{
				if (gameState->TeamColor == FTeamColor::White)
				{
					if (PS1) mode->GameResult(PS1, PS1->userdb.userid, EGameResult::Lose, false);
					if (PS2) mode->GameResult(PS2, PS2->userdb.userid, EGameResult::Win, false);
				}
				else
				{
					if (PS1) mode->GameResult(PS1, PS1->userdb.userid, EGameResult::Win, false);
					if (PS2) mode->GameResult(PS2, PS2->userdb.userid, EGameResult::Lose, false);
				}
			}
		}
		gameState->gameHistory.Push(currentState);
	}
	gameStateHistory.Push(currentState);

	if (!search) gameState->UpdateHistory(FTeamColor::Neutral);

	if (!search && gameState->playState != EPlayState::GameOver)
	{
		if (gameState->playMode == EPlayMode::AIPlay)
		{
			if (teamColors == aiPlayer->AITeamColor)
			{
				FTimerHandle aiHandle;
				GetWorld()->GetTimerManager().SetTimer(aiHandle, aiPlayer, &AMyAI::StartSearch, 0.2f, false);
			}
		}
	}
}
void AMyBoard::UnMakeMoves(MyMove* move, bool inSearch)
{
	int opponentColourIndex = teamColors;
	bool undoingWhiteMove = (1 - teamColors) == FTeamColor::White;
	teamColors = 1 - teamColors;
	gameState->TeamColor = 1 - gameState->TeamColor;
	uint64 originalCastleState = currentState & 0b1111;

	int capturedPieceType = ((int)currentState >> 8) & 0b111;
	int capturedPiece = (capturedPieceType == 0) ? 0 : capturedPieceType | ((1 - teamColors) << 3);

	int movedFrom = move->StartSquare();
	int movedTo = move->TargetSquare();
	int moveFlags = move->Flags();
	bool isEnPassant = moveFlags == Flag::EnPassantCapture;
	bool isPromotion = moveFlags >= Flag::PromoteToKnight && moveFlags <= Flag::PromoteToQueen;

	int toSquarePieceType = (Square[movedTo % 8][movedTo / 8] & 0b111);
	int movedPieceType = (isPromotion) ? FChessPiece::Pawn : toSquarePieceType;

	Zobrist ^= ZobristKey->move;
	Zobrist ^= ZobristKey->ALLPieces[movedPieceType][teamColors][movedFrom];
	Zobrist ^= ZobristKey->ALLPieces[toSquarePieceType][teamColors][movedTo];

	uint64 oldEnPassantFile = (currentState >> 4) & 15;
	if (oldEnPassantFile != 0)
		Zobrist ^= ZobristKey->Enpassants[oldEnPassantFile];
	if (capturedPieceType != 0 && !isEnPassant)
	{
		Zobrist ^= ZobristKey->ALLPieces[capturedPieceType][opponentColourIndex][movedTo];
		GetPieceList(capturedPieceType, 1 - teamColors)->AddPiece(movedTo);
	}
	if (movedPieceType == FChessPiece::King)
	{
		KingSquare1D[teamColors] = movedFrom;
	}
	else if (!isPromotion)
	{
		GetPieceList(movedPieceType, teamColors)->MovePiece(movedTo, movedFrom);
	}
	Square[movedFrom % 8][movedFrom / 8] = movedPieceType | (teamColors << 3);
	Square[movedTo % 8][movedTo / 8] = capturedPiece;

	if (isPromotion)
	{
		GetPieceList(FChessPiece::Pawn, teamColors)->AddPiece(movedFrom);
		switch (moveFlags)
		{
		case Flag::PromoteToQueen:
			GetPieceList(FChessPiece::Queen, teamColors)->RemovePiece(movedTo);
			break;
		case Flag::PromoteToKnight:
			GetPieceList(FChessPiece::Knight, teamColors)->RemovePiece(movedTo);
			break;
		case Flag::PromoteToRook:
			GetPieceList(FChessPiece::Rook, teamColors)->RemovePiece(movedTo);
			break;
		case Flag::PromoteToBishop:
			GetPieceList(FChessPiece::Bishop, teamColors)->RemovePiece(movedTo);
			break;
		}
	}
	else if (isEnPassant)
	{
		int epIndex = movedTo + ((teamColors == FTeamColor::White) ? -8 : 8);
		Square[movedTo % 8][movedTo / 8] = 0;
		Square[epIndex % 8][epIndex / 8] = (int)capturedPiece;
		GetPieceList(FChessPiece::Pawn, 1 - teamColors)->AddPiece(epIndex);
		Zobrist ^= ZobristKey->ALLPieces[FChessPiece::Pawn][1 - teamColors][epIndex];
	}
	else if (moveFlags == Flag::Castling)
	{
		bool kingside = movedTo == 6 || movedTo == 62;
		int castlingRookFromIndex = (kingside) ? movedTo + 1 : movedTo - 2;
		int castlingRookToIndex = (kingside) ? movedTo - 1 : movedTo + 1;

		Square[castlingRookToIndex % 8][castlingRookToIndex / 8] = 0;
		Square[castlingRookFromIndex % 8][castlingRookFromIndex / 8] = FChessPiece::Rook | (teamColors << 3);

		GetPieceList(FChessPiece::Rook, teamColors)->MovePiece(castlingRookToIndex, castlingRookFromIndex);
		Zobrist ^= ZobristKey->ALLPieces[FChessPiece::Rook][teamColors][castlingRookFromIndex];
		Zobrist ^= ZobristKey->ALLPieces[FChessPiece::Rook][teamColors][castlingRookToIndex];

	}

	gameStateHistory.Pop(); 
	currentState = gameStateHistory.Last();

	fiftyMoves = (int)(currentState >> 12) & 0b111111;
	int newEnPassantFile = (int)(currentState >> 4) & 15;
	if (newEnPassantFile != 0)
		Zobrist ^= ZobristKey->Enpassants[newEnPassantFile];

	uint64 newCastleState = currentState & 0b1111;
	if (newCastleState != originalCastleState)
	{
		Zobrist ^= ZobristKey->Castlings[originalCastleState];
		Zobrist ^= ZobristKey->Castlings[newCastleState];
	}
	playCount--;

	if (!inSearch && repetitionHistory.Num() > 0)
	{
		repetitionHistory.Pop();
	}
}