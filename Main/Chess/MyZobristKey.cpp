// Fill out your copyright notice in the Description page of Project Settings.


#include "MyZobristKey.h"
#include "MyBoard.h"

UMyZobristKey::UMyZobristKey()
{
	TArray<uint64> Zobrist1;
	Zobrist1.Init(0, 64);
	TArray<TArray<uint64>> Zobrist2;
	Zobrist2.Init(Zobrist1, 2);
	ALLPieces.Init(Zobrist2, 8);
	Enpassants.Init(0, 9);
	Castlings.Init(0, 16);
}

void UMyZobristKey::SetupZobristHash(FString zobrist)
{
	FString tst = "1234568";
	
	FString temp = "";
	//TArray<uint64> hashes;
	TQueue<uint64> hashes;
	for (int i = 0; i < zobrist.Len(); i++)
	{
		if (zobrist[i] == ',')
		{
			hashes.Enqueue(FCString::Strtoui64(*temp, NULL, 10));
			temp = "";
		}
		else
		{
			temp += zobrist[i];
		}

		if(i == zobrist.Len() - 1) hashes.Enqueue(FCString::Strtoui64(*temp, NULL, 10));
	}
	

	for (int square = 0; square < 64; square++)
	{
		for (int type = 0; type < 8; type++)
		{
			//if (hashes.Num() > 1)
			{
				uint64 whitePiece =0;
				uint64 blackPiece = 0;
				hashes.Dequeue(whitePiece);
				hashes.Dequeue(blackPiece);
				ALLPieces[type][FTeamColor::White][square] = whitePiece;
				ALLPieces[type][FTeamColor::Black][square] = blackPiece;
				//UE_LOG(LogTemp, Warning, TEXT("%llu"), ALLPieces[type][FTeamColor::White][square]);
			}
		}
	}

	for (int i = 0; i < 9; i++)
	{
		uint64 enpassant = 0;
		hashes.Dequeue(enpassant);
		Enpassants[i] = enpassant;
	}
	for (int i = 0; i < 16; i++)
	{
		uint64 castling = 0;
		hashes.Dequeue(castling);
		Castlings[i] = castling;
	}
	uint64 sideToMove = 0;
	hashes.Dequeue(sideToMove);
	move = sideToMove;
}

uint64 UMyZobristKey::CalculateZobristKey(AMyBoard* board)
{
	uint64 key = 0;
	if (board)
	{
		for (int r = 0; r < 8; r++)
		{
			for (int f = 0; f < 8; f++)
			{
				if (board->Square[f][r] != 0)
				{
					int square = r * 8 + f;
					int color = (board->Square[f][r] >> 3) & 1;
					int type = (board->Square[f][r]) & 0b111;
					key ^= ALLPieces[type][color][square];
				}
			}
		}
		for (int i = 0; i < 9; i++)
		{
			key ^= Enpassants[i];
		}
		for (int i = 0; i < 16; i++)
		{
			key ^= Castlings[i];
		}
		key ^= move;
	}
	return key;
}
