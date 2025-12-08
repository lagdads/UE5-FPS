// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Project2PlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;

/**
 *  简单的第一人称玩家控制器
 *  负责输入映射上下文配置，并使用自定义摄像机管理器
 */
UCLASS(abstract, config = "Game")
class PROJECT2_API AProject2PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/** 构造函数 */
	AProject2PlayerController();

protected:
	/** 默认输入映射上下文集合 */
	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<UInputMappingContext *> DefaultMappingContexts;

	/** 仅非触控平台使用的输入映射上下文 */
	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<UInputMappingContext *> MobileExcludedMappingContexts;

	/** 要生成的移动端触控控件类 */
	UPROPERTY(EditAnywhere, Category = "Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** 已生成的触控控件实例指针 */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** 即使不在移动平台也强制使用触控控件 */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** 游戏开始初始化 */
	virtual void BeginPlay() override;

	/** 配置输入映射上下文 */
	virtual void SetupInputComponent() override;

	/** 判断是否应启用 UMG 触控控件 */
	bool ShouldUseTouchControls() const;
};
