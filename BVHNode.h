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
    int leftChild;  
    int  rightChild;
    
    int firstPrim; 
    int triangleCount;
    
    bool isLeaf() {return triangleCount > 0;}
    void Setup();
};


    struct BVH
    {
        void IntersectBVH(const Ray& ray, const int nodeIdx, HitRecord& hitRecord);
        bool IntersectBVH(const Ray& ray, const int nodeIdx);
        
        void BuildBVH(const std::vector<TriangleMesh>& triangleMeshes);
        void UpdateNodeBounds( int nodeIdx );
        static bool IntersectAABB(const Ray& ray, const Vector3 bmin, const Vector3 bmax, const HitRecord& hitRecord);
        void Subdivide( int nodeIdx );

        // triangle count
        std::vector<TriangleMesh> Meshes;
        
        //TriangleMesh m_Mesh;
        int amountOfTriangles;
        std::vector<BVHNode> bvhNode;
        std::vector<int> triangleIndex; 
        int rootNodeIdx = 0;
        int nodesUsed = 1;

        Triangle GetTriangleByIndex(int index) const;
    };
}
