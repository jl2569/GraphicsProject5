#define GLFW_DLL 1

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <linmath.h>
#include <ctype.h>


GLFWwindow* window;


typedef struct {
  float position[3];
  float TexCoord[2];
} Vertex;

// header struct for the header of the image 
typedef struct header{
	char* magicnumber;
	char* width;
	char* height;
	char* colorval;
}header;

header *head;
// pic struct that hold each pixels color value.
typedef struct pic {
  unsigned char r, g, b;
} pic;

pic *render;

Vertex Vertices[] = {
  {{1, -1, 0},  {.9999, .9999}},
  {{1, 1, 0},   {.9999, 0}},
  {{-1, 1, 0},  {0, 0}},
  {{-1, -1, 0}, {0, .9999}}
};


const GLubyte Indices[] = {
  0, 1, 2,
  2, 3, 0
};


char* vertex_shader_src =
  "attribute vec4 Position;\n"
  "\n"
  "attribute vec2 TexCoordIn;\n"
  "varying lowp vec2 TexCoordOut;\n"
  "\n"
  "void main(void) {\n"
  "    TexCoordOut = TexCoordIn;\n"
  "    gl_Position = Position;\n"
  "}\n";


char* fragment_shader_src =
  "varying lowp vec2 TexCoordOut;\n"
  "uniform sampler2D Texture;\n"
  "\n"
  "void main(void) {\n"
  "    gl_FragColor = texture2D(Texture, TexCoordOut);\n"
  "}\n";


GLint simple_shader(GLint shader_type, char* shader_src) {

  GLint compile_success = 0;

  int shader_id = glCreateShader(shader_type);

  glShaderSource(shader_id, 1, &shader_src, 0);

  glCompileShader(shader_id);

  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_success);

  if (compile_success == GL_FALSE) {
    GLchar message[256];
    glGetShaderInfoLog(shader_id, sizeof(message), 0, &message[0]);
    printf("glCompileShader Error: %s\n", message);
    exit(1);
  }

  return shader_id;
}


int simple_program() {

  GLint link_success = 0;

  GLint program_id = glCreateProgram();
  GLint vertex_shader = simple_shader(GL_VERTEX_SHADER, vertex_shader_src);
  GLint fragment_shader = simple_shader(GL_FRAGMENT_SHADER, fragment_shader_src);
  
  glAttachShader(program_id, vertex_shader);
  glAttachShader(program_id, fragment_shader);

  glLinkProgram(program_id);

  glGetProgramiv(program_id, GL_LINK_STATUS, &link_success);

  if (link_success == GL_FALSE) {
    GLchar message[256];
    glGetProgramInfoLog(program_id, sizeof(message), 0, &message[0]);
    printf("glLinkProgram Error: %s\n", message);
    exit(1);
  }

  return program_id;
}


static void error_callback(int error, const char* description) {
  fputs(description, stderr);
}
// skips the whitespace in an file so easier to parse
void skip_ws(FILE* input_file) {
  int c = fgetc(input_file);
  while (isspace(c)) {
    c = fgetc(input_file);
  }
  ungetc(c, input_file);
}
// Function that reads the files header and add the right values to the struct
void read_header(FILE* filee){
	char temp[64];
	int a = 0;
	skip_ws(filee);
	// records the magic number then copies it into struct
	temp[a] = fgetc(filee);
	a +=1;
	temp[a] = fgetc(filee); 	
	a = 0; 
	strcpy(head->magicnumber, temp); 
	// records the width then copies it into struct but takes into account if it runs into a comment
	skip_ws(filee);
	char c = fgetc(filee);
	while(!isspace(c)){
		if(c == '#'){
			skip_ws(filee);
			c = fgetc(filee); 
		}
		temp[a] = c; 
		a +=1;
        c = fgetc(filee);
	}	
	a = 0; 
	strcpy(head->width, temp); 
	// records the height then copies it into struct
	skip_ws(filee);
	c = fgetc(filee);
	while(!isspace(c)){
		temp[a] = c; 
		a +=1;
		c = fgetc(filee);	
	}	
	a =0; 
	strcpy(head->height, temp);
	// records the color value then copies it into struct
	skip_ws(filee);
	c = fgetc(filee);
	while(!isspace(c)){
		temp[a] = c; 
		a +=1;
		c = fgetc(filee);	
	}
	strcpy(head->colorval, temp); 	
	skip_ws(filee);
}
//Reads the rest of the file recording the color of each single pixel and records it into the struct
void read_file(FILE* filee  ){
	int place;
    int size = atoi(head->width) * atoi(head->height);
    int r;
    int g;
    int b;
	// Follows the if statement solely dependent on what the magic number is if it is no p3 or p6 then error
	if ((strcmp(head->magicnumber, "P3") ==0) || (strcmp(head->magicnumber, "p3") ==0) ){
		for(int i = 0; i < size; i++){
			place = fgetc(filee);
			while(place  == ' ' || place  == '\n'){
				place = fgetc(filee);
			}
			ungetc(place, filee);
			fscanf(filee, "%d %d %d", &r, &g, &b);
			if (r > atoi(head->colorval) || g > atoi(head->colorval) || b > atoi(head->colorval) || r < 0 || g < 0 || b < 0){
				printf("Error: Number values are less than 0 or greater than colorvalue");
				exit(1);
			}else{
				render[i].r = r;
				render[i].g = g;
				render[i].b = b;
			}
		}
	}else if ((strcmp(head->magicnumber, "P6") ==0) || (strcmp(head->magicnumber, "p6") ==0) ){
		for(int i = 0; i < size; i++){
			fread(&render[i].r, 1, 1, filee);
			fread(&render[i].g, 1, 1, filee);
			fread(&render[i].b, 1, 1, filee);
			if (render[i].r > atoi(head->colorval) || render[i].g > atoi(head->colorval) || render[i].b > atoi(head->colorval) || render[i].r < 0 || render[i].g < 0 || render[i].b < 0){
				printf("Error: Number values are less than 0 or greater than colorvalue");
				exit(1);
			}
		}
	}else{
		printf("Error: On supports p3 and p6 image files\n");
		exit(1);
	}
}
// Function that translates the image 
void move(int movement){
	// Moves image up
	if (movement == 0 ){
		Vertices[0].position[1] += .15;
        Vertices[1].position[1] += .15;
        Vertices[2].position[1] += .15;
        Vertices[3].position[1] += .15;
	// Moves image down
	}else if (movement == 1 ){
		Vertices[0].position[1] -= .15;
        Vertices[1].position[1] -= .15;
        Vertices[2].position[1] -= .15;
        Vertices[3].position[1] -= .15;
	// Moves image left
	}else if (movement == 2 ){
		Vertices[0].position[0] -= .15;
        Vertices[1].position[0] -= .15;
        Vertices[2].position[0] -= .15;
        Vertices[3].position[0] -= .15;
	// Moves image right
	}else if (movement == 3 ){
		Vertices[0].position[0] += .15;
        Vertices[1].position[0] += .15;
        Vertices[2].position[0] += .15;
        Vertices[3].position[0] += .15;
	}

    
}
// Function that shears the image 
void shear(int movement){
	if (movement == 0){
		Vertices[0].position[0] += 0.1;
        Vertices[3].position[0] += 0.1;
		Vertices[2].position[0] -= 0.1;
        Vertices[1].position[0] -= 0.1;
	}else if (movement == 1){
		Vertices[0].position[0] -= 0.1;
        Vertices[3].position[0] -= 0.1;
		Vertices[2].position[0] += 0.1;
        Vertices[1].position[0] += 0.1;
	}
    
}
float turn = 0;
// Function that rotates the image
void rotate(int movement){
	// rotates 30 degrees
	float x;
	float y;
	// rotate clock-wise
	if (movement == 0 ){
		float mat[2][2] = {{sqrt(1)/2,-sqrt(3)/2},{sqrt(3)/2,sqrt(1)/2}};
		x = (Vertices[0].position[0]*mat[0][0]) + (Vertices[0].position[1]*mat[1][0]);
		y = (Vertices[0].position[0]*mat[0][1]) + (Vertices[0].position[1]*mat[1][1]);
		Vertices[0].position[0] = x;
		Vertices[0].position[1] = y;
		x = (Vertices[1].position[0]*mat[0][0]) + (Vertices[1].position[1]*mat[1][0]);
		y = (Vertices[1].position[0]*mat[0][1]) + (Vertices[1].position[1]*mat[1][1]);
		Vertices[1].position[0] = x;
		Vertices[1].position[1] = y;
		x = (Vertices[2].position[0]*mat[0][0]) + (Vertices[2].position[1]*mat[1][0]);
		y = (Vertices[2].position[0]*mat[0][1]) + (Vertices[2].position[1]*mat[1][1]);
		Vertices[2].position[0] = x;
		Vertices[2].position[1] = y;
		x = (Vertices[3].position[0]*mat[0][0]) + (Vertices[3].position[1]*mat[1][0]);
		y = (Vertices[3].position[0]*mat[0][1]) + (Vertices[3].position[1]*mat[1][1]);
		Vertices[3].position[0] = x;
		Vertices[3].position[1] = y;
	// roatate counter-clock-wise
	}else if (movement == 1){
		float tam[2][2] = {{sqrt(1)/2,sqrt(3)/2},{-sqrt(3)/2,sqrt(1)/2}};
		x = (Vertices[0].position[0]*tam[0][0]) + (Vertices[0].position[1]*tam[1][0]);
		y = (Vertices[0].position[0]*tam[0][1]) + (Vertices[0].position[1]*tam[1][1]);
		Vertices[0].position[0] = x;
		Vertices[0].position[1] = y;
		x = (Vertices[1].position[0]*tam[0][0]) + (Vertices[1].position[1]*tam[1][0]);
		y = (Vertices[1].position[0]*tam[0][1]) + (Vertices[1].position[1]*tam[1][1]);
		Vertices[1].position[0] = x;
		Vertices[1].position[1] = y;
		x = (Vertices[2].position[0]*tam[0][0]) + (Vertices[2].position[1]*tam[1][0]);
		y = (Vertices[2].position[0]*tam[0][1]) + (Vertices[2].position[1]*tam[1][1]);
		Vertices[2].position[0] = x;
		Vertices[2].position[1] = y;
		x = (Vertices[3].position[0]*tam[0][0]) + (Vertices[3].position[1]*tam[1][0]);
		y = (Vertices[3].position[0]*tam[0][1]) + (Vertices[3].position[1]*tam[1][1]);
		Vertices[3].position[0] = x;
		Vertices[3].position[1] = y;
	}
}
// Function that scales the image
void scale(int movement){
	// zooms the image in
	if (movement == 0){
		Vertices[0].position[0] *= 1.1;
        Vertices[0].position[1] *= 1.1;
        Vertices[1].position[0] *= 1.1;
        Vertices[1].position[1] *= 1.1; 
        Vertices[2].position[0] *= 1.1; 
        Vertices[2].position[1] *= 1.1;
        Vertices[3].position[0] *= 1.1;
        Vertices[3].position[1] *= 1.1;
	// zooms the image out
	}else if (movement == 1){
		Vertices[0].position[0] *= .9;
        Vertices[0].position[1] *= .9;
        Vertices[1].position[0] *= .9;
        Vertices[1].position[1] *= .9;  
        Vertices[2].position[0] *= .9; 
        Vertices[2].position[1] *= .9;
        Vertices[3].position[0] *= .9;
        Vertices[3].position[1] *= .9;
	}
    
}
// Function that sets up the keys for the transformations of the image
void keys(GLFWwindow* window, int key, int scancode, int action, int mods){
    //move
	if (key == GLFW_KEY_UP && action == GLFW_PRESS){
        move(0);
	}else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS){
        move(1);
	}else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS){
        move(2);
	}else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS){
        move(3);
	//rotate
	}else if (key == GLFW_KEY_W && action == GLFW_PRESS){
        rotate(0);
	}else if (key == GLFW_KEY_S && action == GLFW_PRESS){
        rotate(1);
	//scale
	}else if (key == GLFW_KEY_A && action == GLFW_PRESS){
        scale(0);
	}else if (key == GLFW_KEY_D && action == GLFW_PRESS){
        scale(1);
	//shear
	}else if (key == GLFW_KEY_Q && action == GLFW_PRESS){
        shear(0);
	}else if (key == GLFW_KEY_E && action == GLFW_PRESS){
        shear(1);
	//closes the window
	}else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
		glfwSetWindowShouldClose(window, 1);
	}
}

int main(int argc , char *argv[]) {
	if (argc != 2) {
		printf("Error: Incorrect amount of arguments as it takes in one.\n");
		return(1);
	}
	
	FILE *fh;
	fh = fopen(argv[1], "r");
	
	if(fh == NULL) {
		printf("Error: File cannot be opened.");
		return(1);
	}
	head = (struct header*)malloc(sizeof(struct header)); 
	head->magicnumber = (char *)malloc(100);
	head->width = (char *)malloc(100);
	head->height = (char *)malloc(100);
	head->colorval = (char *)malloc(100);

	read_header(fh);
    render = (pic *)malloc(sizeof(pic) * atoi(head->width) * atoi(head->height)  + 1);
	read_file(fh );
	fclose(fh);
	   
	GLint program_id, position_slot;
	GLuint vertex_buffer;
	GLuint index_buffer;

	glfwSetErrorCallback(error_callback);

	// Initialize GLFW library
	if (!glfwInit())
		return -1;

	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	// Create and open a window
	window = glfwCreateWindow(atoi(head->width),
                            atoi(head->height),
                            argv[1],
                            NULL,
                            NULL);
	if (!window) {
		glfwTerminate();
		printf("glfwCreateWindow Error\n");
		exit(1);
	}
 
	glfwMakeContextCurrent(window);
  
  
	GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, atoi(head->width), atoi(head->height), 0, GL_RGB, 
				 GL_UNSIGNED_BYTE, render);
	//

				 
 	program_id = simple_program();

	glUseProgram(program_id);

	// sets position_slot and asserts it was set correctly, followed by calling glEnableVertexAttribArray on it
	position_slot = glGetAttribLocation(program_id, "Position");
	assert(position_slot != -1);
	glEnableVertexAttribArray(position_slot);
	
	// sets texcoord_location and asserts it was set correctly
	GLint texcoord_location = glGetAttribLocation(program_id, "TexCoordIn");
	assert(texcoord_location != -1);
	
	// sets tex_location and asserts it was set correctly
	GLint tex_location = glGetUniformLocation(program_id, "Texture");
	assert(tex_location != -1);
	
	// calls glEnableVertexAtribArray on previously set texcoord_location, now that we know the tex_location was set properly
	glEnableVertexAttribArray(texcoord_location);

	// Create Buffer
	glGenBuffers(1, &vertex_buffer);

	// Map GL_ARRAY_BUFFER to this buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

	// Send the data
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

  // Repeat
	while (!glfwWindowShouldClose(window)) {
	  
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW); // updates image if any transformations were performed

		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		glViewport(0, 0, atoi(head->width), atoi(head->height));

		glVertexAttribPointer(position_slot,
							  3,
							  GL_FLOAT,
							  GL_FALSE,
							  sizeof(Vertex),
							  0);		  
		glVertexAttribPointer(texcoord_location,
							  2,
							  GL_FLOAT,
							  GL_FALSE,
							  sizeof(Vertex),
							  (void*) (sizeof(float) * 3)); // modify last parameter? * 7 due to offset
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texID);
		glUniform1i(tex_location, 0);

		
		glDrawElements(GL_TRIANGLES,
					   sizeof(Indices) / sizeof(GLubyte),
					   GL_UNSIGNED_BYTE, 0);

		glfwSwapBuffers(window);
		glfwSetKeyCallback(window, keys);
		glfwPollEvents();
  }

  //fclose(fh);
	
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
