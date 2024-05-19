#include <iostream>
#include <vector>

#include <dx11computesample.h>

#pragma once

using namespace std;

#define NUM_ELEMENTS 10

struct BufType
{
    int i;
    float f;

    void SetValue( int i, float f )
    {
        this->i = i;
        this->f = f;
    }
};

int main(int, char**)
{
    HRESULT hr = S_OK;
    cout << "Hello World" << std::endl;

    ComputeSample app{};

    hr = app.Init();
    if(FAILED(hr))
    {
        cout<< "App Init Failed"<< endl;
        return -1;
    }

    hr = app.CreateShader(L"BasicCompute11.hlsl", "CSMain");
    if(FAILED(hr))
    {
        cout << "Shader Create Failed" << endl;
        return -1;
    }

    vector<BufType> vBuf0, vBuf1;

    vBuf0.resize(NUM_ELEMENTS);
    vBuf1.resize(NUM_ELEMENTS);
    for(int i=0; i<NUM_ELEMENTS; i++)
    {
        vBuf0[i].SetValue(i, (float)i);
        vBuf1[i].SetValue(i, (float)i);
    }

    hr = app.PrepareResources(sizeof(BufType), NUM_ELEMENTS, vBuf0.data(), vBuf1.data());
    if(FAILED(hr))
    {
        cout << "Resource preparation Failed" << endl;
        return -1;
    }

    app.Run(NUM_ELEMENTS,1,1);

    BufType *resultBuf = static_cast<BufType*>(malloc(NUM_ELEMENTS * sizeof(BufType)));

    if( resultBuf == nullptr)
    {
        cout << "Malloc Failed" << endl;
        return -1;
    }

    memset(resultBuf, 0, (NUM_ELEMENTS * sizeof(BufType)));

    hr = app.GetResult((void**)&resultBuf, NUM_ELEMENTS * sizeof(BufType));
    if(FAILED(hr))
    {
        cout << "GetResult Failed" << endl;
        free(resultBuf);
        return -1;
    }


    FILE *fp;
    auto err = fopen_s(&fp, "output.txt", "w");
    if( err == 0)
    {
        for(int i=0; i < NUM_ELEMENTS; i++)
        {
            fprintf(fp, "%d %f \n", resultBuf[i].i, resultBuf[i].f);
        }

        fclose(fp);
    }
    
    free(resultBuf);
    return 0;
}