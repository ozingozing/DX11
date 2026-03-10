#pragma once
////////////////////////////////////////////////////////////////////////////////
// Filename: textureclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _TEXTURECLASS_H_
#define _TEXTURECLASS_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <stdio.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

////////////////////////////////////////////////////////////////////////////////
// Class name: TextureClass
////////////////////////////////////////////////////////////////////////////////
class TextureClass
{
private:
    // 데이터를 더 쉽게 읽기 위해 타가(Targa) 파일 헤더 구조체를 정의합니다.
    struct TargaHeader
    {
        unsigned char data1[12];
        unsigned short width;
        unsigned short height;
        unsigned char bpp;
        unsigned char data2;
    };

public:
    TextureClass();
    TextureClass(const TextureClass&);
    ~TextureClass();

    // 텍스처를 초기화하는 함수입니다.
    bool Initialize(ID3D11Device*, ID3D11DeviceContext*, char*);
    // 텍스처 리소스를 해제하는 함수입니다.
    void Shutdown();

    // 셰이더 리소스 뷰를 반환하는 함수입니다.
    ID3D11ShaderResourceView* GetTexture();
    bool LoadFromMemory(unsigned char* data, int size);
    bool LoadImageFile(const char* filename);
    bool InitializeFromEmbedded(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const aiTexture* texture);

    // 텍스처의 너비와 높이를 반환하는 함수입니다.
    int GetWidth();
    int GetHeight();

    bool InitializeFromMemory(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const unsigned char* data, int size);
    bool InitializeFromRawRGBA(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const unsigned char* data, int width, int height);

private:
    // 타가 파일 읽기 함수입니다. 더 많은 형식을 지원하려면 여기에 읽기 함수를 추가할 수 있습니다.
    bool LoadTarga32Bit(HWND hwnd, char*);

private:
    // 이 클래스에는 다섯 개의 멤버 변수가 있습니다.
    // 첫 번째는 파일에서 직접 읽은 원시 타가 데이터를 담습니다.
    // 두 번째인 m_texture는 DirectX가 렌더링에 사용할 구조화된 텍스처 데이터를 가집니다.
    // 세 번째는 셰이더가 그리기 시 텍스처 데이터에 접근하는 데 사용하는 리소스 뷰입니다.
    // 마지막 두 변수는 텍스처의 크기입니다.
    unsigned char* m_targaData;
    ID3D11Texture2D* m_texture;
    ID3D11ShaderResourceView* m_textureView;
    int m_width, m_height;
};

#endif