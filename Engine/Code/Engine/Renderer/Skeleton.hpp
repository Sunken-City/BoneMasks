#pragma once
#include <vector>
#include <string>
#include "Engine/Math/Matrix4x4.hpp"
#include "Engine/Input/BinaryReader.hpp"
#include "Engine/Input/BinaryWriter.hpp"

class MeshRenderer;
struct BoneMask;

struct Joint
{
    Joint(const std::string& name = "", int parentIndex = -1, const Matrix4x4& modelToBoneSpace = Matrix4x4::IDENTITY, const Matrix4x4& boneToModelSpace = Matrix4x4::IDENTITY);
    Joint(const Joint& other);

    std::string m_name;
    int m_parentIndex;
    std::vector<int> m_children;
    Matrix4x4 m_modelToBoneSpace;
    Matrix4x4 m_boneToModelSpace;
    Matrix4x4 m_localBoneToModelSpace;

    const int GetParentIndex() const;
    const std::string GetName() const;
    const std::vector<int> GetChildren() const;
};

class Skeleton
{
public:
    //CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
    Skeleton() : m_joints(nullptr), m_bones(nullptr) {};
    ~Skeleton();

    //FUNCTIONS//////////////////////////////////////////////////////////////////////////
    inline int GetLastAddedJointIndex() const { return m_jointArray.size() - 1; };
    void AddJoint(const char* str, int parentJointIndex, Matrix4x4 initialBoneToModelMatrix);
    int FindJointIndex(const std::string& name);
    void Render() const;
    void SetWorldBoneToModelAndCacheLocal(const Matrix4x4& mat, const int& index);
    void SetLocalBoneToModelAndWorldUpdate(const Matrix4x4& mat, const int& index);

    //GETTERS//////////////////////////////////////////////////////////////////////////
    uint32_t GetJointCount();
    Joint GetJoint(int index);
    const Matrix4x4 GetWorldBoneToModelOutOfLocal(const int& currentIndex) const;
    const BoneMask GetBoneMaskForJointName(const std::string& name, const float& flo = 1.f) const;
    const BoneMask GetBoneMaskForJointNames(const std::vector<std::string>& name, const float& flo = 1.f) const;
    //FILE IO//////////////////////////////////////////////////////////////////////////
    void WriteToFile(const char* filename);
    void WriteToStream(IBinaryWriter& writer);
    void ReadFromStream(IBinaryReader& reader);
    void ReadFromFile(const char* filename);

    //MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
    std::vector<Joint> m_jointArray;
    //std::vector<std::string> m_names;
    //std::vector<int> m_parentIndices;
    //std::vector<Matrix4x4> m_modelToBoneSpace;
    //std::vector<Matrix4x4> m_boneToModelSpace;
    mutable MeshRenderer* m_joints;
    mutable MeshRenderer* m_bones;

    static const unsigned int FILE_VERSION = 1;
    static const int INVALID_JOINT_INDEX = -1;
};