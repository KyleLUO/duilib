#ifndef UI_CORE_RENDER_PATH_H_
#define UI_CORE_RENDER_PATH_H_

#pragma once
#include <GdiPlus.h>
#include <d2d1.h>
#include <vector>

namespace ui 
{

class UILIB_API Path_Gdiplus : public IPath
{
public:
	Path_Gdiplus();
	Path_Gdiplus(const Path_Gdiplus& r);
	Path_Gdiplus& operator=(const Path_Gdiplus& r) = delete;

	virtual IPath* Clone() override;
	virtual void Reset() override;

	virtual void SetFillMode(FillMode mode) override;
	virtual FillMode GetFillMode() override;

	virtual void StartFigure() override;
	virtual void CloseFigure() override;

	virtual void AddLine(int x1, int y1, int x2, int y2) override;
	virtual void AddLines(const CPoint* points, int count) override;
	virtual void AddBezier(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) override;
	virtual void AddCurve(const CPoint* points, int count) override;
	virtual void AddRect(int left, int top, int right, int bottom) override;
	virtual void AddRect(const UiRect& rect) override;
	virtual void AddEllipse(int left, int top, int right, int bottom) override;
	virtual void AddEllipse(const UiRect& rect) override;
	virtual void AddArc(int x, int y, int width, int height, float startAngle, float sweepAngle) override;
	virtual void AddPie(int x, int y, int width, int height, float startAngle, float sweepAngle) override;
	virtual void AddPolygon(const CPoint* points, int count) override;

	virtual UiRect GetBound(const IPen* pen) override;
	virtual bool IsContainsPoint(int x, int y) override;
	virtual bool IsStrokeContainsPoint(int x, int y, const IPen* pen) override;

	virtual void Transform(const IMatrix* matrix) override;

	Gdiplus::GraphicsPath* GetPath() { return path_.get(); };
protected:
	std::unique_ptr<Gdiplus::GraphicsPath> path_;
};

class UILIB_API Path_Direct2D : public IPath
{
public:
	Path_Direct2D();
	~Path_Direct2D();
	Path_Direct2D(const Path_Direct2D& r);
	Path_Direct2D& operator=(const Path_Direct2D& r) = delete;

	virtual IPath* Clone() override;
	virtual void Reset() override;

	virtual void SetFillMode(FillMode mode) override;
	virtual FillMode GetFillMode() override;

	virtual void StartFigure() override;
	virtual void CloseFigure() override;

	virtual void AddLine(int x1, int y1, int x2, int y2) override;
	virtual void AddLines(const CPoint* points, int count) override;
	virtual void AddBezier(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) override;
	virtual void AddCurve(const CPoint* points, int count) override;
	virtual void AddRect(int left, int top, int right, int bottom) override;
	virtual void AddRect(const UiRect& rect) override;
	virtual void AddEllipse(int left, int top, int right, int bottom) override;
	virtual void AddEllipse(const UiRect& rect) override;
	virtual void AddArc(int x, int y, int width, int height, float startAngle, float sweepAngle) override;
	virtual void AddPie(int x, int y, int width, int height, float startAngle, float sweepAngle) override;
	virtual void AddPolygon(const CPoint* points, int count) override;

	virtual UiRect GetBound(const IPen* pen) override;
	virtual bool IsContainsPoint(int x, int y) override;
	virtual bool IsStrokeContainsPoint(int x, int y, const IPen* pen) override;

	virtual void Transform(const IMatrix* matrix) override;

	ID2D1Geometry* GetPath();

private:
	ID2D1PathGeometry* CreatePath();

protected:
	std::vector<ID2D1Geometry*> paths_;
	ID2D1PathGeometry *main_path_;
	ID2D1GeometrySink *main_sink_;
	ID2D1Geometry *end_path_; //path group
	FillMode mode;
};

} // namespace ui

#endif // UI_CORE_RENDER_PATH_H_
