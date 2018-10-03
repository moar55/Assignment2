#include "SOIL/SOIL.h"
#include <GL/glut.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include "cmath"
#include <vector>
#include <sstream>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// =========================  NOTE ====================================//
//          Right handed coordinate system used here                   //
// =========================         ==================================//

#define DEG2RAD(a) (a * 0.0174532925)
#define GLUT_KEY_ESCAPE 27
#define RGB2FLT(a) ((double)a / (double)255)

int WIDTH = 1280;
int HEIGHT = 720;

const int PATH_SIZE = 2107;
float angle = 0.0;
float lx = 0.0f, lz = 1.0f;
float x = 0.0f, z = 5.0f;

float *vertexArray;
float *normalArray;
float *uvArray;

int numVerts;

int globalTime = 0;

//Textures
GLuint tex_sky, tex_ground, tex_ball, tex_ball2, tex_ball3, water, underwater;

float poleRotate = 0;
bool polClockwise = true;

bool ballAnimate,sbAnimate, houseAnimate, poleAnimate, patrickAnimate, treeAnimate;

bool playAll = false, startScene = false;
int currIteration = 0;
bool topView = false;



struct BallAnimation{
    float offset = 0;
    bool up = true;
};
BallAnimation ballInitial;
BallAnimation ballAnimation;

struct SpongeBobAnimation{
    float rotAngle =0;
    bool clockwise = false;
    float offset = 0;
    bool front = true;
    float shear = 0;
    bool shearFnt = true;
    int delay= 0;
};

SpongeBobAnimation sbInitial;
SpongeBobAnimation sb;


struct PatrickAnimation{
    float rotAngle =0;
    bool clockwise = true;
    float offset = 0;
    bool front = true;
    int delay= 0;
    float scale = 1;
    float scaleUp = true;
};

PatrickAnimation patrickInitial;
PatrickAnimation patrickAnim;

struct HouseAnimation{
    float grassOffset = 0;
    bool grassRev = false;
    float scale = 1;
    bool scaleUp = true;
};
HouseAnimation houseInitial;
HouseAnimation houseAnim;


struct TreeAnimation{
    float shear = 0;
    bool shearFnt = true;
    float rotAngle =0;
    bool clockwise = true;
};
TreeAnimation treeInitial;
TreeAnimation treeAnim;





char title[] = "3D Model Loader Sample";

// 3D Projection Options
GLdouble fovy = 45.0;
GLdouble aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
GLdouble zNear = 0.1;
GLdouble zFar = 250;

class Vector
{
public:
    GLdouble x, y, z;
    Vector() {}
    Vector(GLdouble _x, GLdouble _y, GLdouble _z) : x(_x), y(_y), z(_z) {}
    //================================================================================================//
    // Operator Overloading; In C++ you can override the behavior of operators for you class objects. //
    // Here we are overloading the += operator to add a given value to all vector coordinates.        //
    //================================================================================================//
    void operator +=(float value)
    {
        x += value;
        y += value;
        z += value;
    }


    Vector operator+(Vector &v) {
        return Vector(x + v.x, y + v.y, z + v.z);
    }

    Vector operator-(Vector &v) {
        return Vector(x - v.x, y - v.y, z - v.z);
    }

    Vector operator*(float n) {
        return Vector(x * n, y * n, z * n);
    }

    Vector operator/(float n) {
        return Vector(x / n, y / n, z / n);
    }

    Vector unit() {
        return *this / sqrt(x * x + y * y + z * z);
    }

    Vector cross(Vector v) {
        return Vector(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
    }
};

Vector Eye(20, 5, 20);
Vector At(0, 0, 0);
Vector Up(0, 1, 0);

class Camera {
public:
    Vector eye, center, up;

    Camera(float eyeX =42.865908, float eyeY = 13.923523, float eyeZ = 16.141670 , float centerX = 41.880978, float centerY = 13.750660, float centerZ = 16.136125 , float upX = 0.0f, float upY = 1.0f, float upZ = 0.0f) {
        eye = Vector(eyeX, eyeY, eyeZ);
        center = Vector(centerX, centerY, centerZ);
        up = Vector(upX, upY, upZ);
    }

    void moveX(float d) {
        Vector right = up.cross(center - eye).unit();
        Vector temp = right * d;
        eye = eye + temp;
        center = center + temp;
        printf("%f %f %f %f %f %f \n", eye.x, eye.y, eye.z, center.x, center.y, center.z);

    }

    void moveY(float d) {
        Vector temp = up.unit() * d;
        eye = eye + temp;
        center = center + temp;
        printf("%f %f %f %f %f %f \n", eye.x, eye.y, eye.z, center.x, center.y, center.z);
    }

    void moveZ(float d) {
        Vector view = (center - eye).unit();
        Vector temp = view *d;
        eye = eye + temp;
        center = center + temp;
        printf("%f %f %f %f %f %f \n", eye.x, eye.y, eye.z, center.x, center.y, center.z);
    }

    void rotateX(float a) {
        Vector view = (center - eye).unit();
        Vector right = up.cross(view).unit();
        Vector temp = view * cos(DEG2RAD(a));
        Vector temp2 = up * sin(DEG2RAD(a));
        view = temp+ temp2;
        up = view.cross(right);
        center = eye + view;
        printf("%f %f %f %f %f %f \n", eye.x, eye.y, eye.z, center.x, center.y, center.z);

    }

    void rotateY(float a) {
        Vector view = (center - eye).unit();
        Vector right = up.cross(view).unit();
        Vector temp = view * cos(DEG2RAD(a));
        Vector temp2 = right * sin(DEG2RAD(a));
        view = temp + temp2;
        right = view.cross(up);
        center = eye + view;
        printf("%f %f %f %f %f %f \n", eye.x, eye.y, eye.z, center.x, center.y, center.z);
    }

    void look() {
        gluLookAt(
                eye.x, eye.y, eye.z,
                center.x, center.y, center.z,
                up.x, up.y, up.z
        );
    }
};

Camera camera;
Camera path[PATH_SIZE];


int cameraZoom = 0;

// Model Variables
//Model_3DS model_house;
//Model_3DS model_tree;


//=======================================================================
// Lighting Configuration Function
//=======================================================================
void InitLightSource()
{
    // Enable Lighting for this OpenGL Program
    glEnable(GL_LIGHTING);

    // Enable Light Source number 0
    // OpengL has 8 light sources
    glEnable(GL_LIGHT0);

    // Define Light source 0 ambient light
    GLfloat ambient[] = { 0.1f, 0.1f, 0.1, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

    // Define Light source 0 diffuse light
    GLfloat diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

    // Define Light source 0 Specular light
    GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

    // Finally, define light source 0 position in World Space
    GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

//=======================================================================
// Material Configuration Function
//======================================================================
void InitMaterial()
{
    // Enable Material Tracking
    glEnable(GL_COLOR_MATERIAL);

    // Sich will be assigneet Material Properties whd by glColor
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // Set Material's Specular Color
    // Will be applied to all objects
    GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);

    // Set Material's Shine value (0->128)
    GLfloat shininess[] = { 96.0f };
    glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}

//=======================================================================
// OpengGL Configuration Function
//=======================================================================
void Setup(void)
{
    glClearColor(0.0, 0.0, 0.0, 0.0);

    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();

    aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;

    gluPerspective(fovy, aspectRatio, zNear, zFar);
    //*******************************************************************************************//
    // fovy:			Angle between the bottom and top of the projectors, in degrees.			 //
    // aspectRatio:		Ratio of width to height of the clipping plane.							 //
    // zNear and zFar:	Specify the front and back clipping planes distances from camera.		 //
    //*******************************************************************************************//

    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();

//    Vector Eye = camera.eye, At = camera.center, Up = camera.up;
//
//    gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
    camera.look();
    //*******************************************************************************************//
    // EYE (ex, ey, ez): defines the location of the camera.									 //
    // AT (ax, ay, az):	 denotes the direction where the camera is aiming at.					 //
    // UP (ux, uy, uz):  denotes the upward orientation of the camera.							 //
    //*******************************************************************************************//

    InitLightSource();

    InitMaterial();

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_NORMALIZE);
}

void setupLights() {
    GLfloat ambient[] = { 0.7f, 0.7f, 0.7, 1.0f };
    GLfloat diffuse[] = { 0.6f, 0.6f, 0.6, 1.0f };
    GLfloat specular[] = { 1.0f, 1.0f, 1.0, 1.0f };
    GLfloat shininess[] = { 50 };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, shininess);

    GLfloat lightIntensity[] = { 0.7f, 0.7f, 1, 1.0f };
    GLfloat lightPosition[] = { -7.0f, 6.0f, 3.0f, 0.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightIntensity);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightIntensity);
}

//=======================================================================
// Render Ground Function
//=======================================================================
void RenderGround()
{
    glDisable(GL_LIGHTING);	// Disable lighting

    glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

    glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

    glBindTexture(GL_TEXTURE_2D, tex_ground);	// Bind the ground texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

    glPushMatrix();
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);	// Set quad normal direction.
    glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
    glVertex3f(-20, 0, -40);
    glTexCoord2f(5, 0);
    glVertex3f(20, 0, -40);
    glTexCoord2f(5, 5);
    glVertex3f(20, 0, 70);
    glTexCoord2f(0, 5);
    glVertex3f(-20, 0, 70);
    glEnd();
    glPopMatrix();

    glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

    glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

void orthogonalStart()
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(-WIDTH/2, WIDTH/2, -HEIGHT/2, HEIGHT/2);
    glMatrixMode(GL_MODELVIEW);
}

void orthogonalEnd()
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void DrawCircleOnBall() {
    glPushMatrix();
    glTranslatef(1.55,3,-0.4);
    glColor3f(0,0,0);
    glutSolidSphere(0.5,60,60);
    glColor3f(1,1,1);
    glPopMatrix();
}


void DrawHexagonOnBall() {
    glPushMatrix();
    glBegin(GL_POLYGON);
    for(int i = 0; i < 6; ++i) {
        glVertex2d(sin(i/6.0*2*M_PI),
                   cos(i/6.0*2*M_PI));
    }
    glEnd();
    glPopMatrix();
}

void DrawSphere(float radius, int ss) {
    GLUquadricObj * qobj;
    qobj = gluNewQuadric();
    gluSphere(qobj,radius,ss,ss);
    gluDeleteQuadric(qobj);
}

void DrawLeaf() {
    glPushMatrix();
    glColor3f(RGB2FLT(58), RGB2FLT(95), RGB2FLT(11));
    glTranslatef(0,16,20);
    glScalef(1,1.5,2);
    DrawSphere(1.2,50);
    glPopMatrix();
    glColor3f(1,1,1);
}


void DrawEye() {
    glPushMatrix();
    glTranslatef(0.8,7.5,10);
    glRotatef(-90,0,0.5,0);
    GLUquadricObj * qobj;
    qobj = gluNewQuadric();
    gluDisk(qobj,0.2,0.4,50,50);
//    pupil //
    glPushMatrix();
    glColor3f(0,0,0);
    gluDisk(qobj,0,0.2, 50,50);
    glPopMatrix();
    glColor3f(1,1,1);
    glPopMatrix();
    gluDeleteQuadric(qobj);
}

//=======================================================================
// Display Function
//=======================================================================
void myDisplay(void)
{
    Setup();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



    GLfloat lightIntensity[] = { 0.7, 0.7, 0.7, 1.0f };
    GLfloat lightPosition[] = {0.0f, 100.0f, 0.0f, 0.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);



    // Draw Ground
    RenderGround();

//    Ball

    glPushMatrix();
    GLUquadricObj * qobj;
    qobj = gluNewQuadric();
    glTranslated(4,2 + ballAnimation.offset,-8);
    glRotated(120,0,1,0);
    glBindTexture(GL_TEXTURE_2D, tex_ball2);
    gluQuadricTexture(qobj,true);
    gluQuadricNormals(qobj,GL_SMOOTH);
    gluSphere(qobj,1.6,50,50);
    gluDeleteQuadric(qobj);
    glPopMatrix();

    glPushMatrix();
    glTranslated(4,2 + ballAnimation.offset,-8);
    glRotatef(-60,1,0,0);
    glColor3f(1,0,0);
    glutSolidCone(1,3,50,50);
    glPopMatrix();

    glPushMatrix();
    glTranslated(4,2 + ballAnimation.offset,-8);
    glRotatef(-120,1,0,0);
    glutSolidCone(1,3,50,50);
    glPopMatrix();

    glColor3f(1,1,1);

//===================   Tree Start  ============================//
    glDisable(GL_TEXTURE_2D);

//    animation stuff

    glPushMatrix();
    GLfloat m[16] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            treeAnim.shear, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
    };
    glMultMatrixf(m);

    glRotatef(treeAnim.rotAngle, 1, 0, 0);


    //  STEM //
    glPushMatrix();
    qobj = gluNewQuadric();
    glColor3f(RGB2FLT(139), RGB2FLT(69), RGB2FLT(19));
    glTranslated(0,15,20);
    glRotated(90,1,0,0);
    gluCylinder(qobj,2,1.5,15,100,100);
    glPopMatrix();
    glColor3f(1,1,1);

    //Leaves//
    glPushMatrix();
    glTranslatef(0,0,2);
    DrawLeaf();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0,0,-2);
    DrawLeaf();
    glPopMatrix();

    glPushMatrix();

    glTranslatef(2,16,20);
    glRotatef(90,0,1,0);
    glTranslatef(0,-16,-20);
    DrawLeaf();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-2,16,20);
    glRotatef(90,0,1,0);
    glTranslatef(0,-16,-20);
    DrawLeaf();
    glPopMatrix();
    //Leaves End//

    glPopMatrix(); // for animation

//==================== TREE ENDS ===============================//


//==================== SPONGEBOB ================================//

//    animation stuff //
    glPushMatrix();
    glTranslatef(sb.offset,0,0);
    glTranslatef(0,7,9);
    glRotatef(sb.rotAngle,1,0,0);
    glTranslatef(0,-7,-9);

    GLfloat m2[16] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            sb.shear, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
    };
    glMultMatrixf(m2);

    // FACE //
    glPushMatrix();
    glTranslatef(0,7,9);
    glColor3f(RGB2FLT(255), RGB2FLT(247), RGB2FLT(0));
    glScalef(0.5,1,1);
    glutSolidCube(3);
    glPopMatrix();
    glColor3f(1,1,1);


//    Eyes //

//    Left Eye
    DrawEye();

//   Right eye
    glPushMatrix();
    glTranslatef(0,0,-1.6);
    DrawEye();
    glPopMatrix();

//    Aligning coordinates of following objects relative to upper torso //
    glPushMatrix();
    glTranslatef(0,7,9);

//      Nose      //
    glPushMatrix();

    glColor3f(RGB2FLT(237), RGB2FLT(229), RGB2FLT(0));
    qobj = gluNewQuadric();
    glTranslatef(-7.6,0,-9);
    glTranslatef(0,7,9);
    glRotatef(-90,0,1,0);
    glTranslatef(0,-7,-9);
    gluCylinder(qobj,0.17,0.17,0.8,50,50);
    gluDisk(qobj,0,0.17,50,50);
    glPopMatrix();
    glColor3f(1,1,1);


//      Mouth      //

    glPushMatrix();
    glColor3f(0,0,0);
    glScalef(0.05,1,2);
    glTranslatef(15,-0.8,-0.05  );
    glutSolidCube(0.8);
    glColor3f(1,1,1);
    glPopMatrix();

//    SQUARE PANTS!!   //

//    pants
    glPushMatrix();
    glColor3f(RGB2FLT(32), RGB2FLT(5), RGB2FLT(0));
//    alternative color     glColor3f(RGB2FLT(56), RGB2FLT(48), RGB2FLT(35));
    glTranslatef(0,-2.25,0);
    glScalef(0.5,0.5,1);
    glutSolidCube(3);
    glPopMatrix();
    glColor3f(1,1,1);

//  el geeb el eswed el ymeen
    glPushMatrix();
    glColor3f(0,0,0);
    glScalef(0.05,1,1.3);
    glTranslatef(15,-2.5,-0.7 );
    glutSolidCube(0.8);
    glColor3f(1,1,1);
    glPopMatrix();

//  el geeb el eswed el shmal
    glPushMatrix();
    glColor3f(0,0,0);
    glScalef(0.05,1,1.3);
    glTranslatef(15,-2.5,0.68 );
    glutSolidCube(0.8);
    glColor3f(1,1,1);
    glPopMatrix();


//  el karafata el 7amra

    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.8,4.5,9);
    glBegin(GL_TRIANGLES);
    glColor3f(RGB2FLT(111), RGB2FLT(0), RGB2FLT(0));
    glVertex3f(0.0f, -0.4f, 0);
    glVertex3f(0, 0.4, 0.4f);
    glVertex3f(0, 0.4, -0.4f);
    glEnd();
    glPopMatrix();

//    END OF SQUAREPANT //

//  Aligning coordinates of following objects relative to upper torso //
    glPushMatrix();
    glTranslatef(0,7,9);

//    dem legs ;) //

//    left legs
    glColor3f(RGB2FLT(32), RGB2FLT(5), RGB2FLT(0));
    glPushMatrix();
    glTranslatef(0.1,-3.2,1);
    glScalef(1,1.3,1);
    glutSolidCube(0.6);
    glPopMatrix();

    glColor3f(RGB2FLT(255), RGB2FLT(247), RGB2FLT(0));
    glPushMatrix();
    glTranslatef(0.1,-4.5, 1);
    glScalef(0.5,3,0.5);
    glutSolidCube(0.6);
    glPopMatrix();

    glColor3f(RGB2FLT(244), RGB2FLT(244), RGB2FLT(245));
    glPushMatrix();
    glTranslatef(0.1,-5.8, 1);
    glScalef(0.5,1.6,0.5);
    glutSolidCube(0.6);
    glPopMatrix();


//    right legs
 glColor3f(RGB2FLT(32), RGB2FLT(5), RGB2FLT(0));
    glPushMatrix();
    glTranslatef(0.1,-3.2,-1.1);
    glScalef(1,1.3,1);
    glutSolidCube(0.6);
    glPopMatrix();

    glColor3f(RGB2FLT(255), RGB2FLT(247), RGB2FLT(0));
    glPushMatrix();
    glTranslatef(0.1,-4.5,-1.1);
    glScalef(0.5,3,0.5);
    glutSolidCube(0.6);
    glPopMatrix();

    glColor3f(RGB2FLT(244), RGB2FLT(244), RGB2FLT(245));
    glPushMatrix();
    glTranslatef(0.1,-5.8, -1.1);
    glScalef(0.5,1.6,0.5);
    glutSolidCube(0.6);
    glPopMatrix();


//    shoes  //

    glColor3f(0,0,0);
    glPushMatrix();
    glTranslatef(0.2,-6.5,-1.12);
    glScalef(2,1,1);
    glutSolidCube(0.6);
    glPopMatrix();

    glColor3f(0,0,0);
    glPushMatrix();
    glTranslatef(0.2,-6.5,1);
    glScalef(2,1,1);
    glutSolidCube(0.6);
    glPopMatrix();

//    shoulders //

    glPushMatrix();
    glTranslatef(0,-0.8,1.78);
    glScalef(1,1,1.1);
    glColor3f(RGB2FLT(253), RGB2FLT(253), RGB2FLT(253));
    glutSolidCube(0.5);
    glPopMatrix();
    glColor3f(1,1,1);

    glPushMatrix();
//    glScalef(1,1,1.1);
    glTranslatef(0,-0.8,-1.78);
    glColor3f(RGB2FLT(253), RGB2FLT(253), RGB2FLT(253));
    glutSolidCube(0.5);
    glPopMatrix();
    glColor3f(1,1,1);


//    arms    //

    glPushMatrix();
    glColor3f(RGB2FLT(255), RGB2FLT(247), RGB2FLT(0));
    glTranslatef(0,-2,1.79);
    glScalef(0.5,3.5,0.5);
    glutSolidCube(0.6);
    glPopMatrix();
    glColor3f(1,1,1);

    glPushMatrix();
    glColor3f(RGB2FLT(255), RGB2FLT(247), RGB2FLT(0));
    glTranslatef(0,-2,-1.79);
    glScalef(0.5,3.5,0.5);
    glutSolidCube(0.6);
    glPopMatrix();
    glColor3f(1,1,1);

    glPopMatrix();
    glColor3f(1,1,1);
    glPopMatrix(); // pop for animation

    gluDeleteQuadric(qobj);

//================================= END OF SPONGEBOB ====================================//

//================================= PATRICK =============================================//

//    animation stuff
    glPushMatrix();
    glTranslatef(0,0,patrickAnim.offset);
    glTranslatef(-3.5,6.5,-3);
    glRotatef(patrickAnim.rotAngle, 1, 0,0);
    glTranslatef(3.5,-6.5,3);

    glScalef(patrickAnim.scale, patrickAnim.scale, patrickAnim.scale);


    glPushMatrix();
    glTranslatef(0,-1,0);
//  head    //

    glPushMatrix();
    glColor3f(RGB2FLT(255), RGB2FLT(179), RGB2FLT(181));
    glTranslatef(-3.5,7.5,-3);
    glRotatef(-90,1,0,0);
    glScalef(0.5,1,1);
    glutSolidCone(2,5,50,50);
    glPopMatrix();

//    arms    //
    glPushMatrix();
    glTranslatef(-3.5,6.5,-1);
    glScalef(0.7,0.5,1);
    glutSolidCone(2,5,50,50);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-3.5,6.5,-5);
    glScalef(0.7,0.5,1);
    glRotatef(-180,1,0,0);
    glutSolidCone(2,5,50,50);
    glPopMatrix();

//    kersh     //

    glPushMatrix();
    glTranslatef(-3.3,6.5,-3);
    glScalef(0.7,0.8,1);
    glutSolidSphere(2.6,50,50);
    glPopMatrix();


//    bantalooon //

//    https://www.opengl.org/discussion_boards/showthread.php/136467-Hemispheres/page2?s=252a6e5d80c0ce83d8ea02639e39e296

    static GLUquadricObj * q = 0;
    if(!q)
    {
        q = gluNewQuadric();
        gluQuadricDrawStyle(q, GLU_FILL);
    }

    double radius = 2.2;
    double clipEq[2][4] =
            { { radius, 0.0, 0.0, 1.0 }, { -radius, 0.0, 0.0, 1.0 } };


    glPushMatrix();
    glScalef(0.9,0.9,1);
    glTranslatef(-5.7,5.5,-3);
    glTranslatef(radius,0,0);
    glRotatef(-90,0,0,1);
    glTranslatef(-radius,0,0);
    glPushMatrix();
    glColor3f(RGB2FLT(182), RGB2FLT(198), RGB2FLT(3));
    glTranslatef(radius, 0.0, 0.0);
    glEnable(GL_CLIP_PLANE0);
    glClipPlane(GL_CLIP_PLANE0, clipEq[0]);
    gluSphere(q, radius, 50, 50);
    glPopMatrix();
    glPopMatrix();

    glDisable(GL_CLIP_PLANE0);

//    fat7et el bantaloon //
    glPushMatrix();
    glTranslatef(-3.3,2.9,-2.2);
    glRotatef(-18,1,0,0);
    glTranslatef(3.3,-2.4,3);

    glPushMatrix();
    qobj = gluNewQuadric();
    glColor3f(RGB2FLT(182), RGB2FLT(198), RGB2FLT(3));
    glTranslatef(-3.3,2.4,-3);
    glRotatef(-90,1,0,0);
    gluCylinder(qobj, 1.2, 1.2,1,50,50);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-3.3,2.5,-3);
    glRotatef(-90,1,0,0);
    gluDisk(qobj,0,1.2,50,50);
    glPopMatrix();

    glPushMatrix();
    glColor3f(RGB2FLT(255), RGB2FLT(179), RGB2FLT(181));
    glTranslatef(-3.3,2.6,-3);
    glRotatef(90,1,0,0);
    glutSolidCone(1.2,2.5,50,50);
    glPopMatrix();

    glPopMatrix();


//    fat7et el bantaloon 2 //es. I would like to use this thread to gather the various is
    glPushMatrix();
    glTranslatef(-3.3,2.9,-4);
    glRotatef(18,1,0,0);
    glTranslatef(3.3,-2.4,3);

    glPushMatrix();
    qobj = gluNewQuadric();
    glColor3f(RGB2FLT(182), RGB2FLT(198), RGB2FLT(3));
    glTranslatef(-3.3,2.4,-3);
    glRotatef(-90,1,0,0);
    gluCylinder(qobj, 1.2, 1.2,1,50,50);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-3.3,2.5,-3);
    glRotatef(-90,1,0,0);
    gluDisk(qobj,0,1.2,50,50);
    glPopMatrix();

    glPushMatrix();
    glColor3f(RGB2FLT(255), RGB2FLT(179), RGB2FLT(181));
    glTranslatef(-3.3,2.6,-3);
    glRotatef(90,1,0,0);
    glutSolidCone(1.2,2.5,50,50);
    glPopMatrix();

    glPopMatrix();


//    eyes  (beautiful ones ;) )   //

    glColor3f(1,1,1);
    glPushMatrix();
    glTranslatef(-3.5,4.7,-9.5);
    glScalef(0.7,0.7,0.7);
    DrawEye();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-3.5,4.7,-10.5);
    glScalef(0.7,0.7,0.7);
    DrawEye();
    glPopMatrix();


//    mouth   //

    glPushMatrix();
    glColor3f(0,0,0);
    glTranslatef(-2.7,9,-3);
    glScalef(0.05,1,2);
    glutSolidCube(0.8);
    glColor3f(1,1,1);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0,0,0);
    glTranslatef(-3,10.5,-2.5 );
    glScalef(0.05,0.35,0.8);
    glutSolidCube(0.8);
    glColor3f(1,1,1);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0,0,0);
    glTranslatef(-3,10.5,-3.5 );
    glScalef(0.05,0.35,0.8);
    glutSolidCube(0.8);
    glColor3f(1,1,1);
    glPopMatrix();

    glPopMatrix();
    glColor3f(1,1,1);


    glPopMatrix(); // pop for animation stuff
    gluDeleteQuadric(qobj);

//================ END OF PATRICK ==================//


//================= PATRICK's HOURSE ==========================//

    if(!q)
    {
        q = gluNewQuadric();
        gluQuadricDrawStyle(q, GLU_FILL);
    }

    double radius2 = 8;
    double clipEq2[2][4] =
            { { radius2, 0.0, 0.0, 1.0 }, { -radius2, 0.0, 0.0, 1.0 } };


    glPushMatrix();
//    glScalef(0.9,0.9,1);
    glTranslatef(-5.7,0,34);
    glTranslatef(radius2,0,0);
    glRotatef(90,0,0,1);
    glTranslatef(-radius2,0,0);
    glPushMatrix();
    glColor3f(RGB2FLT(103), RGB2FLT(58), RGB2FLT(54));
    glTranslatef(radius2, 0.0, 0.0);
    glEnable(GL_CLIP_PLANE0);
    glClipPlane(GL_CLIP_PLANE0, clipEq2[0]);
    gluSphere(q, radius2, 50, 50);
    glPopMatrix();
    glPopMatrix();


    glDisable(GL_CLIP_PLANE0);

    glPushMatrix();
    glTranslatef(2,8,34);
    glRotatef(poleRotate,0,1,0);
    glTranslatef(-2,-8,-34);
    glPushMatrix();
    glColor3f(RGB2FLT(107), RGB2FLT(103), RGB2FLT(53));
    glTranslatef(2,8,34);

    glPushMatrix();
    qobj = gluNewQuadric();
    glRotatef(-90,1,0,0);
    gluCylinder(qobj,0.5,0.5,4,50,50);
    glPopMatrix();


    glPushMatrix();
    qobj = gluNewQuadric();
    glTranslatef(0,4,-2.0);
    gluCylinder(qobj,0.5,0.5,4,50,50);
    gluDisk(qobj,0,0.5,50,50);
    glTranslatef(0,0,4);
    gluDisk(qobj,0,0.5,50,50);
    glPopMatrix();

    glPopMatrix();

    glPopMatrix();
    glColor3f(1,1,1);

//================= SPONGEBOB'S HOUSE =========================//
    glPushMatrix();
    glTranslatef(-5.7,-7.5,55);
    glTranslatef(radius2,0,0);
    glRotatef(90,0,0,1);
    glTranslatef(-radius2,0,0);
    glScalef(2,0.8,1);
    glPushMatrix();
    glColor3f(RGB2FLT(190), RGB2FLT(77), RGB2FLT(2));
    glTranslatef(radius2, 0.0, 0.0);
    glEnable(GL_CLIP_PLANE0);
    glClipPlane(GL_CLIP_PLANE0, clipEq2[0]);
    gluSphere(q, radius2, 50, 50);
    glPopMatrix();
    glPopMatrix();
    glDisable(GL_CLIP_PLANE0);
    glColor3f(1,1,1);

//    grass

//    animation

    glPushMatrix();
    glColor3f(RGB2FLT(19), RGB2FLT(94), RGB2FLT(9));
    glLineWidth(4);
    for (int i = -90; i <= 90 ; i+=4) {
        glPushMatrix();
        if(i >= 0) {
            glTranslatef(0,0,-houseAnim.grassOffset);
        } else {
            glTranslatef(0,0,houseAnim.grassOffset);
        }
        glTranslatef(3,15,55);
        glRotatef(i,1,0,0);
        glTranslatef(-3,-15,-55);
        glBegin(GL_LINES);
        glVertex3f(3.0f, 15, 55.0f);
        glVertex3f(3.0f, 22, 55.0f);
        glEnd();
        glPopMatrix();
    }
    glColor3f(1,1,1);
    glPopMatrix();

    glPopMatrix();

    glLineWidth(1);

//    gate

    qobj = gluNewQuadric();
    glPushMatrix();
    glColor3f(RGB2FLT(73), RGB2FLT(144), RGB2FLT(181));
    glTranslatef(5.5,6.2,55);
    glRotatef(90,1,0,0);
    gluCylinder(qobj,4,4,6,50,50);
    glPopMatrix();


//    wheel
    glColor3f(RGB2FLT(53), RGB2FLT(108), RGB2FLT(137));
    glPushMatrix();
    glTranslatef(9.6,3,55);
    glRotatef(-90,0,1,0);
    glScalef(houseAnim.scale,houseAnim.scale,houseAnim.scale);
    glutSolidTorus(0.25,1,50,50);
    glPopMatrix();

    glColor3f(RGB2FLT(27), RGB2FLT(63), RGB2FLT(81));
    glPushMatrix();
    glTranslatef(9.6,3,55);
    glRotatef(-90,0,1,0);
    glScalef(houseAnim.scale,houseAnim.scale,houseAnim.scale);
    gluDisk(qobj,0,1,50,50);
    glPopMatrix();

    glColor3f(1,1,1);


//    glPushMatrix();
//    qobj = gluNewQuadric();
//    glTranslatef(-1,4.5,-1.4);
//    glRotatef(70,0,1,0);
//    glScalef(2,1,1);
//    gluDisk(qobj,0,0.5,50,50);
//    gluCylinder(qobj,0.5,0.5,0.5,50,50);
//    glPopMatrix();

//    glPushMatrix();
//    qobj = gluNewQuadric();
//    glTranslatef(0,4,-2.4);
//    glRotatef(70,0,1,0);
//    gluCylinder(qobj,0.5,0.5,0.5,50,50);
//    glPopMatrix();



//sky box
    glEnable(GL_TEXTURE_2D);
    glPushMatrix();
    qobj = gluNewQuadric();
    glTranslated(60,0,0);
    glBindTexture(GL_TEXTURE_2D, underwater);
    gluQuadricTexture(qobj,true);
    gluQuadricNormals(qobj,GL_SMOOTH);
    gluSphere(qobj,120,100,100);
    glPopMatrix();

//    glPushMatrix();
//    qobj = gluNewQuadric();
////    glTranslated(-60,0,0);
//    glBindTexture(GL_TEXTURE_2D, underwater);
//    gluQuadricTexture(qobj,true);
//    gluQuadricNormals(qobj,GL_SMOOTH);
//    gluSphere(qobj,50,100,100);
//    glPopMatrix();

//    glPushMatrix();
//    glTranslated(40,0,0);
//    gluQuadricTexture(qobj,true);
//    gluQuadricNormals(qobj,GL_SMOOTH);
//    gluSphere(qobj,80,100,100);
//    glPopMatrix();

    gluDeleteQuadric(qobj);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(3,GL_FLOAT,0,vertexArray);
    glNormalPointer(GL_FLOAT,0,normalArray);

    glClientActiveTexture(GL_TEXTURE0_ARB);
    glTexCoordPointer(2,GL_FLOAT,0,uvArray);

    glDrawArrays(GL_TRIANGLES,0,numVerts);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glutSwapBuffers();
}

//=======================================================================
// Keyboard Function
//=======================================================================
void myKeyboard(unsigned char key, int x, int y)
{
    float d = 0.2;

    switch (key) {
        case 'r':
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            break;
        case 'f':
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
        case 'w':
            camera.moveY(d);
            break;
        case 's':
            camera.moveY(-d);
            break;
        case 'a':
            camera.moveX(d);
            break;
        case 'd':
            camera.moveX(-d);
            break;
        case 'q':
            camera.moveZ(d);
            break;
        case 'e':
            camera.moveZ(-d);
            break;

//            bool ballAnimate,sbAnimate, houseAnimate, poleAnimate, patrickAnimate;

        case '1':
            if(playAll) {
                ballAnimate = !ballAnimate;
                if(!ballAnimate) ballAnimation = ballInitial;
                break;
            }

        case '2':
            if(playAll) {
                patrickAnimate = !patrickAnimate;
                if (!patrickAnimate) patrickAnim = patrickInitial;
                break;
            }

        case '3':
            if(playAll) {
                sbAnimate = !sbAnimate;
                if (!sbAnimate) sb = sbInitial;
            }
            break;

        case '4':
            if(playAll) {
                treeAnimate = !treeAnimate;
                if (!treeAnimate) treeAnim = treeInitial;
                break;
            }

        case '5':
            if(playAll) {
                poleAnimate = !poleAnimate;
                if (!poleAnimate) {
                    poleRotate = 0;
                    polClockwise = true;
                }
                break;
            }

        case '6':
            if(playAll) {
                houseAnimate = !houseAnimate;
                if (!sbAnimate) houseAnim = houseInitial;
                break;
            }

        case 'p':
            playAll = !playAll;
            ballAnimation = ballInitial;
            patrickAnim = patrickInitial;
            sb = sbInitial;
            treeAnim = treeInitial;
            poleRotate = 0;
            polClockwise = true;
            houseAnim = houseInitial;
            break;

        case 'v':
            startScene = !startScene;
            if(!startScene) {
                camera.eye.x=42.865908, camera.eye.y = 13.923523, camera.eye.z = 16.141670;
                camera.center.x = 41.880978, camera.center.y = 13.750660, camera.center.z = 16.136125;
                camera.look();
                currIteration = 0;
            }
            break;

        case 't':
            topView = !topView;
            if(!topView) {
                camera.eye.x=42.865908, camera.eye.y = 13.923523, camera.eye.z = 16.141670;
                camera.center.x = 41.880978, camera.center.y = 13.750660, camera.center.z = 16.136125;
                camera.look();
            } else {
                currIteration = 0;
                startScene = false;
                camera.eye.x = 20.021183;
                camera.eye.y = 65.848793;
                camera.eye.z = 26.813238;
                camera.center.x = 19.535455;
                camera.center.y = 64.974687;
                camera.center.z = 26.810503;
                camera.look();
            }
            break;
        case GLUT_KEY_ESCAPE:
            exit(EXIT_SUCCESS);
        default:
            break;
    }
    glutPostRedisplay();

}

//=======================================================================
// Special Keyboard Function
//=======================================================================
void Special(int key, int x, int y) {
    float a = 1.0;

    switch (key) {
        case GLUT_KEY_UP:
            camera.rotateX(a);
            break;
        case GLUT_KEY_DOWN:
            camera.rotateX(-a);
            break;
        case GLUT_KEY_LEFT:
            camera.rotateY(a);
            break;
        case GLUT_KEY_RIGHT:
            camera.rotateY(-a);
            break;
    }

    glutPostRedisplay();
}

//=======================================================================
// Motion Function
//=======================================================================
void myMotion(int x, int y)
{
    y = HEIGHT - y;

    if (cameraZoom - y > 0)
    {
        camera.eye.x += -0.1;
        camera.eye.z += -0.1;
    }
    else
    {
        camera.eye.x += 0.1;
        camera.eye.z += 0.1;
    }

    cameraZoom = y;

    glLoadIdentity();	//Clear Model_View Matrix
    Vector Eye = camera.eye, At = camera.center, Up = camera.up;

    gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);	//Setup Camera with modified paramters

    GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glutPostRedisplay();	//Re-draw scene
}

//=======================================================================
// Mouse Function
//=======================================================================
void myMouse(int button, int state, int x, int y)
{
    y = HEIGHT - y;

    if (state == GLUT_DOWN)
    {
        cameraZoom = y;
    }
}

//=======================================================================
// Reshape Function
//=======================================================================
void myReshape(int w, int h)
{
    if (h == 0) {
        h = 1;
    }

    WIDTH = w;
    HEIGHT = h;

    // set the drawable region of the window
    glViewport(0, 0, w, h);

    // set up the projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fovy, (GLdouble)WIDTH / (GLdouble)HEIGHT, zNear, zFar);

    // go back to modelview matrix so we can move the objects about
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    Vector Eye = camera.eye, At = camera.center, Up = camera.up;
    gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
}


void animateBall() {

        if(ballAnimation.offset >= 9) {
            ballAnimation.up = false;
            ballAnimation.offset -= 0.4;
        } else if(ballAnimation.offset <= 0){
            ballAnimation.up = true;
            ballAnimation.offset += 0.4;
        } else {
            ballAnimation.offset = (ballAnimation.up)? ballAnimation.offset += 0.4 : ballAnimation.offset -= 0.4;
        }
}

void sbAnimation1() {
    if(sb.offset >= 3) {
        sb.front = false;
        sb.offset -= 0.1;
    } else if (sb.offset <= -2){
        sb.front= true;
        sb.offset += 0.1;
    } else {
        (sb.front)? sb.offset += 0.1 : sb.offset -= 0.1;
    }

    if (sb.rotAngle >= 20) {
        sb.clockwise = true;
        sb.rotAngle -= 2;
    } else if (sb.rotAngle <= -20) {
        sb.clockwise = false;
        sb.rotAngle += 2;
    } else {
        (!sb.clockwise)? sb.rotAngle+= 2 : sb.rotAngle -= 2;
    }
}


void sbAnimation2 () {
    if(sb.shear >= 1.5) {
        sb.shearFnt = false;
        sb.shear -= 0.2;
    } else if (sb.shear <= -1.5){
        sb.shearFnt= true;
        sb.shear += 0.2;
    } else {
        (sb.shearFnt)? sb.shear += 0.2 : sb.shear -= 0.2;
    }

    if (sb.rotAngle >= 15) {
        sb.clockwise = true;
        sb.rotAngle -= 2;
    } else if (sb.rotAngle <= -15) {
        sb.clockwise = false;
        sb.rotAngle += 2;
    } else {
        (!sb.clockwise)? sb.rotAngle+= 2 : sb.rotAngle -= 2;
    }

}

void sbAnimation() {
    if(sb.delay >= 600 && sb.shear <= 0.2  && sb.shear >= -0.2 && sb.rotAngle <= 2 && sb.rotAngle >= -2)
        sb.delay = 0;

    if(sb.delay <= 300  || sb.offset > 3 ||  sb.offset < -3  || sb.rotAngle > 4 || sb.rotAngle < -4) {
        sbAnimation1();
    } else {
        sbAnimation2();
    }
    sb.delay ++;
}

void cartwheel() {
    if(patrickAnim.rotAngle <= -360) {
        patrickAnim.clockwise = false;
        patrickAnim.front = true;
        patrickAnim.rotAngle += 4;
        patrickAnim.offset += 0.1;
    } else if (patrickAnim.rotAngle >= 360) {
        patrickAnim.clockwise = true;
        patrickAnim.front = false;
        patrickAnim.rotAngle -= 2;
        patrickAnim.offset -= 0.1;

    } else {
        if(patrickAnim.clockwise) {
            patrickAnim.rotAngle -= 2;
            patrickAnim.offset -= 0.1;
        } else {
            patrickAnim.rotAngle += 2;
            patrickAnim.offset += 0.1;
        }
    }
}

void patrickAnimation1() {
    if(patrickAnim.offset >= 2.5) {
        patrickAnim.front = false;
        patrickAnim.offset -= 0.1;
    } else if (patrickAnim.offset <= -2.5){
        patrickAnim.front= true;
        patrickAnim.offset += 0.1;
    } else {
        (patrickAnim.front)? patrickAnim.offset += 0.1 : patrickAnim.offset -= 0.1;
    }

    if(patrickAnim.scale >= 1.5) {
        patrickAnim.scaleUp = false;
        patrickAnim.scale -= 0.05;
    } else if (patrickAnim.scale <= 0.8){
        patrickAnim.scaleUp= true;
        patrickAnim.scale += 0.05;
    } else {
        (patrickAnim.scaleUp)? patrickAnim.scale += 0.05 : patrickAnim.scale -= 0.05  ;
    }
}

void patrickAnimation() {
    if(patrickAnim.delay >= 1000) {
        patrickAnim = patrickInitial;
        patrickAnim.delay = 0;
    }

    if(patrickAnim.delay <= 300) {
        patrickAnimation1();
    } else {
        cartwheel();
    }
    patrickAnim.delay ++;
}

void poleAnimation() {
    if(poleRotate <= -360) {
        poleRotate += 4;
        polClockwise = false;
    } else if (poleRotate  >= 360) {
        poleRotate -= 4;
        polClockwise = true;
    } else {
        (!polClockwise)? poleRotate += 2 : poleRotate -= 2;
    }
}

void houseAnimation() {
    if(houseAnim.grassOffset >= 0.4) {
        houseAnim.grassRev = true;
        houseAnim.grassOffset -= 0.1;
    } else if(houseAnim.grassOffset <= 0){
        houseAnim.grassRev = false;
        houseAnim.grassOffset += 0.1;
    } else {
        (!houseAnim.grassRev)? houseAnim.grassOffset -= 0.1 : houseAnim.grassOffset += 0.1;
    }

    if(houseAnim.scale >= 1.5) {
        houseAnim.scaleUp = false;
        houseAnim.scale -= 0.05;
    } else if (houseAnim.scale <= 0.8){
        houseAnim.scaleUp= true;
        houseAnim.scale += 0.05;
    } else {
        (houseAnim.scaleUp)? houseAnim.scale += 0.05 : houseAnim.scale -= 0.05  ;
    }
}

void treeAnimation() {
    if(treeAnim.shear >= 1.2) {
       treeAnim.shearFnt = false;
       treeAnim.shear -= 0.1;
    } else if (treeAnim.shear <= -1.2){
       treeAnim.shearFnt= true;
       treeAnim.shear += 0.1;
    } else {
        (treeAnim.shearFnt)?treeAnim.shear += 0.1 :treeAnim.shear -= 0.1;
    }

    if (treeAnim.rotAngle >= 15) {
        treeAnim.clockwise = true;
        treeAnim.rotAngle -= 1;
    } else if (treeAnim.rotAngle <= -15) {
        treeAnim.clockwise = false;
        treeAnim.rotAngle += 1;
    } else {
        (!treeAnim.clockwise)? treeAnim.rotAngle += 1 : treeAnim.rotAngle -= 1;
    }

}

void timef(int val) {

    if(playAll) {
        if(ballAnimate)
            animateBall();
        if(sbAnimate)
            sbAnimation();
        if(poleAnimate)
            poleAnimation();
        if(houseAnimate)
            houseAnimation();
        if(treeAnimate)
            treeAnimation();
        if(patrickAnimate)
            patrickAnimation();
    }


    glutPostRedisplay();                        // redraw
    glutTimerFunc(10, timef, 0);                    //recall the time function after 1000
}

void timef2(int val) {
    if(startScene) {
        camera = path[currIteration];
        camera.look();
        currIteration++;
        if(currIteration > PATH_SIZE)
            currIteration = 0;
    }
    glutPostRedisplay();                        // redraw
    glutTimerFunc(40, timef2, 0);                    //recall the time function after 1000

}

//=======================================================================
// Assets Loading Function
//=======================================================================
void LoadAssets()
{
    // Loading Model files
//    model_house.Load("Models/house/house.3DS");
//    model_tree.Load("Models/tree/Tree1.3ds");

    // Loading texture files
    tex_ground = SOIL_load_OGL_texture // load an image file directly as a new OpenGL texture
            (
                    "/home/moar/CLionProjects/3dbigproject/Textures/ground.png",
                    SOIL_LOAD_AUTO,
                    SOIL_CREATE_NEW_ID,
                    SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
            );

    tex_sky = SOIL_load_OGL_texture // load an image file directly as a new OpenGL texture
            (
                    "/home/moar/CLionProjects/3dbigproject/Textures/sky4-jpg.png",
                    SOIL_LOAD_AUTO,
                    SOIL_CREATE_NEW_ID,
                    SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
            );

    tex_ball = SOIL_load_OGL_texture // load an image file directly as a new OpenGL texture
            (
                    "/home/moar/CLionProjects/3dbigproject/Textures/ball.png",
                    SOIL_LOAD_AUTO,
                    SOIL_CREATE_NEW_ID,
                    SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
            );


    tex_ball2 = SOIL_load_OGL_texture // load an image file directly as a new OpenGL texture
            (
                    "/home/moar/CLionProjects/3dbigproject/Textures/ball2.png",
                    SOIL_LOAD_AUTO,
                    SOIL_CREATE_NEW_ID,
                    SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
            );


    tex_ball3 = SOIL_load_OGL_texture // load an image file directly as a new OpenGL texture
            (
                    "/home/moar/CLionProjects/3dbigproject/Textures/ball3.png",
                    SOIL_LOAD_AUTO,
                    SOIL_CREATE_NEW_ID,
                    SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
            );

    water = SOIL_load_OGL_texture // load an image file directly as a new OpenGL texture
            (
                    "/home/moar/CLionProjects/3dbigproject/Textures/water.png",
                    SOIL_LOAD_AUTO,
                    SOIL_CREATE_NEW_ID,
                    SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
            );

    underwater = SOIL_load_OGL_texture // load an image file directly as a new OpenGL texture
            (
                    "/home/moar/CLionProjects/3dbigproject/Textures/underwater.png",
                    SOIL_LOAD_AUTO,
                    SOIL_CREATE_NEW_ID,
                    SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
            );



    printf(SOIL_last_result());
}

using namespace std;

vector<string> split(string str) {
    vector<string> internal;

    std::istringstream iss(str);
    for(std::string str; iss >> str; )
        internal.push_back(str);

    return internal;

}

//=======================================================================
// Main Function
//=======================================================================
int main(int argc, char** argv)
{
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    glutInitWindowSize(WIDTH, HEIGHT);

    glutInitWindowPosition(100, 150);

    glutCreateWindow(title);

    glutDisplayFunc(myDisplay);

    glutKeyboardFunc(myKeyboard);

    glutSpecialFunc(Special);

    glutMotionFunc(myMotion);

    glutMouseFunc(myMouse);

    glutTimerFunc(0,timef,0);

    glutTimerFunc(0,timef2,0);

    glutReshapeFunc(myReshape);

    string line;
    ifstream myfile;
    myfile.open("/home/moar/CLionProjects/3dbigproject/path.txt");

    if(!myfile.is_open()) {
        perror("Error open");
        exit(EXIT_FAILURE);
    }
    int i = 0;
    while(getline(myfile, line)) {
        vector<string> input = split(line);

        Camera temp;
        temp.eye.x = stof(input[0]);
        temp.eye.y = stof(input[1]);
        temp.eye.z = stof(input[2]);

        temp.center.x = stof(input[3]);
        temp.center.y = stof(input[4]);
        temp.center.z = stof(input[5]);

        temp.up.x = 0;
        temp.up.y = 1;
        temp.up.z = 0;

        path[i] = temp;
        i++;
    }

    Setup();

    LoadAssets();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);

    glShadeModel(GL_SMOOTH);
    setupLights();

    glutMainLoop();
}
