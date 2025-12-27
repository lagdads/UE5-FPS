// Copyright Epic Games, Inc. All Rights Reserved.

#include "PaintManager.h"
#include "InkSystemComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Canvas.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetRenderingLibrary.h"

APaintManager::APaintManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void APaintManager::BeginPlay()
{
    Super::BeginPlay();

    InitializeBrushMaterial();
}

void APaintManager::InitializeBrushMaterial()
{
    if (!BrushSourceMaterial)
    {
        UE_LOG(LogTemp, Warning, TEXT("PaintManager: BrushSourceMaterial is not set! Assign M_Brush_Stamp in the Editor."));
        return;
    }

    // 创建画刷材质的动态实例
    BrushMatInst = UMaterialInstanceDynamic::Create(BrushSourceMaterial, this);

    if (BrushMatInst)
    {
        UE_LOG(LogTemp, Log, TEXT("PaintManager: Brush Material Instance created successfully."));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PaintManager: Failed to create Brush Material Instance!"));
    }
}

void APaintManager::PaintTarget(UInkSystemComponent *TargetComp, FVector2D HitUV, float TeamID, float BrushSize)
{
    // 1. 验证输入
    if (!TargetComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("PaintManager::PaintTarget: TargetComp is null!"));
        return;
    }

    if (!BrushMatInst)
    {
        UE_LOG(LogTemp, Warning, TEXT("PaintManager::PaintTarget: BrushMatInst is null! Did you assign BrushSourceMaterial?"));
        return;
    }

    UTextureRenderTarget2D *RenderTarget = TargetComp->GetRenderTarget();
    if (!RenderTarget)
    {
        UE_LOG(LogTemp, Warning, TEXT("PaintManager::PaintTarget: Target RenderTarget is null!"));
        return;
    }

    // 2. 使用默认画刷大小（如果未指定）
    if (BrushSize <= 0.0f)
    {
        BrushSize = DefaultBrushSize;
    }

    // 3. 计算像素坐标（UV 0-1 转换为像素坐标）
    const int32 Resolution = TargetComp->GetResolution();
    const float PixelX = HitUV.X * Resolution;
    const float PixelY = HitUV.Y * Resolution;

    // 画刷居中绘制
    const float DrawX = PixelX - (BrushSize * 0.5f);
    const float DrawY = PixelY - (BrushSize * 0.5f);

    // 4. 设置队伍 ID 参数
    BrushMatInst->SetScalarParameterValue(FName(TEXT("TeamID")), TeamID);

    // 5. 使用 Canvas 绘制到 Render Target
    UCanvas *Canvas = nullptr;
    FVector2D CanvasSize;
    FDrawToRenderTargetContext Context;

    UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(
        this,
        RenderTarget,
        Canvas,
        CanvasSize,
        Context);

    if (Canvas)
    {
        // 绘制画刷材质到 Canvas
        Canvas->K2_DrawMaterial(
            BrushMatInst,
            FVector2D(DrawX, DrawY),         // 位置
            FVector2D(BrushSize, BrushSize), // 大小
            FVector2D(0.0f, 0.0f),           // UV 起点
            FVector2D(1.0f, 1.0f),           // UV 终点（整个材质）
            0.0f,                            // 旋转
            FVector2D(0.5f, 0.5f)            // 旋转中心
        );
    }

    UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, Context);
}

void APaintManager::PaintTargetByTeam(UInkSystemComponent *TargetComp, FVector2D HitUV, E_Team Team, float BrushSize)
{
    const float TeamIDFloat = TeamToFloat(Team);

    // 无效队伍不涂色
    if (TeamIDFloat < 0.0f)
    {
        return;
    }

    PaintTarget(TargetComp, HitUV, TeamIDFloat, BrushSize);
}

float APaintManager::TeamToFloat(E_Team Team)
{
    switch (Team)
    {
    case E_Team::Team1:
        return 0.0f; // 红色
    case E_Team::Team2:
        return 1.0f; // 绿色
    case E_Team::None:
    default:
        return -1.0f; // 无效
    }
}
