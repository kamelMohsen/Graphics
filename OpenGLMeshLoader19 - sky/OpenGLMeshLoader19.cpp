#include "TextureBuilder.h"
#include <cstdlib>
#include <glut.h>
#include <ctime>
#include <stdio.h>
#include <Model_3DS.h>
#include <iostream>
using namespace std;




float BulletSpeed = 0;
float SphereSpeed = 0;
float TSpeed = 0;
float OSize = 0;

class Vector3f {
public:
	float x, y, z;

	Vector3f(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) {
		x = _x;
		y = _y;
		z = _z;
	}

	Vector3f operator+(Vector3f& v) {
		return Vector3f(x + v.x, y + v.y, z + v.z);
	}

	Vector3f operator-(Vector3f& v) {
		return Vector3f(x - v.x, y - v.y, z - v.z);
	}

	Vector3f operator*(float n) {
		return Vector3f(x * n, y * n, z * n);
	}

	Vector3f operator/(float n) {
		return Vector3f(x / n, y / n, z / n);
	}
	//this function gets unit vector
	Vector3f unit() {
		return (*this / sqrt(x * x + y * y + z * z)) / BulletSpeed; //Change the bullet Speed
	}
	//this get the cross product of two vectors
	Vector3f cross(Vector3f v) {
		return Vector3f(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
};
//if lost set true
bool lose = false;
//set true when level 1 is finished
bool level1finish = false;
//set true when level 2 is finished
bool level2finish = false;
//0=third person 1=first person
bool cameraview = 0;
//saves the player score
int playerscore = 0;
//saves the coordinates of the targets
float zArray[40];
float xArray[40];
float yArray[40];
Vector3f Blockade[40];
//saves the state of the target
bool hitcubes[40];
//for rand generattion
int decZ = -30;
float transModel = 0.0f;
bool Intensity = true;
bool sun = true;
float playerLocations[] = { 0,-1,-1.5,-2,-3 };
bool gotHit = false;


Vector3f missiles[80];
bool missileshitobjects[80];
Vector3f missilesdirection[80];

Vector3f Camera1fpos = Vector3f(-1.5f, -1.50f, 9.4f);
Vector3f crosshairs = Vector3f(-1.5f, -1.5f, -9.0f);

int missilecounter = 0;

Model_3DS model_character;
Model_3DS model_weapon;
Model_3DS model_ammo;
bool jump = false;
float jumpDistance = 0;

GLTexture tex_ground1;
GLuint tex1, tex1LowLight, texsun, texmoon;
GLTexture tex_ground2;
GLuint tex2, tex2LowLight;
bool goUp = true;

GLTexture obstacle_before1;
GLTexture obstacle_after1;
GLTexture obstacle_before2;
GLTexture obstacle_after2;
float riseSun = 0;
int rotationangle = 0;
float rotationAngleSun = 0;
bool down = true;//down direction for sun

bool currentenviroment = 0; // 0=Earth 1=tron



void print(int x, int y, char* string)
{
	int len, i;
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0, 1000, 0.0, 600);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	//set the position of the text in the window using the x and y coordinates
	glRasterPos2i(x, y);

	//get the length of the string to display
	len = (int)strlen(string);

	//loop to display character by character
	for (i = 0; i < len; i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
	}
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void Setup() {
	unsigned seed = time(NULL);
	srand(seed);
	missilecounter = 0;
	decZ = -30;

	FILE* fp;
	fp = fopen("input.txt", "r");
	char line[256];

	int temp;
	while (fgets(line, sizeof(line), fp) != NULL)
	{
		if (line[0] != '#')
		{
			sscanf_s(line, "%f\t%f\t%f\t%d\t%f", &BulletSpeed, &SphereSpeed, &TSpeed, &temp, &OSize);
			currentenviroment = temp;
		}
		else
			continue;
	}


	if (currentenviroment == 0) {
		for (int i = 0; i < 20; i++) {
			xArray[i] = -2 + rand() % 2; //x from -2 to -1
			yArray[i] = -2 + rand() % 2; // y from -2 to -1
			zArray[i] = decZ + rand() % 4; // zfrom -30 to 4

			int indexno = rand() % 4;
			Blockade[i].x = playerLocations[indexno];
			Blockade[i].y = -2.5;
			Blockade[i].z = decZ + rand() % 5;


			hitcubes[i] = false;

			decZ = decZ - 18;

		}
		for (int i = 0; i < 40; i++) {
			missilesdirection[i] = Vector3f(0.0f, 0.0f, 0.0f);
			missiles[i] = Vector3f(-1.49f + transModel, -1.535f, 9.2f);
			missileshitobjects[i] = false;
		}
	}
	else {
		for (int i = 0; i < 40; i++) {
			xArray[i] = -2 + rand() % 2; //x from -2 to -1
			yArray[i] = -2 + rand() % 2; // y from -2 to -1
			zArray[i] = decZ + rand() % 4;
			int index = rand() % 4;
			Blockade[i].x = playerLocations[index];
			Blockade[i].y = -2.5;
			Blockade[i].z = -decZ + rand() % 5;
			hitcubes[i] = false;
			decZ = decZ - 18;
		}
		for (int i = 0; i < 80; i++) {
			missilesdirection[i] = Vector3f(0.0f, 0.0f, 0.0f);
			missiles[i] = Vector3f(-1.49f, -1.535f, 9.2f);
			missileshitobjects[i] = false;
		}

	}
}

void setupLights() {

	GLfloat ambient[] = { 0.7f, 0.7f, 0.7, 1.0f };
	GLfloat diffuse[] = { 0.6f, 0.6f, 0.6, 1.0f };
	GLfloat specular[] = { -1.0f, -1.0f, 1.0, 1.0f };
	GLfloat shininess[] = { 100 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);

	GLfloat lightIntensity[] = { 1.0f, 1.0f, 1, 1.0f };
	GLfloat lightPosition[] = { crosshairs.x,crosshairs.y + 0.5,0.0f, 0.0f };
	GLfloat lightdirection[] = { 0.0f,0.0f,1.0f };
	glLightfv(GL_LIGHT2, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, lightdirection);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, lightIntensity);

	GLfloat lightIntensity2[] = { Intensity, Intensity, Intensity, Intensity };
	GLfloat lightPosition2[] = { Camera1fpos.x + 5,Camera1fpos.y + 0.7,0.0f, 0.0f };
	GLfloat lightdirection2[] = { 0.0f,0.0f,-1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition2);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, lightdirection2);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightIntensity2);

	GLfloat lightIntensity3[] = { Intensity, Intensity,Intensity, Intensity };
	GLfloat lightPosition3[] = { Camera1fpos.x - 4,Camera1fpos.y + 0.7,0.0f, 0.0f };
	GLfloat lightdirection3[] = { 0,0,-1 };
	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition3);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, lightdirection3);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, lightIntensity3);

	GLfloat lightIntensity4[] = { Intensity, Intensity, Intensity, Intensity };
	GLfloat lightPosition4[] = { Camera1fpos.x,Camera1fpos.y + 2.5,0.0f, 0.0f };
	GLfloat lightdirection4[] = { 0.0f,0.0f,-1.0f };
	glLightfv(GL_LIGHT3, GL_POSITION, lightPosition4);
	glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, lightdirection4);
	glLightfv(GL_LIGHT3, GL_DIFFUSE, lightIntensity4);


}

void drawenviroment() {
	glDisable(GL_LIGHTING);	// Disable lighting 
		// Dim the ground texture a bit
	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	if (currentenviroment == 0) {
		glBindTexture(GL_TEXTURE_2D, tex_ground1.texture[0]);	// Bind the ground texture
	}
	else {
		glBindTexture(GL_TEXTURE_2D, tex_ground2.texture[0]);
	}

	glPushMatrix();
	glColor3f(1, 1, 1);
	glBegin(GL_QUADS);
	//glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(40, -2.5, -40);
	glTexCoord2f(50, 0);
	glVertex3f(-40, -2.5, -40);
	glTexCoord2f(50, 50);
	glVertex3f(-40, -2.5, 40);
	glTexCoord2f(0, 50);
	glVertex3f(40, -2.5, 40);
	glEnd();
	glPopMatrix();

	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.


	//----------------------------------DRAW GROUND--------------------------------



	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);





	glPushMatrix();

	GLUquadricObj* qobj;
	qobj = gluNewQuadric();

	glTranslated(0, 0, -60);

	if (currentenviroment == 0) {
		if (Intensity)
		{
			glBindTexture(GL_TEXTURE_2D, tex1);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, tex1LowLight);
		}

		glColor3d(1, 1, 1);
	}
	else {
		glBindTexture(GL_TEXTURE_2D, tex2);
		glColor3f(0.0f, 0.5f, 1.0f);
	}
	gluQuadricTexture(qobj, true);
	gluQuadricNormals(qobj, GL_SMOOTH);
	gluSphere(qobj, 100, 10, 10);
	gluDeleteQuadric(qobj);

	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	// Sun and Moon Drawings
	glEnable(GL_TEXTURE_2D);
	//-1.5f, -1.50f, 9.4f

	glPushMatrix();
	GLUquadricObj* qobj1;
	qobj1 = gluNewQuadric();

	if (sun)
	{
		glBindTexture(GL_TEXTURE_2D, texsun);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, texmoon);
	}

	glTranslated(-1.5 + 15, -1.5 + 30 + riseSun, -30);
	gluQuadricTexture(qobj1, true);
	gluQuadricNormals(qobj1, GL_SMOOTH);
	glRotated(rotationAngleSun, 0, 1, 0);
	gluSphere(qobj1, 4, 30, 30);
	gluDeleteQuadric(qobj1);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);




}

void drawobstacles() {
	glEnable(GL_TEXTURE_2D);
	for (int i = 0; i < 20 + (currentenviroment == 0 ? 0 : 20); i++) {
		if (hitcubes[i] == false) {
			for (int j = 0; j < 40 + (currentenviroment == 0 ? 0 : 40); j++) {
				if ((missiles[j].x <= xArray[i] + 0.5f) && (missiles[j].x >= xArray[i] - 0.5f) && (missiles[j].y <= yArray[i] + 0.5f) && (missiles[j].y >= yArray[i] - 0.5f) && (missiles[j].z <= zArray[i] + 0.5f) && (missiles[j].z >= zArray[i] - 0.5f)) {
					hitcubes[i] = true;
					printf("%d --%d\n", hitcubes[i], i);
					if (currentenviroment == 0)
						playerscore += 100;
					else
						playerscore += 200;
					break;

				}
			}
		}

		if (currentenviroment == 0) {
			if (hitcubes[i] == false) {
				glBindTexture(GL_TEXTURE_2D, obstacle_before1.texture[0]);
			}
			else {
				glBindTexture(GL_TEXTURE_2D, obstacle_after1.texture[0]);
			}
			glPushMatrix();

			GLUquadricObj* qobj;
			qobj = gluNewQuadric();
			glColor3f(1.0f, 1.0f, 1.0f);
			glTranslatef(xArray[i], yArray[i], zArray[i]);
			glRotatef(rotationangle, 1, 0, 0);
			gluQuadricTexture(qobj, true);
			gluQuadricNormals(qobj, GL_SMOOTH);
			gluSphere(qobj, OSize, 10, 10);
			gluDeleteQuadric(qobj);
			glPopMatrix();

		}
		else {
			if (hitcubes[i] == false) {
				glBindTexture(GL_TEXTURE_2D, obstacle_before2.texture[0]);
			}
			else {
				glBindTexture(GL_TEXTURE_2D, obstacle_after2.texture[0]);
			}
			glPushMatrix();
			glColor3f(1.0f, 1.0f, 1.0f);
			glTranslatef(xArray[i], yArray[i], zArray[i]);
			glutSolidCube(.5);
			glPopMatrix();



		}
	}



	glDisable(GL_TEXTURE_2D);
}

void drawmissiles() {

	glDisable(GL_LIGHTING);
	for (int i = 0; i < 40 + (currentenviroment == 0 ? 0 : 40); i++) {
		glPushMatrix();
		if (i < missilecounter)
		{
			glTranslatef(missiles[i].x, missiles[i].y, missiles[i].z);
		}
		else
		{
			glTranslatef(missiles[i].x + transModel, missiles[i].y + jumpDistance, missiles[i].z);
		}
		glColor3f(1.0f, 1.0f, 1.0f);


		glRotatef(-90.f, 0, 1, 0);

		glScalef(0.01f, 0.005f, 0.01f);


		model_ammo.Draw();
		glPopMatrix();
	}
	glEnable(GL_LIGHTING);


}

void drawcharacter() {

	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	glColor3f(1.0f, 1.0f, 1.0f);
	glTranslatef(-1.5f + transModel, -1.65f + jumpDistance, 9.1f);
	glRotatef(90.f, 0, 1, 0);
	glScalef(0.0002f, 0.0002f, 0.0002f);
	model_character.Draw();
	glPopMatrix();
	glPushMatrix();
	float cetax = atan(crosshairs.y / crosshairs.z);
	float cetay = atan(crosshairs.x / crosshairs.z);
	glColor3f(1.f, 1.0f, 0.0f);
	glTranslatef(-1.49f + transModel, -1.535f + jumpDistance, 9.2f);
	glRotatef(cetay * 20, 0, 1, 0);
	glRotatef(-90.f, 0, 1, 0);
	glRotatef(cetax * 40, 1, 0, 0);
	glScalef(0.0002f, 0.0001f, 0.0002f);
	model_weapon.Draw();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
}

void drawcrosshairs() {
	glDisable(GL_LIGHTING);
	glPointSize(5);
	glBegin(GL_POINTS);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(crosshairs.x, crosshairs.y, crosshairs.z);
	glEnd();
	glEnable(GL_LIGHTING);
}

void cameramode() {
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_COLOR_MATERIAL);
	if (cameraview == 0) {
		glLoadIdentity();
		gluLookAt(Camera1fpos.x, Camera1fpos.y, Camera1fpos.z, crosshairs.x, crosshairs.y, -1, 0.0f, 1.0f, 0.0f);
	}
	else
	{
		//glTranslatef(-1.49f, -1.535f, 9.2f);
		glLoadIdentity();
		gluLookAt(Camera1fpos.x + transModel * 7.0, -1.535f, 9.1f, crosshairs.x, crosshairs.y, -1, 0.0f, 1.0f, 0.0f);

	}
}
void drawBlockade()
{
	//-1.5f + transModel, -1.65f + jumpDistance, 9.1f
	glEnable(GL_TEXTURE_2D);
	for (int i = 0; i < 20 + (currentenviroment == 0 ? 0 : 20); i++) {

		if (jumpDistance <= 0 && (Blockade[i].z >= 8.7f && Blockade[i].z <= 8.78f))
		{
			//cout << "BLOCKADE IS " << Blockade[i].z << "tRANS IS " << transModel << endl;
			if (Blockade[i].x == 0 && transModel == 0.2f)
			{
				//cout << "BLOCKADE IS " << Blockade[i].z << "tRANS IS " << transModel << endl;
				lose = true; break;

			}
			else if (Blockade[i].x == -1 && transModel == 0.1f)
			{
				//cout << "BLOCKADE IS " << Blockade[i].z << "tRANS IS " << transModel << endl;
				lose = true; break;

			}
			else if (Blockade[i].x == -1.5 && transModel >= -0.08 && transModel <= 0.08)
			{
				//cout << "BLOCKADE IS " << Blockade[i].z << "tRANS IS " << transModel << endl;
				lose = true; break;

			}
			else if (Blockade[i].x == -2 && transModel == -0.1f)
			{
				//cout << "BLOCKADE IS " << Blockade[i].z << "tRANS IS " << transModel << endl;
				lose = true; break;

			}
			else if (Blockade[i].x == -3 && transModel == -0.2f)
			{
				//cout << "BLOCKADE IS " << Blockade[i].z << "tRANS IS " << transModel << endl;
				lose = true; break;

			}
		}


		if (currentenviroment == 0) {
			glBindTexture(GL_TEXTURE_2D, texmoon);
		}
		glPushMatrix();

		GLUquadricObj* qobj;
		qobj = gluNewQuadric();
		glColor3f(1.0f, 1.0f, 1.0f);
		glTranslatef(Blockade[i].x, Blockade[i].y, Blockade[i].z);
		gluQuadricTexture(qobj, true);
		gluQuadricNormals(qobj, GL_SMOOTH);
		gluSphere(qobj, 0.4f, 7, 3);
		gluDeleteQuadric(qobj);
		glPopMatrix();

	}

}

void score() {
	glDisable(GL_LIGHTING);
	glColor3f(1.0f, 0.7f, 0.5f);
	print(15, 580, "Score :");
	char buffer[20];
	print(90, 580, itoa(playerscore, buffer, 10));
	print(15, 550, "Missiles left:");
	print(140, 550, itoa(((currentenviroment == 0) ? 0 : 40) + 40 - missilecounter, buffer, 10));
	print(15, 520, currentenviroment == 0 ? "Earth" : "Tron");
	glEnable(GL_LIGHTING);
}

void winlose() {
	for (int i = 0; i < 20 + (currentenviroment == 1 ? 20 : 0); i++) {
		if (hitcubes[i] == false && zArray[i] > 15) {
			//lose = true;
			//break;
		}
	}

	if (currentenviroment == 0) {
		bool check = true;
		for (int i = 0; i < 20; i++) {
			if (hitcubes[i] == false) {
				check = false;
			}
		}
		level1finish = check;
	}
	else {
		bool check = true;
		for (int i = 0; i < 40; i++) {
			if (hitcubes[i] == false) {
				check = false;
			}
		}
		level2finish = check;

	}
}

void Anim() {
	for (int i = 0; i < 20 + (currentenviroment == 0 ? 0 : 20); i++) {
		zArray[i] += SphereSpeed + (currentenviroment == 0 ? 0 : 0.005);
		Blockade[i].z += TSpeed + (currentenviroment == 0 ? 0 : 0.005);
	}
	for (int i = 0; i < 40 + (currentenviroment == 0 ? 0 : 40); i++) {
		missiles[i] = missiles[i].operator+(missilesdirection[i]);
	}
	if (down)
	{
		riseSun -= 0.03;
		if (riseSun <= -35)
		{
			down = false;
			sun = !sun;
			Intensity = !Intensity;
		}
	}
	else
	{
		riseSun += 0.03;
		if (riseSun >= 0)
		{
			down = true;
		}
	}
	if (jump)
	{
		if (goUp)
		{
			jumpDistance += 0.007;
			if (jumpDistance >= 0.4)
			{
				goUp = false;
			}
		}
		else
		{
			jumpDistance -= 0.007;
			if (jumpDistance <= 0)
			{
				jumpDistance = 0;
				jump = false;
				goUp = true;
			}

		}

	}
	rotationangle++;
	rotationAngleSun++;



	glutPostRedisplay();
}


void Display(void) {



	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	winlose();
	if (lose) {
		print(50, 50, "lose");
		glFlush();
	}
	if (level1finish == true && currentenviroment == 0) {
		currentenviroment = 1;
		Setup();
	}
	if (level2finish) {
		print(50, 50, "Game Finished");
		glFlush();
	}
	setupLights();
	cameramode();
	drawenviroment();
	drawcrosshairs();
	drawobstacles();
	drawmissiles();
	drawcharacter();
	drawBlockade();
	score();

	glFlush();
	glutSwapBuffers();
}


void Key(unsigned char key, int x, int y) {
	switch (key) {
	case 'w':crosshairs.y += 0.17f; break;
	case 's':crosshairs.y -= 0.17f; break;
	case 'a':crosshairs.x -= 0.17f; break;
	case 'd':crosshairs.x += 0.17f; break;
		//case 'l':Intensity = !Intensity; break;
	case '3':
	{
		if (transModel <= 0.2)
		{
			transModel = transModel + 0.1f;
			crosshairs.x -= 0.3;
		}
		break;

	}
	case '1':
	{
		if (transModel >= -0.2)
		{
			transModel = transModel - 0.1f;
			crosshairs.x -= -0.3;
		}
		break;

	}
	case 'x':if (currentenviroment == 0 && missilecounter == 40)
		break;
		if (currentenviroment == 1 && missilecounter == 80)
			break;
		missiles[missilecounter].x = -1.49f + transModel;
		missiles[missilecounter].y = -1.535f + jumpDistance;
		missiles[missilecounter].z = 9.2f;
		missilesdirection[missilecounter] = (crosshairs.operator-(missiles[missilecounter])).unit();
		missilecounter++;
		break;

	case 'z':cameraview = !cameraview; break;
	case '5': {
		if (jump == false)
		{
			jump = true;
		}
		break;

	}

	default:break;


	}

	glutPostRedisplay();
}

void LoadAssets()
{
	model_character.Load("models/product.3ds");
	model_weapon.Load("models/bazooka.3ds");
	model_ammo.Load("models/missile.3ds");

	tex_ground1.Load("Textures/ground.bmp");
	loadBMP(&tex1, "Textures/blu-sky-3.bmp", true);
	loadBMP(&tex1LowLight, "Textures/starsoriginal.bmp", true);
	tex_ground2.Load("Textures/tronground1.bmp");
	loadBMP(&tex2, "Textures/greenn.bmp", true);
	loadBMP(&texmoon, "Textures/moonbackground.bmp", true);
	loadBMP(&texsun, "textures/sun.bmp", true);

	obstacle_before1.Load("Textures/bricks.bmp");
	obstacle_after1.Load("Textures/bricksafter.bmp");
	obstacle_before2.Load("Textures/blueee.bmp");
	obstacle_after2.Load("Textures/greenn.bmp");

}

void My_mouse_routine(int button, int state, int x, int y)
{
	switch (button)
	{
		// rotate
	case GLUT_LEFT_BUTTON: {
		if (state == GLUT_DOWN)
		{
			if (transModel >= -0.2)
			{
				transModel = transModel - 0.1f;
				crosshairs.x -= -0.3;
			}
			break;

		}
		break;
	}
	case  GLUT_RIGHT_BUTTON:
	{
		if (state == GLUT_DOWN)
		{
			if (transModel <= 0.2)
			{
				transModel = transModel + 0.1f;
				crosshairs.x -= 0.3;
			}
			break;


		}
		break;


	}
	}
	glutPostRedisplay();

}


void main(int argc, char** argv) {
	glutInit(&argc, argv);
	glClearColor(0.1, 0.1, 0.1, 1);
	glutInitWindowSize(1000, 600);
	glutInitWindowPosition(150, 150);

	glutCreateWindow("Shooting Game");
	glutDisplayFunc(Display);
	glutIdleFunc(Anim);
	glutKeyboardFunc(Key);
	glutMouseFunc(My_mouse_routine);


	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	LoadAssets();
	Setup();
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);
	glEnable(GL_LIGHT3);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel(GL_SMOOTH);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90.0f, 1000 / 600, 0.1f, 300.0f);


	glEnable(GL_COLOR_MATERIAL);



	glutMainLoop();
}
