// Fill out your copyright notice in the Description page of Project Settings.


#include "MySquareTable.h"

MySquareTable::MySquareTable()
{

	//for (int i = 0; i < 64; i++)
	{
		//pawns.Add(Pawn_Table[i]);
		//knights.Add(Knight_Table[i]);
		//bishops.Add(Bishop_Table[i]);
		//rooks.Add(Rook_Table[i]);
		//queens.Add(Queen_Table[i]);
	}
}

MySquareTable::~MySquareTable()
{
	Pawn_Table.Empty();
	Knight_Table.Empty();
	Bishop_Table.Empty();
	Rook_Table.Empty();
	Queen_Table.Empty();
}
