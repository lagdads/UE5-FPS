// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
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

	/** 命中后产生的 AI 噪声音量 */
	UPROPERTY(EditAnywhere, Category = "Projectile|Noise", meta = (ClampMin = 0, ClampMax = 100))
	float NoiseLoudness = 3.0f;

	/** 命中后噪声影响范围 */
	UPROPERTY(EditAnywhere, Category = "Projectile|Noise", meta = (ClampMin = 0, ClampMax = 100000, Units = "cm"))
	float NoiseRange = 3000.0f;

	/** 噪声标签 */
	UPROPERTY(EditAnywhere, Category = "Noise")
	FName NoiseTag = FName("Projectile");

	/** 命中时施加的物理冲击 */
	UPROPERTY(EditAnywhere, Category = "Projectile|Hit", meta = (ClampMin = 0, ClampMax = 50000))
	float PhysicsForce = 100.0f;

	/** 命中造成的伤害 */
	UPROPERTY(EditAnywhere, Category = "Projectile|Hit", meta = (ClampMin = 0, ClampMax = 100))
	float HitDamage = 25.0f;

	/** 伤害类型，可表示火焰、爆炸等 */
	UPROPERTY(EditAnywhere, Category = "Projectile|Hit")
	TSubclassOf<UDamageType> HitDamageType;

	/** 是否允许伤害自身 */
	UPROPERTY(EditAnywhere, Category = "Projectile|Hit")
	bool bDamageOwner = false;

	/** 是否爆炸并对周围角色造成径向伤害 */
	UPROPERTY(EditAnywhere, Category = "Projectile|Explosion")
	bool bExplodeOnHit = false;

	/** 爆炸影响的最大距离 */
	UPROPERTY(EditAnywhere, Category = "Projectile|Explosion", meta = (ClampMin = 0, ClampMax = 5000, Units = "cm"))
	float ExplosionRadius = 500.0f;

	/** 是否已命中某个表面 */
	bool bHit = false;

	/** 命中后等待多长时间再销毁 */
	UPROPERTY(EditAnywhere, Category = "Projectile|Destruction", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float DeferredDestructionTime = 5.0f;

	/** 延迟销毁计时器 */
	FTimerHandle DestructionTimer;

public:
	/** 构造函数 */
	AShooterProjectile();

	/** 每帧更新：用于处理水平减速 */
	virtual void Tick(float DeltaSeconds) override;

protected:
	/** 游戏初始化 */
	virtual void BeginPlay() override;

	/** 游戏结束清理 */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	/** 处理碰撞 */
	virtual void NotifyHit(class UPrimitiveComponent *MyComp, AActor *Other, UPrimitiveComponent *OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult &Hit) override;

protected:
	/** 检查爆炸范围内的角色并造成伤害 */
	void ExplosionCheck(const FVector &ExplosionCenter);

	/** 处理单个命中 */
	void ProcessHit(AActor *HitActor, UPrimitiveComponent *HitComp, const FVector &HitLocation, const FVector &HitDirection);

	/** 交给蓝图实现命中特效 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Projectile", meta = (DisplayName = "On Projectile Hit"))
	void BP_OnProjectileHit(const FHitResult &Hit);

	/** 销毁计时器回调 */
	void OnDeferredDestruction();
};
