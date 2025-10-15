#pragma once
#include <d2d1_1.h>
#include <d2d1effectauthor.h>
#include <d2d1effecthelpers.h>
#include <wrl/client.h>
#include <initguid.h>

DEFINE_GUID(GUID_ClearAlphaShader, 0x6716FB29, 0x06A0, 0x460F, 0xAA, 0x9C, 0x8B, 0x8F, 0xF0, 0x33, 0x5E, 0xA6);
DEFINE_GUID(CLSID_ClearAlphaEffect, 0xB7B36C92, 0x3498, 0x4A94, 0x9E, 0x95, 0x9F, 0x24, 0x6F, 0x92, 0x45, 0xBF);

typedef struct
{
	D2D1_RECT_F dirty;
	D2D1_RECT_F	padding;
	D2D1_SIZE_F size;
	float alpha;
} SHADOW_META;

enum CLEAR_ALPHA_PROP
{
	CLEAR_ALPHA_DIRTY_RC = 0,
	CLEAR_ALPHA_PROP_PADDING = 1,
	CLEAR_ALPHA_PROP_SIZE = 2,
	CLEAR_ALPHA_PROP_ALPHA = 3,
};

class ClearAlphaEffect : public ID2D1EffectImpl, public ID2D1DrawTransform
{
public:
	HRESULT SetRcDirty(D2D1_RECT_F dirty);
	D2D1_RECT_F GetRcDirty() const;

	HRESULT SetPadding(D2D1_RECT_F padding);
	D2D1_RECT_F GetPadding() const;

	HRESULT SetSize(D2D1_SIZE_F size);
	D2D1_SIZE_F GetSize() const;

	HRESULT SetAlpha(float alpha);
	float GetAlpha() const;
public:
	// Direct2D calls the Initialize method after the ID2D1DeviceContext::CreateEffect method has been called by the app
	// 此外，还可以使用它来创建效果的初始转换图
	IFACEMETHODIMP Initialize(
		_In_ ID2D1EffectContext* pContextInternal,
		_In_ ID2D1TransformGraph* pTransformGraph
	);

	//  Direct2D calls this method just before it renders an effect if at least one of these is true:
	//  效果之前已初始化，但尚未绘制。
	//  自上次绘制调用以来，效果属性已更改
	//  自上次绘制调用以来，调用 Direct2D 上下文(（如 DPI) ）的状态已更改。
	IFACEMETHODIMP PrepareForRender(D2D1_CHANGE_TYPE changeType);

	// Direct2D calls the SetGraph method when the number of inputs to the effect is changed
	// 大部分effect 有固定的输入， Composite effect有可变的输入，如果不支持可变的输入，则return E_NOTIMPL
	// 此方法允许这些效果更新其转换图，以响应不断变化的输入计数
	IFACEMETHODIMP SetGraph(_In_ ID2D1TransformGraph* pGraph);

	// 2.2 Declare effect registration methods.
	static HRESULT Register(_In_ ID2D1Factory1* pFactory, const std::wstring& resourceRoot);
	static HRESULT CALLBACK CreateClearAlphaEffect(_Outptr_ IUnknown** ppEffectImpl);

	// 2.3 Declare IUnknown implementation methods.
	IFACEMETHODIMP_(ULONG) AddRef();
	IFACEMETHODIMP_(ULONG) Release();
	IFACEMETHODIMP QueryInterface(_In_ REFIID riid, _Outptr_ void** ppOutput);



	// 2.4 dECLARE ID2D1DrawTransform implementation methods.

	// Provides the GPU render info interface to the transform implementation
	IFACEMETHODIMP SetDrawInfo(
		_In_ ID2D1DrawInfo *drawInfo
	);

	// Declare ID2D1Transform implementation methods.
	// 
	IFACEMETHODIMP MapOutputRectToInputRects(
		_In_ const D2D1_RECT_L* pOutputRect,
		_Out_writes_(inputRectCount) D2D1_RECT_L* pInputRects,
		UINT32 inputRectCount
	) const;

	IFACEMETHODIMP MapInputRectsToOutputRect(
		_In_reads_(inputRectCount) CONST D2D1_RECT_L* pInputRects,
		_In_reads_(inputRectCount) CONST D2D1_RECT_L* pInputOpaqueSubRects,
		UINT32 inputRectCount,
		_Out_ D2D1_RECT_L* pOutputRect,
		_Out_ D2D1_RECT_L* pOutputOpaqueSubRect
	);

	IFACEMETHODIMP MapInvalidRect(
		UINT32 inputIndex,
		D2D1_RECT_L invalidInputRect,
		_Out_ D2D1_RECT_L* pInvalidOutputRect
	) const;

	// Declare ID2D1TransformNode implementation methods.
	// 指定自定义效果的Input数量
	IFACEMETHODIMP_(UINT32) GetInputCount() const;

private:
	ClearAlphaEffect();
	LONG m_refCount; // Internal ref count used by AddRef() and Release() methods.

	Microsoft::WRL::ComPtr<ID2D1DrawInfo>      m_drawInfo;
	Microsoft::WRL::ComPtr<ID2D1EffectContext> m_effectContext;
	float                                      m_dpi;
	D2D1_RECT_L                                m_inputRect;
	SHADOW_META								   m_shadowMata;
	static Microsoft::WRL::ComPtr<ID3DBlob>	   m_shaderBlob;
};