// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */

struct Flag
{
public:

	static const int None = 0;
	static const int EnPassantCapture = 1;
	static const int PromoteToKnight = 2;
	static const int PromoteToBishop = 3;
	static const int PromoteToRook = 4;
	static const int PromoteToQueen = 5;
	static const int Castling = 6;
	static const int PawnTwoForward = 7;
	static const int CantMove = 8;
};

class MYCHESS__API MyMove
{

private:
	uint32 movevalue = 0;

public:
	

	MyMove(int startSquare, int targetSquare, int flag)
	{
		movevalue = (uint32)(startSquare | targetSquare << 6 | flag << 12);
	}

	MyMove(int startSquare, int targetSquare)
	{
		movevalue = (uint32)(startSquare | targetSquare << 6);
	}

	MyMove(int targetSquare)
	{
		movevalue |= targetSquare;
	}
	
	int Flags()
	{
		return movevalue >> 12;
	}

	MyMove(int startSquare, int targetSquare, int piecetype, int teamcolor, int attack)
	{
		movevalue |= startSquare;
		movevalue |= ((unsigned long long)targetSquare << 6);
		movevalue |= ((unsigned long long)piecetype << 12);
		movevalue |= ((unsigned long long)teamcolor << 15);
		movevalue |= ((unsigned long long)attack << 19);
	}
	MyMove(int startSquare, int targetSquare, int piecetype, int teamcolor, int etc, int attack)
	{
		movevalue |= startSquare;
		movevalue |= ((unsigned long long)targetSquare << 6);
		movevalue |= ((unsigned long long)piecetype << 12);
		movevalue |= ((unsigned long long)teamcolor << 15);
		movevalue |= ((unsigned long long)etc << 16); //0 = 없음, 1 = 앙파상, 2 = 퀸사이드 캐슬링, 3 = 킹사이드 캐슬링, 4 = 프로모션, 5 = 폰의 2보 전진
		movevalue |= ((unsigned long long)attack << 19);
	}

	~MyMove();

	uint32 MoveValue()
	{
		return movevalue;
	}

	int StartSquare()
	{
		return (movevalue & 0b111111);
	}

	int TargetSquare()
	{
		return (movevalue >> 6) & 0b111111;
	}

	int GetPieceType()
	{
		return (movevalue >> 12) & 0b111;
	}

	int GetTeamColor()
	{
		return (movevalue >> 15) & 1;
	}

	int GetSpecialChessMoves()
	{
		return (movevalue >> 15) & 0b111;
	}
	bool IsAttackMove()
	{
		return (movevalue >> 15) & 1;
	}
};
