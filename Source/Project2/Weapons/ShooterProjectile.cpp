// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "TimerManager.h"

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

	bHit = true;

	// 禁用碰撞，防止二次触发
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 产生 AI 噪声供感知系统使用
	MakeNoise(NoiseLoudness, GetInstigator(), GetActorLocation(), NoiseRange, NoiseTag);

	if (bExplodeOnHit)
	{

		// 执行爆炸伤害检查
		ExplosionCheck(GetActorLocation());
	}
	else
	{

		// 非爆炸的单体投射物，处理直接命中对象
		ProcessHit(Other, OtherComp, Hit.ImpactPoint, -Hit.ImpactNormal);
	}

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

void AShooterProjectile::ExplosionCheck(const FVector &ExplosionCenter)
{
	// 做一个球形重叠查询，寻找爆炸范围内的对象
	TArray<FOverlapResult> Overlaps;

	FCollisionShape OverlapShape;
	OverlapShape.SetSphere(ExplosionRadius);

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectParams.AddObjectTypesToQuery(ECC_PhysicsBody);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	if (!bDamageOwner)
	{
		QueryParams.AddIgnoredActor(GetInstigator());
	}

	GetWorld()->OverlapMultiByObjectType(Overlaps, ExplosionCenter, FQuat::Identity, ObjectParams, OverlapShape, QueryParams);

	TArray<AActor *> DamagedActors;

	// 处理重叠结果
	for (const FOverlapResult &CurrentOverlap : Overlaps)
	{
		// 重叠查询可能返回同一个演员的多个组件，避免重复伤害
		if (DamagedActors.Find(CurrentOverlap.GetActor()) == INDEX_NONE)
		{
			DamagedActors.Add(CurrentOverlap.GetActor());

			// 将爆炸力施加到该演员身上
			const FVector &ExplosionDir = CurrentOverlap.GetActor()->GetActorLocation() - GetActorLocation();

			// 造成伤害并推动演员
			ProcessHit(CurrentOverlap.GetActor(), CurrentOverlap.GetComponent(), GetActorLocation(), ExplosionDir.GetSafeNormal());
		}
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

	// 是否碰到可模拟物理的组件？
	if (HitComp->IsSimulatingPhysics())
	{
		// 添加冲击力制造反馈
		HitComp->AddImpulseAtLocation(HitDirection * PhysicsForce, HitLocation);
	}
}

void AShooterProjectile::OnDeferredDestruction()
{
	// 延迟销毁投射物
	Destroy();
}
