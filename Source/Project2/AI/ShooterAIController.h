// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ShooterAIController.generated.h"

class UStateTreeAIComponent;
class UAIPerceptionComponent;
struct FAIStimulus;

DECLARE_DELEGATE_TwoParams(FShooterPerceptionUpdatedDelegate, AActor *, const FAIStimulus &);
DECLARE_DELEGATE_OneParam(FShooterPerceptionForgottenDelegate, AActor *);

/**
 *  简单的第一人称射击敌人 AI 控制器
 */
UCLASS(abstract)
class PROJECT2_API AShooterAIController : public AAIController
{
	GENERATED_BODY()

	/** 运行 NPC 的 StateTree 行为树组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStateTreeAIComponent *StateTreeAI;

	/** 通过视觉、听觉等感知其他角色的感知组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UAIPerceptionComponent *AIPerception;

protected:
	/** 用于阵营识别的标签（区分友军/敌军） */
	UPROPERTY(EditAnywhere, Category = "Shooter")
	FName TeamTag = FName("Enemy");

	/** 当前锁定的敌人 */
	TObjectPtr<AActor> TargetEnemy;

public:
	/** AI 感知更新时触发的委托（供 StateTree 任务使用） */
	FShooterPerceptionUpdatedDelegate OnShooterPerceptionUpdated;

	/** AI 忘记某个感知目标时触发的委托（供 StateTree 任务使用） */
	FShooterPerceptionForgottenDelegate OnShooterPerceptionForgotten;

public:
	/** 构造函数：创建并配置 AI 组件 */
	AShooterAIController();

protected:
	/** 角色初始化：设置队伍、绑定死亡委托并启动 StateTree */
	virtual void OnPossess(APawn *InPawn) override;

protected:
	/** 被控制的角色死亡时回调，负责清理控制器 */
	UFUNCTION()
	void OnPawnDeath();

public:
	/** 设置当前锁定的敌人 */
	void SetCurrentTarget(AActor *Target);

	/** 清除当前锁定的敌人 */
	void ClearCurrentTarget();

	/** 获取当前锁定的敌人 */
	AActor *GetCurrentTarget() const { return TargetEnemy; };

protected:
	/** 感知组件更新到某个 Actor 时回调（传递给 StateTree） */
	UFUNCTION()
	void OnPerceptionUpdated(AActor *Actor, FAIStimulus Stimulus);

	/** 感知组件遗忘某个 Actor 时回调（传递给 StateTree） */
	UFUNCTION()
	void OnPerceptionForgotten(AActor *Actor);
};
