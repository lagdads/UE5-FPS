// Copyright Epic Games, Inc. All Rights Reserved.

#include "InkSystemComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetRenderingLibrary.h"

UInkSystemComponent::UInkSystemComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UInkSystemComponent::BeginPlay()
{
    Super::BeginPlay();

    // 获取 Owner 的静态网格组件
    if (AActor *Owner = GetOwner())
    {
        CachedMeshComponent = Owner->FindComponentByClass<UStaticMeshComponent>();
    }

    if (!CachedMeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("InkSystemComponent: Owner '%s' has no StaticMeshComponent!"),
               *GetNameSafe(GetOwner()));
        return;
    }

    // 初始化 Render Target 和动态材质
    InitializeRenderTarget();
    InitializeDynamicMaterial();
}

void UInkSystemComponent::InitializeRenderTarget()
{
    // 使用 Kismet 库创建 Render Target（自动处理资源管理）
    MyRenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(
        this,
        Resolution,
        Resolution,
        ETextureRenderTargetFormat::RTF_RG8 // 只需要 R 和 G 通道，节省内存
    );

    if (MyRenderTarget)
    {
        // 设置清除颜色为黑色（无涂色）
        MyRenderTarget->ClearColor = FLinearColor::Black;

        // 立即清除一次确保初始状态正确
        UKismetRenderingLibrary::ClearRenderTarget2D(this, MyRenderTarget, FLinearColor::Black);

        UE_LOG(LogTemp, Log, TEXT("InkSystemComponent: Created RenderTarget %dx%d for '%s'"),
               Resolution, Resolution, *GetNameSafe(GetOwner()));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("InkSystemComponent: Failed to create RenderTarget for '%s'!"),
               *GetNameSafe(GetOwner()));
    }
}

void UInkSystemComponent::InitializeDynamicMaterial()
{
    if (!CachedMeshComponent || !MyRenderTarget)
    {
        return;
    }

    // 获取当前材质
    UMaterialInterface *BaseMaterial = CachedMeshComponent->GetMaterial(MaterialSlotIndex);
    if (!BaseMaterial)
    {
        UE_LOG(LogTemp, Warning, TEXT("InkSystemComponent: No material at slot %d on '%s'!"),
               MaterialSlotIndex, *GetNameSafe(GetOwner()));
        return;
    }

    // 创建动态材质实例
    MyDynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
    if (!MyDynamicMaterial)
    {
        UE_LOG(LogTemp, Error, TEXT("InkSystemComponent: Failed to create Dynamic Material for '%s'!"),
               *GetNameSafe(GetOwner()));
        return;
    }

    // 设置 Render Target 纹理参数（对应 M_Inkable_Surface 中的 "InkRT" 参数）
    MyDynamicMaterial->SetTextureParameterValue(FName(TEXT("InkRT")), MyRenderTarget);

    // 将动态材质应用回网格
    CachedMeshComponent->SetMaterial(MaterialSlotIndex, MyDynamicMaterial);

    UE_LOG(LogTemp, Log, TEXT("InkSystemComponent: Bound InkRT to Dynamic Material on '%s'"),
           *GetNameSafe(GetOwner()));
}
