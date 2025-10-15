#ifndef UI_UTILS_UTILS_H_
#define UI_UTILS_UTILS_H_

#pragma once

#include <oaidl.h> // for VARIANT

namespace ui
{

/////////////////////////////////////////////////////////////////////////////////////
//

class STRINGorID
{
public:
	STRINGorID(LPCTSTR lpString) : m_lpstr(lpString)
	{
	
	}

	STRINGorID(UINT nID) : m_lpstr(MAKEINTRESOURCE(nID))
	{
	
	}

	LPCTSTR m_lpstr;
};

/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CPoint : public tagPOINT
{
public:
	CPoint()
	{
		x = y = 0;
	}

	CPoint(const POINT& src)
	{
		x = src.x;
		y = src.y;
	}

	CPoint(int _x, int _y)
	{
		x = _x;
		y = _y;
	}

	CPoint(LPARAM lParam)
	{
		x = GET_X_LPARAM(lParam);
		y = GET_Y_LPARAM(lParam);
	}

	void Offset(int offsetX, int offsetY)
	{
		x += offsetX;
		y += offsetY;
	}

	void Offset(CPoint offsetPoint)
	{
		x += offsetPoint.x;
		y += offsetPoint.y;
	}
};

/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CSize : public tagSIZE
{
public:
	CSize()
	{
		cx = cy = 0;
	}

	CSize(const CSize& src)
	{
		cx = src.cx;
		cy = src.cy;
	}

	CSize(int _cx, int _cy)
	{
		cx = _cx;
		cy = _cy;
	}

	void Offset(int offsetCX, int offsetCY)
	{
		cx += offsetCX;
		cy += offsetCY;
	}

	void Offset(CSize offsetPoint)
	{
		cx += offsetPoint.cx;
		cy += offsetPoint.cy;
	}
};

/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API UiRect : public tagRECT
{
public:
	UiRect()
	{
		left = top = right = bottom = 0;
	}

	UiRect(const RECT& src)
	{
		left = src.left;
		top = src.top;
		right = src.right;
		bottom = src.bottom;
	}

	UiRect(int iLeft, int iTop, int iRight, int iBottom)
	{
		left = iLeft;
		top = iTop;
		right = iRight;
		bottom = iBottom;
	}

	int GetWidth() const
	{
		return right - left;
	}

	int GetHeight() const
	{
		return bottom - top;
	}

	void Clear()
	{
		left = top = right = bottom = 0;
	}

	bool IsRectEmpty() const
	{
		return ::IsRectEmpty(this) == TRUE; 
	}

	void ResetOffset()
	{
		::OffsetRect(this, -left, -top);
	}

	void Normalize()
	{
		if( left > right ) { int iTemp = left; left = right; right = iTemp; }
		if( top > bottom ) { int iTemp = top; top = bottom; bottom = iTemp; }
	}

	void Offset(int cx, int cy)
	{
		::OffsetRect(this, cx, cy);
	}

	void Offset(const CPoint& offset)
	{
		::OffsetRect(this, offset.x, offset.y);
	}

	void Inflate(int cx, int cy)
	{
		::InflateRect(this, cx, cy);
	}

	void Inflate(const UiRect& rect)
	{
		this->left -= rect.left;
		this->top -= rect.top;
		this->right += rect.right;
		this->bottom += rect.bottom;
	}

	void Deflate(int cx, int cy)
	{
		::InflateRect(this, -cx, -cy);
	}

	void Deflate(const UiRect& rect)
	{
		this->left += rect.left;
		this->top += rect.top;
		this->right -= rect.right;
		this->bottom -= rect.bottom;
	}

	void Union(const UiRect& rc)
	{
		::UnionRect(this, this, &rc);
	}

	void Intersect(const UiRect& rc)
	{
		::IntersectRect(this, this, &rc);
	}

	void Subtract(const UiRect& rc)
	{
		::SubtractRect(this, this, &rc);
	}

	bool IsPointIn(const CPoint& point) const
	{
		return ::PtInRect(this, point) == TRUE;
	}

	bool Equal(const UiRect& rect) const
	{
		return this->left == rect.left && this->top == rect.top 
			&& this->right == rect.right && this->bottom == rect.bottom;
	}
};

/////////////////////////////////////////////////////////////////////////////////////
//

class CVariant : public VARIANT
{
public:
	CVariant() 
	{ 
		VariantInit(this); 
	}

	CVariant(int i)
	{
		VariantInit(this);
		this->vt = VT_I4;
		this->intVal = i;
	}

	CVariant(float f)
	{
		VariantInit(this);
		this->vt = VT_R4;
		this->fltVal = f;
	}

	CVariant(LPOLESTR s)
	{
		VariantInit(this);
		this->vt = VT_BSTR;
		this->bstrVal = s;
	}

	CVariant(IDispatch *disp)
	{
		VariantInit(this);
		this->vt = VT_DISPATCH;
		this->pdispVal = disp;
	}

	~CVariant() 
	{ 
		VariantClear(this); 
	}
};

class PathUtil
{
public:
	static std::wstring GetCurrentModuleDir()
	{
		std::wstring strModulePath;
		strModulePath.resize(MAX_PATH);
		::GetModuleFileNameW(::GetModuleHandle(NULL), &strModulePath[0], (DWORD)strModulePath.length());
		return strModulePath.substr(0, strModulePath.find_last_of(L"\\") + 1);
	}
};



class UILIB_API CStdPtrArray
{
public:
	CStdPtrArray(int iPreallocSize = 0) : m_ppVoid(NULL), m_nCount(0), m_nAllocated(iPreallocSize)
	{
		//ASSERT(iPreallocSize >= 0);
		if (iPreallocSize > 0) m_ppVoid = static_cast<LPVOID*>(malloc(iPreallocSize * sizeof(LPVOID)));
	}

	CStdPtrArray(const CStdPtrArray& src) : m_ppVoid(NULL), m_nCount(0), m_nAllocated(0)
	{
		for (int i = 0; i < src.GetSize(); i++)
			Add(src.GetAt(i));
	}

	~CStdPtrArray()
	{
		if (m_ppVoid != NULL) free(m_ppVoid);
	}

	void Empty()
	{
		if (m_ppVoid != NULL) free(m_ppVoid);
		m_ppVoid = NULL;
		m_nCount = m_nAllocated = 0;
	}

	void Resize(int iSize)
	{
		Empty();
		m_ppVoid = static_cast<LPVOID*>(malloc(iSize * sizeof(LPVOID)));
		::ZeroMemory(m_ppVoid, iSize * sizeof(LPVOID));
		m_nAllocated = iSize;
		m_nCount = iSize;
	}

	bool IsEmpty() const
	{
		return m_nCount == 0;
	}

	bool Add(LPVOID pData)
	{
		if (++m_nCount >= m_nAllocated) {
			int nAllocated = m_nAllocated * 2;
			if (nAllocated == 0) nAllocated = 11;
			LPVOID* ppVoid = static_cast<LPVOID*>(realloc(m_ppVoid, nAllocated * sizeof(LPVOID)));
			if (ppVoid != NULL) {
				m_nAllocated = nAllocated;
				m_ppVoid = ppVoid;
			}
			else {
				--m_nCount;
				return false;
			}
		}
		m_ppVoid[m_nCount - 1] = pData;
		return true;
	}

	bool InsertAt(int iIndex, LPVOID pData)
	{
		if (iIndex == m_nCount) return Add(pData);
		if (iIndex < 0 || iIndex > m_nCount) return false;
		if (++m_nCount >= m_nAllocated) {
			int nAllocated = m_nAllocated * 2;
			if (nAllocated == 0) nAllocated = 11;
			LPVOID* ppVoid = static_cast<LPVOID*>(realloc(m_ppVoid, nAllocated * sizeof(LPVOID)));
			if (ppVoid != NULL) {
				m_nAllocated = nAllocated;
				m_ppVoid = ppVoid;
			}
			else {
				--m_nCount;
				return false;
			}
		}
		memmove(&m_ppVoid[iIndex + 1], &m_ppVoid[iIndex], (m_nCount - iIndex - 1) * sizeof(LPVOID));
		m_ppVoid[iIndex] = pData;
		return true;
	}

	bool SetAt(int iIndex, LPVOID pData)
	{
		if (iIndex < 0 || iIndex >= m_nCount) return false;
		m_ppVoid[iIndex] = pData;
		return true;
	}

	bool Remove(int iIndex)
	{
		if (iIndex < 0 || iIndex >= m_nCount) return false;
		if (iIndex < --m_nCount) ::CopyMemory(m_ppVoid + iIndex, m_ppVoid + iIndex + 1, (m_nCount - iIndex) * sizeof(LPVOID));
		return true;
	}

	int Find(LPVOID pData) const
	{
		for (int i = 0; i < m_nCount; i++) if (m_ppVoid[i] == pData) return i;
		return -1;
	}

	int GetSize() const
	{
		return m_nCount;
	}

	LPVOID* GetData()
	{
		return m_ppVoid;
	}

	LPVOID GetAt(int iIndex) const
	{
		if (iIndex < 0 || iIndex >= m_nCount) return NULL;
		return m_ppVoid[iIndex];
	}

	LPVOID operator[] (int iIndex) const
	{
		//ASSERT(iIndex >= 0 && iIndex < m_nCount);
		return m_ppVoid[iIndex];
	}

protected:
	LPVOID* m_ppVoid;
	int m_nCount;
	int m_nAllocated;
};



}// namespace ui

#endif // UI_UTILS_UTILS_H_