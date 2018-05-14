/*
MIT License

Copyright (c) 2018 Ben Brown

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "Image.hpp"

/*
#include <FreeImage.h>

Image::~Image() {
	FreeImage_Unload(pBitmap);
}

bool Image::load(const char* filename) {
	FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(filename);
	if (!FreeImage_FIFSupportsReading(fif))
		return false;

	pBitmap = FreeImage_Load(fif, filename);
	if (!pBitmap)
		return false;

	// Needs to be a 32 bit bitmap.
	if (FreeImage_GetBPP(pBitmap) != 32) {
		FIBITMAP* tempBitmap = pBitmap;
		pBitmap = FreeImage_ConvertTo32Bits(pBitmap);
		FreeImage_Unload(tempBitmap);
	}

	return true;
}

unsigned int Image::getWidth() {
	return FreeImage_GetWidth(pBitmap);
}

unsigned int Image::getHeight() {
	return FreeImage_GetHeight(pBitmap);
}

unsigned int Image::getPitch() {
	return FreeImage_GetPitch(pBitmap);
}

const unsigned char* Image::getBits() {
	return FreeImage_GetBits(pBitmap);
}

bool Image::transform(ImageTransform transform) {
	switch (transform) {
	case ImageTransform::Horizontal: return FreeImage_FlipHorizontal(pBitmap);
	case ImageTransform::Vertical: return FreeImage_FlipVertical(pBitmap);
	default: return false;
	}
}
*/