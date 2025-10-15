#include "stdafx.h"
#include "DirectXHelp.h"
#include <iostream>
#include <vector>
#include <stdexcept>

namespace DX {

	void GetD2D1Factory(ID2D1Factory **ppFactory)
	{
		static ID2D1Factory* s_d2dFactory = nullptr;
		if (s_d2dFactory == nullptr) {
			ID2D1Factory *pFatory;
			HRESULT hr = D2D1CreateFactory(
				D2D1_FACTORY_TYPE_SINGLE_THREADED,
				__uuidof(ID2D1Factory),
				(void**)&pFatory
			);
			if (FAILED(hr)) {
				throw std::exception("D2D create Factory failure");
			}
			s_d2dFactory = pFatory;
		}
		*ppFactory = s_d2dFactory;
	}

	void BezierSpline::GetCurveControlPoints(const std::vector<D2D1_POINT_2F>& knots,
		std::vector<D2D1_POINT_2F>& firstControlPoints,
		std::vector<D2D1_POINT_2F>& secondControlPoints) {
		if (knots.empty()) {
			throw std::invalid_argument("knots parameter must not be empty.");
		}

		int n = static_cast<int>(knots.size()) - 1;
		if (n < 1) {
			throw std::invalid_argument("At least two knot points required.");
		}

		firstControlPoints.resize(n);
		secondControlPoints.resize(n);

		if (n == 1) {
			// Special case: Bezier curve should be a straight line.
			firstControlPoints[0].x = (2 * knots[0].x + knots[1].x) / 3;
			firstControlPoints[0].y = (2 * knots[0].y + knots[1].y) / 3;

			secondControlPoints[0].x = 2 * firstControlPoints[0].x - knots[0].x;
			secondControlPoints[0].y = 2 * firstControlPoints[0].y - knots[0].y;
			return;
		}

		std::vector<FLOAT> x = GetFirstControlPoints(knots, true);
		std::vector<FLOAT> y = GetFirstControlPoints(knots, false);

		for (int i = 0; i < n; ++i) {
			firstControlPoints[i].x = x[i];
			firstControlPoints[i].y = y[i];

			if (i < n - 1) {
				secondControlPoints[i].x = 2 * knots[i + 1].x - x[i + 1];
				secondControlPoints[i].y = 2 * knots[i + 1].y - y[i + 1];
			}
			else {
				secondControlPoints[i].x = (knots[n].x + x[n - 1]) / 2;
				secondControlPoints[i].y = (knots[n].y + y[n - 1]) / 2;
			}
		}
	}

	std::vector<FLOAT> BezierSpline::GetFirstControlPoints(const std::vector<D2D1_POINT_2F>& knots, bool forX) {
		int n = static_cast<int>(knots.size());
		std::vector<FLOAT> x(n);
		std::vector<FLOAT> tmp(n);

		double b = 2.0;
		x[0] = forX ? (knots[0].x / b) : (knots[0].y / b);

		for (int i = 1; i < n; i++) {
			tmp[i] = 1.0 / b;
			b = (i < n - 1 ? 4.0 : 3.5) - tmp[i];
			x[i] = (forX ? (knots[i].x) : (knots[i].y)) - x[i - 1];
			x[i] /= b;
		}

		for (int i = 1; i < n; i++) {
			x[n - i - 1] -= tmp[n - i] * x[n - i];
		}

		return x;
	}

	

	/*
	int main() {
		std::vector<Point> knots = { { 0, 0 },{ 100, 50 },{ 200, 100 } };
		std::vector<Point> firstControlPoints, secondControlPoints;

		try {
			BezierSpline::GetCurveControlPoints(knots, firstControlPoints, secondControlPoints);

			for (size_t i = 0; i < firstControlPoints.size(); ++i) {
				std::cout << "First Control Point " << i << ": (" << firstControlPoints[i].X << ", " << firstControlPoints[i].Y << ")" << std::endl;
				std::cout << "Second Control Point " << i << ": (" << secondControlPoints[i].X << ", " << secondControlPoints[i].Y << ")" << std::endl;
			}
		}
		catch (const std::exception& ex) {
			std::cerr << "Exception: " << ex.what() << std::endl;
		}

		return 0;
	}*/

}
