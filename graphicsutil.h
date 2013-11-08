#ifndef GRAPHICSUTIL_H_
#define GRAPHICSUTIL_H_

extern char *load_png(char *name, int *width, int *height);


double spectrum(double w, double &R, double &G, double &B) {
  if (w>1)w=1;
  if (w<0)w=0;
  
  w=w*(645-380)+380;
  
  if (w >= 380 && w < 440){
      R = -(w - 440.) /(440. - 350.);
      G = 0.0;
      B = 1.0;
  }
  else if (w >= 440 && w < 490){
      R = 0.0;
      G = (w - 440.) / (490. - 440.);
      B = 1.0;
  }
  else if (w >= 490 && w < 510){
      R = 0.0;
      G = 1.0;
      B = (510-w) / (510. - 490.);
  }
  else if (w >= 510 && w < 580){
      R = (w - 510.) / (580. - 510.);
      G = 1.0;
      B = 0.0;
  }
  else if (w >= 580 && w < 645){
      R = 1.0;
      G = -(w - 645.) / (645. - 580.);
      B = 0.0;
  }
  else if (w >= 645 && w <= 780){
      R = 1.0;
      G = 0.0;
      B = 0.0;
  }
  else{
      R = 0.0;
      G = 0.0;
      B = 0.0;
  }
}


static GLuint png_texture(char *filename)
{
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  
  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  
  int w, h;
  GLubyte *pixels = (GLubyte *)load_png(filename, &w, &h);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  free(pixels);
  
  return tex;
}


typedef struct
{
    unsigned char imageTypeCode;
    short int imageWidth;
    short int imageHeight;
    unsigned char bitCount;
    unsigned char *imageData;
} TGAFILE;

bool LoadTGAFile(TGAFILE *tgaFile)
{

    FILE *filePtr;
    unsigned char ucharBad;
    short int sintBad;
    long imageSize;
    int colorMode;
    unsigned char colorSwap;

    // Open the TGA file.
    filePtr = fopen("line.tga", "rb");
    if (filePtr == NULL)
    {
      std::cout << "Breaks " <<std::endl;
      return false;
    }
std::cout << "Still Good " <<std::endl;
      
    // Read the two first bytes we don't need.
    fread(&ucharBad, sizeof(unsigned char), 1, filePtr);
    fread(&ucharBad, sizeof(unsigned char), 1, filePtr);

    // Which type of image gets stored in imageTypeCode.
    fread(&tgaFile->imageTypeCode, sizeof(unsigned char), 1, filePtr);

    // For our purposes, the type code should be 2 (uncompressed RGB image)
    // or 3 (uncompressed black-and-white images).
    if (tgaFile->imageTypeCode != 2 && tgaFile->imageTypeCode != 3)
    {
        printf("%d\n",tgaFile->imageTypeCode );
      
        fclose(filePtr);
        return false;
    }

    // Read 13 bytes of data we don't need.
    fread(&sintBad, sizeof(short int), 1, filePtr);
    fread(&sintBad, sizeof(short int), 1, filePtr);
    fread(&ucharBad, sizeof(unsigned char), 1, filePtr);
    fread(&sintBad, sizeof(short int), 1, filePtr);
    fread(&sintBad, sizeof(short int), 1, filePtr);

    // Read the image's width and height.
    fread(&tgaFile->imageWidth, sizeof(short int), 1, filePtr);
    fread(&tgaFile->imageHeight, sizeof(short int), 1, filePtr);
    std::cout << &tgaFile->imageWidth << " " <<std::endl;
    
    // Read the bit depth.
    fread(&tgaFile->bitCount, sizeof(unsigned char), 1, filePtr);

    // Read one byte of data we don't need.
    fread(&ucharBad, sizeof(unsigned char), 1, filePtr);

    // Color mode -> 3 = BGR, 4 = BGRA.
    colorMode = tgaFile->bitCount / 8;
    imageSize = tgaFile->imageWidth * tgaFile->imageHeight * colorMode;

    // Allocate memory for the image data.
    tgaFile->imageData = (unsigned char*)malloc(sizeof(unsigned char)*imageSize);

    // Read the image data.
    fread(tgaFile->imageData, sizeof(unsigned char), imageSize, filePtr);

    // Change from BGR to RGB so OpenGL can read the image data.
    for (int imageIdx = 0; imageIdx < imageSize; imageIdx += colorMode)
    {
        colorSwap = tgaFile->imageData[imageIdx];
        tgaFile->imageData[imageIdx] = tgaFile->imageData[imageIdx + 2];
        tgaFile->imageData[imageIdx + 2] = colorSwap;
    }

    fclose(filePtr);
    return true;
}


#endif