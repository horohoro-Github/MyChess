// Fill out your copyright notice in the Description page of Project Settings.


#include "MyMoveCalculator.h"
#include "MyBoard.h"
#include "../Game/MyGameStateBase.h"
#include "MyPieceList.h"
#include "MyMove.h"
#include "MyPreComputedMoveData.h"
UMyMoveCalculator::UMyMoveCalculator()
{
	AddToRoot();

	TArray<int> startSquares;
	TArray<TArray<int>> bit;
	bit.Init(startSquares, 64);
	moveBitBoard.Init(bit, 6);
}

UMyMoveCalculator::~UMyMoveCalculator()
{
}

TArray<MyMove*> UMyMoveCalculator::GenerateMove(AMyBoard* calculatingboard, bool turnCheck, int square, int type, bool movementPath)
{
	genQuiets = turnCheck;
	board = calculatingboard;
	genMovementPath = movementPath;
	Init();
	CalculateAttackData();
	playerSelectPiece = type == FChessPiece::All ? false : true;
	if(type == FChessPiece::King || type == FChessPiece::All) KingMove();

	if (doubleCheck && !playerSelectPiece)
	{
		return returnMoves;
	}

	switch (type)
	{
	case FChessPiece::Pawn: case FChessPiece::All:
	{
		PawnMove(square);
		if(type == FChessPiece::Pawn) break;
		__fallthrough;
	}
	case FChessPiece::Knight:
	{
		KnightMove(square);
		if (type == FChessPiece::Knight) break;
		__fallthrough;
	}
	case FChessPiece::Bishop: case FChessPiece::Rook: case FChessPiece::Queen:
	{
		GenerateSlidingMoves(square, type);
		break;
	}
	}
	return returnMoves;
}

void UMyMoveCalculator::Init()
{
	if (genMovementPath == true)
	{
		TArray<int> startSquares;
		TArray<TArray<int>> bit;
		bit.Init(startSquares, 64);
		moveBitBoard.Init(bit, 6);
	}
	returnMoves.Empty();
	preMove = board->preMove;
	playerSelectPiece = false;
	check = false;
	doubleCheck = false;
	pinsExistInPosition = false;
	checkRayBitmask = 0;
	pinRayBitmask = 0;

	isWhiteToMove = board->GetTeamColor() == FTeamColor::White;
	color = board->GetTeamColor();
	opponentColor = 1 - color;
	friendlyKingSquare = board->KingSquare1D[color];

}


void UMyMoveCalculator::CalculateAttackData()
{
	GenSlidingAttackMap();
	int startDirIndex = 0;
	int endDirIndex = 8;
	if (board->GetPieceList(FChessPiece::Queen, opponentColor)->pieceNum == 0)
	{
		startDirIndex = (board->GetPieceList(FChessPiece::Rook, opponentColor)->pieceNum > 0) ? 0 : 4;
		endDirIndex = (board->GetPieceList(FChessPiece::Bishop, opponentColor)->pieceNum > 0) ? 8 : 4;
	}

	for (int dir = startDirIndex; dir < endDirIndex; dir++)
	{
		bool isDiagonal = dir > 3;
		int n = preMove->numSquaresToEdge[friendlyKingSquare][dir];
		int directionOffset = preMove->directionOffsets[dir];
		bool isFriendlyPieceAlongRay = false;
		uint64 rayMask = 0;

		for (int i = 0; i < n; i++)
		{
			int squareIndex = friendlyKingSquare + directionOffset * (i + 1);
			rayMask |= 1ull << squareIndex;
			int piece = board->Square[squareIndex % 8][squareIndex / 8];

			if (piece != FChessPiece::None)
			{
				if ((piece >> 3) == color)
				{
					if (!isFriendlyPieceAlongRay)
					{
						isFriendlyPieceAlongRay = true;
					}
					else
					{
						break;
					}
				}
				else
				{
					int pieceType = piece & 0b111;

					if (isDiagonal && (((piece & 0b111) == FChessPiece::Queen) || ((piece & 0b111) == FChessPiece::Bishop)) || !isDiagonal && (((piece & 0b111) == FChessPiece::Queen) || ((piece & 0b111) == FChessPiece::Rook)))
					{
						if (isFriendlyPieceAlongRay)
						{
							pinsExistInPosition = true;
							pinRayBitmask |= rayMask;
						}
						else
						{
							checkRayBitmask |= rayMask;
							doubleCheck = check;
							check = true;
						}
						break;
					}
					else
					{
						break;
					}
				}
			}
		}
		if (doubleCheck)
		{
			break;
		}

	}
	
	MyPieceList* opponentKnights = board->GetPieceList(FChessPiece::Knight, opponentColor);
	opponentKnightAttacks = 0;
	bool isKnightCheck = false;

	for (int knightIndex = 0; knightIndex < opponentKnights->pieceNum; knightIndex++)
	{
		int startSquare = opponentKnights->PiecePosition[knightIndex];
		opponentKnightAttacks |= preMove->knightAttackBitboards[startSquare];

		if (!isKnightCheck && ((opponentKnightAttacks >> friendlyKingSquare) & 1) != 0)
		{
			isKnightCheck = true;
			doubleCheck = check;
			check = true;
			checkRayBitmask |= 1ull << startSquare;
		}
	}

	MyPieceList* opponentPawns = board->GetPieceList(FChessPiece::Pawn, opponentColor);
	opponentPawnAttackMap = 0;
	bool isPawnCheck = false;

	for (int pawnIndex = 0; pawnIndex < opponentPawns->pieceNum; pawnIndex++)
	{
		int pawnSquare = opponentPawns->PiecePosition[pawnIndex];
		uint64 pawnAttacks = preMove->pawnAttackBitboards[pawnSquare][opponentColor];
		opponentPawnAttackMap |= pawnAttacks;

		if (!isPawnCheck && ((pawnAttacks >> friendlyKingSquare) & 1) != 0)
		{
			isPawnCheck = true;
			doubleCheck = check;
			check = true;
			checkRayBitmask |= 1ull << pawnSquare;
		}
	}

	int enemyKingSquare = board->KingSquare1D[opponentColor];

	opponentAttackMapNoPawns = opponentSlidingAttackMap | opponentKnightAttacks | preMove->kingAttackBitboards[enemyKingSquare];
	opponentAttackMap = opponentAttackMapNoPawns | opponentPawnAttackMap;
}

void UMyMoveCalculator::GenSlidingAttackMap()
{
	opponentSlidingAttackMap = 0;
	MyPieceList* enemyRooks = board->GetPieceList(FChessPiece::Rook, opponentColor);
	for (int i = 0; i < enemyRooks->pieceNum; i++)
	{
		UpdateSlidingAttackPiece(enemyRooks->PiecePosition[i], 0, 4);
	}

	MyPieceList* enemyQueens = board->GetPieceList(FChessPiece::Queen, opponentColor);
	for (int i = 0; i < enemyQueens->pieceNum; i++)
	{
		UpdateSlidingAttackPiece(enemyQueens->PiecePosition[i], 0, 8);
	}

	MyPieceList* enemyBishops = board->GetPieceList(FChessPiece::Bishop, opponentColor);
	for (int i = 0; i < enemyBishops->pieceNum; i++)
	{
		UpdateSlidingAttackPiece(enemyBishops->PiecePosition[i], 4, 8);
	}
}

void UMyMoveCalculator::UpdateSlidingAttackPiece(int startSquare, int startDirIndex, int endDirIndex)
{
	for (int directionIndex = startDirIndex; directionIndex < endDirIndex; directionIndex++)
	{
		int currentDirOffset = preMove->directionOffsets[directionIndex];
		for (int n = 0; n < preMove->numSquaresToEdge[startSquare][directionIndex]; n++)
		{
			int targetSquare = startSquare + currentDirOffset * (n + 1);
			int targetSquarePiece = board->Square[targetSquare % 8][targetSquare / 8];
			opponentSlidingAttackMap |= 1ull << targetSquare;
			if (targetSquare != friendlyKingSquare)
			{
				if (targetSquarePiece != FChessPiece::None)
				{
					break;
				}
			}
		}
	}
}

void UMyMoveCalculator::KingMove()
{
	for (int i = 0; i < preMove->kingMoves[friendlyKingSquare].Num(); i++)
	{
		int targetSquare = preMove->kingMoves[friendlyKingSquare][i];
		int pieceOnTargetSquare = board->Square[targetSquare % 8][targetSquare / 8];

		if (FTeamColor::GetColor(pieceOnTargetSquare) == color)
		{
			if (playerSelectPiece) returnMoves.Add(new MyMove(friendlyKingSquare, targetSquare, Flag::CantMove));
			continue;
		}

		bool isCapture = FTeamColor::GetColor(pieceOnTargetSquare) == opponentColor;
		if (!isCapture)
		{
			if (!genQuiets || SquareIsInCheckRay(targetSquare))
			{
				if (playerSelectPiece) returnMoves.Add(new MyMove(friendlyKingSquare, targetSquare, Flag::CantMove));
				continue;
			}
		}
		if (!SquareIsAttacked(targetSquare))
		{
			returnMoves.Add(new MyMove(friendlyKingSquare, targetSquare));
			if (!check && !isCapture)
			{
				if ((targetSquare == 5 || targetSquare == 61) && HasKingsideCastleRight())
				{
					int castleKingsideSquare = targetSquare + 1;
					if (board->Square[castleKingsideSquare % 8][castleKingsideSquare / 8] == FChessPiece::None)
					{
						if (!SquareIsAttacked(castleKingsideSquare))
						{
							returnMoves.Add(new MyMove(friendlyKingSquare, castleKingsideSquare, Flag::Castling));
						}
					}
				}
				else if ((targetSquare == 3 || targetSquare == 59) && HasQueensideCastleRight())
				{
					int castleQueensideSquare = targetSquare - 1;
					if (board->Square[castleQueensideSquare % 8][castleQueensideSquare / 8] == FChessPiece::None && board->Square[(castleQueensideSquare - 1) % 8][(castleQueensideSquare - 1) / 8] == FChessPiece::None)
					{
						if (!SquareIsAttacked(castleQueensideSquare))
						{
							returnMoves.Add(new MyMove(friendlyKingSquare, castleQueensideSquare, Flag::Castling));
						}
					}
				}
			}
		}
		else
		{
			if (playerSelectPiece) returnMoves.Add(new MyMove(friendlyKingSquare, targetSquare, Flag::CantMove));
		}
	}
}
void UMyMoveCalculator::GenerateSlidingMoves(int square, int type)
{
	if (type == FChessPiece::All || type == FChessPiece::Rook)
	{
		MyPieceList* rooks = board->GetPieceList(FChessPiece::Rook, color);
		for (int i = 0; i < rooks->pieceNum; i++)
		{
			GenerateSlidingPieceMoves(rooks->PiecePosition[i], 0, 4);
		}
	}

	if (type == FChessPiece::All || type == FChessPiece::Bishop)
	{
		MyPieceList* bishops = board->GetPieceList(FChessPiece::Bishop, color);
		for (int i = 0; i < bishops->pieceNum; i++)
		{
			GenerateSlidingPieceMoves(bishops->PiecePosition[i], 4, 8);
		}
	}

	if (type == FChessPiece::All || type == FChessPiece::Queen)
	{
		MyPieceList* queens = board->GetPieceList(FChessPiece::Queen, color);
		for (int i = 0; i < queens->pieceNum; i++)
		{
			GenerateSlidingPieceMoves(queens->PiecePosition[i], 0, 8);
		}
	}

}

void UMyMoveCalculator::GenerateSlidingPieceMoves(int startSquare, int startDirIndex, int endDirIndex)
{
	bool isPinned = IsPinned(startSquare);

	int type = board->Square[startSquare & 0b111][startSquare >> 3] & 0b111;
	if (check && isPinned && !playerSelectPiece)
	{
		return;
	}

	for (int directionIndex = startDirIndex; directionIndex < endDirIndex; directionIndex++)
	{
		int currentDirOffset = preMove->directionOffsets[directionIndex];
		bool checkPlayableSquare = !IsMovingAlongRay(currentDirOffset, friendlyKingSquare, startSquare);
		if (isPinned && checkPlayableSquare && !playerSelectPiece)
		{
			continue;
		}

		for (int n = 0; n < preMove->numSquaresToEdge[startSquare][directionIndex]; n++)
		{
			int targetSquare = startSquare + currentDirOffset * (n + 1);
			int targetSquarePiece = board->Square[targetSquare % 8][targetSquare / 8];
			if (FTeamColor::GetColor(targetSquarePiece) == color)
			{
				if (playerSelectPiece) returnMoves.Add(new MyMove(startSquare, targetSquare, Flag::CantMove));
				break;
			}
			bool isCapture = targetSquarePiece != FChessPiece::None;

			bool movePreventsCheck = SquareIsInCheckRay(targetSquare);
			if (movePreventsCheck || !check)
			{
				if (genQuiets || isCapture)
				{
					if (!(check && isPinned) && !(isPinned && checkPlayableSquare))
					{
						if (doubleCheck == false)
						{
							if (genMovementPath == true) moveBitBoard[type][targetSquare].Add(startSquare);
							returnMoves.Add(new MyMove(startSquare, targetSquare));
						}
						else
						returnMoves.Add(new MyMove(startSquare, targetSquare, Flag::CantMove));
					}
					else
					{
						if (playerSelectPiece) returnMoves.Add(new MyMove(startSquare, targetSquare, Flag::CantMove));
					}
				}
			}
			else
			{
				if (playerSelectPiece) returnMoves.Add(new MyMove(startSquare, targetSquare, Flag::CantMove));
			}
			
			if ((isCapture || movePreventsCheck) && !playerSelectPiece) break;

			if (isCapture && playerSelectPiece) break;
		}
	}
}

void UMyMoveCalculator::KnightMove(int square)
{
	MyPieceList* myKnights = board->GetPieceList(FChessPiece::Knight, color);
	for (int i = 0; i < myKnights->pieceNum; i++)
	{
		int startSquare = myKnights->PiecePosition[i];


		if (IsPinned(startSquare) && !playerSelectPiece)
		{
			continue;
		}
		for (int knightMoveIndex = 0; knightMoveIndex < preMove->knightMoves[startSquare].Num(); knightMoveIndex++)
		{

			int targetSquare = preMove->knightMoves[startSquare][knightMoveIndex];
			int targetSquarePiece = board->Square[targetSquare % 8][targetSquare / 8];
			bool isCapture = FTeamColor::GetColor(targetSquarePiece) == opponentColor;
			if (genQuiets || isCapture)
			{
				if ((FTeamColor::GetColor(targetSquarePiece) == color) || (check && !SquareIsInCheckRay(targetSquare)))
				{
					if (playerSelectPiece) returnMoves.Add(new MyMove(startSquare, targetSquare, Flag::CantMove));
					continue;
				}

				if (!IsPinned(startSquare))
				{
					if (doubleCheck == false)
					{
						returnMoves.Add(new MyMove(startSquare, targetSquare));
						if (genMovementPath == true) moveBitBoard[FChessPiece::Knight][targetSquare].Add(startSquare);
					}
					else
						returnMoves.Add(new MyMove(startSquare, targetSquare, Flag::CantMove));
				}
				else if (playerSelectPiece) returnMoves.Add(new MyMove(startSquare, targetSquare, Flag::CantMove));
			}
		}
	}
}

void UMyMoveCalculator::PawnMove(int square)
{
	MyPieceList* myPawns = board->GetPieceList(FChessPiece::Pawn, color);
	int pawnOffset = (color == FTeamColor::White) ? 8 : -8;
	int startRank = (board->teamColors == FTeamColor::White) ? 1 : 6;
	int finalRankBeforePromotion = (board->teamColors == FTeamColor::White) ? 6 : 1;

	int enPassantFile = ((int)(board->currentState >> 4) & 15) - 1;
	int enPassantSquare = -1;
	if (enPassantFile != -1)
	{
		enPassantSquare = 8 * ((board->teamColors == FTeamColor::White) ? 5 : 2) + enPassantFile;
	}

	for (int i = 0; i < myPawns->pieceNum; i++)
	{
		int startSquare = myPawns->PiecePosition[i];
		int rank = startSquare >> 3;
		bool oneStepFromPromotion = rank == finalRankBeforePromotion;
		if (genQuiets)
		{
			int squareOneForward = startSquare + pawnOffset;

			if (ArrayValidate(squareOneForward % 8, squareOneForward / 8))
			{
				if (board->Square[squareOneForward % 8][squareOneForward / 8] == FChessPiece::None)
				{
					if (!IsPinned(startSquare) || IsMovingAlongRay(pawnOffset, startSquare, friendlyKingSquare))
					{
						if (!check || SquareIsInCheckRay(squareOneForward))
						{
							if (oneStepFromPromotion)
							{
								MakePromotionMoves(startSquare, squareOneForward);
							}
							else
							{
								if (doubleCheck == false)
								{
									returnMoves.Add(new MyMove(startSquare, squareOneForward));
								}
								else
								{
									if (playerSelectPiece) returnMoves.Add(new MyMove(startSquare, squareOneForward, Flag::CantMove));
								}
							}
						}
						else
						{
							if (playerSelectPiece) returnMoves.Add(new MyMove(startSquare, squareOneForward, Flag::CantMove));
						}
						if (rank == startRank)
						{
							int squareTwoForward = squareOneForward + pawnOffset;
							if (board->Square[squareTwoForward % 8][squareTwoForward / 8] == FChessPiece::None)
							{
								if (!check || SquareIsInCheckRay(squareTwoForward))
								{
									if (doubleCheck == false)
									{
										returnMoves.Add(new MyMove(startSquare, squareTwoForward, Flag::PawnTwoForward));
									}
									else
									{
										if (playerSelectPiece) returnMoves.Add(new MyMove(startSquare, squareTwoForward, Flag::CantMove));
									}
								}
								else
								{
									if (playerSelectPiece) returnMoves.Add(new MyMove(startSquare, squareTwoForward, Flag::CantMove));
								}
							}
							else
							{
								if (playerSelectPiece) returnMoves.Add(new MyMove(startSquare, squareTwoForward, Flag::CantMove));
							}
						}
					}
				}
				else
				{
					if (playerSelectPiece) returnMoves.Add(new MyMove(startSquare, squareOneForward, Flag::CantMove));
				}
			}
		}
		for (int j = 0; j < 2; j++)
		{
			if (preMove->numSquaresToEdge[startSquare][preMove->pawnAttackDirections[color][j]] > 0)
			{
				int pawnCaptureDir = preMove->directionOffsets[preMove->pawnAttackDirections[color][j]];
				int targetSquare = startSquare + pawnCaptureDir;
				int targetPiece = board->Square[targetSquare % 8][targetSquare / 8];
				if (IsPinned(startSquare) && !IsMovingAlongRay(pawnCaptureDir, friendlyKingSquare, startSquare))
				{
					if (playerSelectPiece) returnMoves.Add(new MyMove(startSquare, targetSquare, Flag::CantMove));
					continue;
				}
				if (FTeamColor::GetColor(targetPiece) == (opponentColor))
				{
					if (check && !SquareIsInCheckRay(targetSquare))
					{
						if (playerSelectPiece) returnMoves.Add(new MyMove(startSquare, targetSquare, Flag::CantMove));
						continue;
					}
					if (oneStepFromPromotion)
					{
						MakePromotionMoves(startSquare, targetSquare);
					}
					else
					{
						if (doubleCheck == false)
						{
							returnMoves.Add(new MyMove(startSquare, targetSquare));
						}
						else
						{
							if (playerSelectPiece)	returnMoves.Add(new MyMove(startSquare, targetSquare, Flag::CantMove));
						}
					}
				}
				if (targetSquare == enPassantSquare)
				{
					int epCapturedPawnSquare = targetSquare + ((board->teamColors == FTeamColor::White) ? -8 : 8);
					if (!InCheckAfterEnPassant(startSquare, targetSquare, epCapturedPawnSquare))
					{
						if (doubleCheck == false)
						{
							returnMoves.Add(new MyMove(startSquare, targetSquare, Flag::EnPassantCapture));
						}
						else
						{
							if (playerSelectPiece)returnMoves.Add(new MyMove(startSquare, targetSquare, Flag::CantMove));
						}
					}
					else
					{
						if (playerSelectPiece) returnMoves.Add(new MyMove(startSquare, targetSquare, Flag::CantMove));
					}
				}
			}
		}
	}
}

void UMyMoveCalculator::MakePromotionMoves(int fromSquare, int toSquare)
{
	returnMoves.Add(new MyMove(fromSquare, toSquare, Flag::PromoteToQueen));
	returnMoves.Add(new MyMove(fromSquare, toSquare, Flag::PromoteToKnight));
}

bool UMyMoveCalculator::InCheckAfterEnPassant(int startSquare, int targetSquare, int epCapturedPawnSquare)
{
	board->Square[targetSquare % 8][targetSquare / 8] = board->Square[startSquare % 8][startSquare / 8];
	board->Square[startSquare % 8][startSquare / 8] = FChessPiece::None;
	board->Square[epCapturedPawnSquare % 8][epCapturedPawnSquare / 8] = FChessPiece::None;

	bool inCheckAfterEpCapture = false;
	if (SquareAttackedAfterEPCapture(epCapturedPawnSquare, startSquare))
	{
		inCheckAfterEpCapture = true;
	}
	board->Square[targetSquare % 8][targetSquare / 8] = FChessPiece::None;
	board->Square[startSquare % 8][startSquare / 8] = FChessPiece::Pawn | (color << 3);
	board->Square[epCapturedPawnSquare % 8][epCapturedPawnSquare / 8] = FChessPiece::Pawn | (opponentColor << 3);
	return inCheckAfterEpCapture;
}

bool UMyMoveCalculator::SquareAttackedAfterEPCapture(int epCaptureSquare, int capturingPawnStartSquare)
{
	if (((opponentAttackMapNoPawns >> friendlyKingSquare) & 1) != 0)
	{
		return true;
	}
	int dirIndex = (epCaptureSquare < friendlyKingSquare) ? 2 : 3;
	for (int i = 0; i < preMove->numSquaresToEdge[friendlyKingSquare][dirIndex]; i++)
	{
		int squareIndex = friendlyKingSquare + preMove->directionOffsets[dirIndex] * (i + 1);
		int piece = board->Square[squareIndex % 8][squareIndex / 8];
		if (piece != FChessPiece::None)
		{
			if (FTeamColor::GetColor(piece) == color)
			{
				break;
			}
			else
			{
				if ((piece & 0b111) == FChessPiece::Rook || (piece & 0b111) == FChessPiece::Queen)
				{
					return true;
				}
				else
				{
					break;
				}
			}
		}
	}
	for (int i = 0; i < 2; i++)
	{
		if (preMove->numSquaresToEdge[friendlyKingSquare][preMove->pawnAttackDirections[color][i]] > 0)
		{
			int piece = board->Square[(friendlyKingSquare + preMove->directionOffsets[preMove->pawnAttackDirections[color][i]]) % 8][(friendlyKingSquare + preMove->directionOffsets[preMove->pawnAttackDirections[color][i]]) / 8];
			if (piece == (FChessPiece::Pawn | (opponentColor << 3))) // is enemy pawn
			{
				return true;
			}
		}
	}
	return false;
}

bool UMyMoveCalculator::HasKingsideCastleRight()
{
	int mask = (board->teamColors == FTeamColor::White) ? 1 : 4;
	return (board->currentState & mask) != 0;
}

bool UMyMoveCalculator::HasQueensideCastleRight()
{
	int mask = (board->teamColors == FTeamColor::White) ? 2 : 8;
	return (board->currentState & mask) != 0;
}

bool UMyMoveCalculator::IsMovingAlongRay(int rayDir, int startSquare, int targetSquare)
{
	int moveDir = preMove->directionLookup[targetSquare - startSquare + 63];
	return (rayDir == moveDir || -rayDir == moveDir);
}

bool UMyMoveCalculator::ArrayValidate(int file, int rank)
{
	if (file >= 0 && file < 8 && rank >= 0 && rank < 8) return true;
	return false;
}

int UMyMoveCalculator::GetSquareIndex(int file, int rank)
{
	return file + rank * 8;
}