// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeConditionBase.h"

#include "ShooterStateTreeUtility.generated.h"

class AShooterNPC;
class AAIController;
class AShooterAIController;

/**
 *  用于 FStateTreeLineOfSightToTargetCondition 条件的数据结构
 */
USTRUCT()
struct FStateTreeLineOfSightToTargetConditionInstanceData
{
	GENERATED_BODY()

	/** 参与瞄准的角色 */
	UPROPERTY(EditAnywhere, Category = "Context")
	AShooterNPC *Character;

	/** 要检查视线的目标 */
	UPROPERTY(EditAnywhere, Category = "Condition")
	AActor *Target;

	/** 允许的最大视线角度（度） */
	UPROPERTY(EditAnywhere, Category = "Condition")
	float LineOfSightConeAngle = 35.0f;

	/** 垂直方向上要执行的视线检测次数，用于绕过较低障碍 */
	UPROPERTY(EditAnywhere, Category = "Condition")
	int32 NumberOfVerticalLineOfSightChecks = 5;

	/** 若为 true 则角色必须具备视线才算通过 */
	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bMustHaveLineOfSight = true;
};
STATETREE_POD_INSTANCEDATA(FStateTreeLineOfSightToTargetConditionInstanceData);

/**
 *  StateTree 条件：判断角色是否能看到目标
 */
USTRUCT(DisplayName = "Has Line of Sight to Target", Category = "Shooter")
struct FStateTreeLineOfSightToTargetCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	/** 指定实例数据结构类型 */
	using FInstanceDataType = FStateTreeLineOfSightToTargetConditionInstanceData;
	virtual const UStruct *GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	/** 默认构造函数 */
	FStateTreeLineOfSightToTargetCondition() = default;

	/** 执行条件测试 */
	virtual bool TestCondition(FStateTreeExecutionContext &Context) const override;

#if WITH_EDITOR
	/** 返回节点描述文本 */
	virtual FText GetDescription(const FGuid &ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup &BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif
};

////////////////////////////////////////////////////////////////////

/**
 *  面向 Actor 任务的实例数据
 */
USTRUCT()
struct FStateTreeFaceActorInstanceData
{
	GENERATED_BODY()

	/** 负责设置关注目标的 AI 控制器 */
	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<AAIController> Controller;

	/** 要面对的 Actor */
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<AActor> ActorToFaceTowards;
};

/**
 *  StateTree 任务：让 AI 控制的 Pawn 面向某个 Actor
 */
USTRUCT(meta = (DisplayName = "Face Towards Actor", Category = "Shooter"))
struct FStateTreeFaceActorTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	/* 确保使用匹配的实例数据结构 */
	using FInstanceDataType = FStateTreeFaceActorInstanceData;
	virtual const UStruct *GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	/** 状态进入时执行 */
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext &Context, const FStateTreeTransitionResult &Transition) const override;

	/** 状态退出时执行 */
	virtual void ExitState(FStateTreeExecutionContext &Context, const FStateTreeTransitionResult &Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid &ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup &BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif // WITH_EDITOR
};

////////////////////////////////////////////////////////////////////

/**
 *  面向位置任务的实例数据
 */
USTRUCT()
struct FStateTreeFaceLocationInstanceData
{
	GENERATED_BODY()

	/** 用于确定焦点位置的 AI 控制器 */
	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<AAIController> Controller;

	/** 要面对的位置 */
	UPROPERTY(EditAnywhere, Category = Parameter)
	FVector FaceLocation = FVector::ZeroVector;
};

/**
 *  StateTree 任务：让 AI 控制的 Pawn 面向世界位置
 */
USTRUCT(meta = (DisplayName = "Face Towards Location", Category = "Shooter"))
struct FStateTreeFaceLocationTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	/* 确保使用匹配的实例数据结构 */
	using FInstanceDataType = FStateTreeFaceLocationInstanceData;
	virtual const UStruct *GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	/** 状态进入时执行 */
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext &Context, const FStateTreeTransitionResult &Transition) const override;

	/** 状态退出时执行 */
	virtual void ExitState(FStateTreeExecutionContext &Context, const FStateTreeTransitionResult &Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid &ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup &BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif // WITH_EDITOR
};

////////////////////////////////////////////////////////////////////

/**
 *  生成随机浮点数任务的实例数据
 */
USTRUCT()
struct FStateTreeSetRandomFloatData
{
	GENERATED_BODY()

	/** 随机值最小值 */
	UPROPERTY(EditAnywhere, Category = Parameter)
	float MinValue = 0.0f;

	/** 随机值最大值 */
	UPROPERTY(EditAnywhere, Category = Parameter)
	float MaxValue = 0.0f;

	/** 输出的计算值 */
	UPROPERTY(EditAnywhere, Category = Output)
	float OutValue = 0.0f;
};

/**
 *  StateTree 任务：在指定范围内计算随机浮点数
 */
USTRUCT(meta = (DisplayName = "Set Random Float", Category = "Shooter"))
struct FStateTreeSetRandomFloatTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	/* 确保使用匹配的实例数据结构 */
	using FInstanceDataType = FStateTreeSetRandomFloatData;
	virtual const UStruct *GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	/** 状态进入时执行 */
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext &Context, const FStateTreeTransitionResult &Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid &ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup &BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif // WITH_EDITOR
};

////////////////////////////////////////////////////////////////////

/**
 *  射击目标任务的实例数据
 */
USTRUCT()
struct FStateTreeShootAtTargetInstanceData
{
	GENERATED_BODY()

	/** 负责射击的 NPC */
	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<AShooterNPC> Character;

	/** 要射击的目标 */
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<AActor> Target;
};

/**
 *  StateTree 任务：让 NPC 向 Actor 射击
 */
USTRUCT(meta = (DisplayName = "Shoot at Target", Category = "Shooter"))
struct FStateTreeShootAtTargetTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	/* 确保使用匹配的实例数据结构 */
	using FInstanceDataType = FStateTreeShootAtTargetInstanceData;
	virtual const UStruct *GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	/** 状态进入时执行 */
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext &Context, const FStateTreeTransitionResult &Transition) const override;

	/** 状态退出时执行 */
	virtual void ExitState(FStateTreeExecutionContext &Context, const FStateTreeTransitionResult &Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid &ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup &BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif // WITH_EDITOR
};

////////////////////////////////////////////////////////////////////

/**
 *  感知敌人任务的数据结构
 */
USTRUCT()
struct FStateTreeSenseEnemiesInstanceData
{
	GENERATED_BODY()

	/** 负责感知的 AI 控制器 */
	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<AShooterAIController> Controller;

	/** 执行感知的 NPC */
	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<AShooterNPC> Character;

	/** 感知到的目标 Actor */
	UPROPERTY(EditAnywhere, Category = Output)
	TObjectPtr<AActor> TargetActor;

	/** 感知到需要调查的位置 */
	UPROPERTY(EditAnywhere, Category = Output)
	FVector InvestigateLocation = FVector::ZeroVector;

	/** 是否成功感知到目标 */
	UPROPERTY(EditAnywhere, Category = Output)
	bool bHasTarget = false;

	/** 是否有有效的调查位置 */
	UPROPERTY(EditAnywhere, Category = Output)
	bool bHasInvestigateLocation = false;

	/** 感知目标所需的标签 */
	UPROPERTY(EditAnywhere, Category = Parameter)
	FName SenseTag = FName("Player");

	/** 判断为直接感知的视线锥半角 */
	UPROPERTY(EditAnywhere, Category = Parameter)
	float DirectLineOfSightCone = 85.0f;

	/** 最近一次处理的刺激强度 */
	UPROPERTY(EditAnywhere)
	float LastStimulusStrength = 0.0f;
};

/**
 *  StateTree 任务：让 NPC 处理感知并侦测附近敌人
 */
USTRUCT(meta = (DisplayName = "Sense Enemies", Category = "Shooter"))
struct FStateTreeSenseEnemiesTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	/* 确保使用匹配的实例数据结构 */
	using FInstanceDataType = FStateTreeSenseEnemiesInstanceData;
	virtual const UStruct *GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	/** 状态进入时执行 */
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext &Context, const FStateTreeTransitionResult &Transition) const override;

	/** 状态退出时执行 */
	virtual void ExitState(FStateTreeExecutionContext &Context, const FStateTreeTransitionResult &Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid &ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup &BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif // WITH_EDITOR
};

////////////////////////////////////////////////////////////////////