#include "BVHNode.h"
#include "Utils.h"
#include "Scene.h"

namespace dae
{
    void BVH::IntersectBVH(const Ray& ray, const int nodeIdx, HitRecord& hitRecord)
    {
        BVHNode& node = bvhNode[nodeIdx];
        
        if (!IntersectAABB( ray, node.aabbMin, node.aabbMax, hitRecord)) return;
        if (node.isLeaf())
        {
            for(const auto& mesh : Meshes)
            {
                for (int i{}; i < node.triangleCount; ++i )
                {
                    GeometryUtils::HitTest_Triangle(mesh.GetTriangleByIndex(triangleIndex[node.firstPrim + i]), ray, hitRecord);
                }

                //Todo Figure out when i call this function istead of a hittest on every triangle that the code is only half as fast.
                //GeometryUtils::HitTest_TriangleMesh(mesh, ray, hitRecord);
            }
        }
        else
        {
            IntersectBVH( ray, node.firstPrim, hitRecord );
            IntersectBVH( ray, node.firstPrim + 1 , hitRecord);
        }
    }
    void BVH::BuildBVH(const std::vector<TriangleMesh>& triangleMeshes)
    {
        //Get the mesh
        Meshes = triangleMeshes;

        for(const auto& mesh : Meshes)
        {
            amountOfTriangles += static_cast<int>(mesh.GetAmountOfTriangles());
        }
        
        bvhNode.resize(static_cast<size_t>(amountOfTriangles) * 2);
        triangleIndex.resize(amountOfTriangles);
        
        // populate triangle index array
        for (int i{}; i < amountOfTriangles; ++i)
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
    void BVH::UpdateNodeBounds( int nodeIdx )
    {
        BVHNode& node = bvhNode[nodeIdx];
        node.aabbMin = Vector3( FLT_MAX,FLT_MAX,FLT_MAX  );
        node.aabbMax = Vector3( FLT_MIN,FLT_MIN, FLT_MIN  );
        
        for (int first = node.firstPrim, i = 0; i < node.triangleCount; ++i)
        {
            const int leafTriIdx = triangleIndex[first + i];
            Triangle leafTri = GetTriangleByIndex(leafTriIdx);

            //Todo Ceheck if this works
            //Triangle leafTri = m_Mesh.GetTriangleByIndex(leafTriIdx);
            
            node.aabbMin = Vector3::Min(node.aabbMin, leafTri.v0);
            node.aabbMin = Vector3::Min(node.aabbMin, leafTri.v1);
            node.aabbMin = Vector3::Min(node.aabbMin, leafTri.v2);
            
            node.aabbMax = Vector3::Max(node.aabbMax, leafTri.v0);
            node.aabbMax = Vector3::Max(node.aabbMax, leafTri.v1);
            node.aabbMax = Vector3::Max(node.aabbMax, leafTri.v2);
        }
    }
    void BVH::Subdivide( int nodeIdx )
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
        int i = node.firstPrim;
        int j = i + node.triangleCount - 1;
        
        while (i <= j)
        {
            if (GetTriangleByIndex(triangleIndex[i]).center[axis] < splitPos)
            {
                ++i;
            }
            else
            {
                std::swap( triangleIndex[i], triangleIndex[j--] );
            }
                
        }
        
        // abort split if one of the sides is empty
        const int leftCount = i - node.firstPrim;
        
        if (leftCount == 0 || leftCount == node.triangleCount) return;
        
        // create child nodes
        const int leftChildIdx = nodesUsed++;
        const int rightChildIdx = nodesUsed++;
        
        bvhNode[leftChildIdx].firstPrim = node.firstPrim;
        bvhNode[leftChildIdx].triangleCount = leftCount;
        bvhNode[rightChildIdx].firstPrim = i;
        bvhNode[rightChildIdx].triangleCount = node.triangleCount - leftCount;
        node.firstPrim = leftChildIdx;
        node.triangleCount = 0;
        UpdateNodeBounds( leftChildIdx );
        UpdateNodeBounds( rightChildIdx );

        // Subdivide teh left and right childs
        Subdivide( leftChildIdx );
        Subdivide( rightChildIdx );
    }

    Triangle BVH::GetTriangleByIndex(int index) const
    {
        Triangle triangle;

        //find the triangle with the leafTriIdx
        for(const auto& mesh : Meshes)
        {
            //Get the triangle
            const auto tri = mesh.GetTriangleByIndex(index);

            //Check if the triangle is valid. //We do that by checking if the normal is not 0
            if(tri.normal.Magnitude() > FLT_EPSILON || tri.normal.Magnitude() < -FLT_EPSILON) return tri;
        }

        return triangle;
    }

    bool BVH::IntersectAABB( const Ray& ray, const Vector3 bmin, const Vector3 bmax, const HitRecord& hitRecord)
    {
        const float tx1 = (bmin.x - ray.origin.x) / ray.direction.x;
        const float tx2 = (bmax.x - ray.origin.x) / ray.direction.x;

        float tMin = std::min( tx1, tx2 );
        float tMax = std::max( tx1, tx2 );

        const float ty1 = (bmin.y - ray.origin.y) / ray.direction.y;
        const float ty2 = (bmax.y - ray.origin.y) / ray.direction.y;

        tMin = std::max( tMin, std::min( ty1, ty2 ) );
        tMax = std::min( tMax, std::max( ty1, ty2 ) );

        const float tz1 = (bmin.z - ray.origin.z) / ray.direction.z;
        const float tz2 = (bmax.z - ray.origin.z) / ray.direction.z;

        tMin = std::max( tMin, std::min( tz1, tz2 ) );
        tMax = std::min( tMax, std::max( tz1, tz2 ) );
        
        return tMax >= tMin && tMin < hitRecord.t && tMax > 0;
    }
}