// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterCharacter.h"
#include "ShooterWeapon.h"
#include "EnhancedInputComponent.h"
#include "Components/InputComponent.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "ShooterGameMode.h"

AShooterCharacter::AShooterCharacter()
{
	// 创建 AI 噪声发射器组件
	PawnNoiseEmitter = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("Pawn Noise Emitter"));

	// 配置移动旋转速度
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 600.0f, 0.0f);
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 将生命值重置为最大值
	CurrentHP = MaxHP;

	// 通知 HUD 更新生命状态
	OnDamaged.Broadcast(1.0f);

	// 生成默认武器实例并装备
	if (DefaultWeaponClass)
	{
		AddWeaponClass(DefaultWeaponClass);
	}
}

void AShooterCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// 清理重生计时器
	GetWorld()->GetTimerManager().ClearTimer(RespawnTimer);
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	// 基类处理移动/视角/跳跃输入
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 绑定射击与切换武器输入
	if (UEnhancedInputComponent *EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// 绑定开火动作
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AShooterCharacter::DoStartFiring);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AShooterCharacter::DoStopFiring);

		// 绑定切换武器动作
		EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Triggered, this, &AShooterCharacter::DoSwitchWeapon);
	}
}

float AShooterCharacter::TakeDamage(float Damage, struct FDamageEvent const &DamageEvent, AController *EventInstigator, AActor *DamageCauser)
{
	// 如果已死亡则不再处理伤害
	if (CurrentHP <= 0.0f)
	{
		return 0.0f;
	}

	// 扣除生命值
	CurrentHP -= Damage;

	// 判断是否需要死亡处理
	if (CurrentHP <= 0.0f)
	{
		Die();
	}

	// 通知 HUD 生命值变化
	OnDamaged.Broadcast(FMath::Max(0.0f, CurrentHP / MaxHP));

	return Damage;
}

void AShooterCharacter::DoAim(float Yaw, float Pitch)
{
	// 角色未死亡时才允许处理视角输入
	if (!IsDead())
	{
		Super::DoAim(Yaw, Pitch);
	}
}

void AShooterCharacter::DoMove(float Right, float Forward)
{
	// 角色未死亡时才执行移动
	if (!IsDead())
	{
		Super::DoMove(Right, Forward);
	}
}

void AShooterCharacter::DoJumpStart()
{
	// 角色未死亡时才允许跳跃开始
	if (!IsDead())
	{
		Super::DoJumpStart();
	}
}

void AShooterCharacter::DoJumpEnd()
{
	// 角色未死亡时才允许跳跃结束
	if (!IsDead())
	{
		Super::DoJumpEnd();
	}
}

void AShooterCharacter::DoStartFiring()
{
	// 触发当前武器开火
	if (CurrentWeapon && !IsDead())
	{
		CurrentWeapon->StartFiring();
	}
}

void AShooterCharacter::DoStopFiring()
{
	// 通知当前武器停止开火
	if (CurrentWeapon && !IsDead())
	{
		CurrentWeapon->StopFiring();
	}
}

void AShooterCharacter::DoSwitchWeapon()
{
	// 需要至少两把武器才能切换
	if (OwnedWeapons.Num() > 1 && !IsDead())
	{
		// 关闭当前武器
		CurrentWeapon->DeactivateWeapon();

		// 查找当前武器在列表中的索引
		int32 WeaponIndex = OwnedWeapons.Find(CurrentWeapon);

		// 如果已经是最后一把，则回到数组开头继续循环
		if (WeaponIndex == OwnedWeapons.Num() - 1)
		{
			// 回到武器数组起点
			WeaponIndex = 0;
		}
		else
		{
			// 选择下一把武器
			++WeaponIndex;
		}

		// 更新当前武器
		CurrentWeapon = OwnedWeapons[WeaponIndex];

		// 激活新武器
		CurrentWeapon->ActivateWeapon();
	}
}

void AShooterCharacter::AttachWeaponMeshes(AShooterWeapon *Weapon)
{
	const FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget, false);

	// 附加武器 Actor 到角色
	Weapon->AttachToActor(this, AttachmentRule);

	// 将武器第一/第三人称网格挂到指定插槽
	Weapon->GetFirstPersonMesh()->AttachToComponent(GetFirstPersonMesh(), AttachmentRule, FirstPersonWeaponSocket);
	Weapon->GetThirdPersonMesh()->AttachToComponent(GetMesh(), AttachmentRule, FirstPersonWeaponSocket);
}

void AShooterCharacter::PlayFiringMontage(UAnimMontage *Montage)
{
	// 占位：可在蓝图中扩展射击动画
}

void AShooterCharacter::AddWeaponRecoil(float Recoil)
{
	// 将后坐力转换为俯仰输入
	AddControllerPitchInput(Recoil);
}

void AShooterCharacter::UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize)
{
	OnBulletCountUpdated.Broadcast(MagazineSize, CurrentAmmo);
}

FVector AShooterCharacter::GetWeaponTargetLocation()
{
	// 从相机视角向前发射射线来检测瞄准点
	FHitResult OutHit;

	const FVector Start = GetFirstPersonCameraComponent()->GetComponentLocation();
	const FVector End = Start + (GetFirstPersonCameraComponent()->GetForwardVector() * MaxAimDistance);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, QueryParams);

	// 返回命中点或射线终点
	return OutHit.bBlockingHit ? OutHit.ImpactPoint : OutHit.TraceEnd;
}

void AShooterCharacter::AddWeaponClass(const TSubclassOf<AShooterWeapon> &WeaponClass)
{
	// 是否已拥有该武器
	AShooterWeapon *OwnedWeapon = FindWeaponOfType(WeaponClass);

	if (!OwnedWeapon)
	{
		// 生成新武器并配置
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;

		AShooterWeapon *AddedWeapon = GetWorld()->SpawnActor<AShooterWeapon>(WeaponClass, GetActorTransform(), SpawnParams);

		if (AddedWeapon)
		{
			// 添加到拥有武器列表
			OwnedWeapons.Add(AddedWeapon);

			// 如果已有武器则先停用
			if (CurrentWeapon)
			{
				CurrentWeapon->DeactivateWeapon();
			}

			// 切换到新武器
			CurrentWeapon = AddedWeapon;
			CurrentWeapon->ActivateWeapon();
		}
	}
}

void AShooterCharacter::OnWeaponActivated(AShooterWeapon *Weapon)
{
	// 更新子弹计数 HUD
	OnBulletCountUpdated.Broadcast(Weapon->GetMagazineSize(), Weapon->GetBulletCount());

	// 更新角色网格的动画实例类
	GetFirstPersonMesh()->SetAnimInstanceClass(Weapon->GetFirstPersonAnimInstanceClass());
	GetMesh()->SetAnimInstanceClass(Weapon->GetThirdPersonAnimInstanceClass());
}

void AShooterCharacter::OnWeaponDeactivated(AShooterWeapon *Weapon)
{
	// 未使用：可在蓝图中扩展
}

void AShooterCharacter::OnSemiWeaponRefire()
{
	// 未使用：可用于半自动武器逻辑
}

AShooterWeapon *AShooterCharacter::FindWeaponOfType(TSubclassOf<AShooterWeapon> WeaponClass) const
{
	// 遍历已拥有的武器
	for (AShooterWeapon *Weapon : OwnedWeapons)
	{
		if (Weapon->IsA(WeaponClass))
		{
			return Weapon;
		}
	}

	// 未找到匹配武器
	return nullptr;
}

void AShooterCharacter::Die()
{
	// 停用当前武器，防止继续开火
	if (IsValid(CurrentWeapon))
	{
		CurrentWeapon->DeactivateWeapon();
	}

	// 通知游戏模式增加进攻方得分
	if (AShooterGameMode *GM = Cast<AShooterGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->IncrementTeamScore(TeamByte);
	}

	// 添加死亡标签
	Tags.Add(DeathTag);

	// 停止角色移动
	GetCharacterMovement()->StopMovementImmediately();

	// 禁用玩家输入
	DisableInput(nullptr);

	// 清除子弹 UI
	OnBulletCountUpdated.Broadcast(0, 0);

	// 触发蓝图死亡事件
	BP_OnDeath();

	// 启动重生计时器
	GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &AShooterCharacter::OnRespawn, RespawnTime, false);
}

void AShooterCharacter::OnRespawn()
{
	// 销毁角色以触发玩家重生
	Destroy();
}

bool AShooterCharacter::IsDead() const
{
	// 生命值小于等于 0 视为死亡
	return CurrentHP <= 0.0f;
}
