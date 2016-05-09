#include "Engine/Renderer/Skeleton.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/ShaderProgram.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Vertex.hpp"
#include "Engine/Renderer/AnimationMotion.hpp"
#include "Engine/Input/Console.hpp"

Skeleton* g_loadedSkeleton = nullptr;

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(saveSkel)
{
    if (!args.HasArgs(1))
    {
        Console::instance->PrintLine("saveSkel <filename>", RGBA::RED);
        return;
    }
    std::string filename = args.GetStringArgument(0);
    if (!g_loadedSkeleton)
    {
        Console::instance->PrintLine("Error: No skeleton has been loaded yet, use fbxLoad to bring in a mesh with a skeleton first.", RGBA::RED);
        return;
    }
    g_loadedSkeleton->WriteToFile(filename.c_str());
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(loadSkel)
{
    if (!args.HasArgs(1))
    {
        Console::instance->PrintLine("loadSkel <filename>", RGBA::RED);
        return;
    }
    std::string filename = args.GetStringArgument(0);
    if (g_loadedSkeleton)
    {
        delete g_loadedSkeleton;
    }
    g_loadedSkeleton = new Skeleton();
    g_loadedSkeleton->ReadFromFile(filename.c_str());
}

//-----------------------------------------------------------------------------------
Skeleton::~Skeleton()
{
    delete m_joints->m_mesh;
    delete m_joints->m_material;
    delete m_joints;
    delete m_bones->m_mesh;
    delete m_bones->m_material;
    delete m_bones;
}

//-----------------------------------------------------------------------------------
void Skeleton::AddJoint(const char* str, int parentJointIndex, Matrix4x4 initialBoneToModelMatrix)
{
    //m_names.push_back(std::string(str));
    //m_parentIndices.push_back(parentJointIndex);
    //m_boneToModelSpace.push_back(initialBoneToModelMatrix);
    Matrix4x4 modelToBoneMatrix = initialBoneToModelMatrix;
    Matrix4x4::MatrixInvert(&modelToBoneMatrix);
    //m_modelToBoneSpace.push_back(modelToBoneMatrix);
    Joint joint = Joint(std::string(str), parentJointIndex, modelToBoneMatrix, initialBoneToModelMatrix);

    if (parentJointIndex == -1)
    {
        joint.m_localBoneToModelSpace = initialBoneToModelMatrix;
    }
    else
    {
        //WorldInitial = LocalInitial * WorldParent
        //WorldInitial * WorldParent^-1 = localInitial
        //LocalInverse = inverse(LocalInitial) = WorldParent * WorldInitialInverse (?)
        //WorldInitial = LocalInitial * LocalParent * LocalParentParent ... Ri
        m_jointArray[parentJointIndex].m_children.push_back(m_jointArray.size());
        //Get Parent inverse Matrix
        Matrix4x4 parentModelToBone = m_jointArray[parentJointIndex].m_modelToBoneSpace;
        Matrix4x4 localBoneToModel = Matrix4x4::IDENTITY;
        Matrix4x4 localModelToBone = Matrix4x4::IDENTITY;
        //Calc Local bone to model and model to bones
        Matrix4x4::MatrixMultiply(&localBoneToModel, &initialBoneToModelMatrix, &parentModelToBone);
        joint.m_localBoneToModelSpace = localBoneToModel;

    }

    m_jointArray.push_back(joint);
    SetWorldBoneToModel(initialBoneToModelMatrix, m_jointArray.size() - 1);
 
}

//-----------------------------------------------------------------------------------
int Skeleton::FindJointIndex(const std::string& name)
{
    for (unsigned int i = 0; i < m_jointArray.size(); ++i)
    {
        if (strcmp((m_jointArray[i].m_name).c_str(),name.c_str()) == 0)
        {
            return i;
        }
    }
    return INVALID_JOINT_INDEX;
}

//-----------------------------------------------------------------------------------
Joint Skeleton::GetJoint(int index)
{
    //No case for if index less than 0 or greater than num of joints?
    return m_jointArray.at(index);
}
const Matrix4x4 Skeleton::GetWorldBoneToModelOutOfLocal(const int& currentIndex) const
{
    if (currentIndex < 0 || currentIndex >= (int)m_jointArray.size())
    {
        return Matrix4x4::IDENTITY;
    }
    Matrix4x4 current = m_jointArray[currentIndex].m_localBoneToModelSpace;
    if (m_jointArray[currentIndex].m_parentIndex != -1)
    {
        Matrix4x4 parent = GetWorldBoneToModelOutOfLocal(m_jointArray[currentIndex].m_parentIndex);
        Matrix4x4::MatrixMultiply(&current, &current, &parent);
    }

    return current;
}
const BoneMask Skeleton::GetBoneMaskForJointName(const std::string& name, const float& flo) const
{
    BoneMask mas(m_jointArray.size());
    mas.SetAllBonesTo(0.f);
    for (size_t jointIdx = 0; jointIdx < m_jointArray.size(); jointIdx++)
    {
        if (strcmp(name.c_str(), m_jointArray.at(jointIdx).m_name.c_str()) == 0)
        {
            mas.boneMasks[jointIdx] = flo;

            std::vector<int> children;
            children.insert(children.begin(), m_jointArray.at(jointIdx).m_children.begin(), m_jointArray.at(jointIdx).m_children.end());
            for (size_t i = 0; i < children.size(); i++)
            {
                mas.boneMasks[children.at(i)] = flo;

                std::vector<int> insertMe = m_jointArray.at(children.at(i)).m_children;
                children.insert(children.begin(), insertMe.begin(), insertMe.end());

                children.erase(children.begin());
                i--;
            }
            return mas;
        }
    }
    return mas;
}
const BoneMask Skeleton::GetBoneMaskForJointNames(const std::vector<std::string>& name, const float& flo) const
{
    BoneMask mas(m_jointArray.size());
    mas.SetAllBonesTo(0.f);
    for (size_t jointIdx = 0; jointIdx < m_jointArray.size(); jointIdx++)
    {
        if (strcmp(name.at(jointIdx).c_str(), m_jointArray.at(jointIdx).m_name.c_str()) == 0)
        {
            mas.boneMasks[jointIdx] = flo;
            std::vector<int> children;
            children.insert(children.begin(), m_jointArray.at(jointIdx).m_children.begin(), m_jointArray.at(jointIdx).m_children.end());
            for (size_t i = 0; i < children.size(); i++)
            {
                mas.boneMasks[children.at(i)] = flo;

                std::vector<int> insertMe = m_jointArray.at(children.at(i)).m_children;
                children.insert(children.begin(), insertMe.begin(), insertMe.end());

                children.erase(children.begin());
                i--;
            }
        }
    }
    return mas;
}

//-----------------------------------------------------------------------------------
void Skeleton::Render() const
{
    if (!m_joints)
    {
        MeshBuilder builder;
        for (size_t i = 0; i < m_jointArray.size(); i++)// const Matrix4x4& modelSpaceMatrix : m_boneToModelSpace)
        {
            const Matrix4x4 modelSpaceMatrix = GetWorldBoneToModelOutOfLocal(i);// m_jointArray.at(i).m_boneToModelSpace;
            builder.AddIcoSphere(1.0f, RGBA::BLUE, 0, modelSpaceMatrix.GetTranslation());
        }
        m_joints = new MeshRenderer(new Mesh(), new Material(new ShaderProgram("Data/Shaders/fixedVertexFormat.vert", "Data/Shaders/fixedVertexFormat.frag"), 
            RenderState(RenderState::DepthTestingMode::OFF, RenderState::FaceCullingMode::RENDER_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)));
        m_joints->m_material->SetDiffuseTexture(Renderer::instance->m_defaultTexture);
        builder.CopyToMesh(m_joints->m_mesh, &Vertex_PCUTB::Copy, sizeof(Vertex_PCUTB), &Vertex_PCUTB::BindMeshToVAO);
    }
    if (!m_bones)
    {
        MeshBuilder builder;
        for (unsigned int i = 0; i < m_jointArray.size(); i++)
        {
            int parentIndex = m_jointArray[i].m_parentIndex;
            if (parentIndex >= 0)
            {
                Matrix4x4 currentBoneToModel = GetWorldBoneToModelOutOfLocal(i); //m_jointArray[i].m_boneToModelSpace.GetTranslation()
                Matrix4x4 parentBoneToModel = GetWorldBoneToModelOutOfLocal(parentIndex); //m_jointArray[parentIndex].m_boneToModelSpace.GetTranslation()
                builder.AddLine(currentBoneToModel.GetTranslation(), parentBoneToModel.GetTranslation(), RGBA::SEA_GREEN);
            }
        }
        m_bones = new MeshRenderer(new Mesh(), new Material(new ShaderProgram("Data/Shaders/fixedVertexFormat.vert", "Data/Shaders/fixedVertexFormat.frag"),
            RenderState(RenderState::DepthTestingMode::OFF, RenderState::FaceCullingMode::RENDER_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)));
        m_bones->m_material->SetDiffuseTexture(Renderer::instance->m_defaultTexture);
        builder.CopyToMesh(m_bones->m_mesh, &Vertex_PCUTB::Copy, sizeof(Vertex_PCUTB), &Vertex_PCUTB::BindMeshToVAO);
    }
    m_joints->Render();
    m_bones->Render();
}

void Skeleton::SetWorldBoneToModel(const Matrix4x4& mat, const int& index)
{
    //Verify not accessing invalid index
    if (index < 0 || index >= (int)m_jointArray.size())
    {
        return;
    }

    /*
        m_jointArray[parentJointIndex].m_children.push_back(m_jointArray.size());
        //Get Parent inverse Matrix
        Matrix4x4 parentModelToBone = m_jointArray[parentJointIndex].m_modelToBoneSpace;
        Matrix4x4 localBoneToModel = Matrix4x4::IDENTITY;
        Matrix4x4 localModelToBone = Matrix4x4::IDENTITY;
        //Calc Local bone to model and model to bones
        Matrix4x4::MatrixMultiply(&localBoneToModel, &initialBoneToModelMatrix, &parentModelToBone);
        joint.m_localBoneToModelSpace = localBoneToModel;
    */

    //Set World Position, and Calc new Local Position.
    //Calc local position for parent. basically, parent should be only one we have to check that it's parent is not -1.
    Matrix4x4 copyAble = mat;
    m_jointArray[index].m_boneToModelSpace = mat;
    if (m_jointArray[index].m_parentIndex != -1)
    {
        Matrix4x4 parentMat = GetWorldBoneToModelOutOfLocal(m_jointArray[index].m_parentIndex);
        Matrix4x4::MatrixInvert(&parentMat);
        Matrix4x4::MatrixMultiply(&copyAble, &mat, &parentMat);
    }
    m_jointArray[index].m_localBoneToModelSpace = copyAble;


    //calc local position for children.
    std::vector<int> indexesToUpdate;
    indexesToUpdate.insert(indexesToUpdate.end(), m_jointArray[index].m_children.begin(), m_jointArray[index].m_children.end());
    for (size_t i = 0; i < indexesToUpdate.size(); i++)
    {
        //Get Indices
        int currentJoint = indexesToUpdate.at(i);

        //Calc new local position
        Matrix4x4 curLocal = GetWorldBoneToModelOutOfLocal(currentJoint);
        m_jointArray[currentJoint].m_boneToModelSpace = curLocal;

        //Add Children to iterate through.
        std::vector<int> toAdd = m_jointArray[currentJoint].m_children;
        indexesToUpdate.insert(indexesToUpdate.end(), toAdd.begin(), toAdd.end());

        indexesToUpdate.erase(indexesToUpdate.begin());
        i--;
    }

}
void Skeleton::SetLocalBoneToModel(const Matrix4x4& mat, const int& index)
{
    if (index < 0 || index >= (int)m_jointArray.size())
    {
        return;
    }

    //Set new Local Bone to Model
    m_jointArray[index].m_localBoneToModelSpace = mat;
    //Update World Position.
    Matrix4x4 copyAble = GetWorldBoneToModelOutOfLocal(index);
    m_jointArray[index].m_boneToModelSpace = copyAble;

    //calc local position for children.
    std::vector<int> indexesToUpdate;
    indexesToUpdate.insert(indexesToUpdate.end(), m_jointArray[index].m_children.begin(), m_jointArray[index].m_children.end());
    for (size_t i = 0; i < indexesToUpdate.size(); i++)
    {
        //Get Indices
        int currentJoint = indexesToUpdate.at(i);

        //Calc new local position
        Matrix4x4 curLocal = GetWorldBoneToModelOutOfLocal(currentJoint);
        m_jointArray[currentJoint].m_boneToModelSpace = curLocal;

        //Add Children to iterate through.
        std::vector<int> toAdd = m_jointArray[currentJoint].m_children;
        indexesToUpdate.insert(indexesToUpdate.end(), toAdd.begin(), toAdd.end());

        indexesToUpdate.erase(indexesToUpdate.begin());
        i--;
    }
}

//void Skeleton::SetLocalBoneToModel(const Matrix4x4& mat, const int& index)
//{
//    if (index < 0 || index >= (int)m_jointArray.size())
//    {
//        return;
//    }
//    //Set Local Position, and Calc new World Position.
//    Matrix4x4 parentWorldPosition = GetWorldBoneToModelOutOfLocal([index].m_parentIndex); // already has handling for indice passed in is invalid; returns identity, if is invalid.
//    Matrix4x4 worldPos = Matrix4x4::IDENTITY;
//    m_jointArray[index].m_localBoneToModelSpace = mat;
//    Matrix4x4::MatrixMultiply(worldPos, mat, parentWorldPosition);
//    m_jointArray[index].m_boneToModelSpace = worldPos;
//
//    //calc local position for children.
//    std::vector<int> indexesToUpdate = m_jointArray[index].m_children;
//    for (size_t i = 0; i < indexesToUpdate.size(); i++)
//    {
//        //Get Indices
//        int currentJoint = indexesToUpdate.at(i);
//        int parentJoint = (m_jointArray[currentJoint]).m_parentIndex;
//
//        //Calc new local position
//        Matrix4x4 cur = m_jointArray[currentJoint].m_boneToModelSpace;
//        Matrix4x4 par = m_jointArray[parentJoint].m_modelToBoneSpace;
//        Matrix4x4::MatrixMultiply(&copyAble, &cur, &par);
//        m_jointArray[currentJoint].m_localBoneToModelSpace = copyAble;
//        Matrix4x4 curWorldPos = GetWorldBoneToModelOutOfLocal(currentJoint);
//        m_jointArray[currentJoint].m_boneToModelSpace = curWorldPos;
//
//        //Add Children to iterate through.
//        std::vector<int> toAdd = m_jointArray[currentJoint].m_children;
//        indexesToUpdate.insert(indexesToUpdate.end(), toAdd.begin(), toAdd.end());
//
//        indexesToUpdate.erase(indexesToUpdate.begin());
//        i--;
//    }
//}

//-----------------------------------------------------------------------------------
uint32_t Skeleton::GetJointCount()
{
    return m_jointArray.size();
}

//-----------------------------------------------------------------------------------
Joint::Joint(const std::string& name, int parentIndex, const Matrix4x4& modelToBoneSpace, const Matrix4x4& boneToModelSpace)
    : m_name(name)
    , m_parentIndex(parentIndex)
    , m_modelToBoneSpace(modelToBoneSpace)
    , m_boneToModelSpace(boneToModelSpace)
    , m_localBoneToModelSpace(Matrix4x4::IDENTITY) //Due to the nature of skeleton not using pointers, have to calc these outside of Joint.
{

}
Joint::Joint(const Joint& other)
    : m_name(other.m_name),
    m_parentIndex(other.m_parentIndex),
    m_modelToBoneSpace(other.m_modelToBoneSpace),
    m_boneToModelSpace(other.m_boneToModelSpace),
    m_localBoneToModelSpace(other.m_localBoneToModelSpace)
{
    for (size_t i = 0; i < other.m_children.size(); i++)
    {
        m_children.push_back(other.m_children.at(i));
    }
}
const int Joint::GetParentIndex() const
{
    return m_parentIndex;
}
const std::string Joint::GetName() const
{
    return m_name;
}
const std::vector<int> Joint::GetChildren() const
{
    return m_children;
}

//-----------------------------------------------------------------------------------
void Skeleton::WriteToFile(const char* filename)
{
    BinaryFileWriter writer;
    ASSERT_OR_DIE(writer.Open(filename), "File Open failed!");
    {
        WriteToStream(writer);
    }
    writer.Close();
}

//-----------------------------------------------------------------------------------
void Skeleton::WriteToStream(IBinaryWriter& writer)
{
    //FILE VERSION
    //Number of joints
    //Joint Names
    //Joint Heirarchy
    //Initial Model Space

    writer.Write<uint32_t>(FILE_VERSION);
    writer.Write<uint32_t>(m_jointArray.size());
    
    for (size_t i = 0; i < m_jointArray.size(); i++)//const std::string& str : m_names)
    {
        std::string str = m_jointArray.at(i).m_name;
        writer.WriteString(str.c_str());
    }
    for (size_t i = 0; i < m_jointArray.size(); i++)//int index : m_parentIndices)
    {
        int index = m_jointArray.at(i).m_parentIndex;
        writer.Write<int>(index);
    }
    for (size_t i = 0; i < m_jointArray.size(); i++)//const Matrix4x4& mat : m_boneToModelSpace)
    {
        Matrix4x4 mat = m_jointArray.at(i).m_boneToModelSpace;
        //writer.WriteBytes(mat.data, 16);
        for (int j = 0; j < 16; ++j)
        {
            writer.Write<float>(mat.data[j]);
        }
    }
}

//-----------------------------------------------------------------------------------
void Skeleton::ReadFromStream(IBinaryReader& reader)
{
    //FILE VERSION
    //Number of joints
    //Joint Names
    //Joint Heirarchy
    //Initial Model Space

    uint32_t fileVersion = 0;
    ASSERT_OR_DIE(reader.Read<uint32_t>(fileVersion), "Failed to read file version");
    ASSERT_OR_DIE(fileVersion == FILE_VERSION, "File version didn't match!");
    uint32_t numberOfJoints = 0;
    ASSERT_OR_DIE(reader.Read<uint32_t>(numberOfJoints), "Failed to read number of joints");
    m_jointArray.resize(numberOfJoints);

    for (unsigned int i = 0; i < numberOfJoints; ++i)
    {
        const char* jointName = nullptr;
        reader.ReadString(jointName, 64);
        m_jointArray.at(i).m_name = (jointName);
    }
    for (unsigned int i = 0; i < numberOfJoints; ++i)
    {
        int index = 0;
        reader.Read<int>(index);
        m_jointArray.at(i).m_parentIndex = (index);
    }
    for (unsigned int i = 0; i < numberOfJoints; ++i)
    {
        Matrix4x4 matrix = Matrix4x4::IDENTITY;
        for (int j = 0; j < 16; ++j)
        {
            reader.Read<float>(matrix.data[j]);
        }
        m_jointArray.at(i).m_boneToModelSpace = matrix;
        //m_boneToModelSpace.push_back(Matrix4x4((float*)reader.ReadBytes(16)));
        //Matrix4x4 invertedMatrix = m_boneToModelSpace[i];
        //Matrix4x4::MatrixInvert(&invertedMatrix);
        //m_modelToBoneSpace.push_back(invertedMatrix);
    }
}

//-----------------------------------------------------------------------------------
void Skeleton::ReadFromFile(const char* filename)
{
    BinaryFileReader reader;
    ASSERT_OR_DIE(reader.Open(filename), "File Open failed!");
    {
        ReadFromStream(reader);
    }
    reader.Close();
}