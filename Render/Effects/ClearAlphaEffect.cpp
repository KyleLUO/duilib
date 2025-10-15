#include "stdafx.h"
#include "ClearAlphaEffect.h"
#include "complieHLSL.h"
#include <d2d1effecthelpers.h>

#define XML(X) TEXT(#X)

static std::wstring g_shaderPath;

Microsoft::WRL::ComPtr<ID3DBlob> ClearAlphaEffect::m_shaderBlob;
ClearAlphaEffect::ClearAlphaEffect():m_refCount(1)
{
	if (m_shaderBlob.Get() == nullptr)
	{
		ID3DBlob *psBlob = nullptr;
		HRESULT hr = CompileShader((g_shaderPath + L"\\ClearAlphaShader.hlsl").c_str(), "PSMain", "ps_4_0_level_9_1", &psBlob);
		if (FAILED(hr))
		{
			printf("Failed compiling pixel shader %08X\n", hr);
			throw std::exception("Compiling pixel shader failure");
		}
		m_shaderBlob = psBlob;
	}
	ZeroMemory(&m_shadowMata, sizeof(m_shadowMata));
}

IFACEMETHODIMP ClearAlphaEffect::Initialize(ID2D1EffectContext * pContextInternal, ID2D1TransformGraph * pTransformGraph)
{
	m_effectContext = pContextInternal;
	//ID2D1OffsetTransform;
	//ID2D1BlendTransform;

	HRESULT hr = m_effectContext->LoadPixelShader(GUID_ClearAlphaShader, (BYTE*)m_shaderBlob->GetBufferPointer(), m_shaderBlob->GetBufferSize());
	assert(SUCCEEDED(hr));
	if (SUCCEEDED(hr))
	{
		// The graph consists of a single transform. In fact, this class is the transform,
		// reducing the complexity of implementing an effect when all we need to
		// do is use a single pixel shader.
		hr = pTransformGraph->SetSingleTransformNode(this);
		assert(SUCCEEDED(hr));
	}
	return hr;
}

IFACEMETHODIMP ClearAlphaEffect::PrepareForRender(D2D1_CHANGE_TYPE changeType)
{
	m_effectContext->GetDpi(&m_dpi, &m_dpi);
	// �Զ���ת�� -> ����������ɫ��������
	// m_shadowMata.dpi = m_dpi;
	return m_drawInfo->SetPixelShaderConstantBuffer(reinterpret_cast<BYTE*>(&m_shadowMata), sizeof(m_shadowMata));
}

IFACEMETHODIMP ClearAlphaEffect::SetGraph(ID2D1TransformGraph * pGraph)
{
	return S_OK;
}

HRESULT ClearAlphaEffect::Register(_In_ ID2D1Factory1* pFactory, const std::wstring& resourceRoot)
{
	g_shaderPath = resourceRoot + L"\\shaders";
	// The inspectable metadata of an effect is defined in XML. This can be passed in from an external source
	// as well, however for simplicity we just inline the XML.
	PCWSTR pszXml =
		XML(<?xml version = '1.0' ?>
			<Effect>
			<Property name = 'DisplayName' type = 'string' value = 'Shadow' />
			<Property name = 'Author' type = 'string' value = 'KYE' />
			<Property name = 'Category' type = 'string' value = 'Stylize' />
			<Property name = 'Description' type = 'string' value = 'Adds a ripple effect that can be animated' />
			<Inputs>
			<Input name = 'Source' /> 
			</Inputs>
			<Property name = 'RcDirty' type = 'vector4'>
			<Property name = 'DisplayName' type = 'string' value = 'RcDirty' />
			<Property name = 'Default' type = 'vector4' value = '(0.0, 0.0, 0.0, 0.0)' />
			</Property>
			<Property name = 'Padding' type = 'vector4'>
			<Property name = 'DisplayName' type = 'string' value = 'Padding' />
			<Property name = 'Default' type = 'vector4' value = '(0.0, 0.0, 0.0, 0.0)' />
			</Property>
			<Property name = 'Size' type = 'vector2'>
			<Property name = 'DisplayName' type = 'string' value = 'Size' />
			<Property name = 'Default' type = 'vector2' value = '(0.0, 0.0)' />
			</Property>
			<Property name = 'Alpha' type = 'float'>
			<Property name = 'DisplayName' type = 'string' value = 'Alpha' />
			<Property name = 'Min' type = 'float' value = '0.0' />
			<Property name = 'Max' type = 'float' value = '1.0' />
			<Property name = 'Default' type = 'float' value = '1.0' />
			</Property>
			</Effect>);

	// This defines the bindings from specific properties to the callback functions
	// on the class that ID2D1Effect::SetValue() & GetValue() will call.
	const D2D1_PROPERTY_BINDING bindings[] =
	{
		D2D1_VALUE_TYPE_BINDING(L"RcDirty", &SetRcDirty, &GetRcDirty),
		D2D1_VALUE_TYPE_BINDING(L"Padding", &SetPadding, &GetPadding),
		D2D1_VALUE_TYPE_BINDING(L"Size", &SetSize, &GetSize),
		D2D1_VALUE_TYPE_BINDING(L"Alpha", &SetAlpha, &GetAlpha),
	};

	// This registers the effect with the factory, which will make the effect
	// instantiatable.
	return pFactory->RegisterEffectFromString(
		CLSID_ClearAlphaEffect,
		pszXml,
		bindings,
		ARRAYSIZE(bindings),
		&ClearAlphaEffect::CreateClearAlphaEffect);
}

HRESULT ClearAlphaEffect::CreateClearAlphaEffect(IUnknown ** ppEffectImpl)
{
	*ppEffectImpl = static_cast<ID2D1EffectImpl*>(new(std::nothrow) ClearAlphaEffect());
	if (*ppEffectImpl == nullptr)
	{
		return E_OUTOFMEMORY;
	}
	return S_OK;
}

IFACEMETHODIMP_(ULONG) ClearAlphaEffect::AddRef()
{
	m_refCount++;
	return m_refCount;
}

IFACEMETHODIMP_(ULONG) ClearAlphaEffect::Release()
{
	m_refCount--;

	if (m_refCount == 0)
	{
		delete this;
		return 0;
	}
	else
	{
		return m_refCount;
	}
}

IFACEMETHODIMP ClearAlphaEffect::QueryInterface(REFIID riid, void ** ppOutput)
{
	*ppOutput = nullptr;
	HRESULT hr = S_OK;

	if (riid == __uuidof(ID2D1EffectImpl))
	{
		ID2D1EffectImpl* ptr = this;
		*ppOutput = ptr;
	}
	else if (riid == __uuidof(ID2D1DrawTransform))
	{
		ID2D1DrawTransform* ptr = this;
		*ppOutput = ptr;
	}
	else if (riid == __uuidof(ID2D1Transform))
	{
		ID2D1Transform* ptr = this;
		*ppOutput = ptr;
	}
	else if (riid == __uuidof(ID2D1TransformNode))
	{
		ID2D1TransformNode* ptr = this;
		*ppOutput = ptr;
	}
	else if (riid == __uuidof(IUnknown))
	{
		*ppOutput = this;
	}
	else
	{
		hr = E_NOINTERFACE;
	}

	if (*ppOutput != nullptr)
	{
		AddRef();
	}

	return hr;
}

IFACEMETHODIMP ClearAlphaEffect::SetDrawInfo(ID2D1DrawInfo * drawInfo)
{
	// ����GPU render�����ô��Զ���Ч����ɫ��
	m_drawInfo = drawInfo;

	return drawInfo->SetPixelShader(GUID_ClearAlphaShader);
}

IFACEMETHODIMP ClearAlphaEffect::MapOutputRectToInputRects(const D2D1_RECT_L * pOutputRect, D2D1_RECT_L * pInputRects, UINT32 inputRectCount) const
{
	/*
	Direct2D �� MapInputRectsToOutputRect ֮�� ���� MapOutputRectToInputRects ������ ת�����������Ҫ���ж�ȡ��ͼ�񲿷֣��Ա���ȷ����������������
	*/
	pInputRects[0] = *pOutputRect;
	return S_OK;
}

IFACEMETHODIMP ClearAlphaEffect::MapInputRectsToOutputRect(
	CONST D2D1_RECT_L * pInputRects, 
	CONST D2D1_RECT_L * pInputOpaqueSubRects, 
	UINT32 inputRectCount, 
	D2D1_RECT_L * pOutputRect, 
	D2D1_RECT_L * pOutputOpaqueSubRect)
{
	/*
	ÿ�γ���ת��ʱ��Direct2D ������� MapInputRectsToOutputRect ������ Direct2D ��һ�����δ��ݸ�ת�����þ��α�ʾÿ������ı߽硣 Ȼ��ת������������ͼ��ı߽硣 �˽ӿ������з����ľ��δ�С (ID2D1Transform) �����أ������� DIP�����塣
	�˷��������������ɫ�����߼���ÿ������Ĳ�͸�����������㲻͸����������� ͼ��Ĳ�͸��������Ϊ�������ε� alpha ͨ��Ϊ��1���� ��������ת��������Ƿ�͸������Ӧ�������͸����������Ϊ (0��0��0��0��0) ��Ϊ��ȫֵ�� Direct2D ʹ�ô���Ϣ�ԡ���֤��͸��������ִ�г����Ż��� �����ֵ��׼ȷ������ܻᵼ�²���ȷ�ĳ��֡�
	*/
	if (inputRectCount != 1)
	{
		return E_INVALIDARG;
	}

	*pOutputRect = pInputRects[0];
	m_inputRect = pInputRects[0];

	// Indicate that entire output might contain transparency.
	ZeroMemory(pOutputOpaqueSubRect, sizeof(*pOutputOpaqueSubRect));
	return S_OK;
}

IFACEMETHODIMP ClearAlphaEffect::MapInvalidRect(UINT32 inputIndex, D2D1_RECT_L invalidInputRect, D2D1_RECT_L * pInvalidOutputRect) const
{
	HRESULT hr = S_OK;
	// Indicate that the entire output may be invalid.
	*pInvalidOutputRect = m_inputRect;
	return hr;
}

IFACEMETHODIMP_(UINT32) ClearAlphaEffect::GetInputCount() const
{
	return 1;
}

HRESULT ClearAlphaEffect::SetRcDirty(D2D1_RECT_F dirty)
{
	m_shadowMata.dirty = dirty;
	return S_OK;
}

D2D1_RECT_F ClearAlphaEffect::GetRcDirty() const
{
	return m_shadowMata.dirty;
}


HRESULT ClearAlphaEffect::SetPadding(D2D1_RECT_F padding)
{
	m_shadowMata.padding = padding;
	return S_OK;
}

D2D1_RECT_F ClearAlphaEffect::GetPadding() const
{
	return m_shadowMata.padding;
}

HRESULT ClearAlphaEffect::SetSize(D2D1_SIZE_F size)
{
	m_shadowMata.size = size;
	return S_OK;
}
D2D1_SIZE_F ClearAlphaEffect::GetSize() const
{
	return m_shadowMata.size;
}

HRESULT ClearAlphaEffect::SetAlpha(float alpha)
{
	m_shadowMata.alpha = alpha;
	return S_OK;
}
float ClearAlphaEffect::GetAlpha() const
{
	return m_shadowMata.alpha;
}
