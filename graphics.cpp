#include "glad/gl.h"
#include "KHR/khrplatform.h"
#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>
#include <stdexcept>
#include <fmt/ranges.h> // fmt::join

#include <glm/vec2.hpp>
#include "ktx.hpp"
#include <print>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>



extern int currentAnimation ;


namespace
{

    std::string loadShaderFromFile(const std::string &filepath)
    {
        std::ifstream file(filepath);
        if (!file.is_open())
        {
            std::cerr << "Fehler: Konnte Shader-Datei nicht öffnen: " << filepath << "\n";
            return "";
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    void DebugCallback(GLenum source,
                       GLenum type,
                       GLuint id,
                       GLenum severity,
                       GLsizei length,
                       const GLchar *message,
                       const void *userParam)
    {
        // TODO
    }

    std::vector<std::string> getTextureCompressionExtensions()
    {
        GLint num_extensions = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
        std::vector<std::string> extensions;
        extensions.reserve(num_extensions);

        for (GLint i = 0; i < num_extensions; ++i)
        {
            const std::string ext_str((char *)glGetStringi(GL_EXTENSIONS, i));
            if (ext_str.find("GL_EXT_texture_compression_") != std::string_view::npos)
            {
                extensions.push_back(ext_str);
            }
        }

        return extensions;
    }

} // namespace

namespace bgl
{

    void InitializeGLAD()
    {
        const int gladVersion = gladLoadGL(glfwGetProcAddress);
        if (gladVersion == 0)
        {
            throw std::runtime_error("Failed to initialize GLAD");
        }

        spdlog::info("GLAD Version: {}.{}", (int)GLAD_VERSION_MAJOR(gladVersion), (int)GLAD_VERSION_MINOR(gladVersion));
    }

    void IntitializeOpenGL()
    {
        // print OpenGL info
        const char *version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
        spdlog::info("OpenGL Version: {}", version);
        spdlog::info("Vendor: {}", (const char *)glGetString(GL_VENDOR));
        spdlog::info("Renderer: {}", (const char *)glGetString(GL_RENDERER));
        spdlog::info("GLSL Version: {}", (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION));

        // setup debug callbacks
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(DebugCallback, nullptr);

        // handle extensions
        auto extensions = getTextureCompressionExtensions();
        spdlog::info("Extensions: {}", fmt::join(extensions, ", "));

        if (!(GL_ARB_texture_compression ||
              GL_EXT_texture_compression_s3tc ||
              GL_ARB_texture_compression_rgtc))
        {
            throw std::runtime_error("GL_ARB_texture_compression not supported!");
        }

        if (!GL_ARB_gl_spirv)
        {
            throw std::runtime_error("GL_ARB_gl_spirv not supported!");
        }
    }

    struct QuadMesh
    {
        GLuint VAO;
        GLuint VBO;
        GLuint IBO;
        unsigned int indexCount;
    };

    QuadMesh create2DQuad()
    {
        QuadMesh quad;
        quad.indexCount = 6;

        // 2D Positionen (x, y) - zentriert um den Ursprung
        std::vector<glm::vec2> vertices = {
            glm::vec2(-0.75f, -0.75f), // Unten-Links  (Index 0)
            glm::vec2(0.75f, -0.75f),  // Unten-Rechts (Index 1)
            glm::vec2(0.75f, 0.75f),   // Oben-Rechts  (Index 2)
            glm::vec2(-0.75f, 0.75f)   // Oben-Links   (Index 3)
        };

        // Indices für zwei Dreiecke (Gegen den Uhrzeigersinn)
        std::vector<GLuint> indices = {
            0, 1, 2, // Erstes Dreieck
            2, 3, 0  // Zweites Dreieck
        };

        // 1. Objekte erstellen (DSA nutzt glCreate* anstelle von glGen*)
        glCreateVertexArrays(1, &quad.VAO);
        glCreateBuffers(1, &quad.VBO);
        glCreateBuffers(1, &quad.IBO);

        // 2. Buffer mit Daten füllen (Immutable Storage)
        glNamedBufferStorage(quad.VBO, vertices.size() * sizeof(glm::vec2), vertices.data(), 0);
        glNamedBufferStorage(quad.IBO, indices.size() * sizeof(GLuint), indices.data(), 0);

        // 3. VAO konfigurieren (Alles direkt über die ID, ohne glBindVertexArray)

        // VBO an das VAO hängen (Binding-Index = 0, Offset = 0, Stride = sizeof(vec2))
        glVertexArrayVertexBuffer(quad.VAO, 0, quad.VBO, 0, sizeof(glm::vec2));

        // IBO an das VAO hängen
        glVertexArrayElementBuffer(quad.VAO, quad.IBO);

        // Vertex-Attribut 0 (Position) aktivieren
        glEnableVertexArrayAttrib(quad.VAO, 0);

        // Format für Attribut 0 definieren (2 Floats, nicht normalisiert, relativer Offset im Vertex = 0)
        glVertexArrayAttribFormat(quad.VAO, 0, 2, GL_FLOAT, GL_FALSE, 0);

        // Attribut 0 mit dem VBO-Binding-Index 0 verknüpfen
        glVertexArrayAttribBinding(quad.VAO, 0, 0);

        return quad;
    }

    // Hilfsfunktion zum Kompilieren eines einzelnen Shaders
    GLuint compileShader(GLenum type, std::string_view source)
    {
        GLuint shader = glCreateShader(type);
        const char *src = source.data();

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

            std::cerr << "Shader Compilation Error ("
                      << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment") << "):\n"
                      << infoLog.data() << "\n";

            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    // Hauptfunktion zum Erstellen des Shader-Programms
    GLuint createShaderProgram(std::string_view vertexSrc, std::string_view fragmentSrc)
    {
        GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSrc);
        GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSrc);

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

            std::print("Log: {}", std::string(infoLog.data()));

            glDeleteProgram(program);
            return 0;
        }

        // Cleanup: Nach dem erfolgreichen Linken brauchen wir die einzelnen Shader-Objekte nicht mehr
        glDetachShader(program, vertexShader);
        glDetachShader(program, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return program;
    }

    GLuint _textureIDs[6];

    GLuint _program;

    void loadAssets()
    {
        bgl::ktx::Loader loader1("/home/bastian/code/sprite-animation-demo/assets/idle.ktx2", KTX_TTF_BC3_RGBA);
        bgl::ktx::Loader loader2("/home/bastian/code/sprite-animation-demo/assets/walk.ktx2", KTX_TTF_BC3_RGBA);
        bgl::ktx::Loader loader3("/home/bastian/code/sprite-animation-demo/assets/jump.ktx2", KTX_TTF_BC3_RGBA);
        bgl::ktx::Loader loader4("/home/bastian/code/sprite-animation-demo/assets/run.ktx2", KTX_TTF_BC3_RGBA);
        bgl::ktx::Loader loader5("/home/bastian/code/sprite-animation-demo/assets/slide.ktx2", KTX_TTF_BC3_RGBA);
        bgl::ktx::Loader loader6("/home/bastian/code/sprite-animation-demo/assets/dead.ktx2", KTX_TTF_BC3_RGBA);

        _textureIDs[0] = loader1.upload();
        _textureIDs[1] = loader2.upload();
        _textureIDs[2] = loader3.upload();
        _textureIDs[3] = loader4.upload();
        _textureIDs[4] = loader5.upload();
        _textureIDs[5] = loader6.upload();

        auto vsSrc = loadShaderFromFile("/home/bastian/code/sprite-animation-demo/main.vs");
        auto fsSrc = loadShaderFromFile("/home/bastian/code/sprite-animation-demo/main.fs");

        _program = createShaderProgram(vsSrc, fsSrc);
        create2DQuad();
    }

    void renderQuad(const QuadMesh &quad, GLuint textureID, GLuint shaderProgram, int currentFrameIndex, float tweenFactor)
    {

        // 1. Uniforms updaten (DSA-Style, geht auch wenn der Shader gerade nicht gebunden ist)
        // In einer echten Engine holst du dir die Locations am besten nur einmal beim Laden des Shaders.
        GLint frameLoc = glGetUniformLocation(shaderProgram, "u_FrameIndex");
        GLint tweenLoc = glGetUniformLocation(shaderProgram, "u_TweenFactor");

        glProgramUniform1i(shaderProgram, frameLoc, currentFrameIndex);
        glProgramUniform1f(shaderProgram, tweenLoc, tweenFactor);

        // 2. Shader aktivieren
        glUseProgram(shaderProgram);

        // 3. Textur binden (falls du ein Texture Array nutzt, Unit 0 als Beispiel)
        glBindTextureUnit(0, textureID); // DSA für Texturen (ab GL 4.5)

        // 4. VAO binden und Draw Call abfeuern
        // Für das eigentliche Zeichnen müssen wir das VAO binden, da führt kein Weg dran vorbei.
        glBindVertexArray(quad.VAO);

        // Zeichne die 6 Indices aus dem IBO
        glDrawElements(GL_TRIANGLES, quad.indexCount, GL_UNSIGNED_INT, nullptr);

        // 5. State aufräumen (optional, aber Best Practice um Bugs zu vermeiden)
        glBindVertexArray(0);
        glUseProgram(0);
    }

    void Draw(double time)
    {
        static bool init = false;
        static QuadMesh mesh;
        if (!init)
        {
            loadAssets();
            mesh = create2DQuad();

            init = true;
        }

        int currentTexture = currentAnimation %6;  // currentAnimation ist ne global var

        // 1. Blending global aktivieren
        glEnable(GL_BLEND);

        // 2. Die Standard-Mischfunktion für Alpha-Transparenz setzen
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        auto c = (((int)time) % 10) / 10.0f;

        // set unifroms
        glProgramUniform1f(0, 0, 0);

        glClearColor(c * 2, c, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        GLint layers = 0;
        // Die alte, nicht-DSA Variante:
        glBindTexture(GL_TEXTURE_2D_ARRAY, _textureIDs[currentTexture]);
        glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_DEPTH, &layers);

        const auto num_frames = layers;   // Wie viele Layer dein Texture-Array hat
        const auto target_fps = 24.0; // Deine gewünschte Abspielgeschwindigkeit

        // 1. Wie viele "Animations-Frames" sind seit Start (c = Zeit in Sekunden) vergangen?
        double totalFrames = time * target_fps;

        // 2. Vorkommateil: Welcher Frame ist gerade aktiv? (Modulo für den Loop)
        int currentFrame = static_cast<int>(std::floor(totalFrames)) % num_frames;

        // 3. Nachkommateil: Der Tween-Faktor für die Interpolation im Shader
        float tweenFactor = static_cast<float>(totalFrames - std::floor(totalFrames));

        // 4. Ab an den Shader!
        renderQuad(mesh, _textureIDs[currentTexture], _program, currentFrame, tweenFactor);
    }

    //--------------------------------------------

} // namespace bgl