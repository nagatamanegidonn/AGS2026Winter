#pragma once
#include <chrono>

class PixelMaterial;
class PixelRenderer;

class Timer
{
public:
    static constexpr int SIZE_X = 100;
    static constexpr int SIZE_Y = 100;
    // タイマー描画用マテリアルデータ
	static constexpr FLOAT4 MATERIAL_DATA = { 0.5f, 0.5f, 1.0f, 0.3f };

    // コンストラクタ
    Timer(float limitSeconds); // 制限時間（秒）を指定して初期化
    
    // デストラクタ
    ~Timer(void);

    // タイマー管理
    void Start();  // タイマー開始
    void Reset();  // タイマーを止める（再スタートで再使用可）
    
    // 残り時間（0以下にならない）
    float GetRemainingTime() const;

    // 制限時間を過ぎたかどうか
    bool IsTimeUp() const;

    // タイマーが動作中かどうか
	bool IsRunning() const { return isRunning_; }

    // 秒針の画像設定
    void SetNeedleImage(int handle);

    // 針を描画する（中心座標と画像を指定）
    void DrawTimer(int centerX, int centerY, float startDeg = 0.0f, float endDeg = 270.0f) const;

private:
    
    // 描画用シェーダ
    std::unique_ptr<PixelMaterial> Material_;
    std::unique_ptr<PixelRenderer> Renderer_;

    // 制限時間
    float limitSeconds_;

    // タイマーが動いているか
    bool isRunning_;

    // スタート時間
    std::chrono::steady_clock::time_point startTime_;

    int freamHandle_ = -1;
    int needleHandle_ = -1;
    int endHandle_ = -1;
};
