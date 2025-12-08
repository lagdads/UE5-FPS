// Copyright Epic Games, Inc. All Rights Reserved.

#include "Variant_Shooter/AI/ShooterStateTreeUtility.h"
#include "StateTreeExecutionContext.h"
#include "ShooterNPC.h"
#include "Camera/CameraComponent.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "ShooterAIController.h"
#include "StateTreeAsyncExecutionContext.h"

bool FStateTreeLineOfSightToTargetCondition::TestCondition(FStateTreeExecutionContext &Context) const
{
	const FInstanceDataType &InstanceData = Context.GetInstanceData(*this);

	// 保证目标仍然有效
	if (!IsValid(InstanceData.Target))
	{
		return !InstanceData.bMustHaveLineOfSight;
	}

	// 检查角色是否朝向目标
	const FVector TargetDir = (InstanceData.Target->GetActorLocation() - InstanceData.Character->GetActorLocation()).GetSafeNormal();

	const float FacingDot = FVector::DotProduct(TargetDir, InstanceData.Character->GetActorForwardVector());
	const float MaxDot = FMath::Cos(FMath::DegreesToRadians(InstanceData.LineOfSightConeAngle));

	// 面向方向是否落在视线锥之外？
	if (FacingDot <= MaxDot)
	{
		return !InstanceData.bMustHaveLineOfSight;
	}

	// 获取目标边界盒
	FVector CenterOfMass, Extent;
	InstanceData.Target->GetActorBounds(true, CenterOfMass, Extent, false);

	// 将垂直高度平均分配到每次视线检测
	const float ExtentZOffset = Extent.Z * 2.0f / InstanceData.NumberOfVerticalLineOfSightChecks;

	// 以角色相机位置为射线起点
	const FVector Start = InstanceData.Character->GetFirstPersonCameraComponent()->GetComponentLocation();

	// 忽略角色和目标，确保检测到的是障碍物
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(InstanceData.Character);
	QueryParams.AddIgnoredActor(InstanceData.Target);

	FHitResult OutHit;

	// 多次垂直偏移地执行视线检测
	for (int32 i = 0; i < InstanceData.NumberOfVerticalLineOfSightChecks - 1; ++i)
	{
		// 计算射线终点高度
		const FVector End = CenterOfMass + FVector(0.0f, 0.0f, Extent.Z - ExtentZOffset * i);

		InstanceData.Character->GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, QueryParams);

		// 检查射线是否未被阻挡
		if (!OutHit.bBlockingHit)
		{
			// 只要有一次不受阻就可以提前返回
			return InstanceData.bMustHaveLineOfSight;
		}
	}

	// 没有通过任何视线检测
	return !InstanceData.bMustHaveLineOfSight;
}

#if WITH_EDITOR
FText FStateTreeLineOfSightToTargetCondition::GetDescription(const FGuid &ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup &BindingLookup, EStateTreeNodeFormatting Formatting /*= EStateTreeNodeFormatting::Text*/) const
{
	return FText::FromString("<b>Has Line of Sight</b>");
}
#endif

////////////////////////////////////////////////////////////////////

EStateTreeRunStatus FStateTreeFaceActorTask::EnterState(FStateTreeExecutionContext &Context, const FStateTreeTransitionResult &Transition) const
{
	// 是否从其他状态切换过来？
	if (Transition.ChangeType == EStateTreeStateChangeType::Changed)
	{
		// 获取实例数据
		FInstanceDataType &InstanceData = Context.GetInstanceData(*this);

		// 设置 AI 控制器关注的 Actor
		InstanceData.Controller->SetFocus(InstanceData.ActorToFaceTowards);
	}

	return EStateTreeRunStatus::Running;
}

void FStateTreeFaceActorTask::ExitState(FStateTreeExecutionContext &Context, const FStateTreeTransitionResult &Transition) const
{
	// 是否切换到其他状态？
	if (Transition.ChangeType == EStateTreeStateChangeType::Changed)
	{
		// 获取实例数据
		FInstanceDataType &InstanceData = Context.GetInstanceData(*this);

		// 清除控制器的关注点
		InstanceData.Controller->ClearFocus(EAIFocusPriority::Gameplay);
	}
}

#if WITH_EDITOR
FText FStateTreeFaceActorTask::GetDescription(const FGuid &ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup &BindingLookup, EStateTreeNodeFormatting Formatting /*= EStateTreeNodeFormatting::Text*/) const
{
	return FText::FromString("<b>Face Towards Actor</b>");
}
#endif // WITH_EDITOR

////////////////////////////////////////////////////////////////////

EStateTreeRunStatus FStateTreeFaceLocationTask::EnterState(FStateTreeExecutionContext &Context, const FStateTreeTransitionResult &Transition) const
{
	// 是否从其他状态切换过来？
	if (Transition.ChangeType == EStateTreeStateChangeType::Changed)
	{
		// 取出实例数据
		FInstanceDataType &InstanceData = Context.GetInstanceData(*this);

		// 设置 AI 控制器的注视点
		InstanceData.Controller->SetFocalPoint(InstanceData.FaceLocation);
	}

	return EStateTreeRunStatus::Running;
}

void FStateTreeFaceLocationTask::ExitState(FStateTreeExecutionContext &Context, const FStateTreeTransitionResult &Transition) const
{
	// 是否切换到其他状态？
	if (Transition.ChangeType == EStateTreeStateChangeType::Changed)
	{
		// 取出实例数据
		FInstanceDataType &InstanceData = Context.GetInstanceData(*this);

		// 清除 AI 控制器的注视点
		InstanceData.Controller->ClearFocus(EAIFocusPriority::Gameplay);
	}
}

#if WITH_EDITOR
FText FStateTreeFaceLocationTask::GetDescription(const FGuid &ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup &BindingLookup, EStateTreeNodeFormatting Formatting /*= EStateTreeNodeFormatting::Text*/) const
{
	return FText::FromString("<b>Face Towards Location</b>");
}
#endif // WITH_EDITOR

////////////////////////////////////////////////////////////////////

EStateTreeRunStatus FStateTreeSetRandomFloatTask::EnterState(FStateTreeExecutionContext &Context, const FStateTreeTransitionResult &Transition) const
{
	// 是否刚刚切换到这个状态？
	if (Transition.ChangeType == EStateTreeStateChangeType::Changed)
	{
		// 取出任务对应的实例数据
		FInstanceDataType &InstanceData = Context.GetInstanceData(*this);

		// 计算随机数输出
		InstanceData.OutValue = FMath::RandRange(InstanceData.MinValue, InstanceData.MaxValue);
	}

	return EStateTreeRunStatus::Running;
}

#if WITH_EDITOR
FText FStateTreeSetRandomFloatTask::GetDescription(const FGuid &ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup &BindingLookup, EStateTreeNodeFormatting Formatting /*= EStateTreeNodeFormatting::Text*/) const
{
	return FText::FromString("<b>Set Random Float</b>");
}
#endif // WITH_EDITOR

////////////////////////////////////////////////////////////////////

EStateTreeRunStatus FStateTreeShootAtTargetTask::EnterState(FStateTreeExecutionContext &Context, const FStateTreeTransitionResult &Transition) const
{
	// 是否从其他状态切换过来？
	if (Transition.ChangeType == EStateTreeStateChangeType::Changed)
	{
		// 取出任务实例数据
		FInstanceDataType &InstanceData = Context.GetInstanceData(*this);

		// 让角色开始朝目标射击
		InstanceData.Character->StartShooting(InstanceData.Target);
	}

	return EStateTreeRunStatus::Running;
}

void FStateTreeShootAtTargetTask::ExitState(FStateTreeExecutionContext &Context, const FStateTreeTransitionResult &Transition) const
{
	// 是否切换到其他状态？
	if (Transition.ChangeType == EStateTreeStateChangeType::Changed)
	{
		// 取出任务实例数据
		FInstanceDataType &InstanceData = Context.GetInstanceData(*this);

		// 让角色停止射击
		InstanceData.Character->StopShooting();
	}
}

#if WITH_EDITOR
FText FStateTreeShootAtTargetTask::GetDescription(const FGuid &ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup &BindingLookup, EStateTreeNodeFormatting Formatting /*= EStateTreeNodeFormatting::Text*/) const
{
	return FText::FromString("<b>Shoot at Target</b>");
}
#endif // WITH_EDITOR

EStateTreeRunStatus FStateTreeSenseEnemiesTask::EnterState(FStateTreeExecutionContext &Context, const FStateTreeTransitionResult &Transition) const
{
	// 是否刚刚切换到这个状态？
	if (Transition.ChangeType == EStateTreeStateChangeType::Changed)
	{
		// 取出任务实例数据
		FInstanceDataType &InstanceData = Context.GetInstanceData(*this);

		// 将感知更新委托绑定到控制器
		InstanceData.Controller->OnShooterPerceptionUpdated.BindLambda(
			[WeakContext = Context.MakeWeakExecutionContext()](AActor *SensedActor, const FAIStimulus &Stimulus)
			{
				// 在 lambda 内获取实例数据
				const FStateTreeStrongExecutionContext StrongContext = WeakContext.MakeStrongExecutionContext();
				if (FInstanceDataType *LambdaInstanceData = StrongContext.GetInstanceDataPtr<FInstanceDataType>())
				{
					if (SensedActor->ActorHasTag(LambdaInstanceData->SenseTag))
					{
						bool bDirectLOS = false;

						// 计算刺激来源方向
						const FVector StimulusDir = (Stimulus.StimulusLocation - LambdaInstanceData->Character->GetActorLocation()).GetSafeNormal();

						// 通过点乘推算角色朝向与刺激方向的夹角
						const float DirDot = FVector::DotProduct(StimulusDir, LambdaInstanceData->Character->GetActorForwardVector());
						const float MaxDot = FMath::Cos(FMath::DegreesToRadians(LambdaInstanceData->DirectLineOfSightCone));

						// 该方向是否在感知锥内？
						if (DirDot >= MaxDot)
						{
							// 在角色与感知的 Actor 之间发射一条射线测试遮挡
							FCollisionQueryParams QueryParams;
							QueryParams.AddIgnoredActor(LambdaInstanceData->Character);
							QueryParams.AddIgnoredActor(SensedActor);

							FHitResult OutHit;

							// 若射线未被阻挡则说明直视到目标
							bDirectLOS = !LambdaInstanceData->Character->GetWorld()->LineTraceSingleByChannel(OutHit, LambdaInstanceData->Character->GetActorLocation(), SensedActor->GetActorLocation(), ECC_Visibility, QueryParams);
						}

						// 确认是否真正看到这个刺激源
						if (bDirectLOS)
						{
							// 设置控制器的当前目标
							LambdaInstanceData->Controller->SetCurrentTarget(SensedActor);

							// 填充任务输出值
							LambdaInstanceData->TargetActor = SensedActor;

							// 更新感知状态标记
							LambdaInstanceData->bHasTarget = true;
							LambdaInstanceData->bHasInvestigateLocation = false;

							// 说明当前并未直视目标
						}
						else
						{

							// 如果已有目标则忽略非直视线的感知
							if (!IsValid(LambdaInstanceData->TargetActor))
							{
								// 判断这个刺激是否比之前更强
								if (Stimulus.Strength > LambdaInstanceData->LastStimulusStrength)
								{
									// 更新记录的刺激强度
									LambdaInstanceData->LastStimulusStrength = Stimulus.Strength;

									// 记录调查位置
									LambdaInstanceData->InvestigateLocation = Stimulus.StimulusLocation;

									// 打开调查标记
									LambdaInstanceData->bHasInvestigateLocation = true;
								}
							}
						}
					}
				}
			});

		// 绑定感知遗忘委托。
		InstanceData.Controller->OnShooterPerceptionForgotten.BindLambda(
			[WeakContext = Context.MakeWeakExecutionContext()](AActor *SensedActor)
			{
				// 在 lambda 内获取实例数据
				const FStateTreeStrongExecutionContext StrongContext = WeakContext.MakeStrongExecutionContext();
				if (FInstanceDataType *LambdaInstanceData = StrongContext.GetInstanceDataPtr<FInstanceDataType>())
				{
					bool bForget = false;

					// 是否正在遗忘当前目标？
					if (SensedActor == LambdaInstanceData->TargetActor)
					{
						bForget = true;
					}
					else
					{
						// 是否正在放弃之前部分感知的位置？
						if (!IsValid(LambdaInstanceData->TargetActor))
						{
							bForget = true;
						}
					}

					if (bForget)
					{
						// 清空目标
						LambdaInstanceData->TargetActor = nullptr;

						// 重置标记
						LambdaInstanceData->bHasInvestigateLocation = false;
						LambdaInstanceData->bHasTarget = false;

						// 清除记录的刺激强度
						LambdaInstanceData->LastStimulusStrength = 0.0f;

						// 清空控制器当前目标
						LambdaInstanceData->Controller->ClearCurrentTarget();
						LambdaInstanceData->Controller->ClearFocus(EAIFocusPriority::Gameplay);
					}
				}
			});
	}

	return EStateTreeRunStatus::Running;
}

void FStateTreeSenseEnemiesTask::ExitState(FStateTreeExecutionContext &Context, const FStateTreeTransitionResult &Transition) const
{
	// 退出时是否正在切换到其他状态？
	if (Transition.ChangeType == EStateTreeStateChangeType::Changed)
	{
		// 取出任务实例数据
		FInstanceDataType &InstanceData = Context.GetInstanceData(*this);

		// 解除感知委托的绑定，防止继续接收通知
		InstanceData.Controller->OnShooterPerceptionUpdated.Unbind();
		InstanceData.Controller->OnShooterPerceptionForgotten.Unbind();
	}
}

#if WITH_EDITOR
FText FStateTreeSenseEnemiesTask::GetDescription(const FGuid &ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup &BindingLookup, EStateTreeNodeFormatting Formatting /*= EStateTreeNodeFormatting::Text*/) const
{
	return FText::FromString("<b>Sense Enemies</b>");
}
#endif // WITH_EDITOR