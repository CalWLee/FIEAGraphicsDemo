#pragma once

namespace Library
{
	class Utility
	{

	public:
		static std::string CurrentDirectory();
		static std::wstring ExecutableDirectory();
		static std::string LoadBinaryFile(const std::string& filename);
		static float ConvertDipsToPixels(float dips, float dpi);
		static void Utility::ThrowIfFailed(HRESULT hr, const char* const& message = "");

		static void ToWideString(const std::string& source, std::wstring& dest);
	};

}