// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShooterGameMode.h"
#include "PaintManager.generated.h"

class UInkSystemComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;

/**
 * 涂色管理器
 * 负责将画刷绘制到可涂色表面的 RenderTarget 上
 * 在关卡中放置一个实例，或通过 GameMode 持有
 */
UCLASS(abstract)
class PROJECT2_API APaintManager : public AActor
{
    GENERATED_BODY()

public:
    APaintManager();

protected:
    virtual void BeginPlay() override;

public:
    // ========== 属性 ==========

    /** 画刷材质的源材质（在编辑器中指定 M_Brush_Stamp） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paint|Brush")
    TObjectPtr<UMaterialInterface> BrushSourceMaterial;

    /** 画刷材质的动态实例 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paint|Brush")
    TObjectPtr<UMaterialInstanceDynamic> BrushMatInst;

    /** 默认画刷大小（像素） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paint|Brush", meta = (ClampMin = 8, ClampMax = 256))
    float DefaultBrushSize = 64.0f;

    // ========== 涂色方法 ==========

    /**
     * 在目标表面的指定 UV 位置绘制画刷
     * @param TargetComp		目标可涂色组件
     * @param HitUV				命中的 UV 坐标（0-1 范围）
     * @param TeamID			队伍 ID（0.0 = Team1/红色，1.0 = Team2/绿色）
     * @param BrushSize			画刷大小（像素），默认使用 DefaultBrushSize
     */
    UFUNCTION(BlueprintCallable, Category = "Paint")
    void PaintTarget(UInkSystemComponent *TargetComp, FVector2D HitUV, float TeamID, float BrushSize = 0.0f);

    /**
     * 使用 E_Team 枚举的便捷版本
     * @param TargetComp		目标可涂色组件
     * @param HitUV				命中的 UV 坐标（0-1 范围）
     * @param Team				队伍枚举
     * @param BrushSize			画刷大小（像素），默认使用 DefaultBrushSize
     */
    UFUNCTION(BlueprintCallable, Category = "Paint")
    void PaintTargetByTeam(UInkSystemComponent *TargetComp, FVector2D HitUV, E_Team Team, float BrushSize = 0.0f);

    // ========== 辅助方法 ==========

    /**
     * 将 E_Team 转换为材质使用的 TeamID 浮点值
     * @param Team		队伍枚举
     * @return			0.0 (Team1), 1.0 (Team2), -1.0 (None/无效)
     */
    UFUNCTION(BlueprintPure, Category = "Paint")
    static float TeamToFloat(E_Team Team);

protected:
    /** 初始化画刷材质实例 */
    void InitializeBrushMaterial();
};
