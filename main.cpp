//
//  main.cpp
//  Study_R
//
//  Created by 大森誠 on 2016/10/01.
//  Copyright © 2016年 大森誠. All rights reserved.
//

#include <iostream>
#include <stdlib.h>
#include <GLUT/GLUT.h>
#include <math.h>

#define BALL_NUM 10                 //球の数、ナインボール
#define POCKET_NUM 6                 //ポケットの数
#define WIDTH 6                      // 台の横幅の２分の１
#define DISTANCE 20                  // 台の奥行きの長さ
#define Ball_Radius 0.5              // 球の半径

const double  Coeff_Of_Restitution = 0.05;	// 反発係数
double attack = 0.7;                        //反発の強さの調整
double shot_Speed = 30;                      //打つ球の速さ

double start_ball_posx = 0.0;               //初めの球の初期位置
double start_ball_posz = -3.0;

int Dt = 33;                                //33フレーム

unsigned char	mouseFlag = GL_FALSE;		// 視点回転時のクリック判定用
int				xStart, yStart;				// 視点回転時のクリック位置
double			xAngle = 0.0, yAngle = 0.0;	// 視点回転用角度

enum Step { Aim, Move };                    //現在の操作のステップ
Step step = Aim;                            //最初は打てる

//ポケットの見え方の設定
float	amb_p[]={0.0, 0.0, 0.0, 1.0};
float	dif_p[]={0.0, 0.0, 0.0, 1.0};
float	spe_p[]={0.0, 0.0, 0.0, 1.0};
//白球の設定
float	amb0[]={1.0, 0.0, 0.0, 1.0};
float	dif0[]={1.0, 1.0, 1.0, 1.0};
float	spe0[]={1.0, 1.0, 1.0, 1.0};
//他の球の設定
float	amb1[]={0.0, 0.0, 1.0, 1.0};
float	dif1[]={1.0, 1.0, 0.0, 1.0};
float	spe1[]={1.0, 1.0, 1.0, 1.0};
//他の球の設定
float	amb2[]={0.0, 0.0, 1.0, 1.0};
float	dif2[]={0.0, 0.0, 0.3, 1.0};
float	spe2[]={1.0, 1.0, 1.0, 1.0};
//他の球の設定
float	amb3[]={0.0, 0.0, 1.0, 1.0};
float	dif3[]={0.5, 0.0, 0.0, 1.0};
float	spe3[]={1.0, 1.0, 1.0, 1.0};


/*****************************************************************************
 *  Class CBall
 ******************************************************************************/
class CBall
{							// Class of Ball
    public:                 // coordinates of ball
    double	x, y, z;        //位置
    double	radius;         //半径
    double	theta;          //進行方向の角度
    double  tmp_theta;      //衝突時に計算される一時角度
    double	velocity;       //速さ
    double	tmp_velocity;   //衝突時に計算される一時速度
    
    float 	ambient[4], diffuse[4], specular[4], shininess; //マテリアルの設定
    
    bool exist = true;              //ボールの存在判定用
    
    void	set(double x, double y, double z, double radius,
                double theta, double velocity);     //初期化
    void	material(float ambient[], float diffuse[], float specular[],
                     float shininess);              //マテリアルの初期化
    void	step();         //球の移動処理
    void	draw();         //球の描画
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
    
    velocity *= 0.98;
    if (velocity > 0.1) {
        x -= c * Dt*velocity*sin(theta);
        z -= c * Dt*velocity*cos(theta);
    }else{
        velocity = 0;
    }
    
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

CBall balls[BALL_NUM];        //各ボール
CBall holes[POCKET_NUM];      //各ポケット、ポケットとして黒い玉を置いている

//全部止まってるかの判定、全部止まっていればエイム状態に移行
void count_Speed()
{
    int balls_move_count = 0;   //動いてる球の数
    
    if (step == Move) {
        for (int i = 0; i < BALL_NUM; i++){
            if (balls[i].velocity < 0.1){
                balls[i].theta = 0;
            }else{
                balls_move_count += 1;
            }
        }
    }
    if (balls_move_count > 0) {
        step = Move;
    }else{
        step = Aim;
    }

}

//衝突判定用および衝突時の計算式、塊を幾つかの関数に分けることが出来たら見やすくなると思う
void checkForCollision()
{
    //ボール同士の衝突----------------------------
   //  double	d[BALL_NUM ][BALL_NUM - 1], dx[BALL_NUM ][BALL_NUM - 1], dz[BALL_NUM ][BALL_NUM - 1];
    double	d[BALL_NUM - 1][BALL_NUM], dx[BALL_NUM - 1][BALL_NUM], dz[BALL_NUM - 1][BALL_NUM];
    bool collisionflag[BALL_NUM] ;
    
    if (step == Move) {
        for (int i = 0; i < BALL_NUM - 1; i++) {
      //  for (int i = BALL_NUM - 1; i >0; i--) {
            collisionflag[i] = false;
            for (int j = i + 1; j < BALL_NUM; j++) {
        //        for (int j = i - 1; j >= 0; j--) {
                //どちらも存在している場合のみ衝突判定を行う
                if (balls[i].exist && balls[j].exist) {
                    dx[i][j] = balls[i].x - balls[j].x;
                    dz[i][j] = balls[i].z - balls[j].z;
                    d[i][j] = sqrt(dx[i][j] * dx[i][j] + dz[i][j] * dz[i][j]);
                    
                    if(d[i][j] <= balls[i].radius + balls[j].radius) {
                        collisionflag[i] = true;
                        balls[j].theta = atan2(dx[i][j], dz[i][j]);
                        if(fabs(balls[j].theta - balls[i].theta) < 0.001){
                            balls[i].tmp_theta += balls[j].theta;
                        }else if (balls[j].theta > 0) {
                            balls[i].tmp_theta += balls[j].theta - M_PI_2;
                        }else if(balls[j].theta < 0) {
                            balls[i].tmp_theta += balls[j].theta + M_PI_2;
                        }
                        balls[j].velocity = attack * balls[i].velocity * (1.0 + Coeff_Of_Restitution);
                        balls[i].tmp_velocity += attack * balls[i].velocity * (1.0 - Coeff_Of_Restitution);
                    }
                }
            }
            
            //衝突したフレーム以外では速度を変えない
            if (collisionflag[i]) {
                balls[i].theta = balls[i].tmp_theta;
                balls[i].velocity = balls[i].tmp_velocity;
                collisionflag[i] = false;
            }
            //保存した速度は処理後初期化する
            balls[i].tmp_theta = 0;
            balls[i].tmp_velocity = 0;
        }
    
    //壁との衝突判定-----------------------------
        double balls_to_wallL[BALL_NUM],balls_to_wallR[BALL_NUM];
        double balls_to_wallF[BALL_NUM],balls_to_wallB[BALL_NUM];
        for (int i = 0; i < BALL_NUM; i++) {
            balls_to_wallL[i] = fabs(balls[i].x + 5.5);
            balls_to_wallR[i] = fabs(5.5 - balls[i].x);
            if (balls_to_wallL[i]  <= Ball_Radius * 2.5| balls_to_wallR[i] <= Ball_Radius * 2.5) {
                balls[i].theta *= -1;
            }
            balls_to_wallF[i] = fabs(balls[i].z + DISTANCE - 0.5);
            balls_to_wallB[i] = fabs(-0.5 - balls[i].z);
            if (balls_to_wallF[i]  <= Ball_Radius * 2.5| balls_to_wallB[i] <= Ball_Radius * 2.5) {
                balls[i].theta  = M_PI - balls[i].theta;
            }
        }
    //ポケットとの衝突判定-------------------------------------------
        double	p_d[POCKET_NUM][BALL_NUM], p_dx[POCKET_NUM][BALL_NUM], p_dz[POCKET_NUM][BALL_NUM];
    
        for (int i = 0; i < POCKET_NUM; i++) {
            for (int j = i + 1; j < BALL_NUM; j++) {
            
                p_dx[i][j] = holes[i].x - balls[j].x;
                p_dz[i][j] = holes[i].z - balls[j].z;
                p_d[i][j] = sqrt(p_dx[i][j] * p_dx[i][j] + p_dz[i][j] * p_dz[i][j]);
            
                if(p_d[i][j] <= balls[j].radius * 2) {
                    balls[j].exist = false;
                    balls[j].x = 8.0;
                    balls[j].y = 0;
                    balls[j].z = -j;
                    balls[j].velocity = 0;
                }
            }
        }
    }
    count_Speed();      //速度の判定を行っている
}

//ボールを設置する
void mySetUpBall()
{
    balls[0].set(start_ball_posx, Ball_Radius, start_ball_posz, Ball_Radius, 0.0, 0.0);//打つ球
 //   balls[1].set(0.0 , Ball_Radius, -9.0, Ball_Radius, 0.0, 0.0);
    balls[5].set(0.5 , Ball_Radius, -10.0, Ball_Radius, 0.0, 0.0);
    balls[3].set(-0.5 , Ball_Radius, -10.0, Ball_Radius, 0.0, 0.0);
  //  balls[4].set(-1.0 , Ball_Radius, -11.0, Ball_Radius, 0.0, 0.0);
 /*   balls[5].set(0.0 , Ball_Radius, -11.0, Ball_Radius, 0.0, 0.0);
    balls[6].set(1.0 , Ball_Radius, -11.0, Ball_Radius, 0.0, 0.0);
    balls[7].set(-0.5 , Ball_Radius, -12.0, Ball_Radius, 0.0, 0.0);
    balls[8].set(0.5 , Ball_Radius, -12.0, Ball_Radius, 0.0, 0.0);
    balls[9].set(0.0 , Ball_Radius, -13.0, Ball_Radius, 0.0, 0.0);*/

    
    balls[0].material(amb0, dif0, spe0, 128.0);
    balls[1].material(amb1, dif1, spe1, 128.0);
    balls[2].material(amb2, dif2, spe2, 128.0);
    balls[3].material(amb3, dif3, spe3, 128.0);
    balls[4].material(amb2, dif2, spe2, 128.0);
    balls[5].material(amb2, dif2, spe2, 128.0);
    balls[6].material(amb2, dif2, spe2, 128.0);
    balls[7].material(amb2, dif2, spe2, 128.0);
    balls[8].material(amb2, dif2, spe2, 128.0);
    balls[9].material(amb2, dif2, spe2, 128.0);
    
    //ポケットの設置
    holes[0].set(-WIDTH + 1.3, -0.3, -DISTANCE + 1.3, Ball_Radius * 1.2, 0.0, 0.0);
    holes[1].set(-WIDTH + 0.8, -0.3, -DISTANCE/2, Ball_Radius * 1.2, 0.0, 0.0);
    holes[2].set(-WIDTH  + 1.3, -0.3,  - 1.3, Ball_Radius * 1.2, 0.0, 0.0);
    holes[3].set(WIDTH - 1.3, -0.3,  - 1.3, Ball_Radius * 1.2, 0.0, 0.0);
    holes[4].set(WIDTH - 0.8, -0.3, -DISTANCE/2, Ball_Radius * 1.2, 0.0, 0.0);
    holes[5].set(WIDTH - 1.3, -0.3, -DISTANCE + 1.3, Ball_Radius * 1.2, 0.0, 0.0);
    
    //ポケットの色の設定
    for (int i = 0; i < POCKET_NUM; i++) {
        holes[i].material(amb_p, dif_p, spe_p, 128.0);
    }
}

void xyzAxes(double length) //xyz軸を表示させる、本番では非表示にしておく
{
    glBegin(GL_LINES);
    glColor3d(1.0, 0.0, 0.0);   //x軸
    glVertex3d(0.0, 0.0, 0.0);  glVertex3d(length, 0.0, 0.0);
    glColor3d(0.0, 1.0, 0.0);   //y軸
    glVertex3d(0.0, 0.0, 0.0);  glVertex3d(0.0, length, 0.0);
    glColor3d(0.0, 0.0, 1.0);   //z軸
    glVertex3d(0.0, 0.0, 0.0);  glVertex3d(0.0, 0.0, length);
    glEnd();
}

// テーブルと木の部分の設定、同じような式が多いので関数化したい
static void myGround(double height)
{
    const static GLfloat ground[4] = { 0.2, 0.7, 0.3, 1.0 };
    const static GLfloat udd[4] = {0.7,0.5,0.4};
    //テーブル
    glBegin(GL_QUADS);
    glNormal3d(0.0, 1.0, 0.0);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, ground);
    glVertex3d(-WIDTH, height, -DISTANCE);
    glVertex3d(-WIDTH, height, 0);
    glVertex3d(WIDTH, height, 0);
    glVertex3d(WIDTH, height, -DISTANCE);
    glEnd();
    
    //前方の木
    glBegin(GL_QUADS);
    glNormal3d(0.0, 1.0, 0.0);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, udd);
    glVertex3d(-WIDTH , height + 0.1, -DISTANCE );
    glVertex3d(-WIDTH , height + 0.1, - DISTANCE + 1);
    glVertex3d(WIDTH , height + 0.1, - DISTANCE + 1);
    glVertex3d(WIDTH , height + 0.1, -DISTANCE );
    glEnd();
    
    //左の木
    glBegin(GL_QUADS);
    glNormal3d(0.0, 1.0, 0.0);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, udd);
    glVertex3d(-WIDTH , height + 0.1, - DISTANCE + 1 );
    glVertex3d(-WIDTH , height + 0.1,   -1);
    glVertex3d(-WIDTH + 1 , height + 0.1, -1);
    glVertex3d(-WIDTH + 1 , height + 0.1, -DISTANCE + 1 );
    glEnd();
    //後方の木
    glBegin(GL_QUADS);
    glNormal3d(0.0, 1.0, 0.0);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, udd);
    glVertex3d(-WIDTH , height + 0.1, -1 );
    glVertex3d(-WIDTH , height + 0.1, 0);
    glVertex3d(WIDTH , height + 0.1, 0);
    glVertex3d(WIDTH , height + 0.1, -1 );
    glEnd();
    //右の木
    glBegin(GL_QUADS);
    glNormal3d(0.0, 1.0, 0.0);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, udd);
    glVertex3d(WIDTH - 1 , height + 0.1, -DISTANCE + 1 );
    glVertex3d(WIDTH - 1 , height + 0.1, - 1);
    glVertex3d(WIDTH , height + 0.1, - 1);
    glVertex3d(WIDTH , height + 0.1, -DISTANCE + 1 );
    glEnd();
    
    
}

static void init(void)
{
    /* 時間の初期化 */
    //    glutGet(GLUT_ELAPSED_TIME);
    
    /* 乱数の系列 */
    //    srand(time(0));
    
    /* 初期設定 */
    glClearColor(0.2, 0.2, 0.2, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}

void myInit(char *progname)
{
    int width = 500, height = 500;
    
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE|GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(0, 0);
    glutCreateWindow(progname);
    glClearColor(0.3, 0.5, 0.5, 1.0);
    
    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);
    glEnable(GL_AUTO_NORMAL);
}

void myReshape(int width, int height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(70.0, (double)width / (double)height, 0.1, 50.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslated(0.0, 0.0, -5.0);
}


//メイン描画-----------------------------------------------------
void myDisplay()
{
    //光源の位置
   const static GLfloat lightpos[] = { 10.0, 10.0, 10.0, 0.0 };
                                                                 
    // 画面クリア
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    
    //視点の設定
    glPushMatrix();
    
    // 光源の位置を設定
    glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
    

    // 視点の移動（物体の方を奥に移す）
    glTranslated(0.0, -6.0, -3.0);
    glRotated(45, 1.0, 0.0, 0.0);
    
    
   // glColor3d(1.0, 0.0, 0.0);
    glRotated(xAngle, 1.0, 0.0, 0.0);

    //テーブルの中心を基点に回転させている
    glTranslated(0.0, 0.0, -10.0);
    glRotated(yAngle, 0.0, 1.0, 0.0);
    glTranslated(0.0, 0.0, 10.0);
    
  //      xyzAxes(10);
    //テーブルの描画
        myGround(-Ball_Radius);
    
    //ボールの描写
    for (int i = 0; i < BALL_NUM ; i++) {
            balls[i].step();
            balls[i].draw();
        }
    
    //ショット方向の線の描画
    if (step == Aim) {
        glBegin(GL_LINES);
        glColor3d(1.0, 0.0, 1.0);
        glVertex3d(balls[0].x, balls[0].y, balls[0].z);
        glVertex3d(balls[0].x - 2 * sin(balls[0].theta), balls[0].y, balls[0].z - 2 * cos(balls[0].theta));
        glEnd();
    }

        //ポケットの描写
        for (int i = 0;  i < POCKET_NUM; i++) {
            holes[i].draw();
        }
    
        //衝突判定チェック
        checkForCollision();
        std::cout << "W,"<<balls[0].theta * 180/ M_PI << ";"<< "Y,"<< balls[1].theta * 180/ M_PI << ";" << "B,"<< balls[2].theta * 180/ M_PI << ";"<< "R,"<< balls[3].theta * 180/ M_PI << "\n";
//        std::cout << "W,"<< balls[0].velocity << ";"<< "Y,"<< balls[1].velocity << ";" << "B,"<< balls[2].velocity << ";"<< "R,"<< balls[3].velocity << "\n";
//        std::cout << balls[0].theta << ";"<< balls[1].theta << ";" << balls[2].theta << ";"<< balls[3].theta << "\n";
//        std::cout << balls[0].theta << ";"<< balls[1].theta << ";" << balls[2].theta << ";"<< balls[3].theta << "\n";
   
    glPopMatrix();
    
    glutSwapBuffers();
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
}//---------------------------------------------------------------

//エイム時のみ方向を選べる
void specialkey(int key,int x,int y)
{
    if (step == Aim) {
        switch (key){
            case GLUT_KEY_LEFT:
                balls[0].theta += (3/180.0) * M_PI;
   //             balls[0].theta = (int)balls[0].theta % 180;
                break;
                
            case GLUT_KEY_RIGHT:
                balls[0].theta -= (3/180.0) * M_PI;
                break;
                
            default:break;
        }
    }
    glutPostRedisplay();
}

void myKeyboard(unsigned char key, int x, int y)
{
    switch (key) {
        case 27:
            exit(0);
            break;
            
        case 's':
            if (step == Aim) {
                balls[0].velocity = shot_Speed;
                step = Move;
            }
            break;
            
        case 'g':
            break;
            
        case 'a':
            if (balls[0].x > -4.4) {
                balls[0].x -= 0.2;
            }else{
                balls[0].x = -4.4;
            }
            break;
            
        case 'd':
            if (balls[0].x < 4.4) {
                balls[0].x += 0.2;
            }else{
                balls[0].x = 4.4;
            }
            break;
            
        default:
            break;
    }
}

void myMouseFunc(int button, int state, int x, int y)
{
    if ( button == GLUT_LEFT_BUTTON  && state == GLUT_DOWN ){
        xStart = x;
        yStart = y;
        mouseFlag = GL_TRUE;
    }
    else {
        mouseFlag = GL_FALSE;
    }
}

void myMouseMotion(int x, int y)
{
    int		xdis, ydis;
    double	a = 0.5;    //回転の速さの調整用数値
    
    if (mouseFlag == GL_FALSE) return;
    xdis = x - xStart;
    ydis = y - yStart;

    // マウスの動きと角度の計算
    xAngle += (double)ydis *a;
    yAngle += (double)xdis *a;
    if (xAngle > 45) {
        xAngle = 45;
    }else if(xAngle < -35){
        xAngle = -35;
    }
    
    xStart = x;
    yStart = y;
    glutPostRedisplay();
}



void myTimer(int value){
    if (value == 1) glutTimerFunc(Dt, myTimer, 1);
    // if (startStopFlag) {
    glutPostRedisplay();
    //  frameCounter++;
    //  if (frameCounter >= Frame_Counter_Max) startStopFlag = 0;
    //}
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    myInit("ビリヤード");
    glutKeyboardFunc(myKeyboard);
    glutTimerFunc(Dt, myTimer, 1);
    glutMouseFunc(myMouseFunc);
    glutMotionFunc(myMouseMotion);
    glutSpecialFunc(specialkey);
//    glutIdleFunc(myIdle);
    glutReshapeFunc(myReshape);
    glutDisplayFunc(myDisplay);
    mySetUpBall();
    
    init();
    
    glutMainLoop();
    return 0;
}
