// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Engine/StaticMesh.h"
#include "ShooterPickup.generated.h"

class USphereComponent;
class UPrimitiveComponent;
class AShooterWeapon;

/**
 *  用于数据表中的武器拾取器配置行，描述拾取器所展示的模型与发放的武器类。
 */
USTRUCT(BlueprintType)
struct FWeaponTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 拾取器上展示的静态网格资源 */
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UStaticMesh> StaticMesh;

	/** 玩家触碰时授予的武器类 */
	UPROPERTY(EditAnywhere)
	TSubclassOf<AShooterWeapon> WeaponToSpawn;
};

/**
 *  简单的武器拾取器
 *  通过 SphereCollision 触发与拥有 IShooterWeaponHolder 接口的对象交互，授予武器并隐藏自身。
 */
UCLASS(abstract)
class PROJECT2_API AShooterPickup : public AActor
{
	GENERATED_BODY()

	/** 检测玩家靠近的碰撞球 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USphereComponent *SphereCollision;

	/** 表面展示的武器模型，构造时由 WeaponType 表格填写 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent *Mesh;

protected:
	/** 通过数据表选择的武器型号与可视信息 */
	UPROPERTY(EditAnywhere, Category = "Pickup")
	FDataTableRowHandle WeaponType;

	/** 摆在地图上的武器类，BeginPlay 时读取 WeaponType 表格填充 */
	TSubclassOf<AShooterWeapon> WeaponClass;

	/** 重新生成拾取器前的等待时间（秒） */
	UPROPERTY(EditAnywhere, Category = "Pickup", meta = (ClampMin = 0, ClampMax = 120, Units = "s"))
	float RespawnTime = 4.0f;

	/** Timer to respawn the pickup */
	FTimerHandle RespawnTimer;

public:
	/** 构造函数，创建根节点以及可视与碰撞组件 */
	AShooterPickup();

protected:
	/** 编辑器中构造脚本，用于设置模型 */
	virtual void OnConstruction(const FTransform &Transform) override;

	/** 游戏开始时的初始化 */
	virtual void BeginPlay() override;

	/** 游戏结束时的清理 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 处理玩家接触 */
	UFUNCTION()
	virtual void OnOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

protected:
	/** 延迟后再次显现拾取器 */
	void RespawnPickup();

	/**
	 *  Blueprint 回调，可在动画结束后调用 FinishRespawn 重新启用碰撞。
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Pickup", meta = (DisplayName = "OnRespawn"))
	void BP_OnRespawn();

	/** 在 Blueprint 动画完成后恢复碰撞和绘制 */
	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void FinishRespawn();
};
