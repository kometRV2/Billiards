/* p8-billiard.cpp
 * Animation for collision of two balls in billiard
 */
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
/* =========== Declaration and Data ========================================*/
const int		Dt	= 33;					// [ms] for timer callback fucntion
const double	Coeff_Of_Restitution = 0.5;	// coeffcient of restitution
int				frameCounter = 0;			// Frame counter to control frame
const int		Frame_Counter_Max = 50;
int				startStopFlag = 0;
const double	Table_Width  = 1.27;
const double	Table_Height = 2.54;
const double	Ball_Radius  = 0.04;

const int 		Image_Width  = 256;
const int 		Image_Height = 256;
unsigned char 	texImage[Image_Width][Image_Height][3];

/*****************************************************************************
*  Class CBall
******************************************************************************/
class CBall{							// Class of Ball
public:
	double	x, y, z;					// coordinates of ball
	double	radius;
	double	theta;
	double	velocity;
	float 	ambient[4], diffuse[4], specular[4], shininess;

	void	set(double x, double y, double z, double radius,
				double theta, double velocity);
	void	material(float ambient[], float diffuse[], float specular[],
					 float shininess);
	void	step();
	void	draw();
};
/*---------------------------------------------------------------------------
 * Set Initial values to pendulum state
 *---------------------------------------------------------------------------*/
void CBall::set(double x0, double y0, double z0, double r, double the, double velo)
{
	x = x0;
	y = y0;
	z = z0;
	radius = r;
	theta = (the/180.0)*M_PI;			// degree to radian
	velocity = velo;
}
/*----------------------------------------------------------------------------
 * Set material parameters
 *---------------------------------------------------------------------------*/
void CBall::material(float a[], float d[], float s[], float shin)
{
	int	 i;
	for (i = 0; i < 4; i++){
		ambient[i] = a[i];
		diffuse[i] = d[i];
		specular[i] = s[i];
	}
	shininess = shin;
}
/*----------------------------------------------------------------------------
 * Calculate position of the ball
 *---------------------------------------------------------------------------*/
void CBall::step()
{
	static	double	c = 0.0005;
	x -= c*Dt*velocity*sin(theta);
	z -= c*Dt*velocity*cos(theta);
}
/*----------------------------------------------------------------------------
 * draw the ball
 *---------------------------------------------------------------------------*/
void CBall::draw()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialf (GL_FRONT, GL_SHININESS, shininess);

	glPushMatrix();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glTranslated(x, y, z);
	glutSolidSphere(radius, 20, 20);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glPopMatrix();
}
/*** End of class CBall **************************************************/

CBall ball1, ball2;


void checkForCollision()
{
	double	d, dx, dz;
	static int flag = 1;

	dx = ball1.x - ball2.x;
	dz = ball1.z - ball2.z;
	d = sqrt(dx*dx + dz*dz);

	if (flag){
		if (d <= (ball1.radius + ball2.radius)){
			ball2.theta = asin(ball1.x/(2*ball1.radius));
			ball1.theta = ball2.theta - M_PI_2;
			ball2.velocity = 0.5*ball1.velocity*(1.0 + Coeff_Of_Restitution);
			ball1.velocity = 0.5*ball1.velocity*(1.0 - Coeff_Of_Restitution);
			flag = 0;
		}
	}
}

void mySetUpBall()
{
	float	amb1[]={1.0, 0.0, 0.0, 1.0};
	float	dif1[]={0.6, 0.1, 0.1, 1.0};
	float	spe1[]={1.0, 1.0, 1.0, 1.0};
	float	amb2[]={0.0, 0.0, 1.0, 1.0};
	float	dif2[]={0.0, 0.0, 0.2, 1.0};
	float	spe2[]={1.0, 1.0, 1.0, 1.0};

	ball1.set(0.04, Ball_Radius, -0.1, Ball_Radius, 0.0, 2.0);
	ball2.set(0.0 , Ball_Radius, -0.5, Ball_Radius, 0.0, 0.0);
	ball1.material(amb1, dif1, spe1, 128.0);
	ball2.material(amb2, dif2, spe2, 128.0);
}

void readPPMImage(char *filename)
{
	FILE *fp;
	int  ch, i;

	if ((fp = fopen(filename, "r")) == NULL){
		fprintf(stderr, "Cannot open ppm file %s\n",filename);
		exit(1);
	}
	for (i = 0; i < 4; i++){ 						// skip the four in header
    	while (1){
			if ((ch = fgetc(fp)) != '#') break;		// skip comment
			fgets((char*)texImage, 1024, fp);   	// dummy read
			while(isspace(ch)) ch = fgetc(fp);  	// skip space
    	}
    	while (!isspace(ch)) ch = fgetc(fp);		// read header
	/* Newline or other single terminator after header may exist. */
    	if (i < 3){
      		while (isspace(ch)) ch = fgetc(fp);		// skip terminator
		}
	}
	fread(texImage, 1, Image_Width*Image_Height*3, fp);	// read RGB data
	fclose(fp);
}

void mySetUpTexture()
{
	readPPMImage("texGreenSheet.ppm");
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Image_Width, Image_Height, 0,
								GL_RGB, GL_UNSIGNED_BYTE, texImage);
}

void greenCloth()
{
	float p0[]={-(float)Table_Width/2.0, 0.0, 0.0},
		  p1[]={ (float)Table_Width/2.0, 0.0, 0.0},
		  p2[]={ (float)Table_Width/2.0, 0.0,-(float)Table_Height},
		  p3[]={-(float)Table_Width/2.0, 0.0,-(float)Table_Height};
	
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3fv(p0);
	glTexCoord2f(2.0, 0.0); glVertex3fv(p1);
	glTexCoord2f(2.0, 2.0); glVertex3fv(p2);
	glTexCoord2f(0.0, 2.0); glVertex3fv(p3);
	glEnd();
}

void myDisplay(void)
{
	int i;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);

	greenCloth();
	glDisable(GL_TEXTURE_2D);

	glPushMatrix();

	ball1.step();
	ball2.step();
	checkForCollision();
	ball1.draw();
	ball2.draw();

	glPopMatrix();
	glutSwapBuffers();
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
}

void myReshape(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (double)width / (double)height, 0.1, 20.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 0.4, 0.5, 0.0, 0.0, -0.6, 0.0, 1.0, 0.0);
}

void myTimer(int value)
{
	if (value == 1) glutTimerFunc(Dt, myTimer, 1);
	if (startStopFlag) {
		glutPostRedisplay();
		frameCounter++;
		if (frameCounter >= Frame_Counter_Max) startStopFlag = 0;
	}
}

void myKeyboard( unsigned char key, int x, int y )
{
	switch (key) {
		case 27:
			exit( 0 );
		case 'g':
			startStopFlag = 1;
			break;
		default:
			break;
	}
}

void mySetLight()
{
	float lightPos0[] = {0.0, 1.0, -0.2, 0.0};
	float diffuse0[] = { 0.5, 0.5, 0.5, 1.0 };
	float specular0[] = { 0.2, 0.2, 0.2, 1.0 };

	float lightPos[] = {0.0, 1.0, -0.2, 1.0};
	float spotDir[]  = {0.0, -1.0, -0.5};
	float ambient[]  = {0.2, 0.2, 0.2, 1.0};
	float diffuse[]  = {1.0, 1.0, 1.0, 1.0};
	float specular[] = {1.0, 1.0, 1.0, 1.0};

/* Set up Light0 */
	glLightfv(GL_LIGHT1, GL_POSITION, lightPos0);
	glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse0 );
	glLightfv( GL_LIGHT0, GL_SPECULAR, specular0 );
/* Light1 is spot light */
	glLightfv(GL_LIGHT1, GL_POSITION, lightPos);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spotDir);

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, specular);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 30.0);
	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 2.0);
	
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
}

void myInit(char *progname)
{
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(0, 0);
	glutCreateWindow(progname);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_NORMALIZE);
	glEnable(GL_AUTO_NORMAL);
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	myInit(argv[0]);
	glutKeyboardFunc(myKeyboard);
	glutTimerFunc(Dt, myTimer, 1);
	glutReshapeFunc(myReshape);
	glutDisplayFunc(myDisplay);
	mySetLight();
	mySetUpBall();
	mySetUpTexture();

	glutMainLoop();
	return 0;
}
