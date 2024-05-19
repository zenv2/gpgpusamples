#include <iostream>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <vector>

#define HR_RETURN(x) { HRESULT __hr = x; if(FAILED(__hr)){ std::cout<<"Error:: "<< __FILE__ << ":" << __LINE__ << " " << #x << " Failed (" << std::hex << __hr << ")"<<std::endl; return __hr;} }

class ComputeSample
{
    public:

    ComputeSample();
    ~ComputeSample();

    HRESULT Init();
    HRESULT CreateShader(LPCWSTR hlsl, LPCSTR entryPoint);
    HRESULT PrepareResources(UINT structStride, UINT count, void *pInitData0, void *pInitData1);
    void Run(UINT x, UINT y, UINT z);
    HRESULT GetResult(void **ppBuf, UINT bufLen);

    private:

    ID3D11Device *pDevice;
    ID3D11DeviceContext *pContext;
    ID3DBlob *pShaderBlob;
    ID3D11ComputeShader *pShader;
    ID3D11Buffer *pInBuf0;
    ID3D11Buffer *pInBuf1;
    ID3D11Buffer *pOutBuf;
    ID3D11ShaderResourceView *pInSrv0;
    ID3D11ShaderResourceView *pInSrv1;
    ID3D11UnorderedAccessView *pOutUav;

    void SafeRelease(IUnknown *p);
};