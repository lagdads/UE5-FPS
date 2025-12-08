// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Project2Character.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  基础第一人称角色类
 */
UCLASS(abstract)
class AProject2Character : public ACharacter
{
	GENERATED_BODY()

	/** 第一人称手臂网格，仅自己可见 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent *FirstPersonMesh;

	/** 第一人称相机组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent *FirstPersonCameraComponent;

protected:
	/** 跳跃输入动作 */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction *JumpAction;

	/** 移动输入动作 */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction *MoveAction;

	/** 视角输入动作 */
	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction *LookAction;

	/** 鼠标视角输入动作 */
	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction *MouseLookAction;

public:
	AProject2Character();

protected:
	/** 输入动作回调：处理移动轴 */
	void MoveInput(const FInputActionValue &Value);

	/** 输入动作回调：处理视角轴 */
	void LookInput(const FInputActionValue &Value);

	/** 接收控制器/界面的视角输入 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoAim(float Yaw, float Pitch);

	/** 接收控制器/界面的移动输入 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoMove(float Right, float Forward);

	/** 接收跳跃开始输入 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpStart();

	/** 接收跳跃结束输入 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpEnd();

protected:
	/** 绑定增强输入动作 */
	virtual void SetupPlayerInputComponent(UInputComponent *InputComponent) override;

public:
	/** 获取第一人称网格 **/
	USkeletalMeshComponent *GetFirstPersonMesh() const { return FirstPersonMesh; }

	/** 获取第一人称相机组件 **/
	UCameraComponent *GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
};
