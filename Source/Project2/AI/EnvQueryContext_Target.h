// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EnvQueryContext_Target.generated.h"

/**
 *  自定义 EnvQuery 上下文，用于将当前 NPC 锁定的目标 Actor 作为查询输入。
 *  EOSqS 使用此上下文即可在查询中访问敌人位置或 AI 控制器本身。
 */
UCLASS()
class PROJECT2_API UEnvQueryContext_Target : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	/**
	 *  当 EQS 请求上下文时，尝试返回当前锁定的目标；若目标失效则使用控制器自身。
	 */
	virtual void ProvideContext(FEnvQueryInstance &QueryInstance, FEnvQueryContextData &ContextData) const override;
};
