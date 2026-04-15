#pragma once
////////////////////////////////////////////////////////////////////////////////
// 파일명: textclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _TEXTCLASS_H_
#define _TEXTCLASS_H_

///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "fontclass.h"

////////////////////////////////////////////////////////////////////////////////
// 클래스명: TextClass
////////////////////////////////////////////////////////////////////////////////
class TextClass
{
private:
    // VertexType 구조체는 FontClass에서 정의한 구조체와 반드시 일치해야 합니다.
    struct VertexType
    {
        XMFLOAT3 position; // 정점의 위치 (x, y, z)
        XMFLOAT2 texture;  // 텍스처 좌표 (u, v)
    };

public:
    TextClass();
    TextClass(const TextClass&);
    ~TextClass();

    // Initialize: 원하는 문장에 맞춰 정점 버퍼(Vertex Buffer)를 생성합니다.
    // Shutdown: 생성된 정점 버퍼 및 리소스를 해제합니다.
    // Render: 텍스트 문장을 그리기 위해 버퍼를 파이프라인에 설정(Draw 준비)합니다.
    bool Initialize(ID3D11Device*, ID3D11DeviceContext*, int, int, int, FontClass*, char*, int, int, float, float, float);
    void Shutdown();
    void Render(ID3D11DeviceContext*);

    int GetIndexCount();

    // UpdateText: 문장의 내용을 변경할 수 있게 하여 TextClass 객체를 재사용할 수 있도록 합니다.
    // GetPixelColor: 셰이더가 텍스트를 어떤 색상으로 렌더링할지 알 수 있도록 문장의 색상 값을 반환합니다.
    bool UpdateText(ID3D11DeviceContext*, FontClass*, char*, int, int, float, float, float);
    XMFLOAT4 GetPixelColor();

private:
    // 버퍼 초기화, 해제, 렌더링을 담당하는 내부 함수들입니다.
    bool InitializeBuffers(ID3D11Device*, ID3D11DeviceContext*, FontClass*, char*, int, int, float, float, float);
    void ShutdownBuffers();
    void RenderBuffers(ID3D11DeviceContext*);

private:
    ID3D11Buffer* m_vertexBuffer, * m_indexBuffer; // 텍스트용 정점 및 인덱스 버퍼
    int m_screenWidth, m_screenHeight;            // 화면 해상도 정보
    int m_maxLength;                              // 문장의 최대 길이
    int m_vertexCount, m_indexCount;              // 정점 및 인덱스의 개수
    XMFLOAT4 m_pixelColor;                        // 텍스트 렌더링 시 사용할 색상
};

#endif