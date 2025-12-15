// Copyright Epic Games, Inc. All Rights Reserved.

#include "Project2Character.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Project2.h"


// 
AProject2Character::AProject2Character()
{
	// 设置角色碰撞胶囊大小
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// 创建仅本地玩家可见的第一人称手臂网格
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// 创建第一人称相机并附加到头部骨骼
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// 配置角色网格：本地不可见（只显示第一人称手臂）
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// 配置移动属性
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
}

void AProject2Character::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	// 绑定增强输入动作
	if (UEnhancedInputComponent *EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// 跳跃开始/结束
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AProject2Character::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AProject2Character::DoJumpEnd);

		// 移动
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AProject2Character::MoveInput);

		// 视角/瞄准（手柄与鼠标）
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AProject2Character::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AProject2Character::LookInput);
	}
	else
	{
		UE_LOG(LogProject2, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}
           
void AProject2Character::MoveInput(const FInputActionValue &Value)
{
	// 读取二维移动输入（右、前）
	FVector2D MovementVector = Value.Get<FVector2D>();

	// 转交给实际移动逻辑
	DoMove(MovementVector.X, MovementVector.Y);
}

void AProject2Character::LookInput(const FInputActionValue &Value)
{
	// 读取二维视角输入（偏航、俯仰）
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// 转交给实际视角逻辑
	DoAim(LookAxisVector.X, LookAxisVector.Y);
}

void AProject2Character::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// 将旋转输入传给控制器
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AProject2Character::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// 将移动输入应用到角色
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void AProject2Character::DoJumpStart()
{
	// 触发角色跳跃
	Jump();
}

void AProject2Character::DoJumpEnd()
{
	// 停止跳跃输入
	StopJumping();
}
