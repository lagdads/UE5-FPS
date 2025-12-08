// Copyright Epic Games, Inc. All Rights Reserved.

#include "Variant_Shooter/AI/EnvQueryContext_Target.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "ShooterAIController.h"

void UEnvQueryContext_Target::ProvideContext(FEnvQueryInstance &QueryInstance, FEnvQueryContextData &ContextData) const
{
	// 从查询实例获取控制器
	if (AShooterAIController *Controller = Cast<AShooterAIController>(QueryInstance.Owner))
	{
		// 确保当前目标有效
		if (IsValid(Controller->GetCurrentTarget()))
		{
			// 将目标 Actor 添加到上下文
			UEnvQueryItemType_Actor::SetContextHelper(ContextData, Controller->GetCurrentTarget());
		}
		else
		{

			// 若目标失效则默认使用控制器自身
			UEnvQueryItemType_Actor::SetContextHelper(ContextData, Controller);
		}
	}
}
