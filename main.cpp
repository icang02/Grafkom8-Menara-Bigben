//silahkan isi kodingannya dibawah
#include <stdio.h> // Header File For Standard
#include <stdlib.h>
#include <string.h>
#include <math.h> // Math Library Header File
#include <GL/glut.h>
#include <GL/glu.h> // Header File For The GLu32 Library
#include <GL/gl.h> // Header File For The OpenGL32 Library
#include <time.h>
#include <sys/timeb.h>
#include <assert.h>
#include <fstream>
#include <iostream>

using namespace std;


//====================Men-load Image====================
#ifndef IMAGE_LOADER_H_INCLUDED
#define IMAGE_LOADER_H_INCLUDED
//Represents an image
class Image {
	public:
		Image(char* ps, int w, int h);
		~Image();

		/* An array of the form (R1, G1, B1, R2, G2, B2, ...) indicating the
		 * color of each pixel in image.  Color components range from 0 to 255.
		 * The array starts the bottom-left pixel, then moves right to the end
		 * of the row, then moves up to the next column, and so on.  This is the
		 * format in which OpenGL likes images.
		 */
		char* pixels;
		int width;
		int height;
};
//Reads a bitmap image from file.
Image* loadBMP(const char* filename);
#endif


Image::Image(char* ps, int w, int h) : pixels(ps), width(w), height(h) {}

Image::~Image() {
	delete[] pixels;
}

namespace {
	//Converts a four-character array to an integer, using little-endian form
	int toInt(const char* bytes) {
		return (int)(((unsigned char)bytes[3] << 24) |
					 ((unsigned char)bytes[2] << 16) |
					 ((unsigned char)bytes[1] << 8) |
					 (unsigned char)bytes[0]);
	}

	//Converts a two-character array to a short, using little-endian form
	short toShort(const char* bytes) {
		return (short)(((unsigned char)bytes[1] << 8) |
					   (unsigned char)bytes[0]);
	}

	//Reads the next four bytes as an integer, using little-endian form
	int readInt(ifstream &input) {
		char buffer[4];
		input.read(buffer, 4);
		return toInt(buffer);
	}

	//Reads the next two bytes as a short, using little-endian form
	short readShort(ifstream &input) {
		char buffer[2];
		input.read(buffer, 2);
		return toShort(buffer);
	}

	//Just like auto_ptr, but for arrays
	template<class T>
	class auto_array {
		private:
			T* array;
			mutable bool isReleased;
		public:
			explicit auto_array(T* array_ = NULL) :
				array(array_), isReleased(false) {
			}

			auto_array(const auto_array<T> &aarray) {
				array = aarray.array;
				isReleased = aarray.isReleased;
				aarray.isReleased = true;
			}

			~auto_array() {
				if (!isReleased && array != NULL) {
					delete[] array;
				}
			}

			T* get() const {
				return array;
			}

			T &operator*() const {
				return *array;
			}

			void operator=(const auto_array<T> &aarray) {
				if (!isReleased && array != NULL) {
					delete[] array;
				}
				array = aarray.array;
				isReleased = aarray.isReleased;
				aarray.isReleased = true;
			}

			T* operator->() const {
				return array;
			}

			T* release() {
				isReleased = true;
				return array;
			}

			void reset(T* array_ = NULL) {
				if (!isReleased && array != NULL) {
					delete[] array;
				}
				array = array_;
			}

			T* operator+(int i) {
				return array + i;
			}

			T &operator[](int i) {
				return array[i];
			}
	};
}

//Komentar dari abdi

Image* loadBMP(const char* filename) {
	ifstream input;
	input.open(filename, ifstream::binary);
	assert(!input.fail() || !"Could not find file");
	char buffer[2];
	input.read(buffer, 2);
	assert(buffer[0] == 'B' && buffer[1] == 'M' || !"Not a bitmap file");
	input.ignore(8);
	int dataOffset = readInt(input);

	//Read the header
	int headerSize = readInt(input);
	int width;
	int height;
	switch(headerSize) {
		case 40:
			//V3
			width = readInt(input);
			height = readInt(input);
			input.ignore(2);
			assert(readShort(input) == 24 || !"Image is not 24 bits per pixel");
			assert(readShort(input) == 0 || !"Image is compressed");
			break;
		case 12:
			//OS/2 V1
			width = readShort(input);
			height = readShort(input);
			input.ignore(2);
			assert(readShort(input) == 24 || !"Image is not 24 bits per pixel");
			break;
		case 64:
			//OS/2 V2
			assert(!"Can't load OS/2 V2 bitmaps");
			break;
		case 108:
			//Windows V4
			assert(!"Can't load Windows V4 bitmaps");
			break;
		case 124:
			//Windows V5
			assert(!"Can't load Windows V5 bitmaps");
			break;
		default:
			assert(!"Unknown bitmap format");
	}

	//Read the data
	int bytesPerRow = ((width * 3 + 3) / 4) * 4 - (width * 3 % 4);
	int size = bytesPerRow * height;
	auto_array<char> pixels(new char[size]);
	input.seekg(dataOffset, ios_base::beg);
	input.read(pixels.get(), size);

	//Get the data into the right format
	auto_array<char> pixels2(new char[width * height * 3]);
	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			for(int c = 0; c < 3; c++) {
				pixels2[3 * (width * y + x) + c] =
					pixels[bytesPerRow * y + 3 * x + (2 - c)];
			}
		}
	}

	input.close();
	return new Image(pixels2.release(), width, height);
}
//====================Akhir Men-load Image====================


//========================Konversi Vedtor ke Vec3f========================
#ifndef VEC3F_H_INCLUDED
#define VEC3F_H_INCLUDED

class Vec3f {
	private:
		float v[3];
	public:
		Vec3f();
		Vec3f(float x, float y, float z);

		float &operator[](int index);
		float operator[](int index) const;

		Vec3f operator*(float scale) const;
		Vec3f operator/(float scale) const;
		Vec3f operator+(const Vec3f &other) const;
		Vec3f operator-(const Vec3f &other) const;
		Vec3f operator-() const;

		const Vec3f &operator*=(float scale);
		const Vec3f &operator/=(float scale);
		const Vec3f &operator+=(const Vec3f &other);
		const Vec3f &operator-=(const Vec3f &other);

		float magnitude() const;
		float magnitudeSquared() const;
		Vec3f normalize() const;
		float dot(const Vec3f &other) const;
		Vec3f cross(const Vec3f &other) const;
};

Vec3f operator*(float scale, const Vec3f &v);
std::ostream &operator<<(std::ostream &output, const Vec3f &v);
#endif

Vec3f::Vec3f() {}

Vec3f::Vec3f(float x, float y, float z) {
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

float &Vec3f::operator[](int index) {
	return v[index];
}

float Vec3f::operator[](int index) const {
	return v[index];
}

Vec3f Vec3f::operator*(float scale) const {
	return Vec3f(v[0] * scale, v[1] * scale, v[2] * scale);
}

Vec3f Vec3f::operator/(float scale) const {
	return Vec3f(v[0] / scale, v[1] / scale, v[2] / scale);
}

Vec3f Vec3f::operator+(const Vec3f &other) const {
	return Vec3f(v[0] + other.v[0], v[1] + other.v[1], v[2] + other.v[2]);
}

Vec3f Vec3f::operator-(const Vec3f &other) const {
	return Vec3f(v[0] - other.v[0], v[1] - other.v[1], v[2] - other.v[2]);
}

Vec3f Vec3f::operator-() const {
	return Vec3f(-v[0], -v[1], -v[2]);
}

const Vec3f &Vec3f::operator*=(float scale) {
	v[0] *= scale;
	v[1] *= scale;
	v[2] *= scale;
	return *this;
}

const Vec3f &Vec3f::operator/=(float scale) {
	v[0] /= scale;
	v[1] /= scale;
	v[2] /= scale;
	return *this;
}

const Vec3f &Vec3f::operator+=(const Vec3f &other) {
	v[0] += other.v[0];
	v[1] += other.v[1];
	v[2] += other.v[2];
	return *this;
}

const Vec3f &Vec3f::operator-=(const Vec3f &other) {
	v[0] -= other.v[0];
	v[1] -= other.v[1];
	v[2] -= other.v[2];
	return *this;
}

float Vec3f::magnitude() const {
	return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

float Vec3f::magnitudeSquared() const {
	return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
}

Vec3f Vec3f::normalize() const {
	float m = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	return Vec3f(v[0] / m, v[1] / m, v[2] / m);
}

float Vec3f::dot(const Vec3f &other) const {
	return v[0] * other.v[0] + v[1] * other.v[1] + v[2] * other.v[2];
}

Vec3f Vec3f::cross(const Vec3f &other) const {
	return Vec3f(v[1] * other.v[2] - v[2] * other.v[1],
				 v[2] * other.v[0] - v[0] * other.v[2],
				 v[0] * other.v[1] - v[1] * other.v[0]);
}

Vec3f operator*(float scale, const Vec3f &v) {
	return v * scale;
}

ostream &operator<<(ostream &output, const Vec3f &v) {
	cout << '(' << v[0] << ", " << v[1] << ", " << v[2] << ')';
	return output;
}
//========================Akhir Konversi Vektor ke Vec3f========================

float lastx, lasty;
GLint stencilBits;
static int viewx = 50;
static int viewy = 150;
static int viewz = 450;


//train 2D
//class untuk terain 2D
class Terrain {
private:
	int w; //Width
	int l; //Length
	float** hs; //Heights
	Vec3f** normals;
	bool computedNormals; //Whether normals is up-to-date
public:
	Terrain(int w2, int l2) {
		w = w2;
		l = l2;

		hs = new float*[l];
		for (int i = 0; i < l; i++) {
			hs[i] = new float[w];
		}

		normals = new Vec3f*[l];
		for (int i = 0; i < l; i++) {
			normals[i] = new Vec3f[w];
		}

		computedNormals = false;
	}

	~Terrain() {
		for (int i = 0; i < l; i++) {
			delete[] hs[i];
		}
		delete[] hs;

		for (int i = 0; i < l; i++) {
			delete[] normals[i];
		}
		delete[] normals;
	}

	int width() {
		return w;
	}

	int length() {
		return l;
	}

	//Sets the height at (x, z) to y
	void setHeight(int x, int z, float y) {
		hs[z][x] = y;
		computedNormals = false;
	}

	//Returns the height at (x, z)
	float getHeight(int x, int z) {
		return hs[z][x];
	}

	//Computes the normals, if they haven't been computed yet
	void computeNormals() {
		if (computedNormals) {
			return;
		}

		//Compute the rough version of the normals
		Vec3f** normals2 = new Vec3f*[l];
		for (int i = 0; i < l; i++) {
			normals2[i] = new Vec3f[w];
		}

		for (int z = 0; z < l; z++) {
			for (int x = 0; x < w; x++) {
				Vec3f sum(0.0f, 0.0f, 0.0f);

				Vec3f out;
				if (z > 0) {
					out = Vec3f(0.0f, hs[z - 1][x] - hs[z][x], -1.0f);
				}
				Vec3f in;
				if (z < l - 1) {
					in = Vec3f(0.0f, hs[z + 1][x] - hs[z][x], 1.0f);
				}
				Vec3f left;
				if (x > 0) {
					left = Vec3f(-1.0f, hs[z][x - 1] - hs[z][x], 0.0f);
				}
				Vec3f right;
				if (x < w - 1) {
					right = Vec3f(1.0f, hs[z][x + 1] - hs[z][x], 0.0f);
				}

				if (x > 0 && z > 0) {
					sum += out.cross(left).normalize();
				}
				if (x > 0 && z < l - 1) {
					sum += left.cross(in).normalize();
				}
				if (x < w - 1 && z < l - 1) {
					sum += in.cross(right).normalize();
				}
				if (x < w - 1 && z > 0) {
					sum += right.cross(out).normalize();
				}

				normals2[z][x] = sum;
			}
		}

		//Smooth out the normals
		const float FALLOUT_RATIO = 0.5f;
		for (int z = 0; z < l; z++) {
			for (int x = 0; x < w; x++) {
				Vec3f sum = normals2[z][x];

				if (x > 0) {
					sum += normals2[z][x - 1] * FALLOUT_RATIO;
				}
				if (x < w - 1) {
					sum += normals2[z][x + 1] * FALLOUT_RATIO;
				}
				if (z > 0) {
					sum += normals2[z - 1][x] * FALLOUT_RATIO;
				}
				if (z < l - 1) {
					sum += normals2[z + 1][x] * FALLOUT_RATIO;
				}

				if (sum.magnitude() == 0) {
					sum = Vec3f(0.0f, 1.0f, 0.0f);
				}
				normals[z][x] = sum;
			}
		}

		for (int i = 0; i < l; i++) {
			delete[] normals2[i];
		}
		delete[] normals2;

		computedNormals = true;
	}

	//Returns the normal at (x, z)
	Vec3f getNormal(int x, int z) {
		if (!computedNormals) {
			computeNormals();
		}
		return normals[z][x];
	}
};
//end class
//Inisialiasi pergerakan jam
float pindahx =0.0, pindahy=0.0, pindahz=0.0;
float xpos = 0, ypos = 1.2, zpos = -2.0, xrot = 7, yrot = 0, putary=0; //Rotasi Sudut
float inner, outer; int garisv,garish; //Variabel Torus

const float clockR    = 0.5f,
            clockVol  = 1.0f,

            angle1min = M_PI / 30.0f,

            minStart  = 0.9f,
            minEnd    = 1.0f,

            stepStart = 0.8f,
            stepEnd   = 1.0f;

float angleHour = 0,
      angleMin  = 0,
      angleSec  = 0;

void newLine(float rStart, float rEnd, float angle){
  float c = cos(angle), s = sin(angle);
  glVertex2f( clockR*rStart*c, clockR*rStart*s);
  glVertex2f( clockR*rEnd*c, clockR*rEnd*s);
}

//Function Membuat Bentuk Jarum Jam
void jam (void)
{
  glColor3f(0.0f, 0.0f, 0.0f);
  glLineWidth(1.0f);
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POINT_SMOOTH);
  glEnable(GL_POLYGON_SMOOTH);

  glBegin(GL_LINES);
    for(int i=0; i<60; i++){
      if(i%5){ // normal minute
        if(i%5 == 1)
          glColor3f(0.1f, 0.1f, 0.2f);
        newLine(minStart, minEnd, i*angle1min);
      }else{
        glColor3f(1.0f, 0.0f, 0.0f);
        newLine(stepStart, stepEnd, i*angle1min);
      }
    }
  glEnd();

  glLineWidth(3.0f);
  glBegin(GL_LINES);
    newLine(0.0f, 0.5f, -angleHour+M_PI/2);
    newLine(0.0f, 0.8f, -angleMin+M_PI/2);
  glEnd();

  glLineWidth(1.0f);
  glColor3f(0.9f, 0.0f, 1.0f);
  glBegin(GL_LINES);
    newLine(0.0f, 0.8f, -angleSec+M_PI/2);
  glEnd();
}


//Functin Mengelola Waktu
void TimerFunction(int value){
  struct timeb tb;
  time_t tim=time(0);
  struct tm* t;
  t=localtime(&tim);
  ftime(&tb);

  angleSec = (float)(t->tm_sec+ (float)tb.millitm/1000.0f)/30.0f * M_PI;
  angleMin = (float)(t->tm_min)/30.0f * M_PI + angleSec/60.0f;
  angleHour = (float)(t->tm_hour > 12 ? t->tm_hour-12 : t->tm_hour)/6.0f * M_PI+
              angleMin/12.0f;

  // called if timer event
  // ...advance the state of animation incrementally...
  //rot+=1;
  glutPostRedisplay(); // request redisplay
  glutTimerFunc(33,TimerFunction, 1); // request next timer event
}
//Akhir function mengelola waktu


//Loads a terrain from a heightmap.  The heights of the terrain range from
//-height / 2 to height / 2.
//load terain di procedure inisialisasi
Terrain* loadTerrain(const char* filename, float height) {
	Image* image = loadBMP(filename);
	Terrain* t = new Terrain(image->width, image->height);
	for (int y = 0; y < image->height; y++) {
		for (int x = 0; x < image->width; x++) {
			unsigned char color = (unsigned char) image->pixels[3 * (y
					* image->width + x)];
			float h = height * ((color / 255.0f) - 0.5f);
			t->setHeight(x, y, h);
		}
	}

	delete image;
	t->computeNormals();
	return t;
}


//Membuat tipe data terrain
Terrain* _terrain;
Terrain* _terrainTanah;
Terrain* _terrainAir;

const GLfloat light_ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
const GLfloat light_diffuse[] = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_position[] = { 1.0f, 1.0f, 1.0f, 1.0f };

const GLfloat light_ambient2[] = { 0.3f, 0.3f, 0.3f, 0.0f };
const GLfloat light_diffuse2[] = { 0.3f, 0.3f, 0.3f, 0.0f };

const GLfloat mat_ambient[] = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat high_shininess[] = { 100.0f };

void cleanup() {
	delete _terrain;
	delete _terrainTanah;
}


//Function untuk di display
void drawSceneTanah(Terrain *terrain, GLfloat r, GLfloat g, GLfloat b) {

	float scale = 350.0f / max(terrain->width(), terrain->length());
	glScalef(scale, scale, scale);
	glTranslatef(-(float) (terrain->width()) / 2.5, 0.0f,
			-(float) (terrain->length()) / 2.5);

	glColor3f(r, g, b);
	for (int z = 0; z < terrain->length() - 1; z++) {
        // Process Each Triangle
		//Makes OpenGL draw a triangle at every three consecutive vertices
		glBegin(GL_TRIANGLE_STRIP);
		for (int x = 0; x < terrain->width(); x++) {
			Vec3f normal = terrain->getNormal(x, z);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, terrain->getHeight(x, z), z);
			normal = terrain->getNormal(x, z + 1);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, terrain->getHeight(x, z + 1), z + 1);
		}
		glEnd();
	}
}
unsigned int LoadTextureFromBmpFile(char *filename);
//Function Membuat Bentuk Menara Bigben
void menara()
{
    //kotak yang tinggi
    glPushMatrix();
    glTranslatef(50.0, 75.0, 35.5); // We move the object forward (the model matrix is multiplied by the translation matrix)
    glColor3f(1,0.49,0);
    glScalef(40.0, 155.0, 40.0);
    glutSolidCube(1.0f); //kubus solid
    glPopMatrix();

    //kotak didekat jam
    glPushMatrix();
    glTranslatef(50.0, 125.0, 36.5); // We move the object forward (the model matrix is multiplied by the translation matrix)
    glColor3f(1, 0.49, 0);
    glScalef(43.0, 38.0, 48.0);
    glutSolidCube(1.0f); //kubus solid
    glPopMatrix();

    //limas runcing
    glPushMatrix();
    glTranslatef(50.0,200.5, 35.5); // We move the object forward (the model matrix is multiplied by the translation matrix)
    glRotatef(45.0,0.0,1.0,0.0);
    glRotatef(-90.0,1.0,0.0,0.0);
    glColor3f(1,0.49,0);
    glutSolidCone(1.5, 31.86,4,2); //kerucut
    glPopMatrix();

    //limas besar
    glPushMatrix();
    glTranslatef(50.0, 152.65, 35.5); // We move the object forward (the model matrix is multiplied by the translation matrix)
    glRotatef(45.0,0.0,1.0,0.0);
    glRotatef(-90.0,1.0,0.0,0.0);
    glColor3f(1,0.49,0);
    glutSolidCone(28.5, 50.8, 4, 2); //kerucut
    glPopMatrix();

    // garis hiasan di menara (bawah)
    for(int n=0;n<2;n++)
    {
        for(int t=0;t<8;t++)
        {
            glPushMatrix();
            glTranslatef(33+(t*5), 80.5-(n*45),55); // We move the object forward (the model matrix is multiplied by the translation matrix)
            glScalef(1.0, 20.0, 1.0);
            glColor3f(0.1f, 0.1f, 0.1f);
            glutSolidCube(2); //kerucut
            glPopMatrix();
        }
    }
    // garis hiasan di menara (tengah)
    for(int t=0;t<7;t++)
    {
        glPushMatrix();
        glTranslatef(34+(t*5), 148.5, 57); // We move the object forward (the model matrix is multiplied by the translation matrix)
        glScalef(1.0, 2.0, 1.0);
        glColor3f(0.0f, 0.1f, 0.1f);
        glutSolidCube(2); //kerucut
        glPopMatrix();
    }
    // garis hiasan di menara (atas)
    for(int t=0;t<7;t++)
    {
        glPushMatrix();
        glTranslatef(39+(t*3.5), 173.5, 52); // We move the object forward (the model matrix is multiplied by the translation matrix)
        glScalef(1.0, 5.0, 1.0);
        glColor3f(0.0f, 0.0f, 0.0f);
        glutSolidCube(2); //kerucut
        glPopMatrix();
    }

    for(int n=0;n<2;n++)
    {
        for(int t=0;t<8;t++)
        {
            glPushMatrix();
            glTranslatef(33+(t*5), 80.5-(n*45),16); // We move the object forward (the model matrix is multiplied by the translation matrix)
            glScalef(1.0, 20.0, 1.0);
            glColor3f(0.4f, 0.1f, 0.1f);
            glutSolidCube(2); //kerucut
            glPopMatrix();
        }
    }

    //sekat gedung tengah
    glPushMatrix();
    glTranslatef(50,58,55); // We move the object forward (the model matrix is multiplied by the translation matrix)
    glScalef(20.0, 1.0, 1.0);
    glColor3f(0.1f, 0.1f, 0.1f);
    glutSolidCube(2); //kerucut
    glPopMatrix();

    //sekat gedung bawah
    glPushMatrix();
    glTranslatef(50,12,55); // We move the object forward (the model matrix is multiplied by the translation matrix)
    glScalef(20.0, 1.0, 1.0);
    glColor3f(0.1f, 0.1f, 0.1f);
    glutSolidCube(2); //kerucut
    glPopMatrix();
    //sekat gedung bawah
    glPushMatrix();
    glTranslatef(50,7,55); // We move the object forward (the model matrix is multiplied by the translation matrix)
    glScalef(20.0, 1.0, 1.0);
    glColor3f(0.1f, 0.1f, 0.1f);
    glutSolidCube(2); //kerucut
    glPopMatrix();
    //sekat gedung bawah
    glPushMatrix();
    glTranslatef(50,2,55); // We move the object forward (the model matrix is multiplied by the translation matrix)
    glScalef(20.0, 1.0, 1.0);
    glColor3f(0.1f, 0.1f, 0.1f);
    glutSolidCube(2); //kerucut
    glPopMatrix();
    //sekat gedung atas
    glPushMatrix();
    glTranslatef(50,103,55); // We move the object forward (the model matrix is multiplied by the translation matrix)
    glScalef(20.0, 1.0, 1.0);
    glColor3f(0.1f, 0.1f, 0.1f);
    glutSolidCube(2); //kerucut
    glPopMatrix();

    //bentuk jendela
    for(int z=0; z<3;z++)
    {
        int y=0;
        for(int o=0; o<2;o++)
        {
            glPushMatrix();
            glTranslatef(40+(z*10), 160.5-(o*6),47+y); // We move the object forward (the model matrix is multiplied by the translation matrix)
            glRotatef(45.0,0.0,1.0,0.0);
            glRotatef(-90.0,1.0,0.0,0.0);
            glColor3f(0.9f, 0.9f, 0.9f);
            glutSolidTorus(3,4,5,100); //kerucut
            glPopMatrix();
            y=y+2;
        }
    }

    //kotak dekat jam bagian 2
    glPushMatrix();
    glTranslatef(50.0, 170.0, 35.5); // We move the object forward (the model matrix is multiplied by the translation matrix)
    glColor3f(1,0.49,0);
    glScalef(30.0, 30.0, 30.0);
    glutSolidCube(1.0f); //kubus solid
    glPopMatrix();

    //limas besar
    glPushMatrix();
    glTranslatef(50.0, 185.0, 35.5); // We move the object forward (the model matrix is multiplied by the translation matrix)
    glRotatef(45.0,0.0,1.0,0.0);
    glRotatef(-90.0,1.0,0.0,0.0);
    glColor3f(1,0.49,0);
    glutSolidCone(20.5, 27.86, 4, 2); //kerucut
    glPopMatrix();
}


//Function Membuat Bentuk Jam
void bentukJam(){
     //JAM BIGBEN
     glPushMatrix();
     glTranslatef(50.35, 125, 60); // We move the object forward (the model matrix is multiplied by the translation matrix)
     glColor3f(0.9,0.9,0.9);
     glScalef(38.78, 38.78, 2);
     glutSolidSphere(0.5, 20, 20); //bola
     glPopMatrix();

     //jarum JAM BIGBEN
     glPushMatrix();
     glTranslatef(50.35, 125, 61); // We move the object forward (the model matrix is multiplied by the translation matrix)
     glScalef(30, 30, 30);
     jam();
     glPopMatrix();
}


//Function yang digunakan untuk menampilkan setiap objek yang kita buat
void display(void) {
	glClearStencil(0); //clear the stencil buffer
	glClearDepth(1.0f);
	glClearColor(0.0, 0.6, 0.8, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); //clear the buffers
	glLoadIdentity();
	gluLookAt(viewx-1, viewy, viewz, 0.0, 0.0, 5.0, 0.0, 1.0, 0.0);

	glPushMatrix();
	//glBindTexture(GL_TEXTURE_3D, texture[0]);
	drawSceneTanah(_terrain, 0.3f, 0.9f, 0.0f);
	glPopMatrix();

	glPushMatrix();
	//glBindTexture(GL_TEXTURE_3D, texture[0]);
	drawSceneTanah(_terrainTanah, 0.4f, 0.4f, 0.4f);
	glPopMatrix();

	glPushMatrix();
	//glBindTexture(GL_TEXTURE_3D, texture[0]);
	drawSceneTanah(_terrainAir, 0.0f, 0.2f, 0.5f);
	glPopMatrix();

	bentukJam();
	menara();

    glPushMatrix();
    glTranslatef(0.8,-0.8,1.3);
    glPushMatrix();
    glTranslatef(pindahx,pindahz,pindahy);

    glPopMatrix();
    glPopMatrix();

	glutSwapBuffers();
	glFlush();
}


//Function yang digunakan sebagai tempat menggambar
void init(void) {
	glEnable(GL_DEPTH_TEST); // Enables Depth Testing
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glDepthFunc(GL_LESS); // The Type Of Depth Testing To Do
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
	glDepthFunc(GL_LEQUAL); // The Type Of Depth Testing To Do
	glShadeModel(GL_SMOOTH); // Enable Smooth Shading
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_CULL_FACE);

	_terrain = loadTerrain("assets/heightmap.bmp", 20);
	_terrainTanah = loadTerrain("assets/heightmapTanah.bmp", 20);
	_terrainAir = loadTerrain("assets/heightmapAir.bmp", 20);

}


//Function Tombol Keyboard
void keyboard(unsigned char key, int x, int y) {
	if (key == 'q') {
		viewz += 10;
	}
	if (key == 'e') {
		viewz-=10;
	}
	if (key == 's') {
		viewy-=10;
	}
	if (key == 'w') {
		viewy+=10;
	}
}


//Function Reshape
void reshape(int w, int h) {
    // Viewport transformation
	glViewport(0, 0, (GLsizei) w, (GLsizei) h); // Reset The Current Viewport
	glMatrixMode(GL_PROJECTION); // Select The Projection Matrix
	// We initialize the projection matrix as identity
	glLoadIdentity(); // Reset The Modelview Matrix
	// We define the "viewing volume"
	gluPerspective(60, (GLfloat) w / (GLfloat) h, 0.1, 1000.0); // Calculate The Aspect Ratio Of The Window
	glMatrixMode(GL_MODELVIEW); // Select The Modelview Matrix
}


//Function Main
int main(int argc, char **argv) {
	// Konfigurasi dan Menampilkan Window
    glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_STENCIL | GLUT_DEPTH); //add a stencil buffer to the window
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Big Ben");

	// Fungsi untuk melakukan initialisasi
	init();

    glutTimerFunc(33, TimerFunction, 1);

    // Registrasi Callback Function
	glutDisplayFunc(display);
	glutIdleFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);

	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
	glColorMaterial(GL_FRONT, GL_DIFFUSE);

    // Event Processing Loop
	glutMainLoop();
	return 0;
}