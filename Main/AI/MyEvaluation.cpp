// Fill out your copyright notice in the Description page of Project Settings.


#include "MyEvaluation.h"
#include "../Chess/MyBoard.h"
#include "../Chess/MyPieceList.h"
#include "MySquareTable.h"
#include "../Chess/MyPreComputedMoveData.h"
MyEvaluation::MyEvaluation()
{
}

MyEvaluation::~MyEvaluation()
{
}

int MyEvaluation::Evaluate(AMyBoard* chessBoard)
{
	board = chessBoard;
	int whiteEval = 0;
	int blackEval = 0;

	int whiteMaterial = CountMaterial(FTeamColor::White);
	int blackMaterial = CountMaterial(FTeamColor::Black);

	int whiteMaterialWithoutPawns = whiteMaterial - board->GetPieceList(FChessPiece::Pawn, FTeamColor::White)->pieceNum * pawnValue;
	int blackMaterialWithoutPawns = blackMaterial - board->GetPieceList(FChessPiece::Pawn, FTeamColor::Black)->pieceNum * pawnValue;

	float whiteEndgamePhaseWeight = EndgamePhaseWeight(whiteMaterialWithoutPawns);
	float blackEndgamePhaseWeight = EndgamePhaseWeight(blackMaterialWithoutPawns);

	whiteEval += whiteMaterial;
	blackEval += blackMaterial;
	whiteEval += MopupEval(FTeamColor::White, FTeamColor::Black, whiteMaterial, blackMaterial, blackEndgamePhaseWeight);
	blackEval += MopupEval(FTeamColor::Black, FTeamColor::White, blackMaterial, whiteMaterial, whiteEndgamePhaseWeight);

	whiteEval += EvaluatePieceSquareTables(FTeamColor::White, blackEndgamePhaseWeight);
	blackEval += EvaluatePieceSquareTables(FTeamColor::Black, whiteEndgamePhaseWeight);

	int eval;
	eval = whiteEval - blackEval;

	int perspective = (board->GetTeamColor() == 0) ? 1 : -1;
	return eval * perspective;
}

int MyEvaluation::MopupEval(int friendlyIndex, int opponentIndex, int myMaterial, int opponentMaterial, float endgameWeight)
{
	int MopUpScore = 0;
	if (myMaterial > opponentMaterial + pawnValue * 2 && endgameWeight > 0)
	{
		int friendlyKingSquare = board->KingSquare1D[friendlyIndex];
		int opponentKingSqaure = board->KingSquare1D[opponentIndex];
		MopUpScore += board->preMove->centreManhattanDistance[opponentKingSqaure] * 10;
		MopUpScore += (14 - board->preMove->orthogonalDistance[friendlyKingSquare][opponentKingSqaure]) * 4;

		return (int)(MopUpScore * endgameWeight);
	}
	return 0;
}

int MyEvaluation::EndgamePhaseWeight(int materialCountWithoutPawns)
{
	const float multiplier = 1 / endGameMaterialStart;
	return 1 - FMath::Min(1, materialCountWithoutPawns * multiplier);
}

int MyEvaluation::CountMaterial(int colorIndex)
{
	int Material = 0;
	Material += board->GetPieceList(FChessPiece::Pawn, colorIndex)->pieceNum * pawnValue;
	Material += board->GetPieceList(FChessPiece::Knight, colorIndex)->pieceNum * knightValue;
	Material += board->GetPieceList(FChessPiece::Bishop, colorIndex)->pieceNum * bishopValue;
	Material += board->GetPieceList(FChessPiece::Rook, colorIndex)->pieceNum * rookValue;
	Material += board->GetPieceList(FChessPiece::Queen, colorIndex)->pieceNum * queenValue;
	return Material;
}

int MyEvaluation::EvaluatePieceSquareTables(int colorIndex, float endgamePhaseWeight)
{
	int value = 0;
	bool isWhite = colorIndex == FTeamColor::White;
	MySquareTable table = MySquareTable();
	value += EvaluatePieceSquareTable(table.Pawn_Table, board->GetPieceList(FChessPiece::Pawn, colorIndex), isWhite);
	value += EvaluatePieceSquareTable(table.Knight_Table, board->GetPieceList(FChessPiece::Knight, colorIndex), isWhite);
	value += EvaluatePieceSquareTable(table.Bishop_Table, board->GetPieceList(FChessPiece::Bishop, colorIndex), isWhite);
	value += EvaluatePieceSquareTable(table.Rook_Table, board->GetPieceList(FChessPiece::Rook, colorIndex), isWhite);
	value += EvaluatePieceSquareTable(table.Queen_Table, board->GetPieceList(FChessPiece::Queen, colorIndex), isWhite);

	return value;
}

int MyEvaluation::EvaluatePieceSquareTable(TArray<int>& table, MyPieceList* pieceList, bool white)
{
	int value = 0;
	for (int i = 0; i < pieceList->pieceNum; i++)
	{
		value += MySquareTable::Read(table, pieceList->PiecePosition[i], white);
	}
	return value;
}
