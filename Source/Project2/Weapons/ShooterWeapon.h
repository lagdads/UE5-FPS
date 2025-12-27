// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShooterWeaponHolder.h"
#include "Animation/AnimInstance.h"
#include "ShooterWeapon.generated.h"

class IShooterWeaponHolder;
class AShooterProjectile;
class USkeletalMeshComponent;
class UAnimMontage;
class UAnimInstance;

/**
 *  简单第一人称射击武器基类
 *  同时提供第一人称与第三人称网格
 *  负责弹药与射击流程
 *  通过 IShooterWeaponHolder 接口与角色交互
 */
UCLASS(abstract)
class PROJECT2_API AShooterWeapon : public AActor
{
	GENERATED_BODY()

	/** 第一人称网格 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent *FirstPersonMesh;

	/** 第三人称网格 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent *ThirdPersonMesh;

protected:
	/** 指向武器持有者的接口指针 */
	IShooterWeaponHolder *WeaponOwner;

	/** 武器发射的弹药类型 */
	UPROPERTY(EditAnywhere, Category = "Ammo")
	TSubclassOf<AShooterProjectile> ProjectileClass;

	/** 弹匣容量 */
	UPROPERTY(EditAnywhere, Category = "Ammo", meta = (ClampMin = 0, ClampMax = 100))
	int32 MagazineSize = 10;

	/** 当前弹匣剩余子弹 */
	int32 CurrentBullets = 0;

	/** 开火时播放的动画蒙太奇 */
	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage *FiringMontage;

	/** 激活该武器时第一人称网格应使用的 AnimInstance */
	UPROPERTY(EditAnywhere, Category = "Animation")
	TSubclassOf<UAnimInstance> FirstPersonAnimInstanceClass;

	/** 激活该武器时第三人称网格应使用的 AnimInstance */
	UPROPERTY(EditAnywhere, Category = "Animation")
	TSubclassOf<UAnimInstance> ThirdPersonAnimInstanceClass;

	/** 瞄准时的角度散布半角 */
	UPROPERTY(EditAnywhere, Category = "Aim", meta = (ClampMin = 0, ClampMax = 90, Units = "Degrees"))
	float AimVariance = 0.0f;

	/** 射击后施加给持有者的后坐力大小 */
	UPROPERTY(EditAnywhere, Category = "Aim", meta = (ClampMin = 0, ClampMax = 100))
	float FiringRecoil = 0.0f;

	/** 第一人称枪口插槽名称 */
	UPROPERTY(EditAnywhere, Category = "Aim")
	FName MuzzleSocketName;

	/** 子弹生成点相对于枪口的前置偏移 */
	UPROPERTY(EditAnywhere, Category = "Aim", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm"))
	float MuzzleOffset = 10.0f;

	/** 全自动武器开火标记 */
	UPROPERTY(EditAnywhere, Category = "Refire")
	bool bFullAuto = false;

	/** 射击间隔（秒），影响全/半自动 */
	UPROPERTY(EditAnywhere, Category = "Refire", meta = (ClampMin = 0, ClampMax = 5, Units = "s"))
	float RefireRate = 0.5f;

	/** 上一次开火的游戏时间，用于半自动节奏控制 */
	float TimeOfLastShot = 0.0f;

	/** 当前武器是否仍保持射击状态（按住扳机时为 true） */
	bool bIsFiring = false;

	/** 全自动射击的计时器 */
	FTimerHandle RefireTimer;

public:
	/** 构造函数 */
	AShooterWeapon();

protected:
	/** 游戏初始化 */
	virtual void BeginPlay() override;

	/** 游戏结束清理 */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

protected:
	/** 拥有者被销毁时的回调 */
	UFUNCTION()
	void OnOwnerDestroyed(AActor *DestroyedActor);

public:
	/** 激活武器，恢复可见并通知持有者准备接受输入 */
	void ActivateWeapon();

	/** 停用武器并暂停所有射击 */
	void DeactivateWeapon();

	/** 开始响应扳机事件，保持自动或半自动循环 */
	void StartFiring();

	/** 停止射击并清除等待重射的计时器 */
	void StopFiring();

protected:
	/** 负责一次射击的全部流程：生成投射物、播放反馈、消耗弹药 */
	virtual void Fire();

	/** 半自动模式下计时结束后调用，通知角色可以再次射击 */
	void FireCooldownExpired();

	/** 在目标方向上生成投射物并播放反馈 */
	virtual void FireProjectile(const FVector &TargetLocation);

	/** 计算投射物生成的坐标与朝向（含枪口偏移与散布） */
	FTransform CalculateProjectileSpawnTransform(const FVector &TargetLocation) const;

public:
	/** 返回第一人称网格组件，供 Player 角色挂载 */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	USkeletalMeshComponent *GetFirstPersonMesh() const { return FirstPersonMesh; };

	/** 返回第三人称网格组件，供其他玩家或 AI 展示 */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	USkeletalMeshComponent *GetThirdPersonMesh() const { return ThirdPersonMesh; };

	/** 返回第一人称网格应该使用的动画实例类 */
	const TSubclassOf<UAnimInstance> &GetFirstPersonAnimInstanceClass() const;

	/** 返回第三人称网格应该使用的动画实例类 */
	const TSubclassOf<UAnimInstance> &GetThirdPersonAnimInstanceClass() const;

	/** 查询弹匣容量 */
	int32 GetMagazineSize() const { return MagazineSize; };

	/** 查询当前弹匣剩余子弹 */
	int32 GetBulletCount() const { return CurrentBullets; }
};
