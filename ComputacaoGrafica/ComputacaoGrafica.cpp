#include <GL/freeglut.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

using namespace std;

struct Vec3 {
	float x, y, z;
};

struct Vec2 {
	float u, v;
};

struct Face {
	vector<int> vertexIndices;
	vector<int> normalIndices;
	vector<int> texCoordIndices;
};

vector<vector<float>> vertices;
vector<Face> faces;
vector<vector<float>> normals;
vector<vector<float>> texCoords;

// Configurações de transformação
float angleX = 0.0f;
float angleY = 0.0f;
float angleZ = 0.0f;
float scaleX = 1.0f;
float scaleY = 1.0f;
float scaleZ = 1.0f;
float posX = 0.0f;
float posY = 0.0f;
float posZ = -5.0f;

int lastMouseX, lastMouseY;
bool isRotating = false;
GLuint obj;

vector<bool> lightsStatus = { true, true, true };

void loadObj(string fname)
{
	float x, y, z;
	ifstream arquivo(fname);
	if (!arquivo.is_open()) {
		cout << "Não foi possível abrir o arquivo.";
		exit(1);
	}
	else {
		string type;
		while (arquivo >> type)
		{
			if (type == "v") //Vertices - Define um ponto no espaço tridimensional usando coordenadas X, Y, e Z.
			{
				vector<float> vertice;
				float x, y, z;
				arquivo >> x >> y >> z;
				vertice.push_back(x);
				vertice.push_back(y);
				vertice.push_back(z);
				vertices.push_back(vertice);
			}

			if (type == "vn") //Vertices normais - Define a direção do vetor normal em um vértice.
			{
				vector<float> normal;
				float nx, ny, nz;
				arquivo >> nx >> ny >> nz;
				normal.push_back(nx);
				normal.push_back(ny);
				normal.push_back(nz);
				normals.push_back(normal);
			}

			if (type == "vt") // Vertice textura - Define as coordenadas de textura 
			{                 // (também conhecidas como UVs) para mapeamento de texturas.
				vector<float> texCoord;
				float u, v;
				arquivo >> u >> v;
				texCoord.push_back(u);
				texCoord.push_back(v);
				texCoords.push_back(texCoord);
			}

			if (type == "f") // Faces do objeto - Define uma face do polígono usando índices que referenciam 
			{                // uma lista de vértices e, opcionalmente, texturas e normais.
				Face face;
				string vertexInfo;
				for (int i = 0; i < 3; i++) {
					arquivo >> vertexInfo;
					int first = vertexInfo.find('/'); //separa o índice do vértice do índice da coordenada de textura
					int second = vertexInfo.find('/', first + 1);//separa o índice da coordenada de textura do índice da normal.
					int third = vertexInfo.find('/', second + 1); //encontra a terceira barra que é a de textura caso exista.

					//extrai esses indices
					string vertexIndex = vertexInfo.substr(0, first);
					string normalIndex = vertexInfo.substr(second + 1);
					string texCoordIndex = vertexInfo.substr(third + 1);

					//Converte os indices de string para inteiro:
					face.vertexIndices.push_back(stoi(vertexIndex) - 1);
					face.normalIndices.push_back(stoi(normalIndex) - 1);
					face.texCoordIndices.push_back(stoi(texCoordIndex) - 1);
				}
				faces.push_back(face);
			}
		}
	}

	obj = glGenLists(1);
	glPointSize(2.0);
	glNewList(obj, GL_COMPILE);
	glBegin(GL_TRIANGLES);

	for (const auto& face : faces) //Itera a quantidade de faces.
	{
		for (int i = 0; i < 3; i++) //Como cada face é um triangulo, iteramos 3 vezes aq
		{
			//Extrai os indexes da face atual que está sendo interada
			int vertexIndex = face.vertexIndices[i];
			int normalIndex = face.normalIndices[i];
			int texCoordIndex = face.texCoordIndices[i];

			if (!normals.empty()) // Se a lista de normais não estiver vazia, especifica 
			{					  //a normal do vértice corrente. Normais são vetores usados para iluminação e shading. 
				const auto& normal = normals[normalIndex];
				glNormal3f(normal[0], normal[1], normal[2]);
			}
			if (!vertices.empty()) //Se a lista de vértices não estiver vazia, especifica a posição do vértice no espaço 3D.
			{
				const auto& vertex = vertices[vertexIndex];
				glVertex3f(vertex[0], vertex[1], vertex[2]);
			}
			if (!texCoords.empty()) // Se a lista de coordenadas de textura não estiver vazia, especifica a coordenada de textura do
			{						//vértice que é usada para mapear uma imagem (textura) sobre a superfície do modelo.
				const auto& texCoord = texCoords[texCoordIndex];
				glTexCoord3f(texCoord[0], texCoord[1], 0);
			}
		}
	}
	glEnd();
	glEndList();
	arquivo.close();
}

void setupLight(int lightNum, const Vec3& position, const Vec3& ambient, const Vec3& diffuse, const Vec3& specular) {
	GLfloat lightPos[] = { position.x, position.y, position.z, 1.0f };
	GLfloat lightAmb[] = { ambient.x, ambient.y, ambient.z, 1.0f };
	GLfloat lightDif[] = { diffuse.x, diffuse.y, diffuse.z, 1.0f };
	GLfloat lightSpec[] = { specular.x, specular.y, specular.z, 1.0f };

	glLightfv(GL_LIGHT0 + lightNum, GL_POSITION, lightPos);
	glLightfv(GL_LIGHT0 + lightNum, GL_AMBIENT, lightAmb);
	glLightfv(GL_LIGHT0 + lightNum, GL_DIFFUSE, lightDif);
	glLightfv(GL_LIGHT0 + lightNum, GL_SPECULAR, lightSpec);

	if (lightsStatus[lightNum]) {
		glEnable(GL_LIGHT0 + lightNum);
	}
	else {
		glDisable(GL_LIGHT0 + lightNum);
	}
}

void toggleLight(int lightNum) {
	lightsStatus[lightNum] = !lightsStatus[lightNum];
	if (lightsStatus[lightNum]) {
		glEnable(GL_LIGHT0 + lightNum);
	}
	else {
		glDisable(GL_LIGHT0 + lightNum);
	}
}

void initLighting() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);

	setupLight(0, Vec3{ 0.0f, 5.0f, 5.0f }, Vec3{ 0.1f, 0.1f, 0.1f }, Vec3{ 0.8f, 0.8f, 0.8f }, Vec3{ 1.0f, 1.0f, 1.0f });
	setupLight(1, Vec3{ -5.0f, 0.0f, 5.0f }, Vec3{ 0.1f, 0.1f, 0.1f }, Vec3{ 0.5f, 0.5f, 0.5f }, Vec3{ 1.0f, 1.0f, 1.0f });
	setupLight(2, Vec3{ 5.0f, 0.0f, 5.0f }, Vec3{ 0.1f, 0.1f, 0.1f }, Vec3{ 0.5f, 0.5f, 0.5f }, Vec3{ 1.0f, 1.0f, 1.0f });
}

void drawObject() {
	glPushMatrix();
	glTranslatef(posX, posY, posZ);
	glScalef(scaleX, scaleY, scaleZ);
	glRotatef(angleX, 1.0f, 0.0f, 0.0f);
	glRotatef(angleY, 0.0f, 1.0f, 0.0f);
	glRotatef(angleZ, 0.0f, 0.0f, 1.0f);
	glCallList(obj);
	glPopMatrix();
}

void reshape(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLfloat)w / (GLfloat)h, 0.1, 20.0);
	glMatrixMode(GL_MODELVIEW);
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	drawObject();
	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'x': angleX += 5.0f; break;
	case 'X': angleX -= 5.0f; break;
	case 'y': angleY += 5.0f; break;
	case 'Y': angleY -= 5.0f; break;
	case 'z': angleZ += 5.0f; break;
	case 'Z': angleZ -= 5.0f; break;
	case '+': scaleX *= 1.1f; scaleY *= 1.1f; scaleZ *= 1.1f; break;
	case '-': scaleX /= 1.1f; scaleY /= 1.1f; scaleZ /= 1.1f; break;
	case 'w': posY += 0.1f; break;
	case 's': posY -= 0.1f; break;
	case 'a': posX -= 0.1f; break;
	case 'd': posX += 0.1f; break;
	case '1': toggleLight(0); break;
	case '2': toggleLight(1); break;
	case '3': toggleLight(2); break;
	}
	glutPostRedisplay();
}

void mouseMotion(int x, int y) {
	if (isRotating) {
		// Calcular diferenças
		int dx = x - lastMouseX;
		int dy = y - lastMouseY;

		// Atualizar ângulos de rotação baseados no movimento do mouse
		angleY += (float)dx * 0.3f; // Velocidade de rotação pode ser ajustada alterando o fator
		angleX += (float)dy * 0.3f;

		// Atualizar a última posição do mouse
		lastMouseX = x;
		lastMouseY = y;

		glutPostRedisplay(); // Marcar a janela para ser redesenhada
	}
}

void mouseButton(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			// Iniciar rotação
			isRotating = true;
			lastMouseX = x;
			lastMouseY = y;
		}
		else if (state == GLUT_UP) {
			// Parar rotação
			isRotating = false;
		}
	}
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(1600, 900);
	glutCreateWindow("Carregar OBJ");
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouseButton);
	glutMotionFunc(mouseMotion);
	initLighting();
	loadObj("mba1.obj");
	glutMainLoop();
	return 0;
}