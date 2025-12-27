// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShooterGameMode.generated.h"

class UShooterUI;

UENUM(BlueprintType)
enum class E_Team : uint8
{
	None UMETA(DisplayName = "No Team"),
	Team1 UMETA(DisplayName = "Team 1"),
	Team2 UMETA(DisplayName = "Team 2")
};

/**
 *  简单第一人称射击游戏的 GameMode
 *  管理游戏 UI 和阵营比分
 */
UCLASS(abstract)
class PROJECT2_API AShooterGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	/** 要生成的记分板 UI 类 */
	UPROPERTY(EditAnywhere, Category = "Shooter")
	TSubclassOf<UShooterUI> ShooterUIClass;

	/** 记分板 UI 实例 */
	TObjectPtr<UShooterUI> ShooterUI;

	/** 按队伍 ID 记录的积分表 */
	TMap<uint8, int32> TeamScores;

protected:
	/** 游戏开始时的初始化 */
	virtual void BeginPlay() override;

public:
	/** 为指定队伍增加积分并更新 UI */
	void IncrementTeamScore(E_Team Team);
};
