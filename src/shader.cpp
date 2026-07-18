#include "glad/gl.h"
#include "shader.hpp"

#include <vector>
#include <print>

#include <filesystem>
#include <fstream>
#include <string>

// --- Interne Hilfsfunktionen für GLSL-Kompilierung ---

// Kompiliert einen GLSL-Shader aus einem Quell-String
static GLuint compileGLSLShader(GLenum type, std::string_view source)
{
    GLuint shader = glCreateShader(type);
    const char *src = source.data();

    // GLSL-Quellcode an das Shader-Objekt übergeben
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    // Error Checking
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLint length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> infoLog(length);
        glGetShaderInfoLog(shader, length, nullptr, infoLog.data());

        std::print("GLSL Shader Kompilierungsfehler ({}):\n{}\n",
                   (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment"),
                   infoLog.data());

        glDeleteShader(shader);
        return 0;
    }
    std::print("GLSL Shader erfolgreich kompiliert ({}).\n", (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment"));
    return shader;
}

// Kompiliert einen SPIR-V Shader aus Binärdaten
static GLuint compileSPIRVShader(GLenum type, const std::vector<uint32_t>& spirvBinary)
{
    GLuint shader = glCreateShader(type);
    
    // SPIR-V Binärdaten an das Shader-Objekt übergeben
    glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V, spirvBinary.data(), spirvBinary.size() * sizeof(uint32_t));

    // Shader spezialisieren (Einstiegspunkt "main")
    glSpecializeShaderARB(shader, "main", 0, nullptr, nullptr);

    // Fehlerprüfung
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success); // GL_COMPILE_STATUS wird auch für die Spezialisierung verwendet
    if (!success)
    {
        GLint length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> infoLog(length);
        glGetShaderInfoLog(shader, length, nullptr, infoLog.data());

        std::print("SPIR-V Shader Spezialisierungsfehler ({}):\n{}\n",
                   (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment"),
                   infoLog.data());

        glDeleteShader(shader);
        return 0;
    }
    std::print("SPIR-V Shader erfolgreich spezialisiert ({}).\n", (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment"));
    return shader;
}

namespace bgl
{

    // --- GLSL Shader Laden und Programm-Erstellung ---

    std::string LoadShaderFromFile(const std::filesystem::path &path)
    {
        std::ifstream file(path, std::ios::in);
        if (!file.is_open())
        {
            std::print("Fehler: GLSL Shader-Datei konnte nicht geöffnet werden: {}\n", path.string());
            return "";
        }

        file.seekg(0, std::ios::end);
        std::streampos size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::string buffer(static_cast<size_t>(size), ' ');
        file.read(buffer.data(), size); // Direkt in den Puffer des Strings lesen
        return buffer;
    }

    GLuint CreateShaderProgramFromGLSL(std::string_view vertexSrc, std::string_view fragmentSrc)
    {
        std::print("Erstelle Shader-Programm aus GLSL-Quellcode...\n");
        GLuint vertexShader = compileGLSLShader(GL_VERTEX_SHADER, vertexSrc);
        GLuint fragmentShader = compileGLSLShader(GL_FRAGMENT_SHADER, fragmentSrc);

        // Abbruch, falls ein Shader nicht kompiliert werden konnte
        if (vertexShader == 0 || fragmentShader == 0)
        {
            return 0;
        }

        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        // Error Checking für das Linking
        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            GLint length;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            std::vector<char> infoLog(length);
            glGetProgramInfoLog(program, length, nullptr, infoLog.data());

            std::print("GLSL Shader Programm Linking Fehler:\n{}\n", infoLog.data());

            glDeleteProgram(program);
            return 0;
        }

        // Cleanup: Nach dem erfolgreichen Linken brauchen wir die einzelnen Shader-Objekte nicht mehr
        glDetachShader(program, vertexShader);
        glDetachShader(program, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        std::print("GLSL Shader-Programm erfolgreich erstellt.\n");
        return program;
    }

    // --- SPIR-V Shader Laden und Programm-Erstellung ---

    std::vector<uint32_t> LoadSPIRVShaderFromFile(const std::filesystem::path &path)
    {
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        if (!file.is_open())
        {
            std::print("Fehler: SPIR-V Shader-Datei konnte nicht geöffnet werden: {}\n", path.string());
            return {};
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        // SPIR-V ist ein Array von uint32_t, daher muss die Größe ein Vielfaches von 4 sein
        if (fileSize % sizeof(uint32_t) != 0) {
            std::print("Fehler: SPIR-V Dateigröße ist kein Vielfaches von 4: {}\n", path.string());
            return {};
        }
        std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

        file.seekg(0);
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
        file.close();

        std::print("SPIR-V Datei geladen: {} ({} Bytes)\n", path.string(), fileSize);
        return buffer;
    }

    GLuint CreateShaderProgramFromSPIRV(const std::vector<uint32_t>& vertexSpv, const std::vector<uint32_t>& fragmentSpv)
    {
        std::print("Erstelle Shader-Programm aus SPIR-V Binärdateien...\n");
        GLuint vertexShader = compileSPIRVShader(GL_VERTEX_SHADER, vertexSpv);
        GLuint fragmentShader = compileSPIRVShader(GL_FRAGMENT_SHADER, fragmentSpv);

        if (vertexShader == 0 || fragmentShader == 0)
        {
            return 0;
        }

        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        // Fehlerprüfung für das Linking (wie bei GLSL)
        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            GLint length;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            std::vector<char> infoLog(length);
            glGetProgramInfoLog(program, length, nullptr, infoLog.data());

            std::print("SPIR-V Shader Programm Linking Fehler:\n{}\n", infoLog.data());

            glDeleteProgram(program);
            return 0;
        }

        // Cleanup: Nach erfolgreichem Linking die einzelnen Shader-Objekte detachen und löschen
        glDetachShader(program, vertexShader);
        glDetachShader(program, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        std::print("SPIR-V Shader-Programm erfolgreich erstellt.\n");
        return program;
    }

} // namespace bgl