#pragma once
#include <chrono>

class PixelMaterial;
class PixelRenderer;

class Timer
{
public:
    static constexpr int SIZE_X = 100;
    static constexpr int SIZE_Y = 100;

	static constexpr FLOAT4 MATERIAL_DATA = { 0.5f, 0.5f, 1.0f, 0.3f }; // タイマー描画用マテリアルデータ

    Timer(float limitSeconds); // 制限時間（秒）を指定して初期化
    ~Timer(void);

    void Start();              // タイマー開始
    void Reset();              // タイマーを止める（再スタートで再使用可）
    
    // 残り時間（0以下にならない）
    float GetRemainingTime() const;

    // 制限時間を過ぎたかどうか
    bool IsTimeUp() const;          

    // タイマーが動作中かどうか
	bool IsRunning() const { return isRunning_; }

     // 針を描画する（中心座標と画像を指定）
    void SetNeedleImage(int handle);
    void DrawTimer(int centerX, int centerY, float startDeg = 0.0f, float endDeg = 270.0f) const;

    
private:
    
    // 描画用シェーダ
    std::unique_ptr<PixelMaterial> Material_;
    std::unique_ptr<PixelRenderer> Renderer_;

    float limitSeconds_;
    bool isRunning_;
    std::chrono::steady_clock::time_point startTime_;

    int freamHandle_ = -1;
    int needleHandle_ = -1;
    int endHandle_ = -1;
};
