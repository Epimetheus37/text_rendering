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



// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
float vw;
float vh;

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // Size of glyph
    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};

extern std::map<GLchar, Character> Characters;
extern unsigned int Text_VAO, Text_VBO;
extern unsigned int Quad_VBO, Quad_VAO;

extern unsigned int texture_text;

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, 1);
			break;
		case GLFW_KEY_SPACE:
			break;
		case GLFW_KEY_R:
			break;
		case GLFW_KEY_G:

			break;
		case GLFW_KEY_C:
			break;
		case GLFW_KEY_EQUAL:
		case GLFW_KEY_KP_ADD:
			break;
		case GLFW_KEY_B:

			break;
		case GLFW_KEY_KP_MULTIPLY:
		case GLFW_KEY_8:
		case GLFW_KEY_M:

			break;
		case GLFW_KEY_1:
			break;
		case GLFW_KEY_2:
			break;
		case GLFW_KEY_A:
			break;
		case GLFW_KEY_O:
			break;
		case GLFW_KEY_TAB:
			break;
		}
	}

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// render line of text
// -------------------
int init_render_text(Shader& shader, int width, int height);
void run_render_text(Shader& shader, std::string text, float x, float y, float scale, glm::vec3 color);

void init_quad_object();
 int init_quad_texture_object(GLsizei width, GLsizei height);
 int run_quad_texture_object(Shader& shader, GLsizei width, GLsizei height, int data_size, int index, GLubyte* pData);



int main()
{
	GLubyte* data;
	int w, h;

	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);


	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glfwGetFramebufferSize(window, &w, &h);
	vw = float(w - 3) / w;
	vh = float(h - 3) / h;

	data = new GLubyte[w * h * 4];

	if (data)
	{
		for (int i = 0; i < w * h; i++) {

			data[i * 4 + 0] = 0x0f;
			data[i * 4 + 1] = 0x80;
			data[i * 4 + 2] = 0x80;
			data[i * 4 + 3] = 0xff;
		}
	}
	else
	{
		std::cout << "Failed to initialize TEXTURE data" << std::endl;
		return -1;
	}

	/**************************************************************************************************************************/
	// build and compile our shader program
	// ------------------------------------
	Shader quad_shader("source/quad.vs", "source/quad.fs");

	init_quad_object();

	int data_size = init_quad_texture_object(w, h);
	/**************************************************************************************************************************/

	/**************************************************************************************************************************/
	// compile and setup the text_render_shader
	// ----------------------------
	Shader text_render_shader("source/text.vs", "source/text.fs");
	if (init_render_text(text_render_shader, w, h) == -1)
	{
		std::cout << "Failed to initialize texture shader" << std::endl;
		return -1;
	}
	/**************************************************************************************************************************/

	int count = 0;
	std::string print("FPS : ");
	
	glClearColor(0.2f, 0.5f, 0.7f, 1.0f);

	// render loop
	// -----------

	static int index = 0;
	int add = 0;
	while (!glfwWindowShouldClose(window))
	{
		std::string p = print + std::to_string(count);

		// render
		// ------
		
		glClear(GL_COLOR_BUFFER_BIT);
		data[add++] = 0xff;
		data[add++] = 0x00;
		data[add++] = 0x00;
		data[add++] = 0xff;
		if (add > (data_size - 1000))
		{
			glfwSetWindowShouldClose(window, 1);
		}
		index = run_quad_texture_object(quad_shader, w, h, data_size, index, data);

		run_render_text(text_render_shader, p.c_str(), 15.0f, 15.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
		
		glfwSwapBuffers(window);
		glfwPollEvents();
		count++;
	}


	delete[] data;
	glfwTerminate();
	return 0;
}