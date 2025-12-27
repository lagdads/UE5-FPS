# Project2 - UE5 Splatoon 风格 FPS 代码库指南

## 项目概述
基于 **Unreal Engine 5.7** 开发的第三人称/第一人称射击游戏，具有 Splatoon 风格的涂色机制。核心游戏系统使用 C++ 实现，视觉效果和关卡脚本使用蓝图集成。

## 架构模式

### 墨水与涂色系统 (Ink System)
核心涂色逻辑由 `Ink/` 目录下的类管理：
- **UInkSystemComponent** (`Ink/InkSystemComponent.h`)：
  - 附加到所有可涂色 Actor（墙壁、地板）。
  - 管理专属的 `UTextureRenderTarget2D` 和 `UMaterialInstanceDynamic`。
  - 负责将渲染目标绑定到网格的材质槽（默认 Slot 0）。
- **APaintManager** (`Ink/PaintManager.h`)：
  - 全局绘画管理器，处理 UV 到像素坐标的转换。
  - 使用 `KismetRenderingLibrary` 将画刷材质绘制到目标的 RenderTarget。
  - 核心方法：`PaintTarget(TargetComp, HitUV, TeamID, BrushSize)`。
- **UV 映射要求**：
  - 涂色依赖 **UV Channel 1** (通常是光照贴图 UV)。
  - 表面网格必须具有非重叠且比例均匀的 UV，以避免涂色拉伸或失真。

### 基于接口的武器系统
武器通过 `IShooterWeaponHolder` 接口与角色通信（`Weapons/ShooterWeaponHolder.h`）：
- **不要**直接类耦合 - 当武器需要与持有者交互时，始终转换为接口。
- 角色（玩家/NPC）实现接口方法：武器附着、后坐力、HUD 更新、瞄准目标。
- 示例：使用 `WeaponOwner->GetWeaponTargetLocation()` 而不是 `Cast<ACharacter>(Owner)->GetLocation()`。

### 双网格架构
所有可见实体使用独立的第一人称和第三人称网格：
- **武器**：`AShooterWeapon` 中的 `FirstPersonMesh` + `ThirdPersonMesh` 骨骼网格。
- **角色**：`AProject2Character` 中的 `FirstPersonMesh`（仅手臂）+ 基础 `Mesh`（全身）。
- 附着点：`FirstPersonWeaponSocket` 和 `ThirdPersonWeaponSocket`（通常都是 "HandGrip_R"）。
- 武器激活/停用时切换 AnimInstance 类。

### 队伍系统
- `ShooterGameMode.h` 中的 `E_Team` 枚举：`None`、`Team1`、`Team2`。
- 投射物携带 `OwningTeam` 以标记表面的队伍颜色。
- 涂色逻辑将 `E_Team` 转换为浮点值（0.0/1.0）传递给材质。

## 关键 C++ 约定

### 变量命名与作用域
- 类成员：`bIsFiring`、`CurrentBullets`、`WeaponOwner`。
- 局部变量必须与类成员不同以避免 C4458 错误（例如：当 `bHit` 是成员时使用 `bTraceHit` 而非 `bHit`）。
- UPROPERTY 宏：`BlueprintReadWrite` 用于蓝图可访问，`meta = (AllowPrivateAccess = "true")` 用于编辑器中可见的私有属性。

### UCLASS/UFUNCTION 模式
- 所有游戏类在头文件中标记为 `abstract`（必须在蓝图中继承）。
- BlueprintImplementableEvent 用于 C++ 到蓝图的委托（例如：`BP_OnProjectileHit`、`BP_OnDeath`）。
- BlueprintCallable 用于蓝图到 C++ 的调用（getter/setter、动作）。

### 模块依赖
Build.cs 包含：
```cs
"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
"UMG", "Slate", "PhysicsCore", "RenderCore"
```

## 构建与编译

### UnrealBuildTool 命令
```powershell
# 从命令行编译（不要使用 dotnet build - 对 UE C++ 无效）
cmd /c "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" Project2Editor Win64 Development "-Project=<完整路径>\Project2.uproject"
```

### 自适应非统一构建
UnrealBuildTool 使用 git status 确定要非统一编译的修改文件：
- 修改的文件：`ShooterGameMode.cpp`、`ShooterCharacter.cpp`、`ShooterProjectile.cpp`、`ShooterWeapon.cpp`
- 仅重新编译修改的文件以加快迭代速度

## 关键工作流程

### 投射物与涂色流程
1. **发射**：`AShooterWeapon` 生成 `AShooterProjectile` 并分配 `OwningTeam`。
2. **命中**：`AShooterProjectile::OnHit` 触发，忽略 Instigator。
3. **UV 检测**：`ProcessPainting()` 执行双重射线检测（`bTraceComplex=true`, `bReturnFaceIndex=true`）。
   ```cpp
   // 使用通道 1（光照贴图 UV）进行涂色
   bool bFoundUV = UGameplayStatics::FindCollisionUV(UVHitResult, 1, UV);
   ```
4. **绘制**：调用 `PaintManager->PaintTarget()` 更新纹理。

### 添加新武器
1. 创建武器类型的蓝图子类（例如：`BP_Pistol` 继承自 `AShooterWeapon`）。
2. 在蓝图中设置 `ProjectileClass`、`MagazineSize`、`RefireRate`、`bFullAuto`。
3. 配置双网格：`FirstPersonMesh` 和 `ThirdPersonMesh` 及匹配的 AnimInstances。
4. 添加到拾取系统的 DataTable（`Weapons/ShooterPickup` 使用 FDataTableRowHandle）。

### 角色死亡与重生
`ShooterCharacter::Die()` 流程：
1. 停用武器，停止移动。
2. 通过 `GameMode->IncrementTeamScore(Team)` 增加对方队伍分数。
3. 添加"Dead"标签，禁用输入，广播 `BP_OnDeath()`。
4. 启动重生计时器 → `OnRespawn()` 重置 HP，移除标签，重新启用控制。

## 文件组织
- `Character/`：玩家/NPC 角色类（基类 `Project2Character` + `ShooterCharacter`）。
- `Weapons/`：武器系统（基类 `ShooterWeapon`、投射物、拾取物、持有者接口）。
- `Ink/`：涂色系统核心（`InkSystemComponent`, `PaintManager`）。
- `UI/`：UMG 小部件（通过 `ShooterUI` 的分数显示、弹药计数器）。
- `ShooterGameMode`：队伍计分、UI 生命周期。

