// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Character/ShooterCharacter.h"
#include "UI/ShooterBulletCounterUI.h"
#include "Project2.h"
#include "Widgets/Input/SVirtualJoystick.h"

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 仅在本地控制器上UI
	if (IsLocalPlayerController())
	{

		// 生成并显示子弹计数器 UI
		BulletCounterUI = CreateWidget<UShooterBulletCounterUI>(this, BulletCounterUIClass);

		if (BulletCounterUI)
		{
			BulletCounterUI->AddToPlayerScreen(0);
		}
		else
		{

			UE_LOG(LogProject2, Error, TEXT("Could not spawn bullet counter widget."));
		}
	}
}

void AShooterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// 仅本地控制器添加输入映射上下文
	if (IsLocalPlayerController())
	{
		// 添加输入映射上下文
		if (UEnhancedInputLocalPlayerSubsystem *Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext *CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}
		}
	}
}

void AShooterPlayerController::OnPossess(APawn *InPawn)
{
	Super::OnPossess(InPawn);

	// 订阅角色销毁事件
	InPawn->OnDestroyed.AddDynamic(this, &AShooterPlayerController::OnPawnDestroyed);

	// 判断是否为射击角色
	if (AShooterCharacter *ShooterCharacter = Cast<AShooterCharacter>(InPawn))
	{
		// 添加玩家标签
		ShooterCharacter->Tags.Add(PlayerPawnTag);

		// 订阅子弹数与受伤委托
		ShooterCharacter->OnBulletCountUpdated.AddDynamic(this, &AShooterPlayerController::OnBulletCountUpdated);
		ShooterCharacter->OnDamaged.AddDynamic(this, &AShooterPlayerController::OnPawnDamaged);

		// 强制刷新生命条，立即反映玩家初始血量
		ShooterCharacter->OnDamaged.Broadcast(1.0f);
	}
}

void AShooterPlayerController::OnPawnDestroyed(AActor *DestroyedActor)
{
	// 重置子弹计数器 HUD
	if (IsValid(BulletCounterUI))
	{
		BulletCounterUI->BP_UpdateBulletCounter(0, 0);
	}

	// 查找所有 PlayerStart 以便重生
	TArray<AActor *> ActorList;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), ActorList);

	if (ActorList.Num() > 0)
	{
		// 随机选择一个玩家出生点
		AActor *RandomPlayerStart = ActorList[FMath::RandRange(0, ActorList.Num() - 1)];

		// 在出生点生成角色并接管
		const FTransform SpawnTransform = RandomPlayerStart->GetActorTransform();

		if (AShooterCharacter *RespawnedCharacter = GetWorld()->SpawnActor<AShooterCharacter>(CharacterClass, SpawnTransform))
		{
			// 控制此新生成的角色
			Possess(RespawnedCharacter);
		}
	}
}

void AShooterPlayerController::OnBulletCountUpdated(int32 MagazineSize, int32 Bullets)
{
	// 使用最新弹匣/子弹数量更新 HUD
	if (BulletCounterUI)
	{
		BulletCounterUI->BP_UpdateBulletCounter(MagazineSize, Bullets);
	}
}

void AShooterPlayerController::OnPawnDamaged(float LifePercent)
{
	if (IsValid(BulletCounterUI))
	{
		BulletCounterUI->BP_Damaged(LifePercent);
	}
}

