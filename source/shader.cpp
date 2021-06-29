#include <iostream>
#include <map>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
	unsigned int TextureID; // ID handle of the glyph texture
	glm::ivec2   Size;      // Size of glyph
	glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
	unsigned int Advance;   // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;
unsigned int Text_VAO, Text_VBO;


unsigned int texture_text;

extern float vw;
extern float vh;

// render line of text
// -------------------

int init_render_text(Shader& shader, int width, int height)
{
	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));
	shader.use();
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// FreeType
	// --------
	FT_Library ft;
	// All functions return a value different than 0 whenever an error occurred
	if (FT_Init_FreeType(&ft))
	{
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
		return -1;
	}

	// find path to font
	std::string font_name = FileSystem::getPath("fonts/HJB.ttf");
	if (font_name.empty())
	{
		std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
		return -1;
	}

	// load font as face
	FT_Face face;
	if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
		return -1;
	}
	else {
		// set size to load glyphs as
		FT_Set_Pixel_Sizes(face, 0, 48);

		// disable byte-alignment restriction
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		// load first 128 characters of ASCII set
		for (unsigned char c = 0; c < 128; c++)
		{
			// Load character glyph 
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
				continue;
			}
			// generate texture

			glGenTextures(1, &texture_text);
			glBindTexture(GL_TEXTURE_2D, texture_text);
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RED,
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer
			);
			// set texture options
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			// now store character for later use
			Character character = {
				texture_text,
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				static_cast<unsigned int>(face->glyph->advance.x)
			};
			Characters.insert(std::pair<char, Character>(c, character));
		}
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	// destroy FreeType once we're finished
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	// configure Text_VAO/Text_VBO for texture quads
	// -----------------------------------
	glGenVertexArrays(1, &Text_VAO);
	glGenBuffers(1, &Text_VBO);
	glBindVertexArray(Text_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, Text_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return 1;
}


void run_render_text(Shader& shader, std::string text, float x, float y, float scale, glm::vec3 color)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_COLOR, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);
	
		// activate corresponding render state	
	shader.use();
	glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
	//    glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(Text_VAO);

	// iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = Characters[*c];

		float xpos = x + ch.Bearing.x * scale;
		float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;
		// update Text_VBO for each character
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};
		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// update content of Text_VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, Text_VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale /*(ch.Size.x+1)*/;
		// bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}


GLubyte* ptr_texture_buffer;
GLuint pboIds[3];
GLuint textureObjID;
unsigned int Quad_VBO, Quad_VAO;

void init_quad_object() {

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
		// positions          // colors           // texture coords
		 vw,  vh, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right        2
		 vw, -vh, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right     1
		-vw, -vh, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left      0

		-vw,  vh, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f,  // top left         3
		 vw,  vh, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right        2
		-vw, -vh, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left      0
	};


	glGenVertexArrays(1, &Quad_VAO);
	glGenBuffers(1, &Quad_VBO);

	glBindVertexArray(Quad_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, Quad_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}


int init_quad_texture_object(GLsizei width, GLsizei height) {

	//	init_quad_object();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glGenTextures(1, &textureObjID);
	glBindTexture(GL_TEXTURE_2D, textureObjID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	int data_size = width * height * 4;
	glGenBuffers(3, pboIds);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboIds[0]);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, data_size, 0, GL_STREAM_DRAW);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboIds[1]);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, data_size, 0, GL_STREAM_DRAW);
	//	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboIds[2]);
	//	glBufferData(GL_PIXEL_UNPACK_BUFFER, data_size, 0, GL_STREAM_DRAW);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	return data_size;
}

int run_quad_texture_object(Shader& shader, GLsizei width, GLsizei height, int data_size, int index, GLubyte* pData) {

	glDisable(GL_BLEND);
	int next_index = (index + 1) % 2;

	// bind the texture and PBO
	glBindTexture(GL_TEXTURE_2D, textureObjID);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboIds[index]);

	// copy pixels from PBO to texture object
	// Use offset instead of pointer.
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	// bind PBO to update pixel values
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboIds[next_index]);

	glBufferData(GL_PIXEL_UNPACK_BUFFER, data_size, 0, GL_STREAM_DRAW);
	ptr_texture_buffer = (GLubyte*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);

	if (ptr_texture_buffer)
	{
		memcpy_s(ptr_texture_buffer, data_size, pData, data_size);
		glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);  // release pointer to mapping buffer

		// render container
		shader.use();
		glBindVertexArray(Quad_VAO);
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// it is good idea to release PBOs with ID 0 after use.
	// Once bound with 0, all pixel operations behave normal ways.
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	return next_index;
}