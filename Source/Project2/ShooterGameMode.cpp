// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterGameMode.h"
#include "UI/ShooterUI.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void AShooterGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 创建记分板 UI 并添加到视口
	ShooterUI = CreateWidget<UShooterUI>(UGameplayStatics::GetPlayerController(GetWorld(), 0), ShooterUIClass);
	ShooterUI->AddToViewport(0);
}

void AShooterGameMode::IncrementTeamScore(E_Team Team)
{
	uint8 TeamByte = static_cast<uint8>(Team);

	// 获取目标队伍当前积分（如果有）
	int32 Score = 0;
	if (int32 *FoundScore = TeamScores.Find(TeamByte))
	{
		Score = *FoundScore;
	}

	// 增加该队伍的积分
	++Score;
	TeamScores.Add(TeamByte, Score);

	// 通知 UI 更新
	ShooterUI->BP_UpdateScore(TeamByte, Score);
}
