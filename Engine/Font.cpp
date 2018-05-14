#include "Font.hpp"

bool Font::load(const char * filename) {
    FT_Library library;

    FT_Error initError = FT_Init_FreeType(&library);
    if (initError)
        return false;

    FT_Face face;
    FT_Error faceError = FT_New_Face(library, filename, 0, &face);
    if (faceError)
        return false;

    FT_Done_Face(face);
    FT_Done_FreeType(library);
    return true;
}
