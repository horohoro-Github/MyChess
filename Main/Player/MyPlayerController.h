// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "../MyChess_.h"

#include "MyPlayerController.generated.h"

#define RAW_APP_ID "480"

class AMyPlayer;
class AMyPlayerState;
class AMyCharacter;
class AMyChessPiece;
class AMyTile;
class UMyUserWidget;
class AMyGameStateBase;
class AMyPlayerCameraManager;
class AMyBoard;


UENUM()
enum EWidgetState
{
	None_Widget,
	MainMenu,
	Ingame
};

USTRUCT(BlueprintType)
struct FPlayerListsd
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
		FString PID = "";
	UPROPERTY(BlueprintReadWrite)
		FString PlayerName = "";
	UPROPERTY(BlueprintReadWrite)
		int Win = 0;

	UPROPERTY(BlueprintReadWrite)
		int Draw = 0;
	UPROPERTY(BlueprintReadWrite)
		int Lose = 0;
	UPROPERTY(BlueprintReadWrite)
		FString id = "";
	UPROPERTY(BlueprintReadWrite)
		bool Spectator = false;
	UPROPERTY(BlueprintReadWrite)
		bool MasterPlayer = false;
	UPROPERTY(BlueprintReadWrite)
		bool ME = false;
};


USTRUCT(BlueprintType)
struct FPlayerDataBase
{
	GENERATED_BODY()
public:
	UPROPERTY()
		FString pid = "-1";
	UPROPERTY()
		bool isvalid = false;
	UPROPERTY()
		int win = 0;
	UPROPERTY()
		int lose = 0;
	UPROPERTY()
		int draw = 0;
	UPROPERTY()
		FString username = "";
};
USTRUCT(BlueprintType)
struct FServerDataBase
{
	GENERATED_BODY()
public:
	UPROPERTY()
		FString ServerIP = "172.30.1.71";
	UPROPERTY(BlueprintReadWrite)
		FString Ports = "-1";
	UPROPERTY(BlueprintReadWrite)
		int Players = 0;
	UPROPERTY(BlueprintReadWrite)
		int users = 0;
	UPROPERTY(BlueprintReadWrite)
		FString servername = "";
};



UCLASS()
class MYCHESS__API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	AMyPlayerController();
	UPROPERTY(BlueprintReadWrite)
		bool bPromotion = false;
	UPROPERTY(BlueprintReadWrite)
		int MyTeamColor = 1;
	
	UPROPERTY(BlueprintReadWrite)
		bool bAndroidController = false;
		
	virtual void BeginPlay()override;
	virtual void Tick(float DeltaTime)override;
	virtual void OnPossess(APawn* InPawn) override;

	void InteractionMode();
	void MovableMode();

	UPROPERTY()
		AMyPlayerState* PlayerStates;
	UPROPERTY(BlueprintReadWrite)
		AMyCharacter* MyPlayer;
	UPROPERTY()
		AMyChessPiece* ChessPiece;
	UPROPERTY()
		class AMyTile* ChessTile;
	UPROPERTY()
		bool bInteraction = false;
	UPROPERTY(BlueprintReadWrite)
		bool bSpectator = false;
	UPROPERTY()
		class AMyChessPiece* SelectedPiece = NULL;
	UPROPERTY()
		class AMyTile* SelectedTiles = NULL;
	void TraceForBlock(const FVector& Start, const FVector& End);

	UFUNCTION(Server, Reliable)
		void CallTileEmphasize_Client(class AMyTile* tiles, bool emphasized);
	void CallTileEmphasize_Client_Implementation(class AMyTile* tiles, bool emphasized);

	UFUNCTION(Server, Reliable)
		void GetPieces(AMyChessPiece* piece, bool bReset);
	void GetPieces_Implementation(AMyChessPiece* piece, bool bReset);

	UFUNCTION(Server, Reliable)
		void GetTiles(AMyTile* tiles);
	void GetTiles_Implementation(AMyTile* tiles);
	UPROPERTY(EditAnywhere)
		TSubclassOf<AActor> ObserverClass;

	UPROPERTY(BlueprintReadWrite)
		class AMyObserver* Observer = NULL;

	UFUNCTION(Server, Reliable)
		void PossessActor(bool bPlayer);
	void PossessActor_Implementation(bool bPlayer);

	UFUNCTION(Server, Reliable)
		void ResetPossessActor();
	void ResetPossessActor_Implementation();

	FLatentActionInfo latentinfo;

	//ÅÍÄ¡
	UPROPERTY(EditAnywhere, Category = "TouchSetup")
		UTouchInterface* DefaultInterface;
	UPROPERTY(EditAnywhere, Category = "TouchSetup")
		UTouchInterface* PlayableInterface;
	UPROPERTY(EditAnywhere, Category = "TouchSetup")
		UTouchInterface* IngameInterface;
	UPROPERTY(EditAnywhere, Category = "TouchSetup")
		UTouchInterface* SpectatorInterface;

	TSubclassOf<UUserWidget>CurrentWidgetClass;

	UPROPERTY(BlueprintReadWrite)
		UMyUserWidget* CurrentWidget = NULL;
	UPROPERTY(BlueprintReadWrite)
		UMyUserWidget* SlideWidget = NULL;

	class UMyGameInstance* GameIns;
	UFUNCTION(Client, Reliable)
		void SetSpectator(bool bSpectators);
	void SetSpectator_Implementation(bool bSpectators);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bReadyButton = false;

	UFUNCTION(Client, Reliable)
		void HiddenCeiling();
	void HiddenCeiling_Implementation();

	UFUNCTION(Client, Reliable)
		void FadeScreen(int FadeState);
	void FadeScreen_Implementation(int FadeState);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bWidgetVisibility = false;

	UFUNCTION(BlueprintCallable)
		void ClearWidget();

	UFUNCTION(Server, Reliable)
		void PlayerControllerSetup(bool Spec, bool bIsLobbyLevel);
	void PlayerControllerSetup_Implementation(bool Spec, bool bIsLobbyLevel);

	UFUNCTION(Client, Reliable)
		void AddWidget(int widState, bool showCursor);
	void AddWidget_Implementation(int widState, bool showCursor);

	UFUNCTION(Client, Reliable)
		void KickPlayer(bool bMainServer);
	void KickPlayer_Implementation(bool bMainServer);

	UFUNCTION(BlueprintPure)
		float GetFadeValue();

	UFUNCTION(Client, Reliable)
		void AddBeaconClient();
	void AddBeaconClient_Implementation();
	EWidgetState widgetState = EWidgetState::None_Widget;

	AMyGameStateBase* gameState = NULL;

	TSubclassOf<UUserWidget> LobbyWidgetClass;

	TSubclassOf<UUserWidget> LoginWidgetClass;

	TSubclassOf<UUserWidget> InGameWidgetClass;

	TSubclassOf<UUserWidget> ExitClass;

	AMyBoard* board = NULL;
private:
	
	AMyPlayerCameraManager* CameraManager = NULL;
};
