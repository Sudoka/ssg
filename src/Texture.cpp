//
// Texture.cpp
//  modified from https://github.com/mortennobel/OpenGL_3_2_Utils
//  William.Thibault@csueastbay.edu
//
/*!
 * OpenGL 3.2 Utils - Extension to the Angel library (from the book Interactive Computer Graphics 6th ed
 * https://github.com/mortennobel/OpenGL_3_2_Utils
 *
 * New BSD License
 *
 * Copyright (c) 2011, Morten Nobel-Joergensen
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
 * following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this list of conditions and the following
 * disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/glew.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>



#include "Texture.h"
#include "TextureLoader.h"
#include "InitShader.h"

#include <iostream>

using namespace std;
using namespace glm;

GLuint Texture::drawTextureShader = 0;
GLuint Texture::drawTextureVertexArrayObject = 0;
GLuint Texture::drawTextureUniform = 0;

Texture::Texture(GLuint width, GLuint height, bool floatingPoint, bool mipmaps, unsigned int t )
  : width(width), height(height), floatingPoint(floatingPoint)
{
  glActiveTexture(GL_TEXTURE0 + t);
  setupTexParams( floatingPoint, mipmaps );
    
  if (drawTextureShader == 0){
    setupRenderFullscreenQuad();
  }

  unbind(t);
}


Texture::Texture(const char *bmpfilename, bool floatingPoint, bool mipmaps, unsigned int texUnit )
  : floatingPoint(floatingPoint)
{

  if ( floatingPoint ) {
    std::cout << "texture loading not supported for floating point\n" ;
    return;
  }

  // load from file ( convert to RGBA )
  //  unsigned char *rgbData = loadBMPRaw ( bmpfilename, width, height, false );
  unsigned char *rgbData = loadImage ( bmpfilename, width, height );
  if ( !rgbData ) {
    std::cout << "texture load failed for " << bmpfilename << ", using dummy texture" << std::endl;
    rgbData = (unsigned char *) malloc ( 3 );
    rgbData[0] = rgbData[1] = rgbData[2] = 128;
    width = 1;
    height = 1;
  }
  unsigned char *rgbaData = new unsigned char [width*height*4];
  unsigned char *p = rgbaData;
  unsigned char *q = rgbData;
  for ( int i = 0; i < width*height; i++ ) {
    *p++ = *q++;
    *p++ = *q++;
    *p++ = *q++;
    *p++ = 255;
  }

  glActiveTexture ( GL_TEXTURE0 + texUnit );
  // setup the texture
  setupTexParams ( floatingPoint, mipmaps );

  // upload texture data
  glBindTexture ( GL_TEXTURE_2D, textureId );
  glTexSubImage2D ( GL_TEXTURE_2D, 
		    0, 
		    0,0,
		    width, height,
		    //		    GL_BGRA,
		    GL_RGBA,
		    GL_UNSIGNED_BYTE,
		    static_cast<GLvoid*>(rgbaData) );
  glBindTexture ( GL_TEXTURE_2D, 0 );

  if (drawTextureShader == 0){
    setupRenderFullscreenQuad();
  }

  unbind(texUnit);
  delete rgbaData;
  delete rgbData;
}


void
Texture::setupTexParams( bool floatingpoint, bool mipmaps )
{
  glGenTextures(1, &textureId);
  glBindTexture(GL_TEXTURE_2D, textureId);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
		  GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
		  mipmaps?GL_LINEAR_MIPMAP_LINEAR:GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // allocate the memory for the texels
  glTexImage2D(GL_TEXTURE_2D, 0, 
	       floatingPoint?GL_RGBA32F_ARB:GL_RGBA8, 
	       width, height, 
	       0,
	       GL_RGBA, 
	       floatingPoint?GL_FLOAT:GL_UNSIGNED_BYTE, 
	       0);
  if ( mipmaps ) 
    generateMipmaps();
}

Texture::~Texture(){
  glDeleteTextures(1, &textureId);
}

void Texture::renderFullscreenQuad(){
  glUseProgram(drawTextureShader);
  glBindVertexArray(drawTextureVertexArrayObject);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureId);
  glUniform1i(drawTextureUniform, 0);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glUseProgram(0);
}

void Texture::setupRenderFullscreenQuad(){
  //#ifdef __APPLE__
  drawTextureShader = InitShader("shaders120/fullscreentexture.vert",  
				 "shaders120/fullscreentexture.frag");
  //#else
  //  drawTextureShader = InitShader("shaders/fullscreentexture.vert",  
  //				 "shaders/fullscreentexture.frag");
  //#endif
  GLuint positionAttributeLocation = glGetAttribLocation(drawTextureShader, 
							 "position");
  drawTextureUniform = glGetUniformLocation(drawTextureShader, 
					    "texture1");
    
  glGenVertexArrays(1, &drawTextureVertexArrayObject);
  glBindVertexArray(drawTextureVertexArrayObject);
    
  vec2 array[3] = { // triangle fill screen in clip space
    vec2(-1,-1),
    vec2(3,-1),
    vec2(-1,3)
  };
    
  GLuint vertexBuffer;
  glGenBuffers(1, &vertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(vec2), &(array[0]), GL_STATIC_DRAW);
	
  glEnableVertexAttribArray(positionAttributeLocation);
  glVertexAttribPointer(positionAttributeLocation, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (const GLvoid *)(0));
}

void Texture::generateMipmaps(){
  glBindTexture(GL_TEXTURE_2D, textureId);
  glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture::bind(unsigned int t)
{
  glActiveTexture ( GL_TEXTURE0 + t );
  glBindTexture ( GL_TEXTURE_2D, textureId );
}

void Texture::unbind(unsigned int t)
{
  glActiveTexture ( GL_TEXTURE0 + t );
  glBindTexture ( GL_TEXTURE_2D, 0 );
}

void Texture::loadChecks(unsigned int t)
{
  glActiveTexture(GL_TEXTURE0 + t);
  if ( floatingPoint ) {
    std::cout << "checks not supported for floating point\n" ;
    return;
  }

  unsigned char *data = new unsigned char [width * height * 4];
  unsigned char *p = data;
  // fill with checks
  for ( int row=0;row<height;row++ ) {
    for ( int col=0; col<width; col++ ) {
      unsigned char c = ((((row&0x80)==0)^((col&0x80))==0))*255;
      *p++ = c;
      *p++ = c;
      *p++ = c;
      *p++ = 255;
    }
  }
  // upload texture data
  glBindTexture ( GL_TEXTURE_2D, textureId );

  glTexSubImage2D ( GL_TEXTURE_2D, 
		    0, 
		    0,0,
		    width, height,
		    GL_RGBA,
		    GL_UNSIGNED_BYTE,
		    static_cast<GLvoid*>(data) );

  glBindTexture ( GL_TEXTURE_2D, 0 );
  delete data;
}


