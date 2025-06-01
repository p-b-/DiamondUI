#pragma once
#include "ITextRenderer.h"
#include <vector>

class TextLine : public ITextLine
{
public:
	virtual ~TextLine();
	TextLine();
	TextLine(unsigned int unVAO, unsigned int unVBO, unsigned int unVertexCount, const FontFaceHandle& hFontFace);
	TextLine(const TextLine& toCopy);
	void Deinitialise();

	unsigned int GetVAO() const { return m_unVAO; }
	unsigned int GetVBO() const { return m_unVBO; }
	const FontFaceHandle& GetFontFaceHandle() const { return m_hFontFace; }
	unsigned int GetVertexCount() const { return m_unVertexCount; }
	bool IsValid() const { return m_unVAO != 0; }

	void SetWidth(float fWidth) { m_fWidth = fWidth; }
	void SetHeight(float fHeight, float fDescenderHeight) { m_fHeight = fHeight; m_fDescenderHeight = fDescenderHeight; }
	void SetCharOffsets(const std::vector<float>& vcCharOffsets) { m_vcCharOffsets = vcCharOffsets; }

	// ITextLine declarations
	virtual float GetWidth() const { return m_fWidth; }
	virtual float GetHeight() const { return m_fHeight; }
	virtual float GetDescenderHeight() const { return m_fDescenderHeight; }
	virtual float GetCharOffset(int nChar);

private:
	unsigned int m_unVAO;
	unsigned int m_unVBO;
	FontFaceHandle m_hFontFace;
	unsigned int m_unVertexCount;
	float m_fWidth;
	float m_fHeight;
	float m_fDescenderHeight;

	std::vector<float> m_vcCharOffsets;
};

