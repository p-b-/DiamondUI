#pragma once
#include <string>
#include <atomic>
#include <map>

class Textures
{
	struct Atlas {
	public:
		int nSubTextureWidth;
		int nSubTextureHeight;
		int nHorizCount;
		int nVertCount;
		unsigned int unColourFormat;
		unsigned int unTextureId;
		int nNextFlattenedCoord;

		std::map<int, unsigned int> mpIdToFlattenedCoords;
	};
public:
	Textures();
	~Textures();
	unsigned int CreateTexture(const char* pzFilepath, const std::string& name, bool useAlpha, bool flipVertically);
	unsigned int GetTextureIdFromName(const std::string& name);

	unsigned int CreateTextureAtlas(int nSubtextureWidth, int nSubTextureHeight, int nHorizCount, int nVertCount, unsigned int unColourFormat);
	unsigned int GetTextureIdForTextureAtlas(unsigned int unTextureAtlasId);
	bool GetTexelForTextureInAtlas(unsigned int unAtlasId, unsigned int unSubTextureId, float* pxOrigin, float* pyOrigin, float* pnWidth, float* pnHeight);
	void DeleteTextureAtlas(unsigned int unTextureAtlasId);

	bool AddTextureToAtlas(unsigned int unAtlasId, const char* pzFilepath, unsigned int unId);
private:
	Textures::Atlas* GetAtlas(unsigned int unAtlasId);
	void DeleteAtlas(unsigned int unAtlasId);

private:
	std::map<std::string, unsigned int> m_mapTextureNameToId;

	std::atomic<unsigned int> m_unNextAtlasId = 1;
	std::map<unsigned int, Atlas* > m_mpTextureAtlases;
};

