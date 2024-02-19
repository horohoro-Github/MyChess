// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../MyChess_.h"
#include "MyBoard.generated.h"

// 00111 기물색상 0, 기물타입 1

class AMyGameStateBase;
class AMyTile;
class AMyChessPiece;
class MyPieceList;
class MyMove;
class UMyZobristKey;
class AMyAI;
class MyPreComputedMoveData;
class UMyMoveCalculator;
class AMyDecal;

enum EChessMeshAssets
{
	SQUARE,
	PAWN,
	KNIGHT,
	BISHOP,
	ROOK,
	QUEEN,
	KING,
};
enum EChessMaterialAssets
{
	CHESSPIECE,
	EMPTYSQUARE,
	MOVABLESQUARE,
	CAPTABLESQUARE,
	IMMOVABLESQUARE,
	CASTLINGSQUARE
};
struct FChessPiece
{
public:
	static const int None = 0;
	static const int Pawn = 1;
	static const int Knight = 2;
	static const int Bishop = 3;
	static const int Rook = 4;
	static const int Queen = 5;
	static const int King = 6;
	static const int All = 7;


};

struct FTeamColor
{
public:
	static const int White = 0;
	static const int Black = 1;
	static const int Neutral = 2;
	
	static int GetColor(int value)
	{
		if (value == 0) return Neutral;
		return value >> 3;
	}
};
enum EGamePlayMode
{
	SOLOPLAY,
	VS_AI,
	MULTIPLAY
};

UCLASS()
class MYCHESS__API AMyBoard : public AActor
{
	GENERATED_BODY()
	
public:	
	AMyBoard();

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:	

	AMyGameStateBase* gameState = NULL;

	TArray<TArray<int>> Square;
	TArray<int> KingSquare1D;
	TArray<AMyTile*> TileArray;
	TArray<AMyChessPiece*> PieceArray;
	TArray<TArray<AMyChessPiece*>> DeadPieceArray;
	TArray<AMyChessPiece*> OriginPiecePosition;
	TArray<UStaticMesh*> Meshes;
	
	TArray<MyPieceList*> allPieces;
	TArray<UMaterialInstance*> Materials;

	TArray<MyMove*> canMoves;

	UPROPERTY()
		UMyZobristKey* ZobristKey = NULL;
	uint64 Zobrist = 0;

	TArray<uint64> ZobristHistory;

	int enpassant = -1;

	EGamePlayMode playMode = EGamePlayMode::SOLOPLAY;

	AMyAI* aiPlayer = NULL;

	uint64 currentState = 0;

	int playCount = 0;
	int fiftyMoves = 0;
	int teamColors = 0;
	const uint64 whiteCastleKingsideMask = 0b1111111111111110;
	const uint64 whiteCastleQueensideMask = 0b1111111111111101;
	const uint64 blackCastleKingsideMask = 0b1111111111111011;
	const uint64 blackCastleQueensideMask = 0b1111111111110111;

	const uint64 whiteCastleMask = whiteCastleKingsideMask & whiteCastleQueensideMask;
	const uint64 blackCastleMask = blackCastleKingsideMask & blackCastleQueensideMask;

	TArray<uint64> gameStateHistory;
	TArray<uint64> repetitionHistory;

	MyPreComputedMoveData* preMove;

	TSharedPtr<UMyMoveCalculator> moveCal;
	FPromotionTarget promotionTarget;

	TArray<AMyDecal*> decals;

	bool SpawnBoard();
	bool SpawnPiece(TArray<int> SpawningPosition);

	void ResetPiece();

	TArray<int> BoardInit(bool alreadyCreated);

	MyPieceList* GetPieceList(int type, int color);

	void ChessPromotion(int index, bool bAI);
	
	void SelectedPiece(AMyChessPiece* piece);

	void SelectedSquare(AMyTile* tile);

	void Capture(AMyChessPiece* piece);

	void WayPointingSquare(TArray<MyMove*> moves);

	void EmphasizeSquare(AMyTile* tile, bool emphasized);

	void ClearSquare();

	int GetTeamColor();

	void UpdateRoute(int square, EDecalState decalState);
	
	void MakeMoves(MyMove* move, bool search);
	void UnMakeMoves(MyMove* move, bool inSearch);
};
