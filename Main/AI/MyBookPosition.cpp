// Fill out your copyright notice in the Description page of Project Settings.


#include "MyBookPosition.h"
#include "../Chess/MyMove.h"
MyBookPosition::MyBookPosition()
{
}

MyBookPosition::~MyBookPosition()
{
}

void MyBookPosition::AddMove(MyMove* move, int numTimesPlayed)
{
	
	uint16 moveValue = move->MoveValue();
	if (numTimesMovePlayed.Contains(moveValue))
	{
		numTimesMovePlayed[moveValue]++;
	}
	else
	{
		numTimesMovePlayed.Add(moveValue, numTimesPlayed);
	}
	
}
