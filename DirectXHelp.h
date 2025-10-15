#pragma once
#include <d2d1.h>
#include <math.h>

namespace DX
{
	// Converts a length in device-independent pixels (DIPs) to a length in physical pixels.
	inline float ConvertDipsToPixels(float dips, float dpi)
	{
		static const float dipsPerInch = 96.0f;
		return floorf(dips * dpi / dipsPerInch + 0.5f); // Round to nearest integer.
	}

	// Converts a length in physical pixels to a length in device-independent pixels (DIPs).
	inline float ConvertPixelsToDips(float pixels, float dpi)
	{
		static const float dipsPerInch = 96.0f;
		return pixels * dipsPerInch / dpi; // Do not round.
	}

	extern void GetD2D1Factory(ID2D1Factory **ppFactory);
	
	class BezierSpline {
	public:
		static void GetCurveControlPoints(const std::vector<D2D1_POINT_2F>& knots,
			std::vector<D2D1_POINT_2F>& firstControlPoints,
			std::vector<D2D1_POINT_2F>& secondControlPoints);
	private:
		static std::vector<FLOAT> GetFirstControlPoints(const std::vector<D2D1_POINT_2F>& knots, bool forX);
	};
}



