//#include "C:/Wichtig/System/Static/Library/WindowEngine1.0.h"
#include "/home/codeleaded/System/Static/Library/WindowEngineGL1.0.h"
#include "/home/codeleaded/System/Static/Library/Files.h"
#include "/home/codeleaded/System/Static/Library/Random.h"
#include "/home/codeleaded/System/Static/Library/PerlinNoise.h"
#include "/home/codeleaded/System/Static/Library/RayCast.h"

#include "./Math3D.h"
#include "./Hitbox3D.h"

const int ATLAS_COLS = 16;
const int ATLAS_ROWS = 16;
const float TILE_WIDTH = 1.0f / (float)ATLAS_COLS;
const float TILE_HEIGHT = 1.0f / (float)ATLAS_ROWS;

mat4x4 model;
mat4x4 view;
mat4x4 proj;

vec3d camera = { 10.0f,10.0f,10.0f,1.0f };
vec3d vVelocity = { 0.0f,0.0f,0.0f,1.0f };
vec3d vLength = { 0.5f,1.8f,0.5f,1.0f };
float RotX = 0.0f;
float RotY = 0.0f;
float angle = 0.0f;



typedef unsigned char Block;

#define WORLD_DX            150
#define WORLD_DY            200
#define WORLD_DZ            150

#define BLOCK_ERROR	        255
#define BLOCK_VOID          0
#define BLOCK_DIRT          1
#define BLOCK_GRAS          2
#define BLOCK_SAND          3
#define BLOCK_WATER         4
#define BLOCK_LOG           5
#define BLOCK_LEAF          6

#define ATLAS_VOID          0
#define ATLAS_DIRT          (ATLAS_COLS * 15)
#define ATLAS_GRAS_S        (ATLAS_COLS * 15 + 1)
#define ATLAS_GRAS_T        (ATLAS_COLS * 15 + 2)
#define ATLAS_SAND          (ATLAS_COLS * 15 + 3)
#define ATLAS_WATER         (ATLAS_COLS * 15 + 4)
#define ATLAS_LOG_S         (ATLAS_COLS * 15 + 5)
#define ATLAS_LOG_T         (ATLAS_COLS * 15 + 6)
#define ATLAS_LEAF          (ATLAS_COLS * 15 + 7)

#define CUBE_SIDE_SOUTH      0
#define CUBE_SIDE_WEST       1
#define CUBE_SIDE_NORTH      2
#define CUBE_SIDE_EAST       3
#define CUBE_SIDE_TOP        4
#define CUBE_SIDE_BOTTOM     5


typedef struct CubeSide {
    vec3d pos;
    unsigned char id;
    unsigned char side;
} CubeSide;

Vector cubeSides;
Vector Cubes;
char OnGround = 0;
int Mode = 1;
int Menu = 0;

GLuint VBO, VAO;
GLuint shaderProgram;
GLuint texture1;

vec2d getUVsFromIndex(int tileIndex) {
    int col = tileIndex % ATLAS_COLS;
    int row = tileIndex / ATLAS_COLS;

    float u0 = (float)col * TILE_WIDTH;
    float v0 = (float)row * TILE_HEIGHT;

    return (vec2d){ u0,v0 };
}
void checkCompileErrors(GLuint shader, const char* type) {
    GLint success;
    GLchar infoLog[512];
    if (strcmp(type, "PROGRAM") != 0) {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            printf("ERROR: Shader Compilation [%s]\n%s\n", type, infoLog);
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 512, NULL, infoLog);
            printf("ERROR: Program Linking\n%s\n", infoLog);
        }
    }
}
unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    return shader;
}
unsigned int createShaderProgram() {
    char* vertexShaderSource = Files_Read("./src/vertexShader.glsl");
    if(!vertexShaderSource){
        printf("[vertexShader]: Error\n");
        window.Running = 0;
        return 0;
    }
    
    char* fragmentShaderSource = Files_Read("./src/fragmentShader.glsl");
    if(!fragmentShaderSource){
        printf("[fragmentShader]: Error\n");
        window.Running = 0;
        return 0;
    }
    
    unsigned int vertex = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
	checkCompileErrors(vertex, "VERTEX");

    unsigned int fragment = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
	checkCompileErrors(fragment, "FRAGMENT");

    unsigned int program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);

    glLinkProgram(program);
	checkCompileErrors(program, "PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);

	if(vertexShaderSource)      free(vertexShaderSource);
    if(fragmentShaderSource)    free(fragmentShaderSource);

    return program;
}
GLuint loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // Setze Texturparameter (wie Wrapping und Filter)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Lade Textur (mit einer Bibliothek wie stb_image oder SOIL)
    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        printf("Error: Texture\n");
    }
    stbi_image_free(data);
    return textureID;
}

Block* World;

Block World_GetX(Block* World,float x,float y,float z){
	if(x<0.0f || x>=WORLD_DX) return BLOCK_ERROR;
	if(y<0.0f || y>=WORLD_DY) return BLOCK_ERROR;
	if(z<0.0f || z>=WORLD_DZ) return BLOCK_ERROR;
	return World[(int)x + (int)y * WORLD_DX + (int)z * WORLD_DX * WORLD_DY];
}
void World_SetX(Block* World,float x,float y,float z,Block b){
	if(x<0.0f || x>=WORLD_DX) return;
	if(y<0.0f || y>=WORLD_DY) return;
	if(z<0.0f || z>=WORLD_DZ) return;
	World[(int)x + (int)y * WORLD_DX + (int)z * WORLD_DX * WORLD_DY] = b;
}
Block World_Get(Block* World,vec3d p){
	return World_GetX(World,p.x,p.y,p.z);
}
void World_Set(Block* World,vec3d p,Block b){
	World_SetX(World,p.x,p.y,p.z,b);
}
int World_Height(Block* World,float x,float z){
	int h = WORLD_DY - 1;
	for(;h>=0;h--){
		if(World_GetX(World,x,h,z)!=BLOCK_VOID){
			return h;
		}
	}
	return h;
}
vec3d Neighbour_Side(int s){
	switch (s){
	case CUBE_SIDE_NORTH: 	return (vec3d){ 0.0f, 0.0f,-1.0f,1.0f };
	case CUBE_SIDE_EAST: 	return (vec3d){ 1.0f, 0.0f, 0.0f,1.0f };
	case CUBE_SIDE_SOUTH: 	return (vec3d){ 0.0f, 0.0f, 1.0f,1.0f };
	case CUBE_SIDE_WEST: 	return (vec3d){-1.0f, 0.0f, 0.0f,1.0f };
	case CUBE_SIDE_TOP: 	return (vec3d){ 0.0f, 1.0f, 0.0f,1.0f };
	case CUBE_SIDE_BOTTOM: 	return (vec3d){ 0.0f,-1.0f, 0.0f,1.0f };
	}
	return (vec3d){ 0.0f,0.0f,0.0f,1.0f };
}
unsigned char Block_Id(Block b,int s){
	switch (b){
	case BLOCK_VOID: 	return ATLAS_VOID;
	case BLOCK_DIRT: 	return ATLAS_DIRT;
	case BLOCK_GRAS:	return s==CUBE_SIDE_TOP ? ATLAS_GRAS_T : (s==CUBE_SIDE_BOTTOM ? ATLAS_DIRT : ATLAS_GRAS_S);
	case BLOCK_SAND:	return ATLAS_SAND;
	case BLOCK_WATER:	return ATLAS_WATER;
	case BLOCK_LOG: 	return s==CUBE_SIDE_TOP || s==CUBE_SIDE_BOTTOM ? ATLAS_LOG_T : ATLAS_LOG_S;
	case BLOCK_LEAF: 	return ATLAS_LEAF;
	}
	return ATLAS_VOID;
}
void World_Tree(Block* World,int x,int y,int z){
	World_SetX(World,x,y,z,BLOCK_LOG);
	World_SetX(World,x,y+1,z,BLOCK_LOG);
	World_SetX(World,x,y+2,z,BLOCK_LOG);

	World_SetX(World,x,y+3,z,BLOCK_LEAF);

	World_SetX(World,x+1,y+3,z,BLOCK_LEAF);
	World_SetX(World,x+1,y+3,z+1,BLOCK_LEAF);
	World_SetX(World,x+1,y+3,z-1,BLOCK_LEAF);
	World_SetX(World,x-1,y+3,z,BLOCK_LEAF);
	World_SetX(World,x-1,y+3,z+1,BLOCK_LEAF);
	World_SetX(World,x-1,y+3,z-1,BLOCK_LEAF);
	World_SetX(World,x,y+3,z+1,BLOCK_LEAF);
	World_SetX(World,x,y+3,z-1,BLOCK_LEAF);

	World_SetX(World,x,y+4,z,BLOCK_LEAF);
	World_SetX(World,x+1,y+4,z,BLOCK_LEAF);
	World_SetX(World,x-1,y+4,z,BLOCK_LEAF);
	World_SetX(World,x,y+4,z+1,BLOCK_LEAF);
	World_SetX(World,x,y+4,z-1,BLOCK_LEAF);
}
void World_Generate(Block* World,int dx,int dy,int dz){
	memset(World,0,dx * dy * dz);

	float Seed[dx * dz];
	for(int i = 0;i<dx * dz;i++){
		Seed[i] = (float)Random_f64_New();
	}

	float Out[dx * dz];
	PerlinNoise_2D_Buffer(dx,dz,Seed,15,0.9f,Out);

	for(int i = 0;i<dz;i++){
		for(int j = 0;j<dx;j++){
			int h = 0;
			for(;h<Out[j + i * dx] * ((float)dy * 0.8f);h++){
				World_SetX(World,j,h,i,BLOCK_DIRT);
			}
			World_SetX(World,j,h,i,BLOCK_GRAS);
		}
	}


	for(int i = 2;i<dz-2;i++){
		for(int j = 2;j<dx-2;j++){
			int h = World_Height(World,j,i) + 1;
			
			if(Random_i32_MinMax(0,40)==0){
				World_Tree(World,j,h,i);
			}
		}
	}
}
void Mesh_Reload(){
	Vector_Clear(&cubeSides);

	for(int i = 0;i<WORLD_DZ;i++){
		for(int j = 0;j<WORLD_DY;j++){
			for(int k = 0;k<WORLD_DX;k++){
				unsigned char b = World_GetX(World,k,j,i);

				if(b!=BLOCK_VOID){
					for(int s = 0;s<6;s++){
						vec3d p = { k,j,i,1.0f };
						vec3d n = vec3d_Add(p,Neighbour_Side(s));
						
						if(World_Get(World,n)==BLOCK_VOID){
							Vector_Push(&cubeSides,(CubeSide[]){{
                                .pos = p,
                                .id = b,
                                .side = s
                            }});
						}
					}
				}
			}
		}
	}
}
void Menu_Set(int m){
	if(Menu==0 && m==1){
		AlxWindow_Mouse_SetInvisible(&window);
		SetMouse((Vec2){ GetWidth() / 2,GetHeight() / 2 });
	}
	if(Menu==1 && m==0){
		AlxWindow_Mouse_SetVisible(&window);
	}
	
	Menu = m;
}

int Cubes_Compare(const void* e1,const void* e2) {
	Rect3 r1 = *(Rect3*)e1;
	Rect3 r2 = *(Rect3*)e2;
	
	vec3d pos = vec3d_Add(camera,(vec3d){ vLength.x * 0.5f,vLength.y * 0.9f,vLength.z * 0.5f });
	vec3d d1 = vec3d_Sub(r1.p,pos);
    vec3d d2 = vec3d_Sub(r2.p,pos);
	return vec3d_Length(d1) == vec3d_Length(d2) ? 0 : (vec3d_Length(d1) < vec3d_Length(d2) ? 1 : -1);
}
void Cubes_Reload(){
	Vector_Clear(&Cubes);

	vec3d f = { (int)camera.x,(int)camera.y,(int)camera.z,1.0f };
	for(int i = -2;i<2;i++){
		for(int j = -2;j<2;j++){
			for(int k = -2;k<2;k++){
				vec3d n = { k,j,i,1.0f };
				vec3d r = vec3d_Add(f,n);

				Block b = World_Get(World,r);

				if(b!=BLOCK_VOID && b!=BLOCK_ERROR){
					Vector_Push(&Cubes,(Rect3[]){ { r,(vec3d){ 1.0f,1.0f,1.0f,1.0f } } });
				}
			}
		}
	}

	qsort(Cubes.Memory,Cubes.size,Cubes.ELEMENT_SIZE,Cubes_Compare);
}
void Stand(vec3d* Data){
	Data->y = 0.0f;
	OnGround = 1;
}
void Jump(vec3d* Data){
	Data->y = 0.0f;
}

void Setup(AlxWindow* w){
    World = (Block*)malloc(WORLD_DX * WORLD_DY * WORLD_DZ * sizeof(Block));

    cubeSides = Vector_New(sizeof(CubeSide));
	Menu_Set(0);
	Cubes = Vector_New(sizeof(Rect3));

	RGA_Get(Time_Nano());
	World_Generate(World,WORLD_DX,WORLD_DY,WORLD_DZ);
	Mesh_Reload();
    
    model = Matrix_MakeTranslation(0.0f,0.0f,0.0f);
    view = Matrix_MakeIdentity();
    proj = Matrix_MakeProjection(90.0f,(float)GetHeight() / (float)GetWidth(),0.1f,200.0f);
    
    glEnable(GL_DEPTH_TEST);

    float vertices[] = {
        // Vorderseite (Front)
         0.5f, -0.5f,  0.5f,     0.0f,  0.0f,  1.0f,     1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,     0.0f,  0.0f,  1.0f,     0.0f, 0.0f,   
         0.5f,  0.5f,  0.5f,     0.0f,  0.0f,  1.0f,     1.0f, 1.0f,    
         0.5f,  0.5f,  0.5f,     0.0f,  0.0f,  1.0f,     1.0f, 1.0f,   
        -0.5f, -0.5f,  0.5f,     0.0f,  0.0f,  1.0f,     0.0f, 0.0f,  
        -0.5f,  0.5f,  0.5f,     0.0f,  0.0f,  1.0f,     0.0f, 1.0f, 

        // Westseite (Left)
        -0.5f, -0.5f,  0.5f,     -1.0f,  0.0f,  0.0f,   1.0f, 0.0f, 
        -0.5f, -0.5f, -0.5f,     -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,     
        -0.5f,  0.5f,  0.5f,     -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,   
        -0.5f,  0.5f,  0.5f,     -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,  
        -0.5f, -0.5f, -0.5f,     -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,    
        -0.5f,  0.5f, -0.5f,     -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,    
    
        // Rückseite (Back)
        -0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,    0.0f, 0.0f,   
         0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,    1.0f, 0.0f,   
         0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,    1.0f, 1.0f,   
        -0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,    0.0f, 0.0f,   
         0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,    1.0f, 1.0f,   
        -0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,    0.0f, 1.0f,  
    
        // Ostseite (Right) - korrigierte Texturkoordinaten
         0.5f, -0.5f, -0.5f,      1.0f,  0.0f,  0.0f,   0.0f, 0.0f,   
         0.5f, -0.5f,  0.5f,      1.0f,  0.0f,  0.0f,   1.0f, 0.0f,   
         0.5f,  0.5f,  0.5f,      1.0f,  0.0f,  0.0f,   1.0f, 1.0f,   
         0.5f, -0.5f, -0.5f,      1.0f,  0.0f,  0.0f,   0.0f, 0.0f,   
         0.5f,  0.5f,  0.5f,      1.0f,  0.0f,  0.0f,   1.0f, 1.0f,   
         0.5f,  0.5f, -0.5f,      1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
           
       // Oben (Top) - korrigierte Texturkoordinaten 
        0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,   1.0f, 0.0f,  
       -0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,   0.0f, 0.0f,   
        0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,   1.0f, 1.0f,   
        0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,   1.0f, 1.0f,  
       -0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,   0.0f, 0.0f,    
       -0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,   0.0f, 1.0f, 

       // Südseite (Bottom)
       -0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,   0.0f, 0.0f,   
        0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,   1.0f, 0.0f,   
        0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,   1.0f, 1.0f,   
       -0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,   0.0f, 0.0f,   
        0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,   1.0f, 1.0f,   
       -0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,   0.0f, 1.0f, 
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Positions-Attribut binden
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Normale-Attribut binden
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // Texturkoordinaten binden
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    shaderProgram = createShaderProgram();
    glUseProgram(shaderProgram);

    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);

    stbi_set_flip_vertically_on_load(1);
    int w,h,n;
    unsigned char* data = stbi_load("./data/Atlas.png", &w, &h, &n, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, (n==3 ? GL_RGB : GL_RGBA), GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        stbi_image_free(data);
    }

    glUseProgram(shaderProgram);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);         // Rückseiten cullen (Standard)
    glFrontFace(GL_CCW);         // Dreiecke mit Gegenuhrzeigersinn gelten als Vorderseite

    glUniform1i(glGetUniformLocation(shaderProgram,"texture1"),0);
    glUniform2f(glGetUniformLocation(shaderProgram,"uvScale"),TILE_WIDTH,TILE_HEIGHT);
}

void Update(AlxWindow* w){
    angle += w->ElapsedTime;

    glViewport(0, 0, GetWidth(), GetHeight());

    mat4x4 rotX = Matrix_MakeRotationX(RotX);
    mat4x4 rotY = Matrix_MakeRotationY(RotY);
    model = Matrix_MakeTranslation(0.0f,0.0f,0.0f);
    
    vec3d vUp = (vec3d){ 0.0f,1.0f,0.0f,1.0f };
	vec3d vTarget = (vec3d){ 0.0f,0.0f,1.0f,1.0f };
	vec3d vLookDir = Matrix_MultiplyVector(Matrix_MultiplyMatrix(rotX,rotY),vTarget);
	
	vTarget = vec3d_Add(camera,vLookDir);
	mat4x4 matCamera = Matrix_PointAt(camera,vTarget,vUp);
	view = Matrix_QuickInverse(matCamera);
    //view = Matrix_MultiplyMatrix(rotY,Matrix_MakeRotationX(RotX));
    proj = Matrix_MakeProjection(90.0f,(float)GetHeight() / (float)GetWidth(),0.1f,200.0f);

    vec3d lookFront = Matrix_MultiplyVector(rotY,(vec3d){ 0.0f,0.0f,1.0f,1.0f });
    vec3d lookLeft = Matrix_MultiplyVector(rotY,(vec3d){ 1.0f,0.0f,0.0f,1.0f });
    
    if(Menu==1){
		if(GetMouse().x!=GetMouseBefore().x || GetMouse().y!=GetMouseBefore().y){
			Vec2 d = Vec2_Sub(GetMouse(),GetMouseBefore());
			Vec2 a = Vec2_Mulf(Vec2_Div(d,(Vec2){ window.Width,window.Height }),3 * F32_PI);
	
			RotY += -a.x;
			RotX += a.y;
	
			SetMouse((Vec2){ GetWidth() * 0.5f,GetHeight() * 0.5f });
		}
	}
	
	if(Stroke(ALX_KEY_ESC).PRESSED)
        Menu_Set(!Menu);
    
	if(Stroke(ALX_KEY_Z).PRESSED)
        Mode = Mode < 2 ? Mode+1 : 0;

    if(Stroke(ALX_KEY_W).DOWN){
        vVelocity.x += lookFront.x * 20.0f * w->ElapsedTime;
        vVelocity.z += lookFront.z * 20.0f * w->ElapsedTime;
    }
    if(Stroke(ALX_KEY_S).DOWN){
        vVelocity.x -= lookFront.x * 20.0f * w->ElapsedTime;
        vVelocity.z -= lookFront.z * 20.0f * w->ElapsedTime;
    }
    if(Stroke(ALX_KEY_A).DOWN){
        vVelocity.x -= lookLeft.x * 20.0f * w->ElapsedTime;
        vVelocity.z -= lookLeft.z * 20.0f * w->ElapsedTime;
    }
    if (Stroke(ALX_KEY_D).DOWN){
        vVelocity.x += lookLeft.x * 20.0f * w->ElapsedTime;
        vVelocity.z += lookLeft.z * 20.0f * w->ElapsedTime;
    }
    
    //if(Stroke(ALX_KEY_R).DOWN)      vVelocity.y = 14.0f;
    //if(Stroke(ALX_KEY_R).RELEASED)  vVelocity.y = 0.0f;
    //if(Stroke(ALX_KEY_F).DOWN)      vVelocity.y = -14.0f;
    //if(Stroke(ALX_KEY_F).RELEASED)  vVelocity.y = 0.0f;

    if(Stroke(ALX_KEY_R).DOWN)
		if(OnGround) 
			vVelocity.y = 5.0f;

    Vec2 v = { vVelocity.x,vVelocity.z };
	Vec2 d = Vec2_Norm(v);

	float drag = OnGround ? 12.0f : 10.0f;
	Vec2 da = Vec2_Norm(v);
	v = Vec2_Sub(v,Vec2_Mulf(d,drag * w->ElapsedTime));

	if(F32_Sign(v.x)!=F32_Sign(da.x) || F32_Sign(v.y)!=F32_Sign(da.y)){
		v.x = 0.0f;
		v.y = 0.0f;
	}

	float maxVelo = OnGround ? 4.0f : 6.0f;
	if(Vec2_Mag(v)>maxVelo){
		v = Vec2_Mulf(d,maxVelo);
	}

	vVelocity.x = v.x;
	vVelocity.z = v.y;

	vVelocity = vec3d_Add(vVelocity,vec3d_Mul((vec3d){ 0.0f,-10.0f,0.0f,1.0f },w->ElapsedTime));
	camera = vec3d_Add(camera,vec3d_Mul(vVelocity,w->ElapsedTime));

	Cubes_Reload();
	OnGround = 0;
	for(int i = 0;i<Cubes.size;i++){
		vec3d pos = { -vLength.x * 0.5f,vLength.y * 0.65f,-vLength.z * 0.5f };

		Rect3 r1 = *(Rect3*)Vector_Get(&Cubes,i);
		Rect3 r2 = (Rect3){ vec3d_Sub(camera,pos),vLength };
		Rect3_Static(&r2,r1,&vVelocity,(void (*[])(void*)){ NULL,NULL,NULL,NULL,(void*)Jump,(void*)Stand });
		camera = vec3d_Add(r2.p,pos);
	}

    //if(Stroke(ALX_KEY_UP).DOWN)     RotX -= F32_PI * w->ElapsedTime;
    //if(Stroke(ALX_KEY_DOWN).DOWN)   RotX += F32_PI * w->ElapsedTime;
    //if(Stroke(ALX_KEY_LEFT).DOWN)   RotY += F32_PI * w->ElapsedTime;
    //if(Stroke(ALX_KEY_RIGHT).DOWN)  RotY -= F32_PI * w->ElapsedTime;

    if(RotX<-F32_PI * 0.5f + 0.01f) RotX = -F32_PI * 0.5f + 0.01f;
    if(RotX> F32_PI * 0.5f - 0.01f) RotX =  F32_PI * 0.5f - 0.01f;

    if(Mode==0){
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }else{
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glUseProgram(shaderProgram);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"model"),1,GL_FALSE,(const GLfloat*)&model);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"view"),1,GL_FALSE,(const GLfloat*)&view);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"projection"),1,GL_FALSE,(const GLfloat*)&proj);

    glBindVertexArray(VAO);

    for (unsigned int i = 0; i < cubeSides.size; i++){
        CubeSide cs = *(CubeSide*)Vector_Get(&cubeSides,i);
        
        mat4x4 mat = Matrix_MultiplyMatrix(
            model,
            Matrix_MakeTranslation(cs.pos.x,cs.pos.y,cs.pos.z)
            // Matrix_MultiplyMatrix(
            //     Matrix_MakeTranslation(cubePositions[i].x,cubePositions[i].y,cubePositions[i].z),
            //     Matrix_MultiplyMatrix(
            //         Matrix_MakeRotationX(angle),
            //         Matrix_MakeRotationY(angle * 0.66f)
            //     )
            // )
        );

        vec2d atlaspos = getUVsFromIndex(Block_Id(cs.id,cs.side));
        glUniform2f(glGetUniformLocation(shaderProgram,"uvOffset"),atlaspos.u,atlaspos.v);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"model"),1,GL_FALSE,(const GLfloat*)&mat);
        
        int offset = cs.side * 6;
        glDrawArrays(GL_TRIANGLES, offset, 6);
    }
}

void Delete(AlxWindow* w){
    if(World) free(World);
    World = NULL;

	//Sprite_Free(&sprTex1);
	Vector_Free(&Cubes);

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    Vector_Free(&cubeSides);
}

int main(){
    if(Create("3D Tex OpenGL",800,600,1,1,Setup,Update,Delete))
        Start();
    return 0;
}
