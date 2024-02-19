// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MyMoveCalculator.generated.h"
class MyMove;
class AMyBoard;
class MyPieceList;
class MyPreComputedMoveData;

UCLASS()
class MYCHESS__API UMyMoveCalculator : public UObject
{
	GENERATED_BODY()
public:
	UMyMoveCalculator();
	~UMyMoveCalculator();

	TArray<MyMove*> GenerateMove(AMyBoard* calculatingboard, bool turnCheck, int square, int type, bool movementPath = false);

	bool ArrayValidate(int file, int rank);

	AMyBoard* board = NULL;
	class AMyGameStateBase* gameState = NULL;

	int color = -1;
	int opponentColor = -1;

	TArray<TArray<FString>> actionRadius;

	TArray<TArray<uint64>> AttackBitBoard; // 공격 하는 기물의 위치 1 아니면 0;
	bool check = false;

	const int singleMoveMask = 0b0001;
	const int slideMoveMask = 0b0010;

	TArray<MyMove*> returnMoves;
	TArray<TArray<TArray<int>>> moveBitBoard;

	uint64 checkRayBitmask = 0;
	uint64 pinRayBitmask = 0;
	uint64 opponentKnightAttacks = 0;
	uint64 opponentAttackMapNoPawns = 0;
	uint64 opponentAttackMap = 0;
	uint64 opponentPawnAttackMap = 0;
	uint64 opponentSlidingAttackMap = 0;
	MyPreComputedMoveData* preMove = NULL;
	bool doubleCheck = false;
	bool pinsExistInPosition = false;
	bool isWhiteToMove = false;
	int friendlyKingSquare = 0;
	bool genQuiets = false;
	bool genMovementPath = false;

	bool playerSelectPiece = false;

	int GetSquareIndex(int file, int rank);
	
	void Init();
	void CalculateAttackData();
	void GenSlidingAttackMap();
	void UpdateSlidingAttackPiece(int startSquare, int startDirIndex, int endDirIndex);
	void KingMove();
	void GenerateSlidingMoves(int square, int type);
	void GenerateSlidingPieceMoves(int startSquare, int startDirIndex, int endDirIndex);
	void KnightMove(int square);
	void PawnMove(int square);

	void MakePromotionMoves(int fromSquare, int toSquare);
	bool InCheckAfterEnPassant(int startSquare, int targetSquare, int epCapturedPawnSquare);
	bool SquareAttackedAfterEPCapture(int epCaptureSquare, int capturingPawnStartSquare);

	bool HasKingsideCastleRight();

	bool HasQueensideCastleRight();

	bool SquareIsInCheckRay(int square)
	{
		return check && ((checkRayBitmask >> square) & 1) != 0;
	}
	bool SquareIsAttacked(int square)
	{
		return ((opponentAttackMap >> square) & 1) != 0;
	}
	bool IsPinned(int square)
	{
		return pinsExistInPosition && ((pinRayBitmask >> square) & 1) != 0;
	}
	bool IsMovingAlongRay(int rayDir, int startSquare, int targetSquare);
};
