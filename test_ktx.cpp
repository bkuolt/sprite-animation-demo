#include <ktx.h>
#include <iostream>

int main(int argc, char** argv) {
    ktxTexture* tex;
    if (ktxTexture_CreateFromNamedFile(argv[1], KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &tex) == KTX_SUCCESS) {
        std::cout << "Type: " << (tex->isArray ? "Array" : "Not Array") << "\n";
        std::cout << "Dimensions: " << tex->numDimensions << "D\n";
        std::cout << "Layers: " << tex->numLayers << "\n";
        std::cout << "Depth: " << tex->baseDepth << "\n";
        std::cout << "Width: " << tex->baseWidth << "\n";
        std::cout << "Height: " << tex->baseHeight << "\n";
        std::cout << "Faces: " << tex->numFaces << "\n";
        ktxTexture_Destroy(tex);
    } else {
        std::cout << "Failed to load\n";
    }
}
