#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/TextRendering/StringEffectFragment.hpp"
#include <deque>

class MeshRenderer;
class BitmapFont;

//-----------------------------------------------------------------------------------------------
class TextBox
{
public:
    enum TextAlignment
    {
        LEFT_ALIGNED,
        RIGHT_ALIGNED,
        CENTER_ALIGNED
    };

    TextBox() {};
    TextBox(const Vector3& bottomLeft, const Vector3& upVector, const Vector3& rightVector, float width, float height, float scale, BitmapFont* baseFont, TextAlignment textAlignment = TextAlignment::LEFT_ALIGNED);
    ~TextBox();
    void SetFromXMLNode(const struct XMLNode& node);
    void ResetAnimation();
    void Update(float deltaSeconds);
    void Render() const;
    inline void ShowBox() { m_renderBox = true; };
    inline void HideBox() { m_renderBox = false; };

    std::vector<MeshRenderer*>* m_textRenderers;

private:
    void EvaluateLine(std::deque<StringEffectFragment>& currLine, std::deque<StringEffectFragment>& fragmentQueue);
    void ConstructMeshes();
    void InitializeBorder();

    MeshRenderer* m_borderRenderer;
    //MeshRenderer* m_backgroundRenderer;
    StringEffectFragments m_fragments;
    Vector3 m_bottomLeft;
    Vector3 m_upVector;
    Vector3 m_rightVector;
    BitmapFont* m_baseFont;
    float m_width;
    float m_height;
    float m_scale;
    float m_totalTimeSinceReset;
    TextAlignment m_alignment;
    bool m_renderBox;
};