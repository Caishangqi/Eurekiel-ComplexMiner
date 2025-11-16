#pragma once
//================================================================================================
// ConstantBufferWorld.hpp
//================================================================================================
// World常量缓冲区结构体定义
// 用于Shader的cbuffer WorldConstants(register b4)
// 总大小: 80字节(5*16) - 符合DX11 16字节对齐要求
//================================================================================================

#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/Vec2.hpp"

//-----------------------------------------------------------------------------------------------
// ConstantBufferWorld: 世界渲染常量缓冲区
// 大小: 80字节(5*16字节块)
//-----------------------------------------------------------------------------------------------
struct WorldConstant
{
    // 相机位置 (xyz=世界坐标, w=1.0)
    Vec4 CameraPosition; // 16 bytes | Offset: 0

    // 室内光颜色 (rgb=归一化颜色[0,1], a=1.0)
    // 默认值: (1.0f, 230.0f/255.0f, 204.0f/255.0f, 1.0f) - 暖黄色光照
    Vec4 IndoorLightColor; // 16 bytes | Offset: 16

    // 室外光颜色 (rgb=归一化颜色[0,1], a=1.0)
    // 默认值: (1.0f, 1.0f, 1.0f, 1.0f) - 纯白色光照
    Vec4 OutdoorLightColor; // 16 bytes | Offset: 32

    // 天空颜色 (rgb=归一化颜色[0,1], a=1.0)
    // 默认值: (0.0f, 0.0f, 0.0f, 1.0f) - 黑色(无天空光)
    Vec4 SkyColor; // 16 bytes | Offset: 48

    // 迷雾近距离 (单位: 格子)
    // 默认值: 160.0f - 开始应用迷雾的距离
    float FogNearDistance; // 4 bytes | Offset: 64

    // 迷雾远距离 (单位: 格子)
    // 默认值: 80.0f - 完全迷雾的距离 (FogNearDistance * 0.5)
    float FogFarDistance; // 4 bytes | Offset: 68

    // 16字节对齐填充
    Vec2 Padding; // 8 bytes | Offset: 72

    // 总大小: 80 bytes (5 * 16)
};

// 编译时大小验证
static_assert(sizeof(WorldConstant) == 80, "ConstantBufferWorld must be 80 bytes for DX11 constant buffer alignment");
