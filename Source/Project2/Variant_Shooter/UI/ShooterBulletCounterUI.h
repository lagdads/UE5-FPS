// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShooterBulletCounterUI.generated.h"

/**
 *  简单的第一人称射击弹药计数界面
 *  通过 Blueprint 实现具体的 UI 更新逻辑
 */
UCLASS(abstract)
class PROJECT2_API UShooterBulletCounterUI : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 *  Blueprint 接口，收到当前弹匣容量与剩余子弹值后刷新显示。
	 *  实现时可同步更新进度条/数字文本。
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Shooter", meta = (DisplayName = "UpdateBulletCounter"))
	void BP_UpdateBulletCounter(int32 MagazineSize, int32 BulletCount);

	/**
	 *  Blueprint 接口，传递当前生命比例以播放受伤反馈。
	 *  实现可触发屏幕抖动、血量提示或其他 HUD 效果。
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Shooter", meta = (DisplayName = "Damaged"))
	void BP_Damaged(float LifePercent);
};
