#include <DxLib.h>
#include "../Application.h"
#include "../Utility/Utility.h"

#include "../Renderer/PixelMaterial.h"
#include "../Renderer/PixelRenderer.h"

#include "Timer.h"

Timer::Timer(float limitSeconds)
    : limitSeconds_(limitSeconds), isRunning_(false)
{
    freamHandle_ = LoadGraph((Application::PATH_IMAGE + L"Timer.png").c_str());
    needleHandle_ = LoadGraph((Application::PATH_IMAGE + L"LongNeedle.png").c_str());
    endHandle_ = LoadGraph((Application::PATH_IMAGE + L"ShootNeedle.png").c_str());

    Material_ = std::make_unique<PixelMaterial>(L"Timer.cso", 1);
    Material_->AddConstBuf(MATERIAL_DATA);
    Material_->AddTextureBuf(freamHandle_);
    Material_->AddTextureBuf(needleHandle_);
    Material_->AddTextureBuf(endHandle_);
    Renderer_ = std::make_unique<PixelRenderer>(*Material_);
    Renderer_->SetSize(Vector2(100, 100));

}

Timer::~Timer(void)
{
    if (endHandle_ != -1) {
        DeleteGraph(endHandle_);
    }
    if (needleHandle_ != -1) {
        DeleteGraph(needleHandle_);
    }
    if (freamHandle_ != -1) {
        DeleteGraph(freamHandle_);
    }
}

void Timer::Start()
{
    startTime_ = std::chrono::steady_clock::now();
    isRunning_ = true;
}

void Timer::Reset()
{
    isRunning_ = false;
}

float Timer::GetRemainingTime() const
{
    if (!isRunning_) return limitSeconds_;

    auto now = std::chrono::steady_clock::now();
    float elapsed = std::chrono::duration<float>(now - startTime_).count();
    float remaining = limitSeconds_ - elapsed;
    return (remaining > 0.0f) ? remaining : 0.0f;
}

bool Timer::IsTimeUp() const
{
    return GetRemainingTime() <= 0.0f;
}

void Timer::SetNeedleImage(int handle)
{
    needleHandle_ = handle;
}

void Timer::DrawTimer(int centerX, int centerY, float startDeg, float endDeg) const
{
    if (needleHandle_ == -1 || !isRunning_) return;

    float elapsed = limitSeconds_ - GetRemainingTime();
    if (elapsed < 0.0f) elapsed = 0.0f;
    if (elapsed > limitSeconds_) elapsed = limitSeconds_;

    float t = elapsed / limitSeconds_;
    float angleDeg = startDeg + (endDeg - startDeg) * t;
    float rad = Utility::Deg2RadF(angleDeg);

    int w, h;
    GetGraphSize(needleHandle_, &w, &h);

    Material_->SetConstBuf(0, { 0.5f, 0.5f, rad,  Utility::Deg2RadF(endDeg) });

    Renderer_->Draw(centerX, centerY);
}
