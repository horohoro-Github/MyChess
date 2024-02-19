// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyStructs.generated.h"

/**
 * 
 */

enum EDecalState
{
	Nomal,
	StartPoint,
	TargetPoint
};
struct FPromotionTarget
{
public:

	int startSquare = 0;

	int targetSquare = 0;
};

USTRUCT(BlueprintType)
struct FUsersData
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadWrite)
		FString userid = "";
	UPROPERTY(BlueprintReadWrite)
		FString userpass = "";
	UPROPERTY(BlueprintReadWrite)
		FString player_name = "";
	UPROPERTY(BlueprintReadWrite)
		int win = 0;
	UPROPERTY(BlueprintReadWrite)
		int draw = 0;
	UPROPERTY(BlueprintReadWrite)
		int lose = 0;
	UPROPERTY(BlueprintReadWrite)
		bool login = false;
	UPROPERTY(BlueprintReadWrite)
		FString message_box = "";
	UPROPERTY(BlueprintReadWrite)
		bool can_watch = false;

	FUsersData();
	~FUsersData();
};

USTRUCT(BlueprintType)
struct FVerifyToken
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
	FUsersData userdata;
	UPROPERTY(BlueprintReadWrite)
	int32 Time;
	FVerifyToken();
	~FVerifyToken();
	FVerifyToken(FUsersData db, int32 time)
	{
		userdata = db;
		Time = time;
	}
};

USTRUCT(BlueprintType)
struct FFriendsData
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
		FString userid = "";
	UPROPERTY(BlueprintReadWrite)
		FString friends_id = "";
	UPROPERTY(BlueprintReadWrite)
		FString friends_req = "";
	UPROPERTY(BlueprintReadWrite)
		FString friends_res = "";
	FFriendsData();
	~FFriendsData();
};


USTRUCT(BlueprintType)
struct FServerData
{
	GENERATED_BODY()
public:
	UPROPERTY()
		FString serverip = "";
	UPROPERTY()
		int serverport = 0;
	UPROPERTY()
		FString servertype = "";

	FServerData();
	~FServerData();
};

USTRUCT(BlueprintType)
struct FUserName
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
		TArray<FUsersData> username;
	UPROPERTY(BlueprintReadWrite)
		TArray<FUsersData> name_res;
	UPROPERTY(BlueprintReadWrite)
		TArray<FUsersData> name_req;

	FUserName();
	~FUserName();
};

USTRUCT(BlueprintType)
struct FAlgebraicNotation
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
		int Algebraic_Number = 0;
	UPROPERTY(BlueprintReadWrite)
		FString Algebraic_White = "";
	UPROPERTY(BlueprintReadWrite)
		FString Algebraic_Black = "";
};


UENUM()
enum class EGameResult : uint8
{
	Win,
	Draw,
	Lose
};

//PS Àü¿ë
UENUM()
enum class EPlayerState : uint8
{
	NONE_PLAYERSTATE,
	PLAYER,
	SPECTATOR
};