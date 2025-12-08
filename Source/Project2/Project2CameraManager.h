// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "Project2CameraManager.generated.h"

/**
 *  基础第一人称摄像机管理器
 *  限制俯仰最小/最大角度
 */
UCLASS()
class AProject2CameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	/** 构造函数 */
	AProject2CameraManager();
};
