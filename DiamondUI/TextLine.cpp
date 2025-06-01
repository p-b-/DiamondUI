#include "TextLine.h"
#include <glad/glad.h>

TextLine::~TextLine() {
	Deinitialise();
}

TextLine::TextLine() {
	m_unVAO = 0;
	m_unVBO = 0;
	m_unVertexCount = 0;
	m_fWidth = 0.0f;
	m_fHeight = 0.0f;
	m_fDescenderHeight = 0.0f;
}

TextLine::TextLine(const TextLine& toCopy) {
	m_unVAO = toCopy.m_unVAO;
	m_unVBO = toCopy.m_unVBO;
	m_unVertexCount = toCopy.m_unVertexCount;
	m_hFontFace = toCopy.m_hFontFace;
	m_fWidth = toCopy.m_fWidth;
	m_fHeight = toCopy.m_fHeight;
	m_fDescenderHeight = toCopy.m_fDescenderHeight;
	m_vcCharOffsets = toCopy.m_vcCharOffsets;
}

TextLine::TextLine(unsigned int unVAO, unsigned int unVBO, unsigned int unVertexCount, const FontFaceHandle& hFontFace) {
	m_unVAO = unVAO;
	m_unVBO = unVBO;
	m_unVertexCount = unVertexCount;
	m_hFontFace = hFontFace;
	m_fWidth = 0.0f;
	m_fHeight = 0.0f;
	m_fDescenderHeight = 0.0f;
}

void TextLine::Deinitialise() {
	if (m_unVAO != 0) {
		glDeleteVertexArrays(1, &m_unVAO);
		m_unVAO = 0;
	}
	if (m_unVBO != 0) {
		glDeleteBuffers(1, &m_unVBO);
		m_unVBO = 0;
	}
	m_vcCharOffsets.clear();
}

// ITextLine implementation
//
float TextLine::GetCharOffset(int nChar) {
	if (nChar < 0) {
		return 0.0f;
	}
	if (nChar >= m_vcCharOffsets.size()) {
		return m_vcCharOffsets.back();
	}
	return m_vcCharOffsets[nChar];
}
//
// ITextLine implementation