// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class MyMove;

class MYCHESS__API MyBookPosition
{
public:
	MyBookPosition();
	~MyBookPosition();

	TMap<uint16, int> numTimesMovePlayed;
	void AddMove(MyMove* move, int numTimesPlayed);
};
