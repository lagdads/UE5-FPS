// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Project2GameMode.generated.h"

/**
 *  简单的第一人称游戏模式
 */
UCLASS(abstract)
class AProject2GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AProject2GameMode();
};
