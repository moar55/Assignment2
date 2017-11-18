//#include "lib3ds"
//#include "GLTexture.h"
#include "soil-master/src/SOIL/SOIL.h"
#include <GL/glut.h>
#include <cstdio>
#include "cmath"

#define DEG2RAD(a) (a * 0.0174532925)
#define GLUT_KEY_ESCAPE 27

int WIDTH = 1280;
int HEIGHT = 720;

float angle = 0.0;
float lx = 0.0f, lz = 1.0f;
float x = 0.0f, z = 5.0f;

//Textures
GLuint tex_sky, tex_ground, tex_ball, tex_ball2, tex_ball3;

char title[] = "3D Model Loader Sample";

// 3D Projection Options
GLdouble fovy = 45.0;
GLdouble aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
GLdouble zNear = 0.1;
GLdouble zFar = 100;

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

    Camera(float eyeX =20, float eyeY = 5, float eyeZ = 20, float centerX = 0, float centerY = 0, float centerZ = 0, float upX = 0.0f, float upY = 1.0f, float upZ = 0.0f) {
        eye = Vector(eyeX, eyeY, eyeZ);
        center = Vector(centerX, centerY, centerZ);
        up = Vector(upX, upY, upZ);
    }

    void moveX(float d) {
        Vector right = up.cross(center - eye).unit();
        Vector temp = right * d;
        eye = eye + temp;
        center = center + temp;
    }

    void moveY(float d) {
        Vector temp = up.unit() * d;
        eye = eye + temp;
        center = center + temp;
    }

    void moveZ(float d) {
        Vector view = (center - eye).unit();
        Vector temp = view *d;
        eye = eye + temp;
        center = center + temp;
        printf("the eye is %f %f %f , and the center is %f %f %f \n", eye.x, eye.y, eye.z, center.x, center.y, center.z);
    }

    void rotateX(float a) {
        Vector view = (center - eye).unit();
        Vector right = up.cross(view).unit();
        Vector temp = view * cos(DEG2RAD(a));
        Vector temp2 = up * sin(DEG2RAD(a));
        view = temp+ temp2;
        up = view.cross(right);
        center = eye + view;
    }

    void rotateY(float a) {
        Vector view = (center - eye).unit();
        Vector right = up.cross(view).unit();
        Vector temp = view * cos(DEG2RAD(a));
        Vector temp2 = right * sin(DEG2RAD(a));
        view = temp + temp2;
        right = view.cross(up);
        center = eye + view;
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);


    glBindTexture(GL_TEXTURE_2D, tex_ground);	// Bind the ground texture

    glPushMatrix();
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);	// Set quad normal direction.
    glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
    glVertex3f(-20, 0, -20);
    glTexCoord2f(5, 0);
    glVertex3f(20, 0, -20);
    glTexCoord2f(5, 5);
    glVertex3f(20, 0, 20);
    glTexCoord2f(0, 5);
    glVertex3f(-20, 0, 20);
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

    glPushMatrix();
    GLUquadricObj * qobj;
    qobj = gluNewQuadric();
    glTranslated(4,2,0);
    glRotated(120,0,1,0);
    glBindTexture(GL_TEXTURE_2D, tex_ball2);
    gluQuadricTexture(qobj,true);
    gluQuadricNormals(qobj,GL_SMOOTH);
    gluSphere(qobj,2,50,50);
    gluDeleteQuadric(qobj);
    glPopMatrix();


//    glDisable(GL_TEXTURE_2D);
//    glPushMatrix();
//    glColor3f(0.7,0.7,0.7);
//    glTranslatef(0,3,0);
//    glutSolidSphere(2,50,50);
//    glColor3f(1,1,1);
//    glPopMatrix();
//    glEnable(GL_TEXTURE_2D);


//    DrawCircleOnBall();
//
//    orthogonalStart();
//    glColor3f(1,0,0);
//    DrawHexagonOnBall();
//    glColor3f(1,1,1);
//    orthogonalEnd();
//
//    for (int i = 0; i < 360; i+=50) {
//        glPushMatrix();
//        glRotatef(-i,0,1,0);
//        DrawCircleOnBall();
//        glPopMatrix();
//
//        glPushMatrix();
//        glRotatef(-i,1,1,0);
//        DrawCircleOnBall();
//        glPopMatrix();
//    }
//
////    glPushMatrix();
//    glRotatef(-100,0,1,0);
//    DrawCircleOnBall();
//    glPopMatrix();
////
//    glPushMatrix();
//    glRotatef(-200,0,1,0);
//    DrawCircleOnBall();
//    glPopMatrix();

//    glPushMatrix();
//    glRotatef(-150,0,1,0);
//    DrawCircleOnBall();
//    glPopMatrix();

//    // Draw Tree Model
//    glPushMatrix();
//    glTranslatef(10, 0, 0);
//    glScalef(0.7, 0.7, 0.7);
//    model_tree.Draw();
//    glPopMatrix();
//
//    // Draw house Model
//    glPushMatrix();
//    glRotatef(90.f, 1, 0, 0);
//    model_house.Draw();
//    glPopMatrix();


//sky box
    glPushMatrix();
    qobj = gluNewQuadric();
    glTranslated(50,0,0);
    glRotated(90,1,0,1);
    glBindTexture(GL_TEXTURE_2D, tex_sky);
    gluQuadricTexture(qobj,true);
    gluQuadricNormals(qobj,GL_SMOOTH);
    gluSphere(qobj,100,100,100);
    gluDeleteQuadric(qobj);


    glPopMatrix();



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


    printf(SOIL_last_result());
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

    glutReshapeFunc(myReshape);

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
