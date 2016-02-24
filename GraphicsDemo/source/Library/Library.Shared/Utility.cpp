
#include "pch.h"
#include "Utility.h"

#if(WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
#include <Shlwapi.h>
#endif

using namespace DirectX;

namespace Library
{

	std::string Utility::CurrentDirectory()
	{		
		WCHAR buffer[MAX_PATH];
		
		std::wstring currentDirectoryW(buffer);

		return std::string(currentDirectoryW.begin(), currentDirectoryW.end());
	}

	std::wstring Utility::ExecutableDirectory()
	{
		WCHAR buffer[MAX_PATH];

		return std::wstring(buffer);
	}

	std::string Utility::LoadBinaryFile(const std::string& filename)
	{
#if(WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
			WCHAR buffer[MAX_PATH];
			GetModuleFileName(nullptr, buffer, MAX_PATH);
			PathRemoveFileSpec(buffer);


			SetCurrentDirectory(std::wstring(buffer).c_str()); //c_str retusn the wide strign pointer.
#endif
		std::ifstream file(filename, std::ios::binary);
		if (file.bad())
		{
			throw std::exception("Could not open file.");
		}

		std::string data = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

		file.close();

		return data;
	}
	
	// Converts a length in device-independent pixels (DIPs) to a length in physical pixels.
	float Utility::ConvertDipsToPixels(float dips, float dpi)
	{
		static const float dipsPerInch = 96.0f;
		return floorf(dips * dpi / dipsPerInch + 0.5f); // Round to nearest integer.
	}

	void Utility::ThrowIfFailed(HRESULT hr, const char* const& message)
	{
		if (FAILED(hr))
		{
			// Set a breakpoint on this line to catch DirectX API errors
			throw std::exception(message);
		}
	}
	
	void Utility::ToWideString(const std::string& source, std::wstring& dest)
	{
		dest.assign(source.begin(), source.end());
	}

}