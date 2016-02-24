#include "pch.h"
#include "RasterizerStates.h"

namespace Library
{
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> RasterizerStates::BackCulling = nullptr;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> RasterizerStates::FrontCulling = nullptr;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> RasterizerStates::DisabledCulling = nullptr;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> RasterizerStates::Wireframe = nullptr;

	void RasterizerStates::Initialize(ID3D11Device* direct3DDevice)
	{
		assert(direct3DDevice != nullptr);

		D3D11_RASTERIZER_DESC rasterizerStateDesc;
		ZeroMemory(&rasterizerStateDesc, sizeof(rasterizerStateDesc));
		rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerStateDesc.CullMode = D3D11_CULL_BACK;
		rasterizerStateDesc.DepthClipEnable = true;
		
		Utility::ThrowIfFailed(direct3DDevice->CreateRasterizerState(&rasterizerStateDesc, BackCulling.ReleaseAndGetAddressOf()), "ID3D11Device::CreateRasterizerState() failed.");
		
		ZeroMemory(&rasterizerStateDesc, sizeof(rasterizerStateDesc));
		rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerStateDesc.CullMode = D3D11_CULL_BACK;
		rasterizerStateDesc.FrontCounterClockwise = true;
		rasterizerStateDesc.DepthClipEnable = true;

		Utility::ThrowIfFailed(direct3DDevice->CreateRasterizerState(&rasterizerStateDesc, FrontCulling.ReleaseAndGetAddressOf()), "ID3D11Device::CreateRasterizerState() failed.");

		ZeroMemory(&rasterizerStateDesc, sizeof(rasterizerStateDesc));
		rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerStateDesc.CullMode = D3D11_CULL_NONE;
		rasterizerStateDesc.DepthClipEnable = true;

		Utility::ThrowIfFailed(direct3DDevice->CreateRasterizerState(&rasterizerStateDesc, DisabledCulling.ReleaseAndGetAddressOf()), "ID3D11Device::CreateRasterizerState() failed.");

		ZeroMemory(&rasterizerStateDesc, sizeof(rasterizerStateDesc));
		rasterizerStateDesc.FillMode = D3D11_FILL_WIREFRAME;
		rasterizerStateDesc.CullMode = D3D11_CULL_NONE;
		rasterizerStateDesc.DepthClipEnable = true;

		Utility::ThrowIfFailed(direct3DDevice->CreateRasterizerState(&rasterizerStateDesc, Wireframe.ReleaseAndGetAddressOf()), "ID3D11Device::CreateRasterizerState() failed.");
	}
}