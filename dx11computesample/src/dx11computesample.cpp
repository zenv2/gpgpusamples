#include <dx11computesample.h>

ComputeSample::ComputeSample() :
    pDevice(nullptr),
    pContext(nullptr),
    pShader(nullptr),
    pInBuf0(nullptr),
    pInBuf1(nullptr),
    pOutBuf(nullptr),
    pInSrv0(nullptr),
    pInSrv1(nullptr),
    pOutUav(nullptr)
{

}

ComputeSample::~ComputeSample()
{
    SafeRelease(pOutUav);
    SafeRelease(pInSrv1);
    SafeRelease(pInSrv0);
    SafeRelease(pOutBuf);
    SafeRelease(pInBuf1);
    SafeRelease(pInBuf0);
    SafeRelease(pShader);
    SafeRelease(pShaderBlob);
    SafeRelease(pContext);
    SafeRelease(pDevice);
}

HRESULT ComputeSample::Init()
{
    D3D_FEATURE_LEVEL d3dFeatures[] = { D3D_FEATURE_LEVEL_11_0 };
    HR_RETURN(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, d3dFeatures, ARRAYSIZE(d3dFeatures), D3D11_SDK_VERSION, &pDevice, nullptr, &pContext));

    return S_OK;
}

HRESULT ComputeSample::CreateShader(LPCWSTR hlsl, LPCSTR entryPoint)
{
    ID3DBlob *pErrorBlob = nullptr;
    const D3D_SHADER_MACRO defines[] = 
    {
        "EXAMPLE_DEFINE", "1",
        NULL, NULL
    };    

    HRESULT /*hr = D3DX11CompileFromFile(hlsl, NULL, NULL, "CsMain",  "cs_4_0", D3D10_SHADER_ENABLE_STRICTNESS, NULL, NULL, &pShaderBlob, &pErrorBlob, NULL);

    */hr = D3DCompileFromFile(hlsl, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, "cs_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pShaderBlob, &pErrorBlob);

    if(FAILED(hr) || pShaderBlob == NULL)
    {
        return S_FALSE;
    }

    HR_RETURN(pDevice->CreateComputeShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), NULL, &pShader));

    return S_OK;
}

HRESULT ComputeSample::PrepareResources(UINT structStride, UINT count, void *pInitData0, void *pInitData1)
{
    D3D11_BUFFER_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.BindFlags = D3D10_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    desc.StructureByteStride = structStride;
    desc.ByteWidth = structStride * count;

    D3D11_SUBRESOURCE_DATA initData;
    if(pInitData0)
    {
        initData.pSysMem = pInitData0;
        HR_RETURN(pDevice->CreateBuffer(&desc, &initData, &pInBuf0));
    }
    else
    {
        HR_RETURN(pDevice->CreateBuffer(&desc, NULL, &pInBuf0));
    }

    if(pInitData1)
    {
        initData.pSysMem = pInitData1;
        HR_RETURN(pDevice->CreateBuffer(&desc, &initData, &pInBuf1));
    }
    else
    {
        HR_RETURN(pDevice->CreateBuffer(&desc, NULL, &pInBuf1));
    }

    HR_RETURN(pDevice->CreateBuffer(&desc, NULL, &pOutBuf));

    D3D11_SHADER_RESOURCE_VIEW_DESC descSrv;
    ZeroMemory(&descSrv, sizeof(descSrv));
    descSrv.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
    descSrv.Format = DXGI_FORMAT_UNKNOWN;
    descSrv.BufferEx.FirstElement = 0;
    descSrv.BufferEx.NumElements = count;
    
    HR_RETURN(pDevice->CreateShaderResourceView(pInBuf0, &descSrv, &pInSrv0));
    HR_RETURN(pDevice->CreateShaderResourceView(pInBuf1, &descSrv, &pInSrv1));

    D3D11_UNORDERED_ACCESS_VIEW_DESC descUav;
    ZeroMemory(&descUav, sizeof(descUav));
    descUav.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    descUav.Format = DXGI_FORMAT_UNKNOWN;
    descUav.Buffer.FirstElement = 0;
    descUav.Buffer.NumElements = count;

    HR_RETURN(pDevice->CreateUnorderedAccessView(pOutBuf, &descUav, &pOutUav));

    return S_OK;
}

void ComputeSample::Run(UINT x, UINT y, UINT z)
{
    pContext->CSSetShader(pShader, NULL, 0);

    ID3D11ShaderResourceView* pSrvs[2] = {pInSrv0, pInSrv1};
    pContext->CSSetShaderResources(0, 2, pSrvs);

    pContext->CSSetUnorderedAccessViews(0, 1, &pOutUav, NULL);

    pContext->Dispatch(x, y, z);

    pContext->Flush();

    ID3D11ShaderResourceView* pSrvNull[3] = {NULL, NULL, NULL};
    pContext->CSSetShaderResources(0, 3, pSrvNull);

    ID3D11UnorderedAccessView* pUavNull[2] = {NULL, NULL};
    pContext->CSSetUnorderedAccessViews(0, 2, pUavNull, NULL);
}

HRESULT ComputeSample::GetResult(void **ppBuf, UINT bufLen)
{
    HRESULT ret = S_OK;
    ID3D11Buffer *pResultStaging;
    D3D11_BUFFER_DESC desc;
    pOutBuf->GetDesc(&desc);
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.MiscFlags = 0;

    HR_RETURN(pDevice->CreateBuffer(&desc, NULL, &pResultStaging));

    pContext->CopyResource(pResultStaging, pOutBuf);

    D3D11_MAPPED_SUBRESOURCE mappedSubres;

    HR_RETURN(pContext->Map(pResultStaging, 0, D3D11_MAP_READ, 0, &mappedSubres));
    if(mappedSubres.RowPitch >= bufLen)
    {
        //memcpy_s(pBuf, bufLen, mappedSubres.pData, bufLen);
        memcpy(*ppBuf, mappedSubres.pData, bufLen);
    }
    else
    {
        ret = -1;
    }
    

    pContext->Unmap(pResultStaging, 0);

    SafeRelease(pResultStaging);
    return ret;
}

void ComputeSample::SafeRelease(IUnknown *p)
{
    if(p != nullptr) p->Release();
}