#pragma once
// DisplayPlaneClass는 두 개의 삼각형으로 이루어진 2D 평면을 생성하는 객체입니다.
// 3D 공간의 어디에든 렌더링할 수 있으므로 렌더 텍스처를 테스트하기에 완벽한 객체입니다.
// BitmapClass와 유사하게 작동하지만, 3D 렌더링용으로 설계되었습니다.

////////////////////////////////////////////////////////////////////////////////
// Filename: displayplaneclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _DISPLAYPLANECLASS_H_
#define _DISPLAYPLANECLASS_H_


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "d3dclass.h"


////////////////////////////////////////////////////////////////////////////////
// Class name: DisplayPlaneClass
////////////////////////////////////////////////////////////////////////////////
class DisplayPlaneClass
{
private:
    struct VertexType
    {
        XMFLOAT3 position;
        XMFLOAT2 texture;
    };

public:
    DisplayPlaneClass();
    DisplayPlaneClass(const DisplayPlaneClass&);
    ~DisplayPlaneClass();

    bool Initialize(ID3D11Device*, float, float);
    void Shutdown();
    void Render(ID3D11DeviceContext*);

    int GetIndexCount();

private:
    bool InitializeBuffers(ID3D11Device*, float, float);
    void ShutdownBuffers();
    void RenderBuffers(ID3D11DeviceContext*);

private:
    ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;
    int m_vertexCount, m_indexCount;
};

#endif