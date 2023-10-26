#include "BVHNode.h"
#include "Utils.h"
#include "Scene.h"

namespace dae
{
    bool BVH::IntersectBVH(const Ray& ray, const int nodeIdx)
    {
        if(!isBuild) return false;
        
        BVHNode& node = bvhNode[nodeIdx];
        HitRecord hit_record{};

        //if the AABB is not hit, return false
        const bool AABB = IntersectAABB(ray, node.aabbMin, node.aabbMax, hit_record);
        if(!AABB) return false;
        
        if (node.isLeaf())
        {
            bool didHit = false;
            for (int i{}; i < node.triangleCount; ++i )
            {
               didHit =  GeometryUtils::HitTest_Triangle(GetTriangleByIndex(triangleIndex[node.firstPrim + i]), ray, hit_record);
               if(didHit) return true;
            }                       
        }
        else
        {
            const bool next = IntersectBVH( ray, node.firstPrim);
            const bool nextR = IntersectBVH( ray, node.firstPrim + 1);
        
            if(next || nextR) return true;
        }
        
        return false;
    }

    
    void BVH::IntersectBVH(const Ray& ray, const int nodeIdx, HitRecord& hitRecord)
    {
        if(!isBuild) return;
        
        BVHNode& node = bvhNode[nodeIdx];
        
        if (!IntersectAABB( ray, node.aabbMin, node.aabbMax, hitRecord)) return;

        if (node.isLeaf())
        {
            for (int i{}; i < node.triangleCount; ++i )
            {
                GeometryUtils::HitTest_Triangle(GetTriangleByIndex(triangleIndex[node.firstPrim + i]), ray, hitRecord);
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


        //Add the triangle count
        for(const auto& mesh : Meshes)
        {
            amountOfTriangles += static_cast<int>(mesh.GetAmountOfTriangles());
        }

        //Resize nodes and index of triangles
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

        isBuild = true;
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
        // terminate recursion when there is nothing more to split
        BVHNode& node = bvhNode[nodeIdx];
        if (node.triangleCount <= 2) return;

        // determine split axis and position
        Vector3 extent = node.aabbMax - node.aabbMin;
        int axis = 0;
        if (extent.y > extent.x) axis = 1;
        if (extent.z > extent[axis]) axis = 2;

        // Calculate the position of the splitting plane (median of the chosen axis)
        const float splitPos = node.aabbMin[axis] + extent[axis] * 0.5f;


        
        //Left pointer starts at the beginning of the array
        //Right pointer starts at the end of the array
        int leftPointer = node.firstPrim;
        int rightPointer = leftPointer + node.triangleCount - 1;
        
        while (leftPointer <= rightPointer)
        {
            // Check if the center coordinate of the current triangle is on the left side of the splitting plane
            if (GetTriangleByIndex(triangleIndex[leftPointer]).center[axis] < splitPos)
            {
                ++leftPointer;
            }
            else
            {
                // If the center coordinate is on the right side of the splitting plane, swap the triangles
                // This moves the right-side pointer to the left while maintaining order
                std::swap( triangleIndex[leftPointer], triangleIndex[rightPointer--] );
            }
                
        }
        
        //Abort split if one of the sides is empty
        const int leftCount = leftPointer - node.firstPrim;
        
        if (leftCount == 0 || leftCount == node.triangleCount) return;
        
        // Create child nodes for the left and right sub-nodes
        const int leftChildIdx = nodesUsed++;
        const int rightChildIdx = nodesUsed++;

        // Update child node properties
        bvhNode[leftChildIdx].firstPrim = node.firstPrim;
        bvhNode[leftChildIdx].triangleCount = leftCount;
        bvhNode[rightChildIdx].firstPrim = leftPointer;
        bvhNode[rightChildIdx].triangleCount = node.triangleCount - leftCount;
        node.firstPrim = leftChildIdx;
        node.triangleCount = 0;
        
        // Update the bounding boxes of child nodes
        UpdateNodeBounds( leftChildIdx );
        UpdateNodeBounds( rightChildIdx );

        // Subdivide teh left and right children
        Subdivide( leftChildIdx );
        Subdivide( rightChildIdx );
    }
    
    Triangle BVH::GetTriangleByIndex(int index) const
    {
        int triangleeIndex = index;
        
        for(size_t meshIndex{}; meshIndex < Meshes.size(); ++meshIndex)
        {
            const int numTrianglesInMesh = static_cast<int>(Meshes[meshIndex].GetAmountOfTriangles());

            if (triangleeIndex < numTrianglesInMesh)
            {
                return Meshes[meshIndex].GetTriangleByIndex(triangleeIndex);

            }

            triangleeIndex -= numTrianglesInMesh;
        }
        return {};
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