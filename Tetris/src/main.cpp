#include "Prerequisites.hpp"
#include <GLFW/glfw3.h>
#include "Shaders.hpp"
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include "Image.hpp"
#include "Texture.hpp"
#include "PNGCodec.hpp"


#define WIDTH 800
#define HEIGHT 600

struct Color
{
	float r, g, b, a;

	bool operator==(const Color &col)
	{
		return (r == col.r && g == col.g && b == col.b && a == col.a);
	}
};

#pragma pack(push, 1)
struct Vertex
{
	float x, y;
	float r, g, b, a;
};

struct TextureVertex
{
	float x, y, u, v;
};
#pragma pack(pop)

GLuint LoadProgram(const char *vs, const char *fs);
void CreateGrid(void *vboData, size_t &vboOffset, float x, float y, 
	float width, float height, size_t *linesCount);
size_t PrepareDynamicBuffer(GLuint vbo, Color* table[][10], float x, float y, float width, float height);
size_t CreateLine(GLuint vbo, size_t offset, Color col, float x, float y, float width, float height);
void PlaceTetramino(Color* table[][10], Color* (*tetramino)[4], int32_t x, int32_t y);
void RemoveTetramino(Color* table[][10], Color* (*tetramino)[4], int32_t x, int32_t y);
void GetTopCoords(Color* (*tetramino)[4], int32_t *x, int32_t *y);
bool CheckCollision(Color* table[][10], Color* (*tetramino)[4], int32_t x, int32_t y);
void CopyTetramino(Color* (*dst)[4], Color* (*src)[4]);
void RotateTetraminoLeft(Color* (*tetramino)[4]);
void RotateTetraminoRight(Color* (*tetramino)[4]);
void LookForLines(Color* table[][10], int32_t indices[4]);
void RemoveLine(Color* table[][10], uint32_t index);

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow *wnd = glfwCreateWindow(WIDTH, HEIGHT, "Tetris Score: 0", nullptr, nullptr);
	glfwMakeContextCurrent(wnd);
	gl::sys::LoadFunctions();
	srand(time(nullptr));
	gl::Enable(gl::BLEND);
	gl::BlendFunc(gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA);
	glfwSwapInterval(1);

	glm::mat4x4 orthoMatrix;
	GLuint program;
	GLuint textureProgram;
	GLuint sampler;
	GLint samplerLocation;
	GLint orthoMatrixLocation1;
	GLint orthoMatrixLocation2;
	GLuint dynamicVao;
	GLuint dynamicVbo;
	std::vector<Vertex> staticVertexData;
	GLuint staticVao;
	GLuint staticVbo;
	size_t staticVboOffset;
	void *staticVboData;
	std::vector<TextureVertex> dynamicTextureVertexData;
	GLuint dynamicTextureVao;
	GLuint dynamicTextureVbo;
	size_t linesOffset;
	size_t linesCount;
	Color colors[8] =
	{
		Color { 0.0f, 0.0f, 0.0f, 1.0f },
		Color { 0.192f, 0.78f, 0.937f, 1.0f },
		Color { 0.969f, 0.827f, 0.031f, 1.0f },
		Color { 0.678f, 0.302f, 0.612f, 1.0f },
		Color { 0.004f, 0.8f, 0.0f, 1.0f },
		Color { 0.8f, 0.0f, 0.0f, 1.0f },
		Color { 0.0f, 0.0f, 0.8f, 1.0f },
		Color { 0.937f, 0.475f, 0.129f, 1.0f },
	};

	Color* table[22][10];
	Color* tetraminoType[7][4][4] =
	{
		{ // I-type
			{ nullptr, nullptr, nullptr, nullptr },
			{ &colors[1], &colors[1], &colors[1], &colors[1] },
			{ nullptr, nullptr, nullptr, nullptr },
			{ nullptr, nullptr, nullptr, nullptr }
		},

		{ // O-type
			{ nullptr, nullptr, nullptr, nullptr },
			{ nullptr, &colors[2], &colors[2], nullptr },
			{ nullptr, &colors[2], &colors[2], nullptr },
			{ nullptr, nullptr, nullptr, nullptr }
		},

		{ // T-type
			{ nullptr, nullptr, nullptr, nullptr },
			{ nullptr, &colors[3], nullptr, nullptr },
			{ &colors[3], &colors[3], &colors[3], nullptr },
			{ nullptr, nullptr, nullptr, nullptr }
		},

		{ // S-type
			{ nullptr, nullptr, nullptr, nullptr },
			{ nullptr, &colors[4], &colors[4], nullptr },
			{ &colors[4], &colors[4], nullptr, nullptr },
			{ nullptr, nullptr, nullptr, nullptr }
		},

		{ // Z-type
			{ nullptr, nullptr, nullptr, nullptr },
			{ &colors[5], &colors[5], nullptr, nullptr },
			{ nullptr, &colors[5], &colors[5], nullptr },
			{ nullptr, nullptr, nullptr, nullptr }
		},

		{ // J-type
			{ nullptr, nullptr, nullptr, nullptr },
			{ &colors[6], nullptr, nullptr, nullptr },
			{ &colors[6], &colors[6], &colors[6], nullptr },
			{ nullptr, nullptr, nullptr, nullptr }
		},

		{ // L-type
			{ nullptr, nullptr, nullptr, nullptr },
			{ nullptr, nullptr, &colors[7], nullptr },
			{ &colors[7], &colors[7], &colors[7], nullptr },
			{ nullptr, nullptr, nullptr, nullptr }
		}
	};
	Color* currTetramino[4][4];

	for (int i = 0; i < 22 * 10; i++)
		table[i / 10][i % 10] = nullptr;


	program = LoadProgram(VERTEX_SHADER, FRAGMENT_SHADER);
	textureProgram = LoadProgram(VERTEX_TEXTURE_SHADER, FRAGMENT_TEXTURE_SHADER);
	gl::GenVertexArrays(1, &staticVao);
	gl::GenBuffers(1, &staticVbo);

	// Fullscreen Quad;
	staticVertexData.push_back(Vertex{ 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f });
	staticVertexData.push_back(Vertex{ 400.0f, 600.0f, 0.0f, 1.0f, 0.0f, 1.0f });
	staticVertexData.push_back(Vertex{ 800.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f });
	
	gl::BindVertexArray(staticVao);
	gl::BindBuffer(gl::ARRAY_BUFFER, staticVbo);
	gl::BufferData(gl::ARRAY_BUFFER, 1024*1024*4, 
		nullptr, gl::STATIC_DRAW);

	staticVboData = gl::MapBuffer(gl::ARRAY_BUFFER, gl::WRITE_ONLY);
	staticVboOffset = 0;
	memcpy(staticVboData, staticVertexData.data(), sizeof(Vertex) * staticVertexData.size());
	staticVboOffset += sizeof(Vertex) * staticVertexData.size();
	linesOffset = staticVertexData.size();
	CreateGrid(staticVboData, staticVboOffset, 255.0f, 10.0f, 290.0f, 580.0f, &linesCount);
	gl::UnmapBuffer(gl::ARRAY_BUFFER);

	gl::EnableVertexAttribArray(0);
	gl::VertexAttribPointer(0, 2, gl::FLOAT, gl::FALSE_,
		sizeof(Vertex), 0);
	gl::EnableVertexAttribArray(1);
	gl::VertexAttribPointer(1, 3, gl::FLOAT, gl::FALSE_,
		sizeof(Vertex), (GLvoid*)(2 * sizeof(float)));


	gl::GenVertexArrays(1, &dynamicVao);
	gl::BindVertexArray(dynamicVao);
	gl::GenBuffers(1, &dynamicVbo);
	gl::BindBuffer(gl::ARRAY_BUFFER, dynamicVbo);
	gl::BufferData(gl::ARRAY_BUFFER, 1024 * 1024 * 4, nullptr, gl::DYNAMIC_DRAW);
	gl::EnableVertexAttribArray(0);
	gl::VertexAttribPointer(0, 2, gl::FLOAT, gl::FALSE_,
		sizeof(Vertex), 0);
	gl::EnableVertexAttribArray(1);
	gl::VertexAttribPointer(1, 4, gl::FLOAT, gl::FALSE_,
		sizeof(Vertex), (GLvoid*)(2 * sizeof(float)));


	gl::GenVertexArrays(1, &dynamicTextureVao);
	gl::BindVertexArray(dynamicTextureVao);
	gl::GenBuffers(1, &dynamicTextureVbo);
	gl::BindBuffer(gl::ARRAY_BUFFER, dynamicTextureVbo);
	gl::BufferData(gl::ARRAY_BUFFER, 1024 * 1024, nullptr, gl::DYNAMIC_DRAW);
	gl::EnableVertexAttribArray(0);
	gl::VertexAttribPointer(0, 2, gl::FLOAT, gl::FALSE_, sizeof(TextureVertex), 0);
	gl::EnableVertexAttribArray(1);
	gl::VertexAttribPointer(1, 2, gl::FLOAT, gl::FALSE_, sizeof(TextureVertex), (GLvoid*)(2 * sizeof(float)));


	dynamicTextureVertexData.push_back(TextureVertex{ 0.0f, 0.0f, 0.0f, 0.0f });
	dynamicTextureVertexData.push_back(TextureVertex{ 0.0f, 600.0f, 0.0f, 1.0f });
	dynamicTextureVertexData.push_back(TextureVertex{ 800.0f, 0.0f, 1.0f, 0.0f });

	dynamicTextureVertexData.push_back(TextureVertex{ 0.0f, 600.0f, 0.0f, 1.0f });
	dynamicTextureVertexData.push_back(TextureVertex{ 800.0f, 0.0f, 1.0f, 0.0f });
	dynamicTextureVertexData.push_back(TextureVertex{ 800.0f, 600.0f, 1.0f, 1.0f });

	gl::BufferSubData(gl::ARRAY_BUFFER, 0, dynamicTextureVertexData.size() * sizeof(TextureVertex), dynamicTextureVertexData.data());

	gl::GenSamplers(1, &sampler);
	gl::SamplerParameteri(sampler, gl::TEXTURE_WRAP_S, gl::CLAMP_TO_EDGE);
	gl::SamplerParameteri(sampler, gl::TEXTURE_WRAP_T, gl::CLAMP_TO_EDGE);
	gl::SamplerParameteri(sampler, gl::TEXTURE_MIN_FILTER, gl::LINEAR);
	gl::SamplerParameteri(sampler, gl::TEXTURE_MAG_FILTER, gl::LINEAR);

	gl::ClearColor(0.8f, 0.8f, 0.8f, 1.0f);
	gl::Viewport(0, 0, WIDTH, HEIGHT);
	orthoMatrix = glm::ortho(0.0f, (float)WIDTH, 0.0f, (float)HEIGHT);
	orthoMatrixLocation1 = gl::GetUniformLocation(program, "orthoMatrix");
	orthoMatrixLocation2 = gl::GetUniformLocation(textureProgram, "orthoMatrix");
	samplerLocation = gl::GetUniformLocation(textureProgram, "tex");

	Image endImage;
	Texture endTexture;

	endImage.loadFromFile("endImage.png", &PNGCodec());
	endTexture.createFromImage(endImage);
	
	double lastTime, currTime = lastTime = glfwGetTime();
	double dt;
	double acc = 0.0;
	const double UPDATE_TIME = 1 / 60.0;
	std::stringstream str;
	double downAcceleration = 0.05;
	double downSpeed = 2.0;
	double downFastSpeed = 30.0;
	double sideSpeed = 17.0;
	double x = 0.0, y = 0.0;
	int32_t ix = 0, iy = 0.0;
	double dx = 0.0, dy = 0.0;
	uint32_t points = 0;
	size_t countBlocksVertices = 0;
	size_t countLinesVertices = 0;
	bool needNew = true;
	bool gameOver = false;
	int space;
	bool removeAnimation = false;
	double animationTime = 0.0;
	int32_t linesToRemove[4] = { -1, -1, -1, -1 };

	while (!glfwWindowShouldClose(wnd))
	{
		currTime = glfwGetTime();
		dt = currTime - lastTime;
		lastTime = currTime;
		acc += dt;

		glfwPollEvents();

		gl::Clear(gl::COLOR_BUFFER_BIT);


		if (glfwGetKey(wnd, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(wnd, 0);

		while (acc >= UPDATE_TIME)
		{
			if (gameOver == true)
				break;

			if (glfwGetKey(wnd, GLFW_KEY_SPACE) == GLFW_PRESS)
				space += 1;

			if (glfwGetKey(wnd, GLFW_KEY_SPACE) == GLFW_RELEASE)
				space = 0;

			if (glfwGetKey(wnd, GLFW_KEY_LEFT) == GLFW_PRESS)
				dx = -UPDATE_TIME * sideSpeed;

			if (glfwGetKey(wnd, GLFW_KEY_RIGHT) == GLFW_PRESS)
				dx = UPDATE_TIME * sideSpeed;

			downSpeed += UPDATE_TIME * downAcceleration;
			downFastSpeed += UPDATE_TIME * downAcceleration;

			if (glfwGetKey(wnd, GLFW_KEY_DOWN) == GLFW_PRESS)
				dy = UPDATE_TIME * downFastSpeed;
			else
				dy = UPDATE_TIME * downSpeed;


			// Checks if it's time to choose new tetramino and if so
			// places in on top of the table.
			if (needNew)
			{
				CopyTetramino(currTetramino, tetraminoType[rand() % 7]);
				GetTopCoords(currTetramino, &ix, &iy);
				x = ix;
				y = iy;

				if (!CheckCollision(table, currTetramino, x, y))
				{
					std::stringstream str;
					str << "Tetris Score: " << points << " GAME OVER";
					gameOver = true;
					glfwSetWindowTitle(wnd, str.str().c_str());
				}
				
				needNew = false;
			}

			// Rotating tetramino.
			if (space == 1)
			{
				RotateTetraminoRight(currTetramino);

				if (CheckCollision(table, currTetramino, x, y));
				else if (CheckCollision(table, currTetramino, x + 1.0, y))
					x += 1.0;
				else if (CheckCollision(table, currTetramino, x - 1.0, y))
					x -= 1.0;
				else 
				{
					RotateTetraminoLeft(currTetramino);
					RotateTetraminoLeft(currTetramino);

					if (!CheckCollision(table, currTetramino, x, y))
						RotateTetraminoRight(currTetramino);
				}
			}


			PlaceTetramino(table, currTetramino, x, y);
			countLinesVertices = 0;
			countBlocksVertices = PrepareDynamicBuffer(dynamicVbo, table, 255.0f, 10.0f, 290.0f, 580.0f);
			RemoveTetramino(table, currTetramino, x, y);
			
			// Tetramino go down or stays on its place.
			if (!CheckCollision(table, currTetramino, x, y + dy))
			{
				PlaceTetramino(table, currTetramino, x, y);
				needNew = true;
			}
			else if (CheckCollision(table, currTetramino, x + dx, y + dy))
				x += dx;
		
			y += dy;

			if (removeAnimation == false && needNew == true)
			{
				LookForLines(table, linesToRemove);
				animationTime = 0.0;

				if (linesToRemove[0] != -1)
					removeAnimation = true;
			}

			if (removeAnimation == true)
			{
				if (animationTime > 0.5)
				{
					uint32_t sum = 0;
					uint32_t multiplier = 0;
					for (int i = 0; i < 4 && linesToRemove[i] != -1; i++)
					{
						sum += (22 - linesToRemove[i]) * 5;
						multiplier++;
						RemoveLine(table, linesToRemove[i]);
					}

					points += sum * multiplier;

					std::stringstream str;
					str << "Tetris Score: " << points;
					glfwSetWindowTitle(wnd, str.str().c_str());

					removeAnimation = false;
				}

				float alpha = animationTime < 0.3 ? (float)animationTime / 0.3f : 1.0f;

				for (int i = 0; i < 4 && linesToRemove[i] != -1; i++)
					countLinesVertices += CreateLine(dynamicVbo, (countBlocksVertices + countLinesVertices) * sizeof(Vertex),
						Color{ 0.8f, 0.8f, 0.8f, alpha }, 255.0f, 10.0f + (21 - linesToRemove[i]) * 580.0f / 20.0f,
						290.0f, 580.0f / 20.0f);

				animationTime += UPDATE_TIME;
			}

			dy = 0.0;
			dx = 0.0;

			acc -= UPDATE_TIME;
		}

		gl::UseProgram(program);
		gl::UniformMatrix4fv(orthoMatrixLocation1, 1, gl::FALSE_, &orthoMatrix[0][0]);

		gl::BindVertexArray(dynamicVao);
		gl::DrawArrays(gl::TRIANGLES, 0, countBlocksVertices + countLinesVertices);

		gl::BindVertexArray(staticVao);
		gl::DrawArrays(gl::LINES, linesOffset, linesCount);

		if (gameOver == true)
		{
			gl::UseProgram(textureProgram);
			gl::Uniform1i(samplerLocation, 0);
			gl::UniformMatrix4fv(orthoMatrixLocation1, 1, gl::FALSE_, &orthoMatrix[0][0]);

			endTexture.bind(0);

			gl::BindSampler(0, sampler);

			gl::BindVertexArray(dynamicTextureVao);
			gl::DrawArrays(gl::TRIANGLES, 0, 6);
		}

		glfwSwapBuffers(wnd);
	}

	glfwDestroyWindow(wnd);
	glfwTerminate();
	return 0;
}

GLuint LoadProgram(const char *vs, const char *fs)
{
	GLuint vertexShaderID = gl::CreateShader(gl::VERTEX_SHADER);
	GLuint fragmentShaderID = gl::CreateShader(gl::FRAGMENT_SHADER);
	GLint result;
	GLint logLength;
	gl::ShaderSource(vertexShaderID, 1, (const GLchar**)&vs, nullptr);
	gl::CompileShader(vertexShaderID);
	gl::GetShaderiv(vertexShaderID, gl::COMPILE_STATUS, &result);
	gl::GetShaderiv(vertexShaderID, gl::INFO_LOG_LENGTH, &logLength);
	char *vslog = new char[logLength + 1];
	vslog[logLength] = '\0';
	gl::GetShaderInfoLog(vertexShaderID, logLength, nullptr, vslog);
	std::cout << vslog << std::endl;
	delete[] vslog;

	gl::ShaderSource(fragmentShaderID, 1, (const GLchar**)&fs, nullptr);
	gl::CompileShader(fragmentShaderID);
	gl::GetShaderiv(fragmentShaderID, gl::COMPILE_STATUS, &result);
	gl::GetShaderiv(fragmentShaderID, gl::INFO_LOG_LENGTH, &logLength);
	char *fslog = new char[logLength + 1];
	fslog[logLength] = '\0';
	gl::GetShaderInfoLog(fragmentShaderID, logLength, nullptr, fslog);
	std::cout << fslog << std::endl;
	delete[] fslog;

	GLuint program = gl::CreateProgram();
	gl::AttachShader(program, vertexShaderID);
	gl::AttachShader(program, fragmentShaderID);
	gl::LinkProgram(program);
	gl::GetProgramiv(program, gl::LINK_STATUS, &result);
	gl::GetProgramiv(program, gl::INFO_LOG_LENGTH, &logLength);
	char *proglog = new char[logLength + 1];
	proglog[logLength] = '\0';
	gl::GetProgramInfoLog(program, logLength, nullptr, proglog);
	std::cout << proglog << std::endl;
	delete[] proglog;

	gl::DeleteShader(vertexShaderID);
	gl::DeleteShader(fragmentShaderID);

	return program;
}

void CreateGrid(void *vboData, size_t &vboOffset, float x, float y, 
	float width, float height, size_t *linesCount)
{
	const uint8_t GRID_COLUMNS = 10;
	const uint8_t GRID_ROWS = 20;

	float dw = width / GRID_COLUMNS;
	float dh = height / GRID_ROWS;

	std::vector<Vertex> lines;

	for (uint8_t i = 0; i <= GRID_COLUMNS; i++)
	{
		lines.push_back(Vertex{ x + dw * i, y, 0.0f, 0.0f, 1.0f, 1.0f });
		lines.push_back(Vertex{ x + dw * i, y + height, 0.0f, 0.0f, 1.0f, 1.0f });
	}

	for (uint8_t j = 0; j <= GRID_ROWS; j++)
	{
		lines.push_back(Vertex{ x, y + j * dh, 0.0f, 0.0f, 1.0f, 1.0f });
		lines.push_back(Vertex{ x + width, y + j * dh, 0.0f, 0.0f, 1.0f, 1.0f });
	}

	memcpy(&((char*)vboData)[vboOffset], lines.data(), lines.size() * sizeof(Vertex));
	vboOffset += lines.size() * sizeof(Vertex);
	*linesCount = lines.size();

	return;
}

size_t PrepareDynamicBuffer(GLuint vbo, Color* table[][10], float x, float y, float width, float height)
{
	std::vector<Vertex> vertexData;
	const uint8_t GRID_COLUMNS = 10;
	const uint8_t GRID_ROWS = 20;

	y += height; // up to down.

	float dw = width / GRID_COLUMNS;
	float dh = height / GRID_ROWS;

	for (int i = 2; i < GRID_ROWS + 2; i++)
	{
		for (int j = 0; j < GRID_COLUMNS; j++)
		{
			if (table[i][j] == nullptr) continue;

			vertexData.push_back(Vertex{ x + dw * j, y - dh * (i - 2), (*table[i][j]).r, (*table[i][j]).g, (*table[i][j]).b, (*table[i][j]).a });
			vertexData.push_back(Vertex{ x + dw * (j + 1), y - dh * (i - 2), (*table[i][j]).r, (*table[i][j]).g, (*table[i][j]).b, (*table[i][j]).a });
			vertexData.push_back(Vertex{ x + dw * j, y - dh * (i - 1), (*table[i][j]).r + 0.2f, (*table[i][j]).g + 0.2f, (*table[i][j]).b + 0.2f, (*table[i][j]).a });

			vertexData.push_back(Vertex{ x + dw * j, y - dh * (i - 1), (*table[i][j]).r + 0.2f, (*table[i][j]).g + 0.2f, (*table[i][j]).b + 0.2f, (*table[i][j]).a });
			vertexData.push_back(Vertex{ x + dw * (j + 1), y - dh * (i - 2), (*table[i][j]).r, (*table[i][j]).g, (*table[i][j]).b, (*table[i][j]).a });
			vertexData.push_back(Vertex{ x + dw * (j + 1), y - dh * (i - 1), (*table[i][j]).r, (*table[i][j]).g, (*table[i][j]).b, (*table[i][j]).a });
		}
	}
	
	gl::BindBuffer(gl::ARRAY_BUFFER, vbo);
	void *vboData = gl::MapBuffer(gl::ARRAY_BUFFER, gl::WRITE_ONLY);
	memcpy(vboData, vertexData.data(), vertexData.size() * sizeof(Vertex));
	gl::UnmapBuffer(gl::ARRAY_BUFFER);

	return vertexData.size();
}

size_t CreateLine(GLuint vbo, size_t offset, Color col, float x, float y, float width, float height)
{
	std::vector<Vertex> vertexData;
	void *vboData;

	vertexData.push_back(Vertex{ x, y, col.r, col.g, col.b, col.a });
	vertexData.push_back(Vertex{ x, y + height, col.r, col.g, col.b, col.a });
	vertexData.push_back(Vertex{ x + width, y, col.r, col.g, col.b, col.a });

	vertexData.push_back(Vertex{ x, y + height, col.r, col.g, col.b, col.a });
	vertexData.push_back(Vertex{ x + width, y + height, col.r, col.g, col.b, col.a });
	vertexData.push_back(Vertex{ x + width, y, col.r, col.g, col.b, col.a });

	gl::BindBuffer(gl::ARRAY_BUFFER, vbo);
	vboData = gl::MapBufferRange(gl::ARRAY_BUFFER, offset, vertexData.size() * sizeof(Vertex), gl::MAP_WRITE_BIT);
	memcpy(vboData, vertexData.data(), vertexData.size() * sizeof(Vertex));
	gl::UnmapBuffer(gl::ARRAY_BUFFER);

	return vertexData.size();
}

void PlaceTetramino(Color* table[][10], Color* (*tetramino)[4], int32_t x, int32_t y)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (tetramino[i][j] == nullptr)
				continue;

			table[y + i][x + j] = tetramino[i][j];
		}
	}
}

void RemoveTetramino(Color* table[][10], Color* (*tetramino)[4], int32_t x, int32_t y)
{
	// No error checking.

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (tetramino[i][j] == nullptr)
				continue;

			table[y + i][x + j] = nullptr;
		}
	}
}

void GetTopCoords(Color* (*tetramino)[4], int32_t *x, int32_t *y)
{
	*x = 3;
	
	for (int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++) // Columns first!
		{
			if (!(tetramino[i][j] == nullptr))
			{
				*y = -i;
				return;
			}
		}
	}
}

bool CheckCollision(Color* table[][10], Color* (*tetramino)[4], int32_t x, int32_t y)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (tetramino[i][j] != nullptr)
			{
				if (x + j > 9 || x + j < 0 ||
					y + i > 21 || y + i < 0)
					return false;

				if (table[y + i][x + j] != nullptr)
					return false;
			}
		}
	}

	return true;
}

void CopyTetramino(Color * (*dst)[4], Color* (*src)[4])
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			dst[i][j] = src[i][j];
	}

}

void RotateTetraminoRight(Color* (*tetramino)[4])
{
	// [0][0] [0][1]  [0][2]  [0][3]
	// [1][0] [1][1]  [1][2]  [1][3]
	// [2][0] [2][1]  [2][2]  [2][3]
	// [3][0] [3][1]  [3][2]  [3][3]
	Color *tmp;
	tmp = tetramino[3][0];
	tetramino[3][0] = tetramino[3][3];
	tetramino[3][3] = tetramino[0][3];
	tetramino[0][3] = tetramino[0][0];
	tetramino[0][0] = tmp;

	tmp = tetramino[2][0];
	tetramino[2][0] = tetramino[3][2];
	tetramino[3][2] = tetramino[1][3];
	tetramino[1][3] = tetramino[0][1];
	tetramino[0][1] = tmp;

	tmp = tetramino[1][0];
	tetramino[1][0] = tetramino[3][1];
	tetramino[3][1] = tetramino[2][3];
	tetramino[2][3] = tetramino[0][2];
	tetramino[0][2] = tmp;

	tmp = tetramino[2][1];
	tetramino[2][1] = tetramino[2][2];
	tetramino[2][2] = tetramino[1][2];
	tetramino[1][2] = tetramino[1][1];
	tetramino[1][1] = tmp;
}

void RotateTetraminoLeft(Color* (*tetramino)[4])
{
	// [0][0] [0][1]  [0][2]  [0][3]
	// [1][0] [1][1]  [1][2]  [1][3]
	// [2][0] [2][1]  [2][2]  [2][3]
	// [3][0] [3][1]  [3][2]  [3][3]
	Color *tmp;
	tmp = tetramino[3][0];
	tetramino[3][0] = tetramino[0][0];
	tetramino[0][0] = tetramino[0][3];
	tetramino[0][3] = tetramino[3][3];
	tetramino[3][3] = tmp;

	tmp = tetramino[2][0];
	tetramino[2][0] = tetramino[0][1];
	tetramino[0][1] = tetramino[1][3];
	tetramino[1][3] = tetramino[3][2];
	tetramino[3][2] = tmp;

	tmp = tetramino[1][0];
	tetramino[1][0] = tetramino[0][2];
	tetramino[0][2] = tetramino[2][3];
	tetramino[2][3] = tetramino[3][1];
	tetramino[3][1] = tmp;

	tmp = tetramino[2][1];
	tetramino[2][1] = tetramino[1][1];
	tetramino[1][1] = tetramino[1][2];
	tetramino[1][2] = tetramino[2][2];
	tetramino[2][2] = tmp;

	// [0][3] [1][3] [2][3] [3][3]
	// [0][2] [1][2] [2][2] [3][2]
	// [0][1] [1][1] [2][1] [3][1]
	// [0][0] [1][0] [2][0] [3][0]
}

void LookForLines(Color* table[][10], int32_t indices[4])
{
	int index = 0;
	const uint8_t GRID_COLUMNS = 10;
	const uint8_t GRID_ROWS = 20;
	bool fullLine = true;

	for (int i = 0; i < 4; i++)
		indices[i] = -1;

	for (int i = 2; i < GRID_ROWS + 2; i++)
	{
		for (int j = 0; j < GRID_COLUMNS; j++)
		{
			if (table[i][j] == nullptr)
			{
				fullLine = false;
				break;
			}
		}

		if (fullLine == true)
		{
			if (index > 3) return;

			indices[index] = i;
			index++;
		}

		fullLine = true;
	}
}

void RemoveLine(Color* table[][10], uint32_t index)
{
	const uint8_t GRID_COLUMNS = 10;
	const uint8_t GRID_ROWS = 20;

	for (int i = index - 1; i >= 0; i--)
	{
		for (int j = 0; j < GRID_COLUMNS; j++)
			table[i + 1][j] = table[i][j];
	}

	for (int j = 0; j < GRID_COLUMNS; j++)
		table[0][j] = nullptr;
}

