#pragma once

namespace Library
{
	// Constant buffer used to send MVP matrices to the vertex shader.
	struct ModelViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	};

	// Used to send per-vertex data to the vertex shader.
	struct VertexPositionColor
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 color;
	};

	struct VertexPositionColorAlpha
	{
		DirectX::XMFLOAT4 pos;
		DirectX::XMFLOAT4 color;

		VertexPositionColorAlpha() : pos(0.0f, 0.0f, 0.0f, 0.0f), color(0.0f, 0.0f, 0.0f, 1.0f) { }

		VertexPositionColorAlpha(const DirectX::XMFLOAT4& Position, const DirectX::XMFLOAT4& Color)
			: pos(Position), color(Color) { }
	};
}