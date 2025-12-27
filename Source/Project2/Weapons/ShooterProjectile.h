// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ShooterGameMode.h"
#include "GameFramework/Actor.h"
#include "ShooterProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class ACharacter;
class UPrimitiveComponent;

/**
 *  简单第一人称射击投射物类
 */
UCLASS(abstract)
class PROJECT2_API AShooterProjectile : public AActor
{
	GENERATED_BODY()

	/** 提供碰撞检测的球形组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USphereComponent *CollisionComponent;

	/** 控制投射物移动的组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent *ProjectileMovement;

protected:
	/**重力系数*/
	UPROPERTY(EditAnywhere, Category = "Projectile|Movement")
	float GravityScale = 0.8f;

	/**子弹速度*/
	UPROPERTY(EditAnywhere, Category = "Projectile|Movement")
	float Speed = 5000.0f;

	/** 水平衰减率（每秒），用于指数衰减，越大衰减越快 */
	UPROPERTY(EditAnywhere, Category = "Projectile|Movement", meta = (ClampMin = 0))
	float HorizontalDeceleration = 3.0f;

	/** 命中造成的伤害 */
	UPROPERTY(EditAnywhere, Category = "Projectile|Hit", meta = (ClampMin = 0, ClampMax = 100))
	float HitDamage = 25.0f;

	/** 伤害类型 */
	UPROPERTY(EditAnywhere, Category = "Projectile|Hit")
	TSubclassOf<UDamageType> HitDamageType;

	/** 是否允许伤害自身 */
	UPROPERTY(EditAnywhere, Category = "Projectile|Hit")
	bool bDamageOwner = false;

	/** 是否已命中某个表面 */
	bool bHit = false;

	/** 命中后等待多长时间再销毁 */
	UPROPERTY(EditAnywhere, Category = "Projectile|Destruction", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float DeferredDestructionTime = 5.0f;

	/** 延迟销毁计时器 */
	FTimerHandle DestructionTimer;

public:
	/** 子弹所属的队伍 */
	UPROPERTY(BlueprintReadWrite, Category = "Projectile")
	E_Team OwningTeam;

	// ========== 碰撞后行为设置 ==========

	/** 击中可移动物体时使用的碰撞配置名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Hit Effects")
	FName PhysicsCollisionProfile = FName(TEXT("PhysicsActor"));

protected:
	/** 是否已附着到目标 */
	bool bAttachedToTarget = false;

public:
	/** 构造函数 */
	AShooterProjectile();

	/** 每帧更新：用于处理水平减速 */
	virtual void Tick(float DeltaSeconds) override;

	// 可选的蓝图扩展事件（C++ 已实现核心涂色逻辑）
	// 蓝图可以实现此事件添加额外的视觉效果或自定义行为
	UFUNCTION(BlueprintImplementableEvent, Category = "Painting", meta = (DisplayName = "Trigger Paint On Actor"))
	void TriggerPaintOnActor(AActor *HitActor, FVector2D HitUV, E_Team CurrentTeamID);

protected:
	/** 游戏初始化 */
	virtual void BeginPlay() override;

	/** 游戏结束清理 */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	/** 处理碰撞 */
	virtual void NotifyHit(class UPrimitiveComponent *MyComp, AActor *Other, UPrimitiveComponent *OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult &Hit) override;

protected:
	/** 处理单个命中 */
	void ProcessHit(AActor *HitActor, UPrimitiveComponent *HitComp, const FVector &HitLocation, const FVector &HitDirection);

	/** 交给蓝图实现命中特效 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Projectile", meta = (DisplayName = "On Projectile Hit"))
	void BP_OnProjectileHit(const FHitResult &Hit);

	/** 销毁计时器回调 */
	void OnDeferredDestruction();

	/** 处理涂色逻辑 */
	void ProcessPainting(const FHitResult &ImpactHit);

	/** 根据被击中组件的 Mobility 处理碰撞后的行为 */
	void ProcessHitBehavior(UPrimitiveComponent *HitComp, const FHitResult &Hit);

	/** 将投射物附着到静态目标 */
	void AttachToStaticTarget(UPrimitiveComponent *TargetComp, const FHitResult &Hit);

	/** 启用物理模拟（击中可移动物体时） */
	void EnablePhysicsOnHit(const FVector &HitVelocity);
};
