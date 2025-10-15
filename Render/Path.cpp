#include "StdAfx.h"
#include <d2d1.h>
#include "Render.h"
#include "DirectXHelp.h"
namespace ui {

using namespace Gdiplus;
Path_Gdiplus::Path_Gdiplus()
{
	path_.reset(new GraphicsPath());
}

Path_Gdiplus::Path_Gdiplus(const Path_Gdiplus& r)
	: IPath(r)
{
	path_.reset(r.path_->Clone());
}

ui::IPath* Path_Gdiplus::Clone()
{
	return new Path_Gdiplus(*this);
}

void Path_Gdiplus::Reset()
{
	path_->Reset();
}

void Path_Gdiplus::SetFillMode(FillMode mode)
{
	path_->SetFillMode((Gdiplus::FillMode)mode);
}

IPath::FillMode Path_Gdiplus::GetFillMode()
{
	return (IPath::FillMode)path_->GetFillMode();
}

void Path_Gdiplus::StartFigure()
{
	path_->StartFigure();
}

void Path_Gdiplus::CloseFigure()
{
	path_->CloseFigure();
}

void Path_Gdiplus::AddLine(int x1, int y1, int x2, int y2)
{
	path_->AddLine(x1, y1, x2, y2);
}

void Path_Gdiplus::AddLines(const CPoint* points, int count)
{
	std::vector<Point> p;
	for (int i = 0; i < count; i++)
	{
		p.emplace_back(points[i].x, points[i].y);
	}
	path_->AddLines(&p[0], p.size());
}

void Path_Gdiplus::AddBezier(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
	path_->AddBezier(x1, y1, x2, y2, x3, y3, x4, y4);
}

void Path_Gdiplus::AddCurve(const CPoint* points, int count)
{
	std::vector<Point> p;
	for (int i = 0; i < count; i++)
	{
		p.emplace_back(points[i].x, points[i].y);
	}
	path_->AddCurve(&p[0], p.size());
}

void Path_Gdiplus::AddRect(int left, int top, int right, int bottom)
{
	path_->AddRectangle(Rect(left, top, right -left, bottom - top));
}

void Path_Gdiplus::AddRect(const UiRect& rect)
{
	path_->AddRectangle(Rect(rect.left, rect.top, rect.GetWidth(), rect.GetHeight()));
}

void Path_Gdiplus::AddEllipse(int left, int top, int right, int bottom)
{
	path_->AddEllipse(Rect(left, top, right - left, bottom - top));
}

void Path_Gdiplus::AddEllipse(const UiRect& rect)
{
	path_->AddEllipse(Rect(rect.left, rect.top, rect.GetWidth(), rect.GetHeight()));
}

void Path_Gdiplus::AddArc(int x, int y, int width, int height, float startAngle, float sweepAngle)
{
	path_->AddArc(x, y, width, height, startAngle, sweepAngle);
}

void Path_Gdiplus::AddPie(int x, int y, int width, int height, float startAngle, float sweepAngle)
{
	path_->AddPie(x, y, width, height, startAngle, sweepAngle);
}

void Path_Gdiplus::AddPolygon(const CPoint* points, int count)
{
	std::vector<Point> p;
	for (int i = 0; i < count; i++)
	{
		p.emplace_back(points[i].x, points[i].y);
	}
	path_->AddPolygon(&p[0], p.size());
}

ui::UiRect Path_Gdiplus::GetBound(const IPen* pen)
{
	auto p = (Pen_GdiPlus*)(pen);
	Rect rc;
	path_->GetBounds(&rc, NULL, p ? p->GetPen() : NULL);
	return UiRect(rc.X, rc.Y, rc.GetRight(), rc.GetBottom());
}

bool Path_Gdiplus::IsContainsPoint(int x, int y)
{
	return path_->IsVisible(x, y) == TRUE;
}

bool Path_Gdiplus::IsStrokeContainsPoint(int x, int y, const IPen* pen)
{
	return path_->IsOutlineVisible(x, y, ((Pen_GdiPlus*)pen)->GetPen()) == TRUE;
}

void Path_Gdiplus::Transform(const IMatrix* matrix)
{
	path_->Transform(((Matrix_Gdiplus*)matrix)->GetMatrix());
}


/************************************************************************/
/* Direct2d   Path                                                                     */
/************************************************************************/



Path_Direct2D::Path_Direct2D()
{
	Reset();
}

Path_Direct2D::~Path_Direct2D()
{
	Reset();
}

Path_Direct2D::Path_Direct2D(const Path_Direct2D& r)
{
	paths_ = r.paths_;
	for (auto path : paths_)
	{
		path->AddRef();
	}
	main_path_ = r.main_path_;
	if (main_path_ != nullptr)
		main_path_->AddRef();
	main_sink_ = r.main_sink_;
	if (main_sink_ != nullptr)
		main_sink_->AddRef();
	end_path_ = r.end_path_;
	if (end_path_ != nullptr)
		end_path_->AddRef();
	mode = r.mode;
}

IPath* Path_Direct2D::Clone()
{
	return new Path_Direct2D(*this);
}

void Path_Direct2D::Reset()
{
	for (auto path : paths_) {
		path->Release();
	}
	paths_.clear();
	if (main_sink_ != nullptr)
	{
		main_sink_->Release();
		main_sink_ = nullptr;
	}
	if (main_path_ != nullptr)
	{
		main_path_->Release();
		main_path_ = nullptr;
	}
	if (end_path_ != nullptr)
	{
		end_path_->Release();
		end_path_ = nullptr;
	}
}

void Path_Direct2D::SetFillMode(FillMode mode)
{
	mode = mode;
}

IPath::FillMode Path_Direct2D::GetFillMode()
{
	return mode;
}

void Path_Direct2D::StartFigure() 
{
	ASSERT(end_path_ == nullptr);
}

void Path_Direct2D::CloseFigure()
{
	if (main_path_ != nullptr)
	{
		if (main_sink_ != nullptr)
		{
			main_sink_->EndFigure(D2D1_FIGURE_END_CLOSED);
			HRESULT hr = main_sink_->Close();
			ASSERT(SUCCEEDED(hr));
			main_sink_->Release();
			main_sink_ = nullptr;
		}
		main_path_->Release();
		main_path_ = nullptr;
	}
}

ID2D1PathGeometry* Path_Direct2D::CreatePath()
{
	ID2D1PathGeometry *pPath = nullptr;
	ID2D1Factory *pFactory = nullptr;
	DX::GetD2D1Factory(&pFactory);
	if (nullptr != pFactory)
	{
		HRESULT hr = pFactory->CreatePathGeometry(&pPath);
		if (SUCCEEDED(hr))
		{
			pPath->AddRef();
			paths_.push_back(pPath);
		}
	}
	return pPath;
}

ID2D1Geometry* Path_Direct2D::GetPath()
{
	if (end_path_ != nullptr)
		return end_path_;

	ID2D1GeometryGroup *pPathGroup = nullptr;
	ID2D1Factory *pFactory = nullptr;
	DX::GetD2D1Factory(&pFactory);
	CloseFigure();
	if (nullptr != pFactory && !paths_.empty())
	{
		HRESULT hr = pFactory->CreateGeometryGroup((D2D1_FILL_MODE)GetFillMode(), &paths_[0], paths_.size(), &pPathGroup);
		if (SUCCEEDED(hr))
		{
			end_path_ = pPathGroup;
		}
	}
	return pPathGroup;
}

void Path_Direct2D::AddLine(int x1, int y1, int x2, int y2)
{
	ID2D1PathGeometry *path = nullptr;
	ID2D1GeometrySink *sink = nullptr;
	if (main_path_ == nullptr)
	{
		path = CreatePath();
		main_path_ = path;
		HRESULT hr = path->Open(&sink);
		ASSERT(SUCCEEDED(hr));
		if (SUCCEEDED(hr))
		{
			main_sink_ = sink;
		}
	}
	else {
		path = main_path_;
		sink = main_sink_;
	}

	if (nullptr != sink)
	{
		sink->BeginFigure(D2D1::Point2F(x1, y1),
			D2D1_FIGURE_BEGIN_HOLLOW);
		sink->AddLine(D2D1::Point2F(x2, y2));
		sink->EndFigure(D2D1_FIGURE_END_OPEN);
	}
}

void Path_Direct2D::AddLines(const CPoint* points, int count) 
{
	ASSERT(count > 1);
	ID2D1PathGeometry *path = nullptr;
	ID2D1GeometrySink *sink = nullptr;
	if (main_path_ == nullptr)
	{
		path = CreatePath();
		main_path_ = path;
		HRESULT hr = path->Open(&sink);
		ASSERT(SUCCEEDED(hr));
		if (SUCCEEDED(hr))
		{
			main_sink_ = sink;
		}
	}
	else {
		path = main_path_;
		sink = main_sink_;
	}
	if (nullptr != sink)
	{
		sink->BeginFigure(D2D1::Point2F(points[0].x, points[0].y), D2D1_FIGURE_BEGIN_HOLLOW);
		std::vector<D2D1_POINT_2F> d2dPts;
		for (int i = 1; i < count; i++)
		{
			d2dPts.push_back(std::move(D2D1::Point2F(points[i].x, points[i].y)));
		}
		sink->AddLines(&d2dPts[0], d2dPts.size());
		sink->EndFigure(D2D1_FIGURE_END_OPEN);
	}
}

void Path_Direct2D::AddBezier(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
	ID2D1PathGeometry *path = nullptr;
	ID2D1GeometrySink *sink = nullptr;
	if (main_path_ == nullptr)
	{
		path = CreatePath();
		main_path_ = path;
		HRESULT hr = path->Open(&sink);
		ASSERT(SUCCEEDED(hr));
		if (SUCCEEDED(hr))
		{
			main_sink_ = sink;
		}
	}
	else {
		path = main_path_;
		sink = main_sink_;
	}

	if (nullptr != sink)
	{
		sink->BeginFigure(
			D2D1::Point2F(x1, y1),
			D2D1_FIGURE_BEGIN_HOLLOW
		);
		sink->AddBezier(D2D1::BezierSegment(
			D2D1::Point2F(x2, y2),
			D2D1::Point2F(x3, y3),
			D2D1::Point2F(x4, y4)));

		sink->EndFigure(D2D1_FIGURE_END_OPEN);
	}
}

void Path_Direct2D::AddCurve(const CPoint* points, int count) {
	ASSERT(count > 1);
	ID2D1PathGeometry *path = nullptr;
	ID2D1GeometrySink *sink = nullptr;
	if (main_path_ == nullptr)
	{
		path = CreatePath();
		main_path_ = path;
		HRESULT hr = path->Open(&sink);
		ASSERT(SUCCEEDED(hr));
		if (SUCCEEDED(hr))
		{
			main_sink_ = sink;
		}
	}
	else {
		path = main_path_;
		sink = main_sink_;
	}
	if (nullptr != sink)
	{
		sink->BeginFigure(D2D1::Point2F(points[0].x, points[0].y), D2D1_FIGURE_BEGIN_HOLLOW);
		std::vector<D2D1_POINT_2F> d2dPts;
		for (int i = 0; i < count; i++)
		{
			d2dPts.push_back(std::move(D2D1::Point2F(points[i].x, points[i].y)));
		}
		std::vector<D2D1_POINT_2F> firstCtrls, secondCtrls;
		DX::BezierSpline::GetCurveControlPoints(d2dPts, firstCtrls, secondCtrls);
		std::vector<D2D1_BEZIER_SEGMENT> bsLst;
		for (int i = 0; i < firstCtrls.size(); ++i)
		{
			D2D1_BEZIER_SEGMENT bs;
			bs.point1 = firstCtrls[i];
			bs.point2 = secondCtrls[i];
			bs.point3 = d2dPts[i + 1];
			bsLst.push_back(bs);
		}
		
		sink->AddBeziers(&bsLst[0], bsLst.size());
		sink->EndFigure(D2D1_FIGURE_END_OPEN);
	}
}

void Path_Direct2D::AddRect(int left, int top, int right, int bottom) {
	ID2D1RectangleGeometry* rectPath;
	ID2D1Factory *pFactory = nullptr;
	DX::GetD2D1Factory(&pFactory);
	HRESULT hr = pFactory->CreateRectangleGeometry(D2D1::RectF(left, top, right, bottom),&rectPath);
	ASSERT(SUCCEEDED(hr));
	if (SUCCEEDED(hr))
	{
		paths_.push_back(rectPath);
	}
}

void Path_Direct2D::AddRect(const UiRect& rect) {
	AddRect(rect.left, rect.top, rect.right, rect.bottom);
}

void Path_Direct2D::AddEllipse(int left, int top, int right, int bottom) {
	ID2D1EllipseGeometry* ellipsePath;
	ID2D1Factory *pFactory = nullptr;
	DX::GetD2D1Factory(&pFactory);
	FLOAT radiuX = (right - left) / 2.0f;
	FLOAT radiuY = (bottom - top) / 2.0f;
	HRESULT hr = pFactory->CreateEllipseGeometry(D2D1::Ellipse(D2D1::Point2F(left + radiuX, top + radiuY), radiuX, radiuY), &ellipsePath);
	ASSERT(SUCCEEDED(hr));
	if (SUCCEEDED(hr))
	{
		paths_.push_back(ellipsePath);
	}
}

void Path_Direct2D::AddEllipse(const UiRect& rect) {
	AddEllipse(rect.left, rect.top, rect.right, rect.bottom);
}

void Path_Direct2D::AddArc(int x, int y, int width, int height, float startAngle, float sweepAngle) {
	ID2D1PathGeometry *path = CreatePath();
	if (nullptr != path)
	{
		ID2D1GeometrySink *path_sink_ = nullptr;
		HRESULT hr = path->Open(&path_sink_);
		ASSERT(SUCCEEDED(hr));
		if (SUCCEEDED(hr))
		{
			float centerX = x + width / 2;
			float centerY = y + height / 2;

			float radiusX = width / 2;
			float radiusY = height / 2;

			FLOAT cosStart, sinStart;
			FLOAT cosEnd, sinEnd;
			D2D1SinCos(startAngle, &sinStart, &cosStart);
			D2D1SinCos(startAngle + sweepAngle, &sinEnd, &cosEnd);

			//计算椭圆弧长的起始和终点
			float startX = centerX + radiusX * cosStart;
			float startY = centerY + radiusY * sinStart;
			float endX = centerX + radiusX * cosEnd;
			float endY = centerY + radiusY * sinEnd;

			path_sink_->BeginFigure(
				D2D1::Point2F(startX, startY),
				D2D1_FIGURE_BEGIN_HOLLOW
			);

			FLOAT radiuX = width / 2.0f;
			FLOAT radiuY = height / 2.0f;
			path_sink_->AddArc(D2D1::ArcSegment(
				D2D1::Point2F(endX, endY),
				D2D1::SizeF(radiuX, radiuY),
				0.0f,
				sweepAngle < 0 ? D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE : D2D1_SWEEP_DIRECTION_CLOCKWISE,
				fabs(sweepAngle) > 180.0f ? D2D1_ARC_SIZE_LARGE : D2D1_ARC_SIZE_SMALL));
			path_sink_->EndFigure(D2D1_FIGURE_END_OPEN);
			hr = path_sink_->Close();
			ASSERT(SUCCEEDED(hr));
			path_sink_->Release();
			paths_.push_back(path);
		}
	}
}

void Path_Direct2D::AddPie(int x, int y, int width, int height, float startAngle, float sweepAngle) {
	ID2D1PathGeometry *path = CreatePath();
	if (nullptr != path)
	{
		ID2D1GeometrySink *path_sink_ = nullptr;
		HRESULT hr = path->Open(&path_sink_);
		ASSERT(SUCCEEDED(hr));
		if (SUCCEEDED(hr))
		{
			float centerX = x + width / 2;
			float centerY = y + height / 2;

			float radiusX = width / 2;
			float radiusY = height / 2;

			FLOAT cosStart, sinStart;
			FLOAT cosEnd, sinEnd;
			D2D1SinCos(startAngle, &sinStart, &cosStart);
			D2D1SinCos(startAngle + sweepAngle, &sinEnd, &cosEnd);

			//计算椭圆弧长的起始和终点
			float startX = centerX + radiusX * cosStart;
			float startY = centerY + radiusY * sinStart;
			float endX = centerX + radiusX * cosEnd;
			float endY = centerY + radiusY * sinEnd;

			path_sink_->BeginFigure(
				D2D1::Point2F(startX, startY),
				D2D1_FIGURE_BEGIN_HOLLOW
			);

			FLOAT radiuX = width / 2.0f;
			FLOAT radiuY = height / 2.0f;
			path_sink_->AddArc(D2D1::ArcSegment(
				D2D1::Point2F(endX, endY),
				D2D1::SizeF(radiuX, radiuY),
				0.0f,
				sweepAngle < 0 ? D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE : D2D1_SWEEP_DIRECTION_CLOCKWISE,
				fabs(sweepAngle) > 180.0f ? D2D1_ARC_SIZE_LARGE : D2D1_ARC_SIZE_SMALL));
			path_sink_->EndFigure(D2D1_FIGURE_END_OPEN);

			path_sink_->BeginFigure(D2D1::Point2F(startX, startY), D2D1_FIGURE_BEGIN_FILLED);
			D2D1_POINT_2F pts[] = { D2D1::Point2F(centerX, centerY), D2D1::Point2F(endX, endY) };
			path_sink_->AddLines(pts, ARRAYSIZE(pts));
			path_sink_->EndFigure(D2D1_FIGURE_END_CLOSED);
			hr = path_sink_->Close();
			ASSERT(SUCCEEDED(hr));
			path_sink_->Release();
			paths_.push_back(path);
		}
	}
}

void Path_Direct2D::AddPolygon(const CPoint* points, int count) {
	ASSERT(count > 1);
	ID2D1PathGeometry *path = CreatePath();
	if (nullptr != path)
	{
		ID2D1GeometrySink *path_sink_ = nullptr;
		HRESULT hr = path->Open(&path_sink_);
		ASSERT(SUCCEEDED(hr));
		if (SUCCEEDED(hr))
		{
			path_sink_->BeginFigure(D2D1::Point2F(points[0].x, points[0].y), D2D1_FIGURE_BEGIN_HOLLOW);
			std::vector<D2D1_POINT_2F> d2dPts;
			for (int i = 1; i < count; i++)
			{
				d2dPts.push_back(std::move(D2D1::Point2F(points[i].x, points[i].y)));
			}
			path_sink_->AddLines(&d2dPts[0], d2dPts.size());
			path_sink_->EndFigure(D2D1_FIGURE_END_CLOSED);
			hr = path_sink_->Close();
			ASSERT(SUCCEEDED(hr));
			path_sink_->Release();
			paths_.push_back(path);
		}
	}
}

UiRect Path_Direct2D::GetBound(const IPen* pen) {
	auto path = GetPath();
	D2D1_RECT_F d2dRt;
	HRESULT hr = path->GetBounds(D2D1::Matrix3x2F::Identity(),&d2dRt);
	ASSERT(SUCCEEDED(hr));
	UiRect rt;
	rt.left = d2dRt.left;
	rt.top = d2dRt.top;
	rt.right = d2dRt.right;
	rt.bottom = d2dRt.bottom;
	return rt;
}

bool Path_Direct2D::IsContainsPoint(int x, int y) {
	auto path = GetPath();
	BOOL contains = FALSE;
	HRESULT hr = path->StrokeContainsPoint(D2D1::Point2F(x, y),1.0f,nullptr,D2D1::Matrix3x2F::Identity(),&contains);
	ASSERT(SUCCEEDED(hr));
	return contains;
}

bool Path_Direct2D::IsStrokeContainsPoint(int x, int y, const IPen* pen) {
	auto path = GetPath();
	BOOL contains = FALSE;
	ID2D1StrokeStyle *pStrokeStyle = RenderContext_D3D::CreateStrokeFromPen(pen);
	HRESULT hr = path->StrokeContainsPoint(D2D1::Point2F(x, y), pen->GetWidth(), pStrokeStyle, D2D1::Matrix3x2F::Identity(), &contains);
	ASSERT(SUCCEEDED(hr));
	pStrokeStyle->Release();
	pStrokeStyle = nullptr;
	return contains;
}

void Path_Direct2D::Transform(const IMatrix* matrix) {
	auto path = GetPath();
	Matrix_Direct2D *d2dMat = reinterpret_cast<Matrix_Direct2D*>(const_cast<IMatrix*>(matrix));
	ASSERT(d2dMat != nullptr);
	ID2D1Factory *pFactory = nullptr;
	DX::GetD2D1Factory(&pFactory);
	ID2D1TransformedGeometry *pTransformedPath = nullptr;
	HRESULT hr = pFactory->CreateTransformedGeometry(path, (d2dMat)->GetMatrix(), &pTransformedPath);
	ASSERT(SUCCEEDED(hr));
	if (SUCCEEDED(hr))
	{
		end_path_->Release();
		end_path_ = pTransformedPath;
	}
}

} // namespace ui
