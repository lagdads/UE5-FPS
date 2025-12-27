// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Project2Character.h"
#include "Weapons/ShooterWeaponHolder.h"
#include "ShooterGameMode.h"
#include "ShooterCharacter.generated.h"

class AShooterWeapon;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBulletCountUpdatedDelegate, int32, MagazineSize, int32, Bullets);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDamagedDelegate, float, LifePercent);

/**
 *  可由玩家控制的第一人称射击角色
 *  通过 IShooterWeaponHolder 接口管理武器
 *  管理生命与死亡流程
 */
UCLASS(abstract)
class PROJECT2_API AShooterCharacter : public AProject2Character, public IShooterWeaponHolder
{
	GENERATED_BODY()

protected:
	/** 射击输入动作 */
	UPROPERTY(EditAnywhere, Category = "Input|Actions")
	UInputAction *FireAction;

	/** 切换武器输入动作 */
	UPROPERTY(EditAnywhere, Category = "Input|Actions")
	UInputAction *SwitchWeaponAction;

	/** 切换鱿鱼形态输入动作 */
	UPROPERTY(EditAnywhere, Category = "Input|Actions")
	UInputAction *SquidFormAction;

	/** 切换鱿鱼形态输入动作 */
	UPROPERTY(EditAnywhere, Category = "Input|Actions")
	UInputAction *SquidFormToggleAction;

	/** 第一人称网格的武器挂点名称 */
	UPROPERTY(EditAnywhere, Category = "Weapons|Configuration")
	FName FirstPersonWeaponSocket = FName("HandGrip_R");

	/** 第三人称网格的武器挂点名称 */
	UPROPERTY(EditAnywhere, Category = "Weapons|Configuration")
	FName ThirdPersonWeaponSocket = FName("HandGrip_R");

	/** 瞄准射线的最大距离 */
	UPROPERTY(EditAnywhere, Category = "Aim|Configuration", meta = (ClampMin = 0, ClampMax = 100000, Units = "cm"))
	float MaxAimDistance = 10000.0f;

	/** 最大生命值 */
	UPROPERTY(EditAnywhere, Category = "Health|Configuration")
	float MaxHP = 500.0f;

	/** 当前剩余生命值 */
	float CurrentHP = 0.0f;

	/** 所属队伍 ID（用于识别/计分）*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team|Configuration")
	E_Team Team = E_Team::Team1;

	/** 死亡时添加的标签 */
	UPROPERTY(EditAnywhere, Category = "Team|Configuration")
	FName DeathTag = FName("Dead");

	/** 默认武器类别 */
	UPROPERTY(EditAnywhere, Category = "Weapons|Configuration")
	TSubclassOf<AShooterWeapon> DefaultWeaponClass;

	/** 拥有的武器列表 */
	TArray<AShooterWeapon *> OwnedWeapons;

	/** 当前装备的武器 */
	TObjectPtr<AShooterWeapon> CurrentWeapon;

	UPROPERTY(EditAnywhere, Category = "Health|Configuration", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float RespawnTime = 5.0f;

	FTimerHandle RespawnTimer;

	// 第三人称潜水视角,鱿鱼形态摄像机组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Squid Form")
	class USpringArmComponent *SquidSpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Squid Form")
	class UCameraComponent *SquidCamera;

	// 潜水形态的网格体
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Squid Form")
	class USkeletalMeshComponent *SquidMesh;

	// --- 状态变量 ---
	bool bIsSquidForm;

	// --- 潜水参数配置 ---
	UPROPERTY(EditAnywhere, Category = "Squid Mechanics|Settings")
	float SquidMoveSpeed = 900.f;

	UPROPERTY(EditAnywhere, Category = "Squid Mechanics|Settings")
	float NormalMoveSpeed = 600.f;

	UPROPERTY(EditAnywhere, Category = "Squid Mechanics|Settings")
	float SquidCapsuleHeight = 12.f; // 潜水时变扁

	UPROPERTY(EditAnywhere, Category = "Squid Mechanics|Settings")
	float NormalCapsuleRadius = 34.f;

	UPROPERTY(EditAnywhere, Category = "Squid Mechanics|Settings")
	float SquidCapsuleRadius = 12.f;

	UPROPERTY(EditAnywhere, Category = "Squid Mechanics|Settings")
	float NormalCapsuleHeight = 88.f; // 正常站立高度

public:
	/** 子弹数更新的委托 */
	FBulletCountUpdatedDelegate OnBulletCountUpdated;

	/** 受伤委托 */
	FDamagedDelegate OnDamaged;

public:
	/** 构造函数 */
	AShooterCharacter();

protected:
	/** 游戏初始化 */
	virtual void BeginPlay() override;

	/** 游戏结束清理 */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	/** 绑定输入动作 */
	virtual void SetupPlayerInputComponent(UInputComponent *InputComponent) override;

public:
	/** 处理伤害 */
	virtual float TakeDamage(float Damage, struct FDamageEvent const &DamageEvent, AController *EventInstigator, AActor *DamageCauser) override;

public:
	/** 处理视角输入 */
	virtual void DoAim(float Yaw, float Pitch) override;

	/** 处理移动输入 */
	virtual void DoMove(float Right, float Forward) override;

	/** 处理跳跃开始输入 */
	virtual void DoJumpStart() override;

	/** 处理跳跃结束输入 */
	virtual void DoJumpEnd() override;

	/** 开始射击输入 */
	UFUNCTION(BlueprintCallable, Category = "Input|Callbacks")
	void DoStartFiring();

	/** 停止射击输入 */
	UFUNCTION(BlueprintCallable, Category = "Input|Callbacks")
	void DoStopFiring();

	/** 切换武器输入 */
	UFUNCTION(BlueprintCallable, Category = "Input|Callbacks")
	void DoSwitchWeapon();

	/** 进入鱿鱼形态输入（按下按键） */
	UFUNCTION(BlueprintCallable, Category = "Input|Callbacks")
	void DoEnterSquidForm();

	/** 退出鱿鱼形态输入（松开按键） */
	UFUNCTION(BlueprintCallable, Category = "Input|Callbacks")
	void DoExitSquidForm();

	/** 切换鱿鱼形态输入 */
	UFUNCTION(BlueprintCallable, Category = "Input|Callbacks")
	void DoToggleSquidForm();

	/** 进入鱿鱼形态 */
	UFUNCTION(BlueprintCallable, Category = "Squid Mechanics|Callbacks")
	void EnterSquidForm();

	/** 退出鱿鱼形态 */
	UFUNCTION(BlueprintCallable, Category = "Squid Mechanics|Callbacks")
	void ExitSquidForm();

	/** 获取当前是否为鱿鱼形态 */
	UFUNCTION(BlueprintPure, Category = "Squid Mechanics|State")
	bool IsSquidForm() const { return bIsSquidForm; }

	/** 获取角色所属队伍 */
	UFUNCTION(BlueprintPure, Category = "Team")
	E_Team GetTeam() const { return Team; }

	/** 设置角色所属队伍 */
	UFUNCTION(BlueprintCallable, Category = "Team")
	void SetTeam(E_Team NewTeam) { Team = NewTeam; }

public:
	//~Begin IShooterWeaponHolder interface

	/** 将武器网格附加到角色 */
	virtual void AttachWeaponMeshes(AShooterWeapon *Weapon) override;

	/** 播放射击蒙太奇 */
	virtual void PlayFiringMontage(UAnimMontage *Montage) override;

	/** 应用武器后坐力 */
	virtual void AddWeaponRecoil(float Recoil) override;

	/** 更新子弹 HUD */
	virtual void UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize) override;

	/** 计算武器瞄准位置 */
	virtual FVector GetWeaponTargetLocation() override;

	/** 增加一把指定类别的武器 */
	virtual void AddWeaponClass(const TSubclassOf<AShooterWeapon> &WeaponClass) override;

	/** 激活武器 */
	virtual void OnWeaponActivated(AShooterWeapon *Weapon) override;

	/** 取消激活武器 */
	virtual void OnWeaponDeactivated(AShooterWeapon *Weapon) override;

	/** 半自动武器冷却完毕时回调 */
	virtual void OnSemiWeaponRefire() override;

	//~End IShooterWeaponHolder interface

protected:
	/** 判断角色是否已拥有指定类型武器 */
	AShooterWeapon *FindWeaponOfType(TSubclassOf<AShooterWeapon> WeaponClass) const;

	/** 生命耗尽时调用 */
	void Die();

	/** 允许蓝图响应角色死亡 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Health|Callbacks", meta = (DisplayName = "On Death"))
	void BP_OnDeath();

	/** 由重生计时器触发，销毁本体并迫使玩家重生 */
	void OnRespawn();

public:
	/** 判断角色是否已经死亡 */
	bool IsDead() const;
};
