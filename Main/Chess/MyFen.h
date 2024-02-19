// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class AMyBoard;

class MYCHESS__API MyFen
{
public:
	MyFen();
	~MyFen();


	const FString StartFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";


	FString CurrentFEN(AMyBoard* board, bool extendedData);
};
