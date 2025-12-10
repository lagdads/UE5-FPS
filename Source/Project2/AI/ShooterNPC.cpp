// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/ShooterNPC.h"
#include "Weapons/ShooterWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "ShooterGameMode.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

void AShooterNPC::BeginPlay()
{
	Super::BeginPlay();

	// 生成武器实例并附着到角色
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	Weapon = GetWorld()->SpawnActor<AShooterWeapon>(WeaponClass, GetActorTransform(), SpawnParams);
}

void AShooterNPC::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// 清理死亡计时器，避免残留回调
	GetWorld()->GetTimerManager().ClearTimer(DeathTimer);
}

float AShooterNPC::TakeDamage(float Damage, struct FDamageEvent const &DamageEvent, AController *EventInstigator, AActor *DamageCauser)
{
	// 已死亡则忽略后续伤害
	if (bIsDead)
	{
		return 0.0f;
	}

	// 扣减生命值
	CurrentHP -= Damage;

	// 判断是否死亡
	if (CurrentHP <= 0.0f)
	{
		Die();
	}

	return Damage;
}

void AShooterNPC::AttachWeaponMeshes(AShooterWeapon *WeaponToAttach)
{
	const FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget, false);

	// 附加武器 Actor 到角色（保持变换）
	WeaponToAttach->AttachToActor(this, AttachmentRule);

	// 附加第一人称与第三人称武器网格到各自挂点
	WeaponToAttach->GetFirstPersonMesh()->AttachToComponent(GetFirstPersonMesh(), AttachmentRule, FirstPersonWeaponSocket);
	WeaponToAttach->GetThirdPersonMesh()->AttachToComponent(GetMesh(), AttachmentRule, ThirdPersonWeaponSocket);
}

void AShooterNPC::PlayFiringMontage(UAnimMontage *Montage)
{
	// 未使用：NPC 不播放开火蒙太奇
}

void AShooterNPC::AddWeaponRecoil(float Recoil)
{
	// 未使用：NPC 不应用后坐力给玩家
}

void AShooterNPC::UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize)
{
	// 未使用：NPC 不更新 HUD
}

FVector AShooterNPC::GetWeaponTargetLocation()
{
	// 从相机位置开始计算射线
	const FVector AimSource = GetFirstPersonCameraComponent()->GetComponentLocation();

	FVector AimDir, AimTarget = FVector::ZeroVector;

	// 是否有指定目标
	if (CurrentAimTarget)
	{
		// 以目标位置为基准
		AimTarget = CurrentAimTarget->GetActorLocation();

		// 在竖直方向随机偏移（模拟命中头/躯干）
		AimTarget.Z += FMath::RandRange(MinAimOffsetZ, MaxAimOffsetZ);

		// 计算方向并在锥体内加入随机
		AimDir = (AimTarget - AimSource).GetSafeNormal();
		AimDir = UKismetMathLibrary::RandomUnitVectorInConeInDegrees(AimDir, AimVarianceHalfAngle);
	}
	else
	{

		// 无目标则使用相机朝向并加随机散布
		AimDir = UKismetMathLibrary::RandomUnitVectorInConeInDegrees(GetFirstPersonCameraComponent()->GetForwardVector(), AimVarianceHalfAngle);
	}

	// 预估无阻挡的命中位置
	AimTarget = AimSource + (AimDir * AimRange);

	// 做一次可见性射线检测，确认是否被阻挡
	FHitResult OutHit;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(OutHit, AimSource, AimTarget, ECC_Visibility, QueryParams);

	// 返回命中点或射线终点
	return OutHit.bBlockingHit ? OutHit.ImpactPoint : OutHit.TraceEnd;
}

void AShooterNPC::AddWeaponClass(const TSubclassOf<AShooterWeapon> &InWeaponClass)
{
	// 未使用：NPC 不在此添加武器类型
}

void AShooterNPC::OnWeaponActivated(AShooterWeapon *InWeapon)
{
	// 未使用：NPC 不在此处理武器激活
}

void AShooterNPC::OnWeaponDeactivated(AShooterWeapon *InWeapon)
{
	// 未使用：NPC 不在此处理武器停用
}

void AShooterNPC::OnSemiWeaponRefire()
{
	// 仍在射击状态则继续触发开火
	if (bIsShooting)
	{
		// 通知武器开火
		Weapon->StartFiring();
	}
}

void AShooterNPC::Die()
{
	// 已死亡则不重复处理
	if (bIsDead)
	{
		return;
	}

	// 标记死亡
	bIsDead = true;

	// 添加死亡标签
	Tags.Add(DeathTag);

	// 广播死亡事件
	OnPawnDeath.Broadcast();

	// 计分：通知游戏模式
	if (AShooterGameMode *GM = Cast<AShooterGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->IncrementTeamScore(TeamByte);
	}

	// 关闭胶囊体碰撞
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 停止移动
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->StopActiveMovement();

	// 启用第三人称网格布娃娃
	GetMesh()->SetCollisionProfileName(RagdollCollisionProfile);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetPhysicsBlendWeight(1.0f);

	// 启动延迟销毁计时器
	GetWorld()->GetTimerManager().SetTimer(DeathTimer, this, &AShooterNPC::DeferredDestruction, DeferredDestructionTime, false);
}

void AShooterNPC::DeferredDestruction()
{
	Destroy();
}

void AShooterNPC::StartShooting(AActor *ActorToShoot)
{
	// 记录当前目标
	CurrentAimTarget = ActorToShoot;

	// 标记正在射击
	bIsShooting = true;

	// 通知武器开始开火
	Weapon->StartFiring();
}

void AShooterNPC::StopShooting()
{
	// 取消射击标记
	bIsShooting = false;

	// 通知武器停止开火
	Weapon->StopFiring();
}
