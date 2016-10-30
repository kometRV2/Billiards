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
#include "Calc.hpp"

#define TEXWIDTH  256                      // テクスチャの幅
#define TEXHEIGHT 256                      // テクスチャの高さ

enum Step { Aim, Move };                    //現在の操作のステップ
Step step = Aim;

/*****************************************************************************
 *  Class CBall
 ******************************************************************************/
class CBall
{							// Class of Ball
public:                     // coordinates of ball
    double	x, y, z;        //位置
    double	radius;         //半径

    double	velocity_x;     //x方向の速さ
    double	velocity_z;     //x方向の速さ
    
    double	tmp_velocity;   //衝突時に計算される一時速度
    
    float 	ambient[4], diffuse[4], specular[4], shininess; //マテリアルの設定
    
    bool exist = true;              //ボールの存在判定用
    
    void	set(double x, double y, double z, double radius);     //ボールのパラメータ
    void	material(float ambient[], float diffuse[], float specular[],
                     float shininess);              //マテリアルの初期化
    void	step();         //球の移動処理
    void	draw();         //球の描画
};

/*---------------------------------------------------------------------------
 * Set Initial values to pendulum state
 *---------------------------------------------------------------------------*/
void CBall::set(double x0, double y0, double z0, double r)
{
    x = x0;
    y = y0;
    z = z0;
    radius = r;
}
/*----------------------------------------------------------------------------
 * Set material parameters
 *---------------------------------------------------------------------------*/
void CBall::material(float a[], float d[], float s[], float shin)
{
    for (int i = 0; i < 4; i++){
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
    velocity_x *= 0.99;
    velocity_z *= 0.99;
    double velocity = sqrt(velocity_x * velocity_x + velocity_z * velocity_z);

    x += velocity_x;
    z += velocity_z;
    if (velocity < 0.005) {
        velocity_x = 0;
        velocity_z = 0;
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

/*全部止まってるかの判定、全部止まっていればエイム状態に移行*/
void count_Speed()
{
    int balls_move_count = 0;   //動いてる球の数
    double velocity[BALL_NUM];
    
    if (step == Move) {
        for (int i = 0; i < BALL_NUM; i++){
            velocity[i] = sqrt(balls[i].velocity_x * balls[i].velocity_x + balls[i].velocity_z * balls[i].velocity_z);
            
            if (velocity[i] > 0.005){
                balls_move_count += 1;
            }
        }
    }
    if (balls_move_count > 0) {
        step = Move;
    }else{
        step = Aim;
        balls_move_count = 0;
        /*打つ球が落ちたらテーブルの真ん中に置き直す*/
        if (balls[0].exist == false) {
            balls[0].x = 0,balls[0].y = Ball_Radius,balls[0].z = -DISTANCE / 2;
            balls[0].exist = true;
        }
    }
}
/*壁との衝突判定*/
void collision_wall(){
    double balls_WallL_dis[BALL_NUM],balls_WallR_dis[BALL_NUM],balls_WallF_dis[BALL_NUM],balls_WallB_dis[BALL_NUM];
    for (int i = 0; i < BALL_NUM; i++) {
        balls_WallL_dis[i] = fabs(balls[i].x + WIDTH - 1.0);
        balls_WallR_dis[i] = fabs(WIDTH - 1.0 - balls[i].x );
        if (balls_WallL_dis[i]  <= Ball_Radius  ) {
            balls[i].x = -4.5;
            balls[i].velocity_x *= -1;
        }
        if ( balls_WallR_dis[i] <= Ball_Radius  ) {
            balls[i].x = 4.5;
            balls[i].velocity_x *= -1;
        }
        balls_WallF_dis[i] = fabs(balls[i].z + DISTANCE - 1);
        balls_WallB_dis[i] = fabs(- 1 - balls[i].z);
        if (balls_WallF_dis[i]  <= Ball_Radius ) {
            balls[i].z = -18.5;
            balls[i].velocity_z *= -1;
        }
        if (balls_WallB_dis[i] <= Ball_Radius ) {
            balls[i].z = -1.5;
            balls[i].velocity_z *= -1;
        }
    }
}
/*ポケットとの衝突判定を行う、*/
void collision_pocket(){
    double	p_d[POCKET_NUM][BALL_NUM], p_dx[POCKET_NUM][BALL_NUM], p_dz[POCKET_NUM][BALL_NUM];
   
    for (int i = 0; i < POCKET_NUM; i++) {
        for (int j = 0; j < BALL_NUM; j++) {
            
            p_dx[i][j] = holes[i].x - balls[j].x;
            p_dz[i][j] = holes[i].z - balls[j].z;
            p_d[i][j] = sqrt(p_dx[i][j] * p_dx[i][j] + p_dz[i][j] * p_dz[i][j]);
            
            if(p_d[i][j] <= balls[j].radius * 2) {
                balls[j].exist = false;
                balls[j].x = -7.0;
                balls[j].y = 0;
                balls[j].z = -j-3;
                balls[j].velocity_x = 0;
                balls[j].velocity_z = 0;
            }
        }
    }
}
 /*ボール同士の衝突----------------------------*/
void collision_ball()
{
    double dif[BALL_NUM ][BALL_NUM], dif_x[BALL_NUM ][BALL_NUM], dif_z[BALL_NUM ][BALL_NUM];//ボール同士の距離
    double kaku[BALL_NUM ][BALL_NUM];          //ボール間の角度
    double oth_Vx[BALL_NUM],oth_Vz[BALL_NUM];  //衝突時の相手に加わる速度
    double aft_Ux[BALL_NUM ][BALL_NUM],aft_Uz[BALL_NUM ][BALL_NUM]; //衝突時の自分に加わる速度
    
    for (int i = 0;i < BALL_NUM - 1  ; i++) {
        for (int j = i + 1; j < BALL_NUM; j++) {
            /*ぶつかったら場所を少しずらす*/
            dif_x[i][j] = balls[i].x - balls[j].x;
            dif_z[i][j] = balls[i].z - balls[j].z;
            dif[i][j] = sqrt(dif_x[i][j] * dif_x[i][j] + dif_z[i][j] * dif_z[i][j] );
            
            /*球iとｊの衝突開始*/
            if (dif[i][j] < 2 * Ball_Radius) {
                /*番号の小さい方の球を少しずらす*/
                kaku[i][j] = atan2(dif_x[i][j], dif_z[i][j]);
                balls[i].x = balls[j].x + 2 * Ball_Radius * sin(kaku[i][j]);
                balls[i].z = balls[j].z + 2 * Ball_Radius * cos(kaku[i][j]);
                /*i番目の球の分解------------*/
                //↓配列に速度をセット
                Col_cal(balls[i].velocity_x, balls[i].velocity_z, balls[j].velocity_x, balls[j].velocity_z, kaku[i][j]);
                
                aft_Ux[i][j] = velocitys[0];
                aft_Uz[i][j] = velocitys[1];
                oth_Vx[i] = velocitys[2];
                oth_Vz[i] = velocitys[3];
                /*j番目の球の分解------------*/
                dif_x[j][i] = -dif_x[i][j];
                dif_z[j][i] = -dif_z[i][j];
                dif[j][i] = sqrt(dif_x[j][i] * dif_x[j][i] + dif_z[j][i] * dif_z[j][i]);
                kaku[j][i] = atan2(dif_x[j][i], dif_z[j][i]);
                //↓配列に速度をセット
                Col_cal(balls[j].velocity_x, balls[j].velocity_z, balls[i].velocity_x, balls[i].velocity_z, kaku[j][i]);
                
                aft_Ux[j][i] = velocitys[0];
                aft_Uz[j][i] = velocitys[1];
                oth_Vx[j] = velocitys[2];
                oth_Vz[j] = velocitys[3];
                /*以下で最終的な速度を設定-------*/
                balls[i].velocity_x = oth_Vx[i] + aft_Ux[j][i];
                balls[i].velocity_z = oth_Vz[i] + aft_Uz[j][i];
                
                balls[j].velocity_x = oth_Vx[j] + aft_Ux[i][j];
                balls[j].velocity_z = oth_Vz[j] + aft_Uz[i][j];
            }
        }
    }
}

/*ボール同士、壁、ポケットとの衝突判定*/
void checkForCollision()
{
    if (step == Move) {
        collision_ball();   //ボールとの衝突判定
        collision_wall();   //壁との衝突判定
        collision_pocket(); //ポケットとの衝突判定
        count_Speed();      //速度の判定
    }
}

/*ボールとポケットの設置*/
void mySetUpBall()
{
    /*球の配置(set)および色の設定(material)*/
    for (int i = 0; i < BALL_NUM; i++) {
        balls[i].set(balls_set_position[i][0],balls_set_position[i][1],balls_set_position[i][2],balls_set_position[i][3]);
        balls[i].material(amb[i], dif[i], spe[i], 128.0);
    }
    
    /*ポケットの配置(set)および色の設定(material)*/
    for (int i = 0; i < POCKET_NUM; i++) {
        holes[i].set(pockets_set_position[i][0],pockets_set_position[i][1],pockets_set_position[i][2],
                     pockets_set_position[i][3]);
        holes[i].material(amb[10], dif[10], spe[10], 128.0);
    }
}

/* テーブルと木の部分の設定、同じような式が多いので簡略化したい*/
static void myGround(double height)
{
    const static GLfloat ground[4] = { 0.18, 0.55, 0.34, 1.0 };
    const static GLfloat udd[4] = {0.4,0.25,0.2};
    /*テーブル*/
    glBegin(GL_QUADS);
    glNormal3d(0.0, 1.0, 0.0);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, ground);
    glVertex3d(-WIDTH, height, -DISTANCE);
    glVertex3d(-WIDTH, height, 0);
    glVertex3d(WIDTH, height, 0);
    glVertex3d(WIDTH, height, -DISTANCE);
    glEnd();
    
    /*前方の木*/
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
        glNormal3d(0.0, 1.0, 0.0);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, udd);
        glTexCoord2d(0.0, 1.0);
        glVertex3d(-WIDTH , height + 0.1, -DISTANCE );
        glTexCoord2d(1.0, 1.0);
        glVertex3d(-WIDTH , height + 0.1, - DISTANCE + 1);
        glTexCoord2d(1.0, 0.0);
        glVertex3d(WIDTH , height + 0.1, - DISTANCE + 1);
        glTexCoord2d(0.0, 0.0);
        glVertex3d(WIDTH , height + 0.1, -DISTANCE );
    glEnd();
    glDisable(GL_TEXTURE_2D);
    
    /*左の木*/
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
        glNormal3d(0.0, 1.0, 0.0);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, udd);
        glTexCoord2d(0.0, 1.0);
        glVertex3d(-WIDTH , height + 0.1, - DISTANCE + 1 );
        glTexCoord2d(1.0, 1.0);
        glVertex3d(-WIDTH , height + 0.1,   -1);
        glTexCoord2d(1.0, 0.0);
        glVertex3d(-WIDTH + 1 , height + 0.1, -1);
        glTexCoord2d(0.0, 0.0);
        glVertex3d(-WIDTH + 1 , height + 0.1, -DISTANCE + 1 );
    glEnd();
    glDisable(GL_TEXTURE_2D);
    
    /*後方の木*/
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
        glNormal3d(0.0, 1.0, 0.0);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, udd);
        glTexCoord2d(0.0, 1.0);
        glVertex3d(-WIDTH , height + 0.1, -1 );
        glTexCoord2d(1.0, 1.0);
        glVertex3d(-WIDTH , height + 0.1, 0);
        glTexCoord2d(1.0, 0.0);
        glVertex3d(WIDTH , height + 0.1, 0);
        glTexCoord2d(0.0, 0.0);
        glVertex3d(WIDTH , height + 0.1, -1 );
    glEnd();
    glDisable(GL_TEXTURE_2D);
    
    /*右の木*/
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
        glNormal3d(0.0, 1.0, 0.0);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, udd);
        glTexCoord2d(0.0, 1.0);
        glVertex3d(WIDTH - 1 , height + 0.1, -DISTANCE + 1 );
        glTexCoord2d(1.0, 1.0);
        glVertex3d(WIDTH - 1 , height + 0.1, - 1);
        glTexCoord2d(1.0, 0.0);
        glVertex3d(WIDTH , height + 0.1, - 1);
        glTexCoord2d(0.0, 0.0);
        glVertex3d(WIDTH , height + 0.1, -DISTANCE + 1 );
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

static void init(void)
{
    /* テクスチャの読み込みに使う配列 */
    GLubyte udd_tex[TEXHEIGHT][TEXWIDTH][3];
    FILE *udd_fp;
    
    /* テクスチャ画像の読み込み */
    if ((udd_fp = fopen("/Users/oomorimakoto/Desktop/mokume.data", "rb")) != NULL) {
        fread(udd_tex, sizeof udd_tex, 1, udd_fp);
        fclose(udd_fp);
        /* テクスチャの割り当て */
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXWIDTH, TEXHEIGHT, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, udd_tex);
    }
    else {
        perror("/Users/oomorimakoto/Desktop/mokume.data");
    }
    
    /* テクスチャ画像はバイト単位に詰め込まれている */
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    /* テクスチャを拡大・縮小する方法の指定 */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    glClearColor(0.3, 0.5, 0.5, 1.0);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}

float gage_num = -3.5;  //打つ強さ

void gage(){
    static const GLfloat color[] = { 1.0, 1.0, 1.0, 1.0 };
    float gage_bottom = -3.5;
    
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
    glNormal3d(1.0, 1.0, 1.0);
    glBegin(GL_QUADS);
        glVertex2d(3.4, gage_num);
        glVertex2d(2.9, gage_num);
        glVertex2d(2.9, gage_bottom);
        glVertex2d(3.4, gage_bottom);
    glEnd();
}
/*初期化*/
void myInit(char *progname)
{
    int width = 500, height = 500;
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE|GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(0, 0);
    glutCreateWindow(progname);
    glClearColor(0.3, 0.5, 0.5, 1.0);   //背景色
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

 /*次の球の表示*/
void next_ball_disp(){
    int k,l, n = 20;						// 分割数n
    double x1, y1, r1 = 0.15;				// 円周上の座標(x,y)と半径r
    
    for (k = 0; k < BALL_NUM; k++) {
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, Next_ball_color[k]);
        if (balls[k].exist == true) {
            glNormal3d(1.0, 1.0, 1.0);
            glBegin(GL_POLYGON);			// ポリゴンの頂点記述開始
            // 円周上の座標(x,y)を計算して円を描画
            for (l = 0; l < n; l++) {
                x1 = r1 * cos(2.0 * M_PI * ((double)l/n) ) + 0.3 * k + 0.6;
                y1 = r1 * sin(2.0 * M_PI * ((double)l/n) ) + 3.0;
                glVertex3f(x1, y1, 0.0);		// 頂点の座標
            }
        }
        glEnd();							// 頂点の記述終了
    }
}

double	xAngle = 0.0, yAngle = 0.0;	//視点回転用角度
double  shot_Angle = 0;             //ショット方向の角度

/*メイン描画-----------------------------------------------------*/
void myDisplay()
{
    /*光源の位置*/
    const static GLfloat lightpos[] = { 10.0, 10.0, 10.0, 0.0 };

    /* 画面クリア*/
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    
    /*視点の設定*/
    glPushMatrix();
  
    /* 光源の位置を設定*/
    glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
    
   /*次のボールの表示*/
    next_ball_disp();
    
    /*ショットゲージ表示*/
    gage();
 
    /* 視点の移動（物体の方を奥に移す、斜めから見下ろす角度）*/
    glTranslated(0.0, -6.0, -3.0);
    glRotated(45, 1.0, 0.0, 0.0);
    
    /*マウスで縦に回転*/
    glRotated(xAngle, 1.0, 0.0, 0.0);
    
    /*テーブルの中心を基点に回転させている*/
    glTranslated(0.0, 0.0, -DISTANCE / 2);
    glRotated(yAngle, 0.0, 1.0, 0.0);
    glTranslated(0.0, 0.0, DISTANCE / 2);
    
    /*テーブルの描画*/
    myGround(-Ball_Radius);
    
    /*ボールの挙動、描写*/
    for (int i = 0; i < BALL_NUM ; i++) {
        balls[i].step();
        balls[i].draw();
    }
    
    /*ショット方向の線の描画*/
    if (step == Aim) {
        glBegin(GL_LINES);
        glColor3d(1.0, 0.0, 1.0);
        glVertex3d(balls[0].x, balls[0].y, balls[0].z);
        glVertex3d(balls[0].x - 2 * sin(shot_Angle), balls[0].y, balls[0].z - 2 * cos(shot_Angle));
        glEnd();
    }
    
    /*ポケットの描写*/
    for (int i = 0;  i < POCKET_NUM; i++) {
        holes[i].draw();
    }
    
    checkForCollision();     //衝突判定チェック
    glPopMatrix();
    glutSwapBuffers();
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
}//---------------------------------------------------------------

/*エイム時の方向の設定*/
void specialkey(int key,int x,int y)
{
    if (step == Aim) {
        switch (key){
            case GLUT_KEY_LEFT:
                shot_Angle += (3/180.0) * M_PI;
                break;
                
            case GLUT_KEY_RIGHT:
                shot_Angle -= (3/180.0) * M_PI;
                break;
                
            case GLUT_KEY_UP:
                if (gage_num < 1.0) gage_num += 0.2;
                break;
                
            case GLUT_KEY_DOWN:
                if (gage_num > -3.5) gage_num -= 0.2;
                break;
            
            default:break;
        }
    }
    glutPostRedisplay();
}

void myKeyboard(unsigned char key, int x, int y)
{
    double shot_Speed = (2.0/15.0) * gage_num + (2.0/3.0);  //打つ球の速さ、ゲージとリンクさせる
    
    switch (key) {
        case 27:
            exit(0);
            break;
            
        case 's':
             if (step == Aim) {
                balls[0].velocity_x = -shot_Speed * sin(shot_Angle);
                balls[0].velocity_z = -shot_Speed * cos(shot_Angle);

                 step = Move;
             }
            break;
            
        case 'a':
            if (step == Aim) {
                if (balls[0].x > -4.4) {
                    balls[0].x -= 0.2;
                }else{
                    balls[0].x = -4.4;
                }
            }
            break;
            
        case 'd':
            if (step == Aim) {
                if (balls[0].x < 4.4) {
                    balls[0].x += 0.2;
                }else{
                    balls[0].x = 4.4;
                }
            }
            break;
            
        default:
            break;
    }
}

unsigned char	mouseFlag = GL_FALSE;		// 視点回転時のクリック判定用
int				xStart, yStart;				// 視点回転時のクリック位置

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
    int		xdis, ydis; //マウスの移動距離
    double	a = 0.5;    //回転の速さの調整用数値
    
    if (mouseFlag == GL_FALSE) return;
    xdis = x - xStart;
    ydis = y - yStart;
    
    /* マウスの動きと角度の計算*/
    xAngle += (double)ydis * a;
    yAngle += (double)xdis * a;
    if (xAngle > 45) {
        xAngle = 45;
    }else if(xAngle < -35){
        xAngle = -35;
    }
    xStart = x; //スタート位置の更新
    yStart = y;
    glutPostRedisplay();
}

int Dt = 33;

void myTimer(int value){
      glutTimerFunc(Dt, myTimer, 1);
      glutPostRedisplay();
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
    glutReshapeFunc(myReshape);
    glutDisplayFunc(myDisplay);
    mySetUpBall();
    
    init();
    
    glutMainLoop();
    return 0;
}
