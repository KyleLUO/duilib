#ifndef UI_CORE_RENDER_MATRIX_H_
#define UI_CORE_RENDER_MATRIX_H_

#pragma once
#include <GdiPlus.h>
#include <d2d1.h>

namespace ui 
{

class UILIB_API Matrix_Gdiplus : public IMatrix
{
public:
	virtual void Translate(int offsetX, int offsetY) override;
	virtual void Scale(int scaleX, int scaleY) override;
	virtual void Rotate(float angle) override;
	virtual void RotateAt(float angle, const CPoint& center) override;

	Gdiplus::Matrix* GetMatrix() { return &matrix_; };
private:
	Gdiplus::Matrix matrix_;
};

class UILIB_API Matrix_Direct2D : public IMatrix
{
public:
	virtual void Translate(int offsetX, int offsetY) override;
	virtual void Scale(int scaleX, int scaleY) override;
	virtual void Rotate(float angle) override;
	virtual void RotateAt(float angle, const CPoint& center) override;

	D2D1::Matrix3x2F* GetMatrix() { return &matrix_; };
private:
	D2D1::Matrix3x2F matrix_;
};

} // namespace ui

#endif // UI_CORE_RENDER_MATRIX_H_
