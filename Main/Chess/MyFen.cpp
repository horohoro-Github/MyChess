// Fill out your copyright notice in the Description page of Project Settings.


#include "MyFen.h"
#include "MyBoard.h"
#include "../Game/MyGameStateBase.h"

MyFen::MyFen()
{
}

MyFen::~MyFen()
{
}

FString MyFen::CurrentFEN(AMyBoard* board, bool extendedData)
{
	FString fen = "";

	for (int r = 7; r >= 0; r--)
	{
		int numEmptyFiles = 0;
		for (int f = 0; f < 8; f++)
		{
			int i = r * 8 + f;
			int piece = board->Square[f][r];

			if (piece != 0)
			{
				if (numEmptyFiles != 0)
				{
					fen += FString::FromInt(numEmptyFiles);
					numEmptyFiles = 0;
				}
				bool isBlack = FTeamColor::GetColor(piece) == FTeamColor::Black ? true : false;

				int type = piece & 0b111;

				char pieceChar = ' ';

				switch (type)
				{
				case FChessPiece::Pawn:
					pieceChar = (isBlack) ? 'p' : 'P';
					break;
				case FChessPiece::Knight:
					pieceChar = (isBlack) ? 'n' : 'N';
					break;
				case FChessPiece::Bishop:
					pieceChar = (isBlack) ? 'b' : 'B';
					break;
				case FChessPiece::Rook:
					pieceChar = (isBlack) ? 'r' : 'R';
					break;
				case FChessPiece::Queen:
					pieceChar = (isBlack) ? 'q' : 'Q';
					break;
				case FChessPiece::King:
					pieceChar = (isBlack) ? 'k' : 'K';
					break;
				}

				fen += pieceChar;

			}
			else
			{
				numEmptyFiles++;
			}
		}

		if (numEmptyFiles != 0)
		{
			fen += FString::FromInt(numEmptyFiles);
		}
		if (r != 0)
		{
			fen += '/';
		}
	}

	int teamColor = board->gameState->TeamColor;
	fen += ' ';
	fen += teamColor == 0 ? 'w' : 'b';
	

	int castling = board->currentState & 0b1111;
	bool whiteKingSide = castling & 0b0001;
	bool whiteQueenSide = castling & 0b0010;
	bool blackKingSide = castling & 0b0100;
	bool blackQueenSide = castling & 0b1000;

	fen += ' ';
	fen += (whiteKingSide) ? "K" : "";
	fen += (whiteQueenSide) ? "Q" : "";
	fen += (blackKingSide) ? "k" : "";
	fen += (blackQueenSide) ? "q" : "";

	fen += castling == 0 ? "-" : "";


	fen += ' ';

	if (extendedData)
	{
		int epFileIndex = ((board->currentState >> 4) & 0b1111) - 1;
		int epRankIndex = teamColor == 0 ? 5 : 2;

		bool isEnpassant = epFileIndex != -1;

		if (isEnpassant)
		{
			FString File = "abcdefgh";
			FString Rank = "12345678";
			fen += File[epFileIndex];
			fen += Rank[epRankIndex];
		}
		else
		{
			fen += '-';
		}

		fen += ' ';
		fen += FString::FromInt((board->currentState >> 14) & 0b111111);

		fen += ' ';
		fen += FString::FromInt((board->playCount / 2) + 1);
	}
	else
	{
		fen += '-';
	}
	return fen;
}
