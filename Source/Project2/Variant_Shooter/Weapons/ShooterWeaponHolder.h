// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ShooterWeaponHolder.generated.h"

class AShooterWeapon;
class UAnimMontage;

// 此接口类由实现者在角色中继承并负责武器的可视与操作逻辑。
UINTERFACE(MinimalAPI)
class UShooterWeaponHolder : public UInterface
{
	GENERATED_BODY()
};

/**
 *  射击游戏中武器持有者的通用接口，角色通过它与武器通信。
 */
class PROJECT2_API IShooterWeaponHolder
{
	GENERATED_BODY()

public:
	/** 将武器的第一/第三人称网格挂到拥有者对应的骨骼上 */
	virtual void AttachWeaponMeshes(AShooterWeapon *Weapon) = 0;

	/** 播放武器开火动画片段，一般由第三人称角色实现 */
	virtual void PlayFiringMontage(UAnimMontage *Montage) = 0;

	/** 把武器后坐力应用给角色（手柄震动或镜头抖动） */
	virtual void AddWeaponRecoil(float Recoil) = 0;

	/** 通知 HUD 当前弹药信息，用于刷新弹药计数器 */
	virtual void UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize) = 0;

	/** 计算武器的瞄准点，Weapon 使用该点构造投射物朝向 */
	virtual FVector GetWeaponTargetLocation() = 0;

	/** 将指定武器类添加给拥有者，当角色拾取时调用 */
	virtual void AddWeaponClass(const TSubclassOf<AShooterWeapon> &WeaponClass) = 0;

	/** 激活指定武器，让其可见且响应输入 */
	virtual void OnWeaponActivated(AShooterWeapon *Weapon) = 0;

	/** 停用指定武器，隐藏并暂停开火 */
	virtual void OnWeaponDeactivated(AShooterWeapon *Weapon) = 0;

	/** 半自动武器冷却完成后由 Weapon 通知角色可以再次开火 */
	virtual void OnSemiWeaponRefire() = 0;
};
