// Copyright Epic Games, Inc. All Rights Reserved.

#include "Project2PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Project2CameraManager.h"
#include "Blueprint/UserWidget.h"
#include "Project2.h"
#include "Widgets/Input/SVirtualJoystick.h"

AProject2PlayerController::AProject2PlayerController()
{
	// 指定玩家使用的摄像机管理器类
	PlayerCameraManagerClass = AProject2CameraManager::StaticClass();
}

void AProject2PlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 仅本地控制器生成触控 UI
	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		// 生成移动端触控控件
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// 将控件添加到玩家屏幕
			MobileControlsWidget->AddToPlayerScreen(0);
		}
		else
		{

			UE_LOG(LogProject2, Error, TEXT("Could not spawn mobile controls widget."));
		}
	}
}

void AProject2PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// 仅本地控制器添加输入映射上下文
	if (IsLocalPlayerController())
	{
		// 添加默认输入映射
		if (UEnhancedInputLocalPlayerSubsystem *Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext *CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// 仅在未使用触控时添加额外映射
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext *CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

bool AProject2PlayerController::ShouldUseTouchControls() const
{
	// 判断平台是否需要触控，或是否被配置强制启用
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
