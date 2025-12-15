// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

class UInputMappingContext;
class AShooterCharacter;
class UShooterBulletCounterUI;

/**
 *  简单第一人称射击游戏的玩家控制器
 *  管理输入映射并监听角色死亡协助重生
 */
UCLASS(abstract, config = "Game")
class PROJECT2_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	/** 默认输入映射上下文集合 */
	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<UInputMappingContext *> DefaultMappingContexts;

	/** 被控制角色死亡后重生用的角色类 */
	UPROPERTY(EditAnywhere, Category = "Shooter|Respawn")
	TSubclassOf<AShooterCharacter> CharacterClass;

	/** 要生成的子弹计数器 UI 类 */
	UPROPERTY(EditAnywhere, Category = "Shooter|UI")
	TSubclassOf<UShooterBulletCounterUI> BulletCounterUIClass;

	/** 赋予玩家角色的标签，方便识别 */
	UPROPERTY(EditAnywhere, Category = "Shooter|Player")
	FName PlayerPawnTag = FName("Player");

	/** 子弹计数器 UI 实例 */
	UPROPERTY()
	TObjectPtr<UShooterBulletCounterUI> BulletCounterUI;

protected:
	/** 游戏初始化 */
	virtual void BeginPlay() override;

	/** 配置输入绑定 */
	virtual void SetupInputComponent() override;

	/** 拥有新角色时的初始化 */
	virtual void OnPossess(APawn *InPawn) override;

	/** 被控制角色销毁时回调 */
	UFUNCTION()
	void OnPawnDestroyed(AActor *DestroyedActor);

	/** 被控制角色子弹数更新时回调 */
	UFUNCTION()
	void OnBulletCountUpdated(int32 MagazineSize, int32 Bullets);

	/** 被控制角色受伤时回调 */
	UFUNCTION()
	void OnPawnDamaged(float LifePercent);


};
