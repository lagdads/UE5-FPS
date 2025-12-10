// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterPickup.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ShooterWeaponHolder.h"
#include "ShooterWeapon.h"
#include "Engine/World.h"
#include "TimerManager.h"

AShooterPickup::AShooterPickup()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建根节点组件
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// 创建检测玩家靠近的碰撞球
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Collision"));
	SphereCollision->SetupAttachment(RootComponent);

	SphereCollision->SetRelativeLocation(FVector(0.0f, 0.0f, 84.0f));
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollision->SetCollisionObjectType(ECC_WorldStatic);
	SphereCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereCollision->bFillCollisionUnderneathForNavmesh = true;

	// 监听碰撞球的开始重叠事件
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AShooterPickup::OnOverlap);

	// 创建用于展示武器外观的静态网格
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(SphereCollision);

	Mesh->SetCollisionProfileName(FName("NoCollision"));
}

void AShooterPickup::OnConstruction(const FTransform &Transform)
{
	Super::OnConstruction(Transform);

	if (FWeaponTableRow *WeaponData = WeaponType.GetRow<FWeaponTableRow>(FString()))
	{
		// 根据数据表填充网格资源
		Mesh->SetStaticMesh(WeaponData->StaticMesh.LoadSynchronous());
	}
}

void AShooterPickup::BeginPlay()
{
	Super::BeginPlay();

	if (FWeaponTableRow *WeaponData = WeaponType.GetRow<FWeaponTableRow>(FString()))
	{
		// 备份武器类以便触发拾取时生成
		WeaponClass = WeaponData->WeaponToSpawn;
	}
}

void AShooterPickup::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// 清除可能排队的重新生成计时器
	GetWorld()->GetTimerManager().ClearTimer(RespawnTimer);
}

void AShooterPickup::OnOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	// 碰撞到实现了武器持有接口的对象了吗？
	if (IShooterWeaponHolder *WeaponHolder = Cast<IShooterWeaponHolder>(OtherActor))
	{
		WeaponHolder->AddWeaponClass(WeaponClass);

		// 隐藏拾取器并暂时禁用交互
		SetActorHiddenInGame(true);

		// 关闭碰撞，避免再次触发
		SetActorEnableCollision(false);

		// 暂停 Tick，减少性能成本
		SetActorTickEnabled(false);

		// 安排延迟重生
		GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &AShooterPickup::RespawnPickup, RespawnTime, false);
	}
}

void AShooterPickup::RespawnPickup()
{
	// 重新出现拾取器
	SetActorHiddenInGame(false);

	// 通知蓝图播放重生效果
	BP_OnRespawn();
}

void AShooterPickup::FinishRespawn()
{
	// 恢复碰撞和 Tick 使拾取器再次可交互
	SetActorEnableCollision(true);

	// 重新启用 Tick
	SetActorTickEnabled(true);
}
