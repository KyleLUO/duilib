#include "StdAfx.h"

namespace ui {

using namespace Gdiplus;

void Matrix_Gdiplus::Translate(int offsetX, int offsetY)
{
	matrix_.Translate(offsetX, offsetY);
}

void Matrix_Gdiplus::Scale(int scaleX, int scaleY)
{
	matrix_.Scale(scaleX, scaleY);
}

void Matrix_Gdiplus::Rotate(float angle)
{
	matrix_.Rotate(angle);
}

void Matrix_Gdiplus::RotateAt(float angle, const CPoint& center)
{
	matrix_.RotateAt(angle, PointF(center.x, center.y));
}


void Matrix_Direct2D::Translate(int offsetX, int offsetY)
{
	matrix_.TransformPoint(D2D1::Point2F(offsetX, offsetY));
}

void Matrix_Direct2D::Scale(int scaleX, int scaleY)
{
	matrix_.Scale(scaleX, scaleY);
}

void Matrix_Direct2D::Rotate(float angle)
{
	matrix_.Rotation(angle);
}

void Matrix_Direct2D::RotateAt(float angle, const CPoint& center)
{
	matrix_.Rotation(angle, D2D1::Point2F(center.x, center.y));
}

} // namespace ui
