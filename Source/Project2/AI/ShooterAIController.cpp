// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/ShooterAIController.h"
#include "ShooterNPC.h"
#include "Components/StateTreeAIComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "AI/Navigation/PathFollowingAgentInterface.h"

// 该文件实现敌方 AI 控制器：负责感知、StateTree 逻辑启动与清理。
AShooterAIController::AShooterAIController()
{
	// 创建 StateTree 组件（延后手动启动）
	StateTreeAI = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeAI"));
	StateTreeAI->SetStartLogicAutomatically(false);

	// 创建感知组件，具体感知配置在蓝图中完成
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

	// 绑定感知更新/遗忘委托，转发给 StateTree
	AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &AShooterAIController::OnPerceptionUpdated);
	AIPerception->OnTargetPerceptionForgotten.AddDynamic(this, &AShooterAIController::OnPerceptionForgotten);
}

void AShooterAIController::OnPossess(APawn *InPawn)
{
	Super::OnPossess(InPawn);

	// 确保控制的是 Shooter NPC
	if (AShooterNPC *NPC = Cast<AShooterNPC>(InPawn))
	{
		// 给角色添加队伍标签，便于识别友军/敌军
		NPC->Tags.Add(TeamTag);

		// 订阅角色死亡事件，便于清理控制器
		NPC->OnPawnDeath.AddDynamic(this, &AShooterAIController::OnPawnDeath);

		// 启动 StateTree AI 逻辑
		StateTreeAI->StartLogic();
	}
}

void AShooterAIController::OnPawnDeath()
{
	// 停止当前移动
	GetPathFollowingComponent()->AbortMove(*this, FPathFollowingResultFlags::UserAbort);

	// 停止 StateTree 逻辑
	StateTreeAI->StopLogic(FString(""));

	// 解除对角色的控制
	UnPossess();

	// 销毁控制器自身
	Destroy();
}

void AShooterAIController::SetCurrentTarget(AActor *Target)
{
	TargetEnemy = Target;
}

void AShooterAIController::ClearCurrentTarget()
{
	TargetEnemy = nullptr;
}

void AShooterAIController::OnPerceptionUpdated(AActor *Actor, FAIStimulus Stimulus)
{
	// 将感知数据转发给 StateTree 委托
	OnShooterPerceptionUpdated.ExecuteIfBound(Actor, Stimulus);
}

void AShooterAIController::OnPerceptionForgotten(AActor *Actor)
{
	// 将遗忘事件转发给 StateTree 委托
	OnShooterPerceptionForgotten.ExecuteIfBound(Actor);
}
