// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Project2Character.h"
#include "ShooterWeaponHolder.h"
#include "ShooterNPC.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPawnDeathDelegate);

class AShooterWeapon;

/**
 *  简单的 AI 射击 NPC
 *  通过 AI 控制器管理的 StateTree 执行行为逻辑
 *  持有并管理一把武器
 */
UCLASS(abstract)
class PROJECT2_API AShooterNPC : public AProject2Character, public IShooterWeaponHolder
{
	GENERATED_BODY()

public:
	/** 当前生命值；降到 0 即触发死亡 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	float CurrentHP = 100.0f;

protected:
	/** 布娃娃死亡时使用的碰撞配置名 */
	UPROPERTY(EditAnywhere, Category = "Damage")
	FName RagdollCollisionProfile = FName("Ragdoll");

	/** 死亡后延迟销毁的时间 */
	UPROPERTY(EditAnywhere, Category = "Damage")
	float DeferredDestructionTime = 5.0f;

	/** 队伍编号（用于计分/识别） */
	UPROPERTY(EditAnywhere, Category = "Team")
	uint8 TeamByte = 1;

	/** 死亡时授予的标签 */
	UPROPERTY(EditAnywhere, Category = "Team")
	FName DeathTag = FName("Dead");

	/** 当前持有的武器指针 */
	TObjectPtr<AShooterWeapon> Weapon;

	/** 角色出生时要生成的武器类型 */
	UPROPERTY(EditAnywhere, Category = "Weapon")
	TSubclassOf<AShooterWeapon> WeaponClass;

	/** 第一人称网格挂点名称 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons")
	FName FirstPersonWeaponSocket = FName("HandGrip_R");

	/** 第三人称网格挂点名称 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons")
	FName ThirdPersonWeaponSocket = FName("HandGrip_R");

	/** 瞄准射线的最大距离 */
	UPROPERTY(EditAnywhere, Category = "Aim")
	float AimRange = 10000.0f;

	/** 瞄准锥形散布半角（越大越随机） */
	UPROPERTY(EditAnywhere, Category = "Aim")
	float AimVarianceHalfAngle = 10.0f;

	/** 瞄准时相对目标中心的最小竖直偏移 */
	UPROPERTY(EditAnywhere, Category = "Aim")
	float MinAimOffsetZ = -35.0f;

	/** 瞄准时相对目标中心的最大竖直偏移 */
	UPROPERTY(EditAnywhere, Category = "Aim")
	float MaxAimOffsetZ = -60.0f;

	/** 当前瞄准的目标 Actor */
	TObjectPtr<AActor> CurrentAimTarget;

	/** 是否正在射击 */
	bool bIsShooting = false;

	/** 是否已死亡，避免重复处理 */
	bool bIsDead = false;

	/** 延迟销毁计时器句柄 */
	FTimerHandle DeathTimer;

public:
	/** NPC 死亡时触发的委托 */
	FPawnDeathDelegate OnPawnDeath;

protected:
	/** 游戏开始初始化（生成武器等） */
	virtual void BeginPlay() override;

	/** 结束时清理（取消计时器） */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	/** 处理受到的伤害 */
	virtual float TakeDamage(float Damage, struct FDamageEvent const &DamageEvent, AController *EventInstigator, AActor *DamageCauser) override;

public:
	//~Begin IShooterWeaponHolder interface

	/** 将武器网格附加到角色 */
	virtual void AttachWeaponMeshes(AShooterWeapon *Weapon) override;

	/** 播放开火蒙太奇（此实现未使用） */
	virtual void PlayFiringMontage(UAnimMontage *Montage) override;

	/** 应用武器后坐力（此实现未使用） */
	virtual void AddWeaponRecoil(float Recoil) override;

	/** 更新 HUD 弹药显示（此实现未使用） */
	virtual void UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize) override;

	/** 计算武器瞄准位置 */
	virtual FVector GetWeaponTargetLocation() override;

	/** 给予指定类型的武器（此实现未使用） */
	virtual void AddWeaponClass(const TSubclassOf<AShooterWeapon> &WeaponClass) override;

	/** 激活传入的武器（此实现未使用） */
	virtual void OnWeaponActivated(AShooterWeapon *Weapon) override;

	/** 关闭传入的武器（此实现未使用） */
	virtual void OnWeaponDeactivated(AShooterWeapon *Weapon) override;

	/** 半自动武器冷却结束后回调，准备再次射击 */
	virtual void OnSemiWeaponRefire() override;

	//~End IShooterWeaponHolder interface

protected:
	/** 生命值耗尽时执行死亡逻辑 */
	void Die();

	/** 延迟死亡后销毁自身 */
	void DeferredDestruction();

public:
	/** 开始朝指定目标射击 */
	void StartShooting(AActor *ActorToShoot);

	/** 停止射击 */
	void StopShooting();
};
