#ifndef UI_ANIMATION_ANIMATIONMANAGER_H_
#define UI_ANIMATION_ANIMATIONMANAGER_H_

#pragma once

namespace ui 
{

class UILIB_API AnimationManager
{
public:
	AnimationManager();
	AnimationManager(const AnimationManager& r);
	AnimationManager& operator=(const AnimationManager& r) = delete;

	void Init(Control* control)	{ m_pControl = control;	}
	AnimationPlayer* GetAnimationPlayer(AnimationType animationType) const;

	AnimationPlayer* SetFadeHot(bool bFadeHot);
	AnimationPlayer* SetFadeAlpha(bool bFadeVisible);
	AnimationPlayer* SetFadeWidth(bool bFadeWidth);
	AnimationPlayer* SetFadeHeight(bool bFadeHeight);
	AnimationPlayer* SetFadeInOutX(bool bFade, bool bIsFromRight);
	AnimationPlayer* SetFadeInOutY(bool bFade, bool bIsFromBottom);

	void Appear();
	void Disappear();
	void MouseEnter();
	void MouseLeave();
	void SetSpeedRatio(float speed_ratio);

private:
	Control *m_pControl;
	std::map<AnimationType, std::unique_ptr<AnimationPlayer>> m_animationMap;

	float speed_ratio_ = 0;
};

} // namespace ui

#endif // UI_ANIMATION_ANIMATIONMANAGER_H_
