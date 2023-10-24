#pragma once
#include "Vector3.h"
#include "DataTypes.h"

namespace dae
{
struct Ray;    
struct BVHNode
{
    Vector3 aabbMin;
    Vector3 aabbMax;  
    unsigned int leftChild;  
    unsigned int  rightChild;
    
    unsigned int firstPrim; 
    unsigned int triangleCount;
    
    bool isLeaf() {return triangleCount > 0;}
    void Setup();
};


    struct BVH
    {
        void IntersectBVH(const Ray& ray, const unsigned int nodeIdx, HitRecord& hitRecord);
        void BuildBVH(TriangleMesh& triangleMesh);
        void UpdateNodeBounds( unsigned int nodeIdx );
        bool IntersectAABB(const Ray& ray, const Vector3 bmin, const Vector3 bmax, HitRecord& hitRecord);
        void Subdivide( unsigned int nodeIdx );

        // triangle count
        TriangleMesh m_Mesh;
        int amountOfTriangles;
        std::vector<BVHNode> bvhNode;
        std::vector<unsigned int> triangleIndex; 
        unsigned int rootNodeIdx = 0;
        unsigned int nodesUsed = 1;
    };
}
