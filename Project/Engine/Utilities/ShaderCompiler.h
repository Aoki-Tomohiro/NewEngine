#pragma once
#include "Log.h"
#include <dxcapi.h>
#include <string>
#include <wrl.h>
#pragma comment(lib,"dxcompiler.lib")

class ShaderCompiler
{
public:
    static const std::wstring& kBaseDirectory;

    static void Initialize();

    static Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile);

private:
    static Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;

    static Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;

    static Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;
};

