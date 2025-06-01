#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

unsigned int Shader::s_unShaderInUse = 0;
const char* g_ShaderTypeNames[] = { "fragment", "vertex", "program" };

Shader::Shader() {
    m_unId = 0;
    m_pzVertexPath = nullptr;
    m_pzFragmentPath = nullptr;
}

Shader::~Shader() {
    if (m_unId != 0) {
        glDeleteProgram(m_unId);
    }
}

bool Shader::InitialiseShaders(const char* pzVertexPath, const char* pzFragmentPath) {
    m_pzVertexPath = pzVertexPath;
    m_pzFragmentPath = pzFragmentPath;
    if (!LoadShaders()) {
        return false;
    }
    if (!CompileShaders()) {
        return false;
    }
    return true;
}

bool Shader::LoadShaders() {
    if (LoadShaderFile(m_pzVertexPath, eSPT_Vertex) && 
        LoadShaderFile(m_pzFragmentPath, eSPT_Fragment)) {
        return true;
    }
    return false;
}

bool Shader::LoadShaderFile(const char* pzFilepath, ShaderProgramType eType) {
    std::string sCode;
    std::ifstream fShader;
    fShader.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        std::cout << "Opening " << g_ShaderTypeNames[eType] << " shader file " << m_pzVertexPath << std::endl;
        fShader.open(pzFilepath);

        std::stringstream ssShader;
        std::stringstream sFragShader;
        ssShader << fShader.rdbuf();
        fShader.close();
        std::string sCode = ssShader.str();

        switch (eType) {
        case eSPT_Vertex:
            m_sVertexCode = sCode;
            break;
        case eSPT_Fragment:
            m_sFragmentCode = sCode;
            break;
        default:
            return false;
        }
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "Shader error, file not read: " << e.what() << std::endl;
        return false;
    }

    return true;
}

bool Shader::CompileShaders() {
    bool bSuccess = false;
    const char* pzVertCode = m_sVertexCode.c_str();
    const char* pzFragCode = m_sFragmentCode.c_str();

    unsigned int unVertex = 0;
    unsigned int unFragment = 0;
    // vertex shader
    unVertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(unVertex, 1, &pzVertCode, NULL);
    glCompileShader(unVertex);
    std::cout << "Checking file " << m_pzVertexPath << " for compile errors" << std::endl;
    if (CheckCompileErrors(unVertex, eSPT_Vertex)) {
        // fragment Shader
        unFragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(unFragment, 1, &pzFragCode, NULL);
        glCompileShader(unFragment);
        std::cout << "Checking file " << m_pzFragmentPath << " for compile errors" << std::endl;
        if (CheckCompileErrors(unFragment, eSPT_Fragment)) {
            // shader Program
            m_unId = glCreateProgram();
            glAttachShader(m_unId, unVertex);
            glAttachShader(m_unId, unFragment);
            glLinkProgram(m_unId);
            std::cout << "Linking shaders " << m_pzVertexPath << ", " << m_pzFragmentPath << std::endl;
            if (CheckCompileErrors(m_unId, eSPT_Program)) {
                bSuccess = true;
            }
        }
    }
    // delete the shaders as they're linked into our program now and no longer necessary
    if (unVertex != 0) {
        glDeleteShader(unVertex);
    }
    if (unFragment != 0) {
        glDeleteShader(unFragment);
    }
    return bSuccess;
}


void Shader::Use() const
{
    if (s_unShaderInUse != m_unId) {
        s_unShaderInUse = m_unId;
        glUseProgram(m_unId);
    }
}

void Shader::SetBool(const std::string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(m_unId, name.c_str()), (int)value);
}

void Shader::SetInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(m_unId, name.c_str()), value);
}

void Shader::SetFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(m_unId, name.c_str()), value);
}

void Shader::SetVec2(const std::string& name, const glm::vec2& value) const
{
    glUniform2fv(glGetUniformLocation(m_unId, name.c_str()), 1, &value[0]);
}

void Shader::SetVec2(const std::string& name, float x, float y) const
{
    glUniform2f(glGetUniformLocation(m_unId, name.c_str()), x, y);
}

void Shader::SetVec3(const std::string& name, const glm::vec3& value) const
{
    glUniform3fv(glGetUniformLocation(m_unId, name.c_str()), 1, &value[0]);
}

void Shader::SetVec3(const std::string& name, const std::tuple<float, float, float>& value) const {
    glUniform3f(glGetUniformLocation(m_unId, name.c_str()), std::get<0>(value), std::get<1>(value), std::get<2>(value));
}

void Shader::SetVec3(const std::string& name, float x, float y, float z) const
{
    glUniform3f(glGetUniformLocation(m_unId, name.c_str()), x, y, z);
}

void Shader::SetVec4(const std::string& name, const glm::vec4& value) const
{
    glUniform4fv(glGetUniformLocation(m_unId, name.c_str()), 1, &value[0]);
}

void Shader::SetVec4(const std::string& name, const std::tuple<float, float, float,float>& value) const {
    glUniform4f(glGetUniformLocation(m_unId, name.c_str()), std::get<0>(value), std::get<1>(value), std::get<2>(value), std::get<3>(value));
}

void Shader::SetVec4(const std::string& name, float x, float y, float z, float w) const
{
    glUniform4f(glGetUniformLocation(m_unId, name.c_str()), x, y, z, w);
}

void Shader::SetMat2(const std::string& name, const glm::mat2& mat) const
{
    glUniformMatrix2fv(glGetUniformLocation(m_unId, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::SetMat3(const std::string& name, const glm::mat3& mat) const
{
    glUniformMatrix3fv(glGetUniformLocation(m_unId, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::SetMat4(const std::string& name, const glm::mat4& mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(m_unId, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

bool Shader::CheckCompileErrors(GLuint shader, ShaderProgramType eType) {
    GLint success;
    GLchar szIinfoLog[1024];
    if (eType!= eSPT_Program) {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, szIinfoLog);
            std::cerr << "Error compiling shader of type: " << g_ShaderTypeNames[eType] << std::endl;
            std::cerr << szIinfoLog << std::endl;
            return false;
        }
    }
    else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, szIinfoLog);
            std::cerr << "Error linking program of type: " << g_ShaderTypeNames[eType] << std::endl;
            std::cerr << szIinfoLog << std::endl;
            return false;
        }
    }
    return true;
} 