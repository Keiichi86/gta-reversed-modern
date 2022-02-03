/*
    Plugin-SDK (Grand Theft Auto San Andreas) file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "ColSphere.h"
#include "ColBox.h"
#include "ColLine.h"
#include "CompressedVector.h"
#include "ColTriangle.h"
#include "ColTrianglePlane.h"
#include "ColDisk.h"
#include "Link.h"

namespace ColHelpers {
    struct TFaceGroup;
};

//
// https://gtamods.com/wiki/Collision_File
//

class CCollisionData {
public:
    CCollisionData();

public:
    uint16 m_nNumSpheres;       // 0x00
    uint16 m_nNumBoxes;         // 0x02
    uint16 m_nNumTriangles;     // 0x04
    uint8  m_nNumLines;         // 0x06


    struct { // 0x07
        uint8 bUsesDisks : 1;       // 0x1 - Always set to false
        uint8 bHasFaceGroups : 1;   // 0x2 - See the huge comment below
        uint8 bHasShadowInfo : 1;   // 0x4 - See wiki.
    };

    CColSphere* m_pSpheres;    // 0x08
    CColBox*    m_pBoxes;      // 0x0C

    union { // 0x10
        CColLine* m_pLines;
        CColDisk* m_pDisks;
    };
        
    CompressedVector*  m_pVertices;             // 0x14

    // If you take a look here: https://gtamods.com/wiki/Collision_File#Body
    // You may notiuce there's an extra section called `TFaceGroups` before `TFace` (triangles)
    // And it is used in `CCollision::ProcessColModels`.
    //
    // The following is only true if `bHasFaceGroups` flag is set (Which is the case only when loaded by `CFileLoader::LoadCollisionModelVer2/3/4`). **
    // All the data is basically stored in a big buffer, and all the data pointers - except `m_pTrianglePlanes` - point into it.
    // If case the flag `bHasFaceGroups` is set there's an array of `TFaceGroup` before the triangles, but there's no pointer to it.
    // In order to access it you have to do some black magic with `pTriangles`. Here's the memory layout of the `TFaceGroups` data:
    //
    // FaceGroup[]          - -0x8 - And growing downwards by `sizeof(FaceGroup)` (Which is 28)
    // uint32 nFaceGroups;  - -0x4
    // <Triangles>          - FaceGroup data is before the triangles!
    //
    // Whenever accessing this section make sure both `CColModel::bSingleAlloc` and `CCollisionData::bHasFaceGroups` is set
    // (also, please, assert if `bHasFaceGroups` is set but `bSingleAlloc` isnt)
    //
    // NOTEs:
    // ** Col models may also be loaded by the Collision plugin from a clump file - In this case `CFileLoader::LoadCollisionModelVer2/3` is called, but then
    //    the col data is reallocated using `MakeMultipleAlloc` which uses `Copy` to copy the data, in this case the face groups aren't copied. (And the flag is set to false in the ctor)
    CColTriangle*      m_pTriangles;            // 0x18
    CColTrianglePlane* m_pTrianglePlanes;       // 0x1C
    uint32             m_nNumShadowTriangles;   // 0x20
    uint32             m_nNumShadowVertices;    // 0x24
    CompressedVector*  m_pShadowVertices;       // 0x28
    CColTriangle*      m_pShadowTriangles;      // 0x2C

    // <size 0x30>

public:
    static void InjectHooks();

    void RemoveCollisionVolumes();
    void Copy(CCollisionData const& src);
    void CalculateTrianglePlanes();
    void RemoveTrianglePlanes();
    void GetTrianglePoint(CVector& outVec, int32 vertId);
    void GetShadTrianglePoint(CVector& outVec, int32 vertId);
    void SetLinkPtr(CLink<CCollisionData*>* link);
    CLink<CCollisionData*>* GetLinkPtr();

    // NOTSA
    auto GetNumFaceGroups() const -> uint32;


    auto GetSpheres() const { return std::span{ m_pSpheres, m_nNumSpheres }; }
    auto GetBoxes() const { return std::span{ m_pBoxes, m_nNumBoxes }; }
    auto GetTris() const { return std::span{ m_pTriangles, m_nNumTriangles }; }
    auto GetLines() const { return std::span{ m_pLines, m_nNumLines }; }

    auto GetFaceGroups() const -> std::span<ColHelpers::TFaceGroup>;

private:
    // HELPERS
    template <typename T> T* GetPointerToColArray(uint32 byteOffset) {
        return reinterpret_cast<T*>(&reinterpret_cast<uint8*>(this)[byteOffset]);
    }

    friend class CColModel;
};

VALIDATE_SIZE(CCollisionData, 0x30);
