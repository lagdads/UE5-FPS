// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShooterNPCSpawner.generated.h"

class UCapsuleComponent;
class UArrowComponent;
class AShooterNPC;

/**
 *  用于生成 Shooter NPC 的基本 Actor，并监听其死亡
 *  逐个生成 NPC：每次等待当前 NPC 死亡后才产生下一个
 */
UCLASS()
class PROJECT2_API AShooterNPCSpawner : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent *SpawnCapsule;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UArrowComponent *SpawnDirection;

protected:
	/** 要生成的 NPC 类型 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC Spawner")
	TSubclassOf<AShooterNPC> NPCClass;

	/** 游戏开始后第一次生成 NPC 前的等待时间 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC Spawner", meta = (ClampMin = 0, ClampMax = 10))
	float InitialSpawnDelay = 5.0f;

	/** 要生成的 NPC 总数 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC Spawner", meta = (ClampMin = 0, ClampMax = 100))
	int32 SpawnCount = 1;

	/** 当前 NPC 死亡后等待下一个生成的时间 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC Spawner", meta = (ClampMin = 0, ClampMax = 10))
	float RespawnDelay = 5.0f;

	/** 延迟生成 NPC 的计时器 */
	FTimerHandle SpawnTimer;

public:
	/** 构造函数 */
	AShooterNPCSpawner();

public:
	/** 游戏开始初始化 */
	virtual void BeginPlay() override;

	/** 游戏结束清理 */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

protected:
	/** 生成 NPC 并监听其死亡事件 */
	void SpawnNPC();

	/** 生成的 NPC 死亡时回调 */
	UFUNCTION()
	void OnNPCDied();
};
