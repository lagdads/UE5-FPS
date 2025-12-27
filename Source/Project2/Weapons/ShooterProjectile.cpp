// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Ink/InkSystemComponent.h"
#include "Ink/PaintManager.h"

AShooterProjectile::AShooterProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建碰撞球组件并设为根节点
	RootComponent = CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Component"));

	CollisionComponent->SetSphereRadius(16.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionComponent->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;

	// 创建投射物移动组件（非场景组件，无需 Attach）
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));

	// 注意：不在构造函数中设置速度，应该在 PostInitializeComponents 中设置
	ProjectileMovement->bShouldBounce = true;

	// 设置默认伤害类型
	HitDamageType = UDamageType::StaticClass();
}

void AShooterProjectile::BeginPlay()
{
	Super::BeginPlay();

	// 在 BeginPlay 时应用速度和重力设置
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ProjectileMovement->ProjectileGravityScale = GravityScale;

	// 设置投射物沿着 Actor 的 Forward 方向发射
	ProjectileMovement->Velocity = GetActorForwardVector() * Speed;

	// 忽略发射该投射物的 Pawn，避免自伤
	CollisionComponent->IgnoreActorWhenMoving(GetInstigator(), true);
}

void AShooterProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!ProjectileMovement || bHit)
	{
		return;
	}

	// 获取当前速度
	FVector Vel = ProjectileMovement->Velocity;

	// 分离水平速度（忽略 Z）
	FVector HorizontalVel = FVector(Vel.X, Vel.Y, 0.0f);
	float HorSpeed = HorizontalVel.Size();

	if (HorSpeed > KINDA_SMALL_NUMBER)
	{
		// 指数衰减：newSpeed = speed * exp(-decayRate * dt)
		float DecayFactor = FMath::Exp(-HorizontalDeceleration * DeltaSeconds);
		float NewHorSpeed = HorSpeed * DecayFactor;

		// 重新设置速度向量
		FVector NewHorizontalVel = (HorizontalVel / HorSpeed) * NewHorSpeed;
		ProjectileMovement->Velocity = FVector(NewHorizontalVel.X, NewHorizontalVel.Y, Vel.Z);
	}
}

void AShooterProjectile::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// 清除可能正在等待的销毁定时器
	GetWorld()->GetTimerManager().ClearTimer(DestructionTimer);
}

void AShooterProjectile::NotifyHit(class UPrimitiveComponent *MyComp, AActor *Other, class UPrimitiveComponent *OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult &Hit)
{
	// 已经命中过目标则不再重复处理
	if (bHit)
	{
		return;
	}

	// 忽略与发射者的碰撞（防止子弹刚发射就击中玩家）
	if (Other == GetInstigator() || Other == GetOwner())
	{
		return;
	}

	bHit = true;

	// 禁用碰撞，防止二次触发
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 处理直接命中对象
	ProcessHit(Other, OtherComp, Hit.ImpactPoint, -Hit.ImpactNormal);

	// 处理碰撞后的行为（附着/物理）
	ProcessHitBehavior(OtherComp, Hit);

	// 处理涂色逻辑
	ProcessPainting(Hit);

	// 交给蓝图触发额外特效
	BP_OnProjectileHit(Hit);

	// 决定是否延迟销毁
	if (DeferredDestructionTime > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(DestructionTimer, this, &AShooterProjectile::OnDeferredDestruction, DeferredDestructionTime, false);
	}
	else
	{
		// 立即销毁
		Destroy();
	}
}

void AShooterProjectile::ProcessHit(AActor *HitActor, UPrimitiveComponent *HitComp, const FVector &HitLocation, const FVector &HitDirection)
{
	// 是否命中角色？
	if (ACharacter *HitCharacter = Cast<ACharacter>(HitActor))
	{
		// 默认忽略该投射物的拥有者，除非允许自伤
		if (HitCharacter != GetOwner() || bDamageOwner)
		{
			// 对角色造成伤害
			UGameplayStatics::ApplyDamage(HitCharacter, HitDamage, GetInstigator()->GetController(), this, HitDamageType);
		}
	}
}

void AShooterProjectile::OnDeferredDestruction()
{
	// 延迟销毁投射物
	Destroy();
}

/** 处理涂色逻辑 */
void AShooterProjectile::ProcessPainting(const FHitResult &ImpactHit)
{
	// 1. 二次射线检测 (Double Check)
	// 高速物体的 HitResult 有时可能会丢失 UV 信息，或者 Hit 的位置不够深。
	// 我们沿着子弹反方向做一次短距离射线，确保 "TraceComplex" 开启。

	FVector Start = ImpactHit.Location + (ImpactHit.ImpactNormal * 10.0f); // 从击中点外面一点
	FVector End = ImpactHit.Location - (ImpactHit.ImpactNormal * 20.0f);   // 射入物体内部

	FCollisionQueryParams TraceParams(FName(TEXT("PaintTrace")), true, this);
	TraceParams.bReturnFaceIndex = true; // 必须开启！为了获取 FaceIndex
	TraceParams.bTraceComplex = true;	 // 必须开启！为了通过 Mesh 计算 UV

	FHitResult UVHitResult;
	bool bTraceHit = GetWorld()->LineTraceSingleByChannel(
		UVHitResult,
		Start,
		End,
		ECC_Visibility, // 确保你的地板阻挡这个 Channel
		TraceParams);

	UE_LOG(LogTemp, Log, TEXT("ProcessPainting: bTraceHit=%d, HitActor=%s"),
		   bTraceHit, *GetNameSafe(UVHitResult.GetActor()));

	if (bTraceHit && UVHitResult.Component.IsValid())
	{
		FVector2D UV;
		// 2. 获取 UV (对应蓝图的 FindCollisionUV)
		// 关键点：Channel 设为 1 (Lightmap UV)
		bool bFoundUV = UGameplayStatics::FindCollisionUV(UVHitResult, 1, UV);

		UE_LOG(LogTemp, Log, TEXT("ProcessPainting: bFoundUV=%d, UV=(%f, %f)"),
			   bFoundUV, UV.X, UV.Y);

		if (bFoundUV)
		{
			// 查找目标 Actor 上的 InkSystemComponent
			AActor *HitActor = UVHitResult.GetActor();
			UInkSystemComponent *InkComp = HitActor->FindComponentByClass<UInkSystemComponent>();

			UE_LOG(LogTemp, Log, TEXT("ProcessPainting: InkComp=%s"),
				   InkComp ? TEXT("Found") : TEXT("NULL"));

			if (InkComp)
			{
				// 获取场景中的 PaintManager
				APaintManager *PaintMgr = Cast<APaintManager>(
					UGameplayStatics::GetActorOfClass(GetWorld(), APaintManager::StaticClass()));

				UE_LOG(LogTemp, Log, TEXT("ProcessPainting: PaintMgr=%s, OwningTeam=%d"),
					   PaintMgr ? TEXT("Found") : TEXT("NULL"), (int32)OwningTeam);

				if (PaintMgr)
				{
					// 使用 C++ 涂色系统
					PaintMgr->PaintTargetByTeam(InkComp, UV, OwningTeam);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("ProcessPainting: PaintManager not found in level!"));
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("ProcessPainting: Hit actor '%s' has no InkSystemComponent!"),
					   *GetNameSafe(HitActor));
			}

			// 保留蓝图事件以供自定义扩展（可选）
			TriggerPaintOnActor(HitActor, UV, OwningTeam);
		}
	}
}

/** 根据被击中组件的 Mobility 处理碰撞后的行为 */
void AShooterProjectile::ProcessHitBehavior(UPrimitiveComponent *HitComp, const FHitResult &Hit)
{
	if (!HitComp)
	{
		return;
	}

	// 根据组件的 Mobility 属性分支处理
	switch (HitComp->Mobility)
	{
	case EComponentMobility::Static:
		// 静态物体：附着到目标
		AttachToStaticTarget(HitComp, Hit);
		break;

	case EComponentMobility::Movable:
	case EComponentMobility::Stationary:
		// 可移动物体：启用物理模拟
		EnablePhysicsOnHit(ProjectileMovement ? ProjectileMovement->Velocity : FVector::ZeroVector);
		break;

	default:
		break;
	}
}

/** 将投射物附着到静态目标 */
void AShooterProjectile::AttachToStaticTarget(UPrimitiveComponent *TargetComp, const FHitResult &Hit)
{
	if (!TargetComp || bAttachedToTarget)
	{
		return;
	}

	// 停用投射物移动组件
	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->SetComponentTickEnabled(false);
	}

	// 附着到目标，保持世界位置
	AttachToComponent(
		TargetComp,
		FAttachmentTransformRules::KeepWorldTransform,
		NAME_None);

	bAttachedToTarget = true;

	UE_LOG(LogTemp, Log, TEXT("Projectile attached to static target: %s"), *GetNameSafe(TargetComp->GetOwner()));
}

/** 启用物理模拟（击中可移动物体时） */
void AShooterProjectile::EnablePhysicsOnHit(const FVector &HitVelocity)
{
	// 停用投射物移动组件
	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->SetComponentTickEnabled(false);
	}

	// 切换碰撞配置为物理 Actor
	if (CollisionComponent)
	{
		CollisionComponent->SetCollisionProfileName(PhysicsCollisionProfile);
		CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

		// 启用物理模拟
		CollisionComponent->SetSimulatePhysics(true);

		// 应用当前速度（可选，保持动量）
		if (!HitVelocity.IsNearlyZero())
		{
			CollisionComponent->SetPhysicsLinearVelocity(HitVelocity * 0.5f); // 减半速度避免过于剧烈
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Projectile physics enabled on movable hit"));
}
