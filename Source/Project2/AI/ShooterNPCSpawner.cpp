// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/ShooterNPCSpawner.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ArrowComponent.h"
#include "TimerManager.h"
#include "ShooterNPC.h"

// 设置默认属性值
AShooterNPCSpawner::AShooterNPCSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	// 创建根场景组件
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// 创建用于定位刷新的胶囊体
	SpawnCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Spawn Capsule"));
	SpawnCapsule->SetupAttachment(RootComponent);

	SpawnCapsule->SetRelativeLocation(FVector(0.0f, 0.0f, 90.0f));
	SpawnCapsule->SetCapsuleSize(35.0f, 90.0f);
	SpawnCapsule->SetCollisionProfileName(FName("NoCollision"));

	SpawnDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("Spawn Direction"));
	SpawnDirection->SetupAttachment(RootComponent);
}

void AShooterNPCSpawner::BeginPlay()
{
	Super::BeginPlay();

	// 初始计数为 0 时不生成 NPC
	if (SpawnCount > 0)
	{
		// 计划第一次生成
		GetWorld()->GetTimerManager().SetTimer(SpawnTimer, this, &AShooterNPCSpawner::SpawnNPC, InitialSpawnDelay);
	}
}

void AShooterNPCSpawner::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// 清理生成计时器
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
}

void AShooterNPCSpawner::SpawnNPC()
{
	// 检查 NPC 类型是否合法
	if (IsValid(NPCClass))
	{
		// 在参考胶囊位置生成 NPC
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AShooterNPC *SpawnedNPC = GetWorld()->SpawnActor<AShooterNPC>(NPCClass, SpawnCapsule->GetComponentTransform(), SpawnParams);

		// NPC 是否成功创建
		if (SpawnedNPC)
		{
			// 订阅死亡委托
			SpawnedNPC->OnPawnDeath.AddDynamic(this, &AShooterNPCSpawner::OnNPCDied);
		}
	}
}

void AShooterNPCSpawner::OnNPCDied()
{
	// 削减剩余生成数量
	--SpawnCount;

	// 还剩多少 NPC 可以生成？
	if (SpawnCount <= 0)
	{
		return;
	}

	// 计划下一次生成
	GetWorld()->GetTimerManager().SetTimer(SpawnTimer, this, &AShooterNPCSpawner::SpawnNPC, RespawnDelay);
}
