#include "BVHNode.h"
#include "Utils.h"
#include "Scene.h"

namespace dae
{
    void BVH::IntersectBVH(const Ray& ray, const unsigned nodeIdx, HitRecord& hitRecord)
    {
        BVHNode& node = bvhNode[nodeIdx];
        
        if (!IntersectAABB( ray, node.aabbMin, node.aabbMax, hitRecord)) return;
        if (node.isLeaf())
        {
            for (unsigned int i = 0; i < node.triangleCount; ++i )
            {
                   GeometryUtils::HitTest_Triangle(m_Mesh.GetTriangleByIndex(triangleIndex[node.firstPrim + i]), ray, hitRecord);
            }
        }
        else
        {
            IntersectBVH( ray, node.firstPrim, hitRecord );
            IntersectBVH( ray, node.firstPrim + 1 , hitRecord);
        }
    }
    void BVH::BuildBVH(TriangleMesh& triangleMesh)
    {
        m_Mesh = triangleMesh;
        amountOfTriangles = static_cast<int>(m_Mesh.GetAmountOfTriangles());
        bvhNode.resize(static_cast<size_t>(amountOfTriangles) * 2);
        triangleIndex.resize(amountOfTriangles);
        
        // populate triangle index array
        for (unsigned int i{}; i < static_cast<unsigned int>(amountOfTriangles); ++i)
        {
            triangleIndex[i] = i;
        }
        
        // assign all triangles to root node
        BVHNode& root = bvhNode[rootNodeIdx];
        
        root.firstPrim = 0;
        root.triangleCount = amountOfTriangles;
        
        UpdateNodeBounds( rootNodeIdx );
        
        // subdivide recursively
        Subdivide( rootNodeIdx );
    }
    void BVH::UpdateNodeBounds( unsigned nodeIdx )
    {
        BVHNode& node = bvhNode[nodeIdx];
        node.aabbMin = Vector3( 1e30f,1e30f,1e30f  );
        node.aabbMax = Vector3( -1e30f,-1e30f, -1e30f  );
        
        for (unsigned int first = node.firstPrim, i = 0; i < node.triangleCount; ++i)
        {
            const unsigned int leafTriIdx = triangleIndex[first + i];
            
            Triangle leafTri = m_Mesh.GetTriangleByIndex(leafTriIdx);
            
            node.aabbMin = Vector3::Min(node.aabbMin, leafTri.v0);
            node.aabbMin = Vector3::Min(node.aabbMin, leafTri.v1);
            node.aabbMin = Vector3::Min(node.aabbMin, leafTri.v2);
            
            node.aabbMax = Vector3::Max(node.aabbMax, leafTri.v0);
            node.aabbMax = Vector3::Max(node.aabbMax, leafTri.v1);
            node.aabbMax = Vector3::Max(node.aabbMax, leafTri.v2);
        }
    }
    void BVH::Subdivide( unsigned int nodeIdx )
    {
        // terminate recursion
        BVHNode& node = bvhNode[nodeIdx];
        if (node.triangleCount <= 2) return;

        // determine split axis and position
        Vector3 extent = node.aabbMax - node.aabbMin;

        int axis = 0;

        if (extent.y > extent.x) axis = 1;
        if (extent.z > extent[axis]) axis = 2;

        const float splitPos = node.aabbMin[axis] + extent[axis] * 0.5f;

        // in-place partition
        unsigned int i = node.firstPrim;
        unsigned int j = i + node.triangleCount - 1;
        
        while (i <= j)
        {
            if (m_Mesh.GetTriangleByIndex(triangleIndex[i]).center[axis] < splitPos)
            {
                ++i;
            }
            else
            {
                std::swap( triangleIndex[i], triangleIndex[j--] );
            }
                
        }
        
        // abort split if one of the sides is empty
        const unsigned int leftCount = i - node.firstPrim;
        
        if (leftCount == 0 || leftCount == node.triangleCount) return;
        
        // create child nodes
        const unsigned int leftChildIdx = nodesUsed++;
        const unsigned int rightChildIdx = nodesUsed++;
        
        bvhNode[leftChildIdx].firstPrim = node.firstPrim;
        bvhNode[leftChildIdx].triangleCount = leftCount;
        bvhNode[rightChildIdx].firstPrim = i;
        bvhNode[rightChildIdx].triangleCount = node.triangleCount - leftCount;
        node.firstPrim = leftChildIdx;
        node.triangleCount = 0;
        UpdateNodeBounds( leftChildIdx );
        UpdateNodeBounds( rightChildIdx );

        // recurse
        Subdivide( leftChildIdx );
        Subdivide( rightChildIdx );
    }
    
    bool BVH::IntersectAABB( const Ray& ray, const Vector3 bmin, const Vector3 bmax,  HitRecord& /*hitRecord*/)
    {
        const float tx1 = (bmin.x - ray.origin.x) / ray.direction.x;
        float tx2 = (bmax.x - ray.origin.x) / ray.direction.x;

        float tmin = std::min( tx1, tx2 );
        float tmax = std::max( tx1, tx2 );

        float ty1 = (bmin.y - ray.origin.y) / ray.direction.y;
        float ty2 = (bmax.y - ray.origin.y) / ray.direction.y;

        tmin = std::max( tmin, std::min( ty1, ty2 ) );
        tmax = std::min( tmax, std::max( ty1, ty2 ) );

        float tz1 = (bmin.z - ray.origin.z) / ray.direction.z;
        float tz2 = (bmax.z - ray.origin.z) / ray.direction.z;

        tmin = std::max( tmin, std::min( tz1, tz2 ) );
        tmax = std::min( tmax, std::max( tz1, tz2 ) );
        
        return tmax >= tmin && tmax > 0;
    }
}