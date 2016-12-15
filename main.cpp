//
//  main.cpp
//  Study_R
//
//  Created by 大森誠 on 2016/10/01.
//  Copyright © 2016年 大森誠. All rights reserved.
//

#include <iostream>
#include <time.h>   /* 乱数関数のために使う（時刻情報を利用） */

#include "Calc.hpp"
#include "CBall.hpp"



enum Step { Aim, Move };    //現在の操作のステップ
Step step = Aim;

CBall balls[BALL_NUM];      //各ボール
CBall holes[POCKET_NUM];    //各ポケット、ポケットとして黒い玉を置いている

int next_ball_number = 1;   //次に当てるべきボールの番号

double random_point[2];     //下の関数によって他の点には触れないx,zのランダムな点を代入

bool penalty = false;       //ペナルティーの有無
int collision_num = 0;      //ボールの衝突回数、間違った球に当たったかどうかの判定に使う
int shot_num = 1;           //打った回数

float power = -3.5;         //打つ強さ

double	xAngle = 0.0, yAngle = 0.0;	//視点回転用角度
double  shot_Angle = 0;             //ショット方向の角度

double shot_dis = 0.0;      //キューの突く距離
double shot_time = 0;       //突いてからキューを表示する時間

unsigned char	mouseFlag = GL_FALSE;		// 視点回転時のクリック判定用
int				xStart, yStart;				// 視点回転時のクリック位置
int Dt = 33;                                //ステップ数

/*次のボールがどれかを判定する*/
void exist_check()
{
    int tmp_num = next_ball_number;
    while (!balls[tmp_num].exist)
    {
        tmp_num++;
        if (balls[BALL_NUM - 1].exist == false) break;  //全部落とした場合何もしない、終わり
    }
    next_ball_number = tmp_num;
}

void Rabdom_Set(int a,int b,int c,int d )   //乱数の初期化が必要かもしれない
{
    double dif_x[BALL_NUM],dif_z[BALL_NUM],dif[BALL_NUM];
   /*他の球に重ならないようにランダムな場所を作る*/
    bool miss = false;
    do {
        miss = false;
        double rand_x = rand() % a - b;
        double rand_z = rand() % c - d;
        for (int i = 1; i < BALL_NUM; i++)
        {
            dif_x[i] = rand_x - balls[i].x;
            dif_z[i] = rand_x - balls[i].x;
            dif[i] = sqrt(dif_x[i] * dif_x[i] + dif_z[i] * dif_z[i]);
            if (dif[i] < Ball_Radius) {
                miss = true;
                break;
            }
        }
        random_point[0] = rand_x;
        random_point[1] = rand_z;
    } while (miss);
}

/*ボールが全部止まってるかの判定、全部止まっていればエイム状態に移行*/
void count_Speed()
{
    int balls_move_count = 0;   //動いてるボールの数
    double velocity[BALL_NUM];
    /*動いているボールの数を数える*/
    if (step == Move)
    {
        for (int i = 0; i < BALL_NUM; i++)
        {
            velocity[i] = sqrt(balls[i].velocity_x * balls[i].velocity_x +
                               balls[i].velocity_z * balls[i].velocity_z);
            if (velocity[i] > 0.005)
            {
                balls_move_count += 1;
            }
        }
    }
    /*全てのボールが止まったら初期化*/
    if (balls_move_count > 0)
    {
        step = Move;
    }
    else
    {
        if (penalty || collision_num == 0 || balls[0].exist == false)
        {   /*ペナルティー時はランダムな点に置かれる*/
            Rabdom_Set(9,4,17,18);          //random_point[]にランダムな座標を入れる
            balls[0].x = random_point[0];
            balls[0].z = random_point[1];
            std::cout<<"penalty"<<"\n";
        }
        else
        {
            std::cout<<"ok"<<"\n";
        }
        /*各種数値を初期化*/
        step = Aim;          //エイム状態に
        balls_move_count = 0;//動いている球の数
        shot_time = 0;
        exist_check();       //次のボールの番号を決める
        penalty = false;     //ペナルティーの初期化
        collision_num = 0;   //衝突回数の初期化
        shot_num++;          //打った回数の合計を+１
        shot_Angle = 0;      //ショット方向を元に戻す
    }
}
/*壁との衝突判定*/
void collision_wall()
{
    //各ボールと壁との距離
    double balls_WallL_dis[BALL_NUM],balls_WallR_dis[BALL_NUM],
    balls_WallF_dis[BALL_NUM],balls_WallB_dis[BALL_NUM];

    for (int i = 0; i < BALL_NUM; i++)
    {
        balls_WallL_dis[i] = fabs(balls[i].x + WIDTH - 1.0);
        balls_WallR_dis[i] = fabs(WIDTH - 1.0 - balls[i].x );
        if (balls_WallL_dis[i] <= Ball_Radius)
        {
            balls[i].x = -4.5;
            balls[i].velocity_x *= -1;
        }
        if (balls_WallR_dis[i] <= Ball_Radius)
        {
            balls[i].x = 4.5;
            balls[i].velocity_x *= -1;
        }
        balls_WallF_dis[i] = fabs(balls[i].z + DISTANCE - 1);
        balls_WallB_dis[i] = fabs(- 1 - balls[i].z);
        if (balls_WallF_dis[i] <= Ball_Radius)
        {
            balls[i].z = -18.5;
            balls[i].velocity_z *= -1;
        }
        if (balls_WallB_dis[i] <= Ball_Radius)
        {
            balls[i].z = -1.5;
            balls[i].velocity_z *= -1;
        }
    }
}
/*ポケットとの衝突判定を行う、*/
void collision_pocket()
{
    //各ボールとポケットとの距離
    double	p_d[POCKET_NUM][BALL_NUM], p_dx[POCKET_NUM][BALL_NUM], p_dz[POCKET_NUM][BALL_NUM];
   
    for (int i = 0; i < POCKET_NUM; i++)
    {
        for (int j = 0; j < BALL_NUM; j++)
        {
            p_dx[i][j] = holes[i].x - balls[j].x;
            p_dz[i][j] = holes[i].z - balls[j].z;
            p_d[i][j] = sqrt(p_dx[i][j] * p_dx[i][j] + p_dz[i][j] * p_dz[i][j]);
            /*ポケットに当たったら横に並べる*/
            if(p_d[i][j] <= balls[j].radius * 2)
            {
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
    
    for (int i = 0;i < BALL_NUM - 1; i++)
    {
        for (int j = i + 1;j < BALL_NUM; j++)
        {
            /*ぶつかったら場所を少しずらす*/
            dif_x[i][j] = balls[i].x - balls[j].x;
            dif_z[i][j] = balls[i].z - balls[j].z;
            dif[i][j] = sqrt(dif_x[i][j] * dif_x[i][j] + dif_z[i][j] * dif_z[i][j] );
            /*球iとｊの衝突開始*/
            if (dif[i][j] < 2 * Ball_Radius)
            {
                /*最初の衝突が次の番号のボールじゃなかったらペナルティー*/
                if (collision_num == 0)
                {
                    if (j != next_ball_number)
                    {
                        penalty = true;
                    }
                    collision_num++;
                }
                /*番号の小さい方のボールを少しずらす*/
                kaku[i][j] = atan2(dif_x[i][j], dif_z[i][j]);
                balls[i].x = balls[j].x + 2 * Ball_Radius * sin(kaku[i][j]);
                balls[i].z = balls[j].z + 2 * Ball_Radius * cos(kaku[i][j]);
                /*i番目のボールの分解------------*/
                //↓配列に速度をセット
                Col_cal(balls[i].velocity_x, balls[i].velocity_z, balls[j].velocity_x, balls[j].velocity_z, kaku[i][j]);
                
                aft_Ux[i][j] = Velocitys[0];
                aft_Uz[i][j] = Velocitys[1];
                oth_Vx[i]    = Velocitys[2];
                oth_Vz[i]    = Velocitys[3];
                /*j番目のボールの分解------------*/
                dif_x[j][i] = -dif_x[i][j];
                dif_z[j][i] = -dif_z[i][j];
                dif[j][i]   = sqrt(dif_x[j][i] * dif_x[j][i] + dif_z[j][i] * dif_z[j][i]);
                kaku[j][i]  = atan2(dif_x[j][i], dif_z[j][i]);
                //↓配列に速度をセット
                Col_cal(balls[j].velocity_x, balls[j].velocity_z, balls[i].velocity_x, balls[i].velocity_z, kaku[j][i]);
                
                aft_Ux[j][i] = Velocitys[0];
                aft_Uz[j][i] = Velocitys[1];
                oth_Vx[j]    = Velocitys[2];
                oth_Vz[j]    = Velocitys[3];
                /*最終的な速度を設定-------*/
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
    if (step == Move)
    {
        collision_ball();   //ボールとの衝突判定
        collision_wall();   //壁との衝突判定
        collision_pocket(); //ポケットとの衝突判定
        count_Speed();      //速度の判定
    }
}

/*ボールとポケットの設置*/
void mySetUpBall()
{
    /*ボールの配置(set)および色の設定(material)*/
    for (int i = 0; i < BALL_NUM; i++)
    {
        balls[i].set(Balls_set_position[i][0],Balls_set_position[i][1],Balls_set_position[i][2],Balls_set_position[i][3]);
        balls[i].material(Amb[i], Dif[i], Spe[i], 128.0);
    }
    
    /*ポケットの配置(set)および色の設定(material)*/
    for (int i = 0; i < POCKET_NUM; i++)
    {
        holes[i].set(Pockets_set_position[i][0],Pockets_set_position[i][1],Pockets_set_position[i][2],
                     Pockets_set_position[i][3]);
        holes[i].material(Amb[10], Dif[10], Spe[10], 128.0);
    }
}

/* テーブルと木の部分の設定、同じような式が多いので簡略化したい*/
static void myGround(double height)
{
    const static GLfloat ground[4] = { 0.18, 0.55, 0.34, 1.0 };
    const static GLfloat udd[4] = {0.2,0.1,0.0};
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

/*パワーゲージの表示*/
void power_gage()
{
    static const GLfloat color[] = { 1.0, 1.0, 1.0, 1.0 };
    float gage_bottom = -3.5;
    
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
    /*パワーゲージを右下に表示*/
    glBegin(GL_QUADS);
        glVertex2d(3.4, power);
        glVertex2d(2.9, power);
        glVertex2d(2.9, gage_bottom);
        glVertex2d(3.4, gage_bottom);
    glEnd();
}

 /*次のボールの表示*/
void next_ball_disp()
{
    int k,m, n = 20;						// 分割数n
    double x1, y1, r1 = 0.15;				// 円周上の座標(x,y)と半径r
    
    for (k = 1; k < BALL_NUM ; k++)
    {
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, Next_ball_color[k-1]);
        if (balls[k].exist == true)
        {
            glNormal3d(1.0, 1.0, 1.0);
            glBegin(GL_POLYGON);			// ポリゴンの頂点記述開始
                /* 円周上の座標(x,y)を計算して円を描画*/
                for (m = 0; m < n; m++)
                {
                    x1 = r1 * cos(2.0 * M_PI * ((double)m/n) ) + 0.3 * k + 0.6;
                    y1 = r1 * sin(2.0 * M_PI * ((double)m/n) ) + 3.0;
                    glVertex3f(x1, y1, 0.0);	// 頂点の座標
                }
            glEnd();							// 頂点の記述終了
        }
    }
}

/*キューの設定*///--------------------------------------------------
void myCylinder(double radius, double height, int sides)
{
    /*白ボールを中心にキューを回転、及び描画する*/
    glTranslated(balls[0].x, 0.0, balls[0].z);
    glRotated(shot_Angle * 57.35, 0.0, 1.0, 0.0);
    glTranslated(-balls[0].x, 0.0, -balls[0].z);
    
    /*ショット時はキューを前に動かす*/
    if (step == Aim) {
        Set_Que(radius, height, sides,balls[0].x,balls[0].z);
    }
    
    glTranslated(balls[0].x, 0.0, balls[0].z);
    glRotated(-shot_Angle * 57.35, 0.0, 1.0, 0.0);
    glTranslated(-balls[0].x, 0.0, -balls[0].z);
}

float scale = 1;

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
    power_gage();
 
    /* 視点の移動（物体の方を奥に移す、斜めから見下ろす角度）*/
    glTranslated(0.0, -6.0, -3.0);
    glRotated(45, 1.0, 0.0, 0.0);
  
    /*マウスで縦に回転*/
    glRotated(xAngle, 1.0, 0.0, 0.0);
    
    /*テーブルの中心を基点に回転させている*/
    glTranslated(0.0, 0.0, -DISTANCE / 2);
    glRotated(yAngle, 0.0, 1.0, 0.0);
    glScaled(scale, scale, scale);
    glTranslated(0.0, 0.0, DISTANCE / 2);
   
    /*テーブルの描画*/
    myGround(-Ball_Radius);
    /*キューの描画*/
    if (shot_time < 0.3) {
        myCylinder(0.08, 10, 10);
    }
 
    /*ボールの挙動、描写*/
    for (int i = 0; i < BALL_NUM ; i++)
    {
        balls[i].step();
        balls[i].draw();
    }
    
    /*ショット方向の線の描画*/
    if (step == Aim)
    {
        glBegin(GL_LINES);
            glColor3d(1.0, 0.0, 1.0);
            glVertex3d(balls[0].x, balls[0].y, balls[0].z);
            glVertex3d(balls[0].x - 2 * sin(shot_Angle), balls[0].y, balls[0].z - 2 * cos(shot_Angle));
        glEnd();
    }
  
    /*ポケットの描写*/
    for (int i = 0;  i < POCKET_NUM; i++)
    {
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
    if (step == Aim)
    {
        switch (key)
        {
            case GLUT_KEY_LEFT:
                shot_Angle += (3/180.0) * M_PI;
                break;
                
            case GLUT_KEY_RIGHT:
                shot_Angle -= (3/180.0) * M_PI;
                break;
                
            case GLUT_KEY_UP:
                if (power < 1.0) power += 0.2;
                break;
                
            case GLUT_KEY_DOWN:
                if (power > -3.5) power -= 0.2;
                break;
            
            default:
                break;
        }
    }
    glutPostRedisplay();
}


/*キーボード操作*/
void myKeyboard(unsigned char key, int x, int y)
{
    double shot_Speed = (2.0/15.0) * power + (2.0/3.0);  //打つボールの速さ、ゲージとリンクさせる
    
    switch (key)
    {
        case 27:
            exit(0);
            break;
            
        case 's':
             if (step == Aim)
             {
                balls[0].velocity_x = -shot_Speed * sin(shot_Angle);
                balls[0].velocity_z = -shot_Speed * cos(shot_Angle);
                step = Move;
             }
            break;
        case 'a':
            if (shot_num == 1)
            {
                if (step == Aim)
                {
                    if (balls[0].x > -4.4)
                    {
                        balls[0].x -= 0.2;
                    }
                    else
                    {
                        balls[0].x = -4.4;
                    }
                }
            }
            break;
            
        case 'd':
            if (shot_num == 1)
            {
                if (step == Aim)
                {
                    if (balls[0].x < 4.4)
                    {
                        balls[0].x += 0.2;
                    }
                    else
                    {
                        balls[0].x = 4.4;
                    }
                }
            }
            break;
        case 'w':
            if (scale < 1.6)
            {
                scale += 0.1;
            }
            break;
            
        case 'z':
            if (scale > 0.7) {
                scale -= 0.1;
            }
            break;
            
        default:
            break;
    }
}
/*マウス操作１*/
void myMouseFunc(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        xStart = x;
        yStart = y;
        mouseFlag = GL_TRUE;
    }
    else
    {
        mouseFlag = GL_FALSE;
    }
}
/*マウス操作２*/
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
    if (xAngle > 45)
    {
        xAngle = 45;
    }
    else if(xAngle < -35)
    {
        xAngle = -35;
    }
    xStart = x; //スタート位置の更新
    yStart = y;
    glutPostRedisplay();
}

void myTimer(int value)
{
      glutTimerFunc(Dt, myTimer, 1);
      glutPostRedisplay();
}
/*
float gZoom = 0;
void wheel(int wheel_number,int direction,int x,int y){
    gZoom-=(float)direction*0.1;
    glutPostRedisplay();
}*/

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    MyInit("ビリヤード");
    glutKeyboardFunc(myKeyboard);
    glutTimerFunc(Dt, myTimer, 1);
    glutMouseFunc(myMouseFunc);
    glutMotionFunc(myMouseMotion);
    glutSpecialFunc(specialkey);
//    glutMouseWheelFunc(wheel);
    glutReshapeFunc(MyReshape);
    glutDisplayFunc(myDisplay);
    mySetUpBall();
    init();
    glutMainLoop();
    return 0;
}
