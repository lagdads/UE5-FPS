// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterWeapon.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "ShooterProjectile.h"
#include "ShooterWeaponHolder.h"
#include "Components/SceneComponent.h"
#include "TimerManager.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"

AShooterWeapon::AShooterWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建根节点组件
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// 创建第一人称网格
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));
	FirstPersonMesh->SetupAttachment(RootComponent);

	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));
	FirstPersonMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::FirstPerson);
	FirstPersonMesh->bOnlyOwnerSee = true;

	// 创建第三人称网格
	ThirdPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Third Person Mesh"));
	ThirdPersonMesh->SetupAttachment(RootComponent);

	ThirdPersonMesh->SetCollisionProfileName(FName("NoCollision"));
	ThirdPersonMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::WorldSpaceRepresentation);
	ThirdPersonMesh->bOwnerNoSee = true;
}

void AShooterWeapon::BeginPlay()
{
	Super::BeginPlay();

	// 订阅拥有者销毁事件
	GetOwner()->OnDestroyed.AddDynamic(this, &AShooterWeapon::OnOwnerDestroyed);

	// 缓存武器拥有者接口与 Pawn
	WeaponOwner = Cast<IShooterWeaponHolder>(GetOwner());
	PawnOwner = Cast<APawn>(GetOwner());

	// 填充初始弹匣
	CurrentBullets = MagazineSize;

	// 将网格附加给角色
	WeaponOwner->AttachWeaponMeshes(this);
}

void AShooterWeapon::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// 清空射击计时器
	GetWorld()->GetTimerManager().ClearTimer(RefireTimer);
}

void AShooterWeapon::OnOwnerDestroyed(AActor *DestroyedActor)
{
	// 拥有者销毁时同时销毁武器
	Destroy();
}

void AShooterWeapon::ActivateWeapon()
{
	// 恢复武器可见
	SetActorHiddenInGame(false);

	// 通知拥有者武器激活
	WeaponOwner->OnWeaponActivated(this);
}

void AShooterWeapon::DeactivateWeapon()
{
	// 停止射击以禁用武器
	StopFiring();

	// 隐藏武器
	SetActorHiddenInGame(true);

	// 通知拥有者武器停用
	WeaponOwner->OnWeaponDeactivated(this);
}

void AShooterWeapon::StartFiring()
{
	// 记录正在开火
	bIsFiring = true;

	// 计算距离上次开火的时间
	// 如果武器攻击慢且玩家狂按扳机，可能已经超过射击间隔
	const float TimeSinceLastShot = GetWorld()->GetTimeSeconds() - TimeOfLastShot;

	if (TimeSinceLastShot >= RefireRate)
	{
		// 立即射击
		Fire();
	}
	else
	{

		// 全自动武器需要等待剩余的冷却时间
		if (bFullAuto)
		{
			const float RemainingCooldown = RefireRate - TimeSinceLastShot;
			GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AShooterWeapon::Fire, RemainingCooldown, false);
		}
	}
}

void AShooterWeapon::StopFiring()
{
	// 取消开火状态
	bIsFiring = false;

	// 清除射击计时器
	GetWorld()->GetTimerManager().ClearTimer(RefireTimer);
}

void AShooterWeapon::Fire()
{
	// 如果玩家松开扳机则停止继续射击
	if (!bIsFiring)
	{
		return;
	}

	// 检查弹药是否耗尽
	if (CurrentBullets <= 0)
	{
		// 自动重新装填弹匣
		CurrentBullets = MagazineSize;
		WeaponOwner->UpdateWeaponHUD(CurrentBullets, MagazineSize);
		
		// 本次不射击，等待下次触发
		// 可在此处播放换弹音效/动画
		return;
	}

	// 向目标位置发射投射物
	FireProjectile(WeaponOwner->GetWeaponTargetLocation());

	// 记录本次开火时间
	TimeOfLastShot = GetWorld()->GetTimeSeconds();

	// 产生噪声供 AI 感知
	MakeNoise(ShotLoudness, PawnOwner, PawnOwner->GetActorLocation(), ShotNoiseRange, ShotNoiseTag);

	// 全自动模式会继续调度射击
	if (bFullAuto)
	{
		// 继续定时射击
		GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AShooterWeapon::Fire, RefireRate, false);
	}
	else
	{
		// 半自动武器到时间后通知上层
		GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AShooterWeapon::FireCooldownExpired, RefireRate, false);
	}
}

void AShooterWeapon::FireCooldownExpired()
{
	// 通知拥有者半自动武器可以再次射击
	WeaponOwner->OnSemiWeaponRefire();
}

void AShooterWeapon::FireProjectile(const FVector &TargetLocation)
{
	// 检查投射物类是否有效
	if (!ProjectileClass)
	{
		return;
	}

	// 计算投射物生成变换
	FTransform ProjectileTransform = CalculateProjectileSpawnTransform(TargetLocation);

	// 生成投射物
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::OverrideRootScale;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = PawnOwner;

	AShooterProjectile *Projectile = GetWorld()->SpawnActor<AShooterProjectile>(ProjectileClass, ProjectileTransform, SpawnParams);

	// 播放射击动画
	WeaponOwner->PlayFiringMontage(FiringMontage);

	// 添加后坐力反馈
	WeaponOwner->AddWeaponRecoil(FiringRecoil);

	// 消耗子弹
	--CurrentBullets;

	// 通知 HUD 弹药变化
	WeaponOwner->UpdateWeaponHUD(CurrentBullets, MagazineSize);
}

FTransform AShooterWeapon::CalculateProjectileSpawnTransform(const FVector &TargetLocation) const
{
	// 获取枪口位置
	const FVector MuzzleLoc = FirstPersonMesh->GetSocketLocation(MuzzleSocketName);

	// 计算子弹生成点
	const FVector SpawnLoc = MuzzleLoc + ((TargetLocation - MuzzleLoc).GetSafeNormal() * MuzzleOffset);

	// 计算带随机散布的朝向
	const FRotator AimRot = UKismetMathLibrary::FindLookAtRotation(SpawnLoc, TargetLocation + (UKismetMathLibrary::RandomUnitVector() * AimVariance));

	// 返回最终变换
	return FTransform(AimRot, SpawnLoc, FVector::OneVector);
}

const TSubclassOf<UAnimInstance> &AShooterWeapon::GetFirstPersonAnimInstanceClass() const
{
	return FirstPersonAnimInstanceClass;
}

const TSubclassOf<UAnimInstance> &AShooterWeapon::GetThirdPersonAnimInstanceClass() const
{
	return ThirdPersonAnimInstanceClass;
}
