// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterCharacter.h"
#include "Weapons/ShooterWeapon.h"
#include "EnhancedInputComponent.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
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

	// 创建第三人称潜水视角弹簧臂
	// 1. 初始化弹簧臂 (Spring Arm)
	SquidSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SquidSpringArm"));
	SquidSpringArm->SetupAttachment(RootComponent);
	SquidSpringArm->TargetArmLength = 300.0f;				// 摄像机距离
	SquidSpringArm->bUsePawnControlRotation = true;			// 跟随鼠标旋转
	SquidSpringArm->SocketOffset = FVector(0.f, 0.f, 50.f); // 稍微抬高一点

	// 2. 初始化 TPS 摄像机
	SquidCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("SquidCamera"));
	SquidCamera->SetupAttachment(SquidSpringArm, USpringArmComponent::SocketName);
	SquidCamera->bUsePawnControlRotation = false; // 摄像机不需要再旋转，跟随弹簧臂即可

	// 3. 初始化潜水模型 (假设是一个简单的球体或鱿鱼模型)
	SquidMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SquidMesh"));
	SquidMesh->SetupAttachment(RootComponent);
	SquidMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 碰撞由胶囊体负责
	SquidMesh->SetVisibility(false);								// 默认隐藏

	// 默认关闭 TPS 摄像机
	SquidCamera->SetActive(false);

	// 初始化变量
	bIsSquidForm = false;
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

		// 绑定切换鱿鱼形态动作（长按进入，松开退出）
		if (SquidFormAction)
		{
			EnhancedInputComponent->BindAction(SquidFormAction, ETriggerEvent::Started, this, &AShooterCharacter::DoEnterSquidForm);
			EnhancedInputComponent->BindAction(SquidFormAction, ETriggerEvent::Completed, this, &AShooterCharacter::DoExitSquidForm);
			UE_LOG(LogTemp, Warning, TEXT("[DEBUG] SquidFormAction 绑定成功！（长按进入，松开退出）"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[DEBUG] SquidFormAction 为空，未能绑定！请在蓝图中设置 SquidFormAction"));
		}
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
	// 触发当前武器开火（鱿鱼形态下不能开火）
	if (CurrentWeapon && !IsDead() && !bIsSquidForm)
	{
		CurrentWeapon->StartFiring();
	}
}

void AShooterCharacter::DoStopFiring()
{
	// 通知当前武器停止开火
	if (CurrentWeapon && !IsDead() && !bIsSquidForm)
	{
		CurrentWeapon->StopFiring();
	}
}

void AShooterCharacter::DoSwitchWeapon()
{
	// 需要至少两把武器才能切换（鱿鱼形态下不能切换武器）
	if (OwnedWeapons.Num() > 1 && !IsDead() && !bIsSquidForm)
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

void AShooterCharacter::DoEnterSquidForm()
{
	UE_LOG(LogTemp, Warning, TEXT("[DEBUG] DoEnterSquidForm 被调用（按键按下）"));

	// 死亡状态下不允许切换形态
	if (IsDead())
	{
		UE_LOG(LogTemp, Warning, TEXT("[DEBUG] 角色已死亡，无法切换形态"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[DEBUG] 进入鱿鱼形态"));
	EnterSquidForm();
}

void AShooterCharacter::DoExitSquidForm()
{
	UE_LOG(LogTemp, Warning, TEXT("[DEBUG] DoExitSquidForm 被调用（按键松开）"));

	// 死亡状态下不允许切换形态
	if (IsDead())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[DEBUG] 退出鱿鱼形态"));
	ExitSquidForm();
}

void AShooterCharacter::EnterSquidForm()
{
	// 已经是鱿鱼形态则直接返回
	if (bIsSquidForm)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DEBUG] 已经在鱿鱼形态中"));
		return;
	}

	// 标记为鱿鱼形态
	bIsSquidForm = true;
	UE_LOG(LogTemp, Warning, TEXT("[DEBUG] 成功进入鱿鱼形态！"));

	// 1. 切换摄像机：禁用第一人称，启用第三人称
	if (UCameraComponent *FPCamera = GetFirstPersonCameraComponent())
	{
		FPCamera->SetActive(false);
	}
	if (SquidCamera)
	{
		SquidCamera->SetActive(true);
	}

	// 2. 切换网格可见性：隐藏第一人称手臂，显示鱿鱼网格
	if (USkeletalMeshComponent *FPMesh = GetFirstPersonMesh())
	{
		FPMesh->SetVisibility(false, true);
	}
	if (SquidMesh)
	{
		SquidMesh->SetVisibility(true, true);
	}

	// 3. 调整角色碰撞体：变扁以适应潜水姿态
	if (UCapsuleComponent *Capsule = GetCapsuleComponent())
	{
		Capsule->SetCapsuleHalfHeight(SquidCapsuleHeight);
	}

	// 4. 调整移动速度：鱿鱼形态移动更快
	if (UCharacterMovementComponent *Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = SquidMoveSpeed;
	}

	// 5. 调整第三人称身体网格可见性：在鱿鱼形态下可以看到自己
	if (USkeletalMeshComponent *BodyMesh = GetMesh())
	{
		BodyMesh->SetOwnerNoSee(false);
	}

	// 6. 停用当前武器（鱿鱼形态不能使用武器）
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFiring();
		CurrentWeapon->DeactivateWeapon();
	}
}

void AShooterCharacter::ExitSquidForm()
{
	// 不是鱿鱼形态则直接返回
	if (!bIsSquidForm)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DEBUG] 当前不在鱿鱼形态中"));
		return;
	}

	// 标记为普通形态
	bIsSquidForm = false;
	UE_LOG(LogTemp, Warning, TEXT("[DEBUG] 成功退出鱿鱼形态！"));

	// 1. 切换摄像机：启用第一人称，禁用第三人称
	if (UCameraComponent *FPCamera = GetFirstPersonCameraComponent())
	{
		FPCamera->SetActive(true);
	}
	if (SquidCamera)
	{
		SquidCamera->SetActive(false);
	}

	// 2. 切换网格可见性：显示第一人称手臂，隐藏鱿鱼网格
	if (USkeletalMeshComponent *FPMesh = GetFirstPersonMesh())
	{
		FPMesh->SetVisibility(true, true);
	}
	if (SquidMesh)
	{
		SquidMesh->SetVisibility(false, true);
	}

	// 3. 恢复角色碰撞体：恢复正常站立高度
	if (UCapsuleComponent *Capsule = GetCapsuleComponent())
	{
		Capsule->SetCapsuleHalfHeight(NormalCapsuleHeight);
	}

	// 4. 恢复移动速度：普通形态移动较慢
	if (UCharacterMovementComponent *Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = NormalMoveSpeed;
	}

	// 5. 恢复第三人称身体网格可见性：第一人称模式下自己不可见
	if (USkeletalMeshComponent *BodyMesh = GetMesh())
	{
		BodyMesh->SetOwnerNoSee(true);
	}

	// 6. 重新激活当前武器
	if (CurrentWeapon)
	{
		CurrentWeapon->ActivateWeapon();
	}
}
