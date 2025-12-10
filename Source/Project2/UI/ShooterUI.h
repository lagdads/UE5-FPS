// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShooterUI.generated.h"

/**
 *  简单的第一人称射击记分板 UI
 *  Blueprint 可以根据数据刷新得分、团队信息等 HUD 元素
 */
UCLASS(abstract)
class PROJECT2_API UShooterUI : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 *  Blueprint 接口，用于接收团队编号与分数，更新顶部记分栏或队伍面板。
	 *  参数 TeamByte 可用于区分玩家所在队伍。
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Shooter", meta = (DisplayName = "Update Score"))
	void BP_UpdateScore(uint8 TeamByte, int32 Score);
};
