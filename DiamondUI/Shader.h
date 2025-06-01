#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <tuple>

enum ShaderProgramType {
    eSPT_Fragment,
    eSPT_Vertex,
    eSPT_Program
};

class Shader
{
public:
	Shader();
    ~Shader();
    bool InitialiseShaders(const char* pzVertexPath, const char* pzFragmentPath);
    void Use() const;

    void SetBool(const std::string& name, bool value) const;
    void SetInt(const std::string& name, int value) const;
    void SetFloat(const std::string& name, float value) const;
    void SetVec2(const std::string& name, const glm::vec2& value) const;
    void SetVec2(const std::string& name, float x, float y) const;
    void SetVec3(const std::string& name, const glm::vec3& value) const;
    void SetVec3(const std::string& name, const std::tuple<float,float,float>& value) const;
    void SetVec3(const std::string& name, float x, float y, float z) const;
    void SetVec4(const std::string& name, const glm::vec4& value) const;
    void SetVec4(const std::string& name, const std::tuple<float,float,float,float>& value) const;
    void SetVec4(const std::string& name, float x, float y, float z, float w) const;
    void SetMat2(const std::string& name, const glm::mat2& mat) const;
    void SetMat3(const std::string& name, const glm::mat3& mat) const;
    void SetMat4(const std::string& name, const glm::mat4& mat) const;

public:
    unsigned int GetId() const { return m_unId; }

private:
    bool LoadShaders();
    bool LoadShaderFile(const char* pzFilepath, ShaderProgramType eType);
    bool CompileShaders();

    bool CheckCompileErrors(GLuint shader, ShaderProgramType eType);

private:
    const char* m_pzVertexPath;
    const char* m_pzFragmentPath;
    unsigned int m_unId;
    static unsigned int s_unShaderInUse;

    std::string m_sVertexCode;
    std::string m_sFragmentCode;
};


