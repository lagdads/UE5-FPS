// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InkSystemComponent.generated.h"

class UTextureRenderTarget2D;
class UMaterialInstanceDynamic;
class UStaticMeshComponent;

/**
 * 可涂色表面组件
 * 附加到可被涂色的 Actor 上（墙壁、地板等）
 * 管理该 Actor 专属的 RenderTarget 和动态材质实例
 */
UCLASS(ClassGroup = (Ink), meta = (BlueprintSpawnableComponent))
class PROJECT2_API UInkSystemComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInkSystemComponent();

protected:
    virtual void BeginPlay() override;

public:
    // ========== 属性 ==========

    /** 存储涂色数据的 Render Target */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ink")
    TObjectPtr<UTextureRenderTarget2D> MyRenderTarget;

    /** M_Inkable_Surface 的动态材质实例 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ink")
    TObjectPtr<UMaterialInstanceDynamic> MyDynamicMaterial;

    /** Render Target 分辨率（宽高相同） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ink", meta = (ClampMin = 128, ClampMax = 2048))
    int32 Resolution = 512;

    /** 要应用动态材质的材质槽索引 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ink", meta = (ClampMin = 0))
    int32 MaterialSlotIndex = 0;

    // ========== 公开方法 ==========

    /** 获取 RenderTarget 用于绘制 */
    UFUNCTION(BlueprintCallable, Category = "Ink")
    UTextureRenderTarget2D *GetRenderTarget() const { return MyRenderTarget; }

    /** 获取当前分辨率 */
    UFUNCTION(BlueprintPure, Category = "Ink")
    int32 GetResolution() const { return Resolution; }

protected:
    /** 缓存的 Owner 的静态网格组件 */
    UPROPERTY()
    TObjectPtr<UStaticMeshComponent> CachedMeshComponent;

    /** 初始化 Render Target */
    void InitializeRenderTarget();

    /** 初始化动态材质并绑定 Render Target */
    void InitializeDynamicMaterial();
};
