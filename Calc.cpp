//
//  Calc.cpp
//  Biliyard
//
//  Created by 大森誠 on 2016/10/25.
//  Copyright © 2016年 大森誠. All rights reserved.
//


#include <math.h>
#include "Calc.hpp"

/*白球の初期位置*/
float Start_ball_posx = 0.0;
float Start_ball_posz = -4.0;

double Velocitys[4]; //下の衝突計算式での答えを格納する配列

/*球とポケットの各種光源の設定、環境光(amb)、拡散光(dif)、鏡面光(spe)*/
float Amb[BALL_NUM + 1][4] =
{
    {1.0,0.0,0.0,1.0},{0.0,0.0,1.0,1.0},{0.0,0.0,1.0,1.0},{0.0,0.0,1.0,1.0},
    {0.0,0.0,1.0,1.0},{0.0,0.0,1.0,1.0},{0.0,0.0,1.0,1.0},{0.0,0.0,1.0,1.0},
    {0.0,0.0,1.0,1.0},{0.0,0.0,1.0,1.0},{0.0,0.0,0.0,1.0}
};
float Dif[BALL_NUM + 1][4] =
{
    {1.0,1.0,1.0,1.0},{1.0,1.0,0.0,1.0},{0.0,0.0,0.5,1.0},{0.8,0.0,0.0,1.0},
    {0.3,0.0,0.7,1.0},{1.0,0.4,0.1,1.0},{0.0,0.8,0.0,1.0},{0.6,0.3,0.3,1.0},
    {0.0,0.0,0.0,1.0},{0.5,0.5,0.2,1.0},{0.0,0.0,0.0,1.0}
};
float Spe[BALL_NUM + 1][4] =
{
    {1.0,1.0,1.0,1.0},{1.0,1.0,1.0,1.0},{1.0,1.0,1.0,1.0},{1.0,1.0,1.0,1.0},
    {1.0,1.0,1.0,1.0},{1.0,1.0,1.0,1.0},{1.0,1.0,1.0,1.0},{1.0,1.0,1.0,1.0},
    {1.0,1.0,1.0,1.0},{1.0,1.0,1.0,1.0},{0.0,0.0,0.0,1.0}
};
/*上に表示する次の球の色の設定(RGBA)*/
float Next_ball_color[9][4] =
{
    {1.0,1.0,0.0,1.0},{0.0,0.0,1.0,1.0},{1.0,0.0,0.0,1.0},
    {0.3,0.0,0.7,1.0},{1.0,0.4,0.1,1.0},{0.0,0.8,0.0,1.0},
    {0.6,0.3,0.3,1.0},{0.0,0.0,0.0,1.0},{0.5,0.5,0.2,1.0}
};

/*的球全ての初期位置*/
float Balls_set_position[BALL_NUM][4] =
{
    {Start_ball_posx, 0, Start_ball_posz, Ball_Radius},{0.0 , 0, -11.7, Ball_Radius},
    {0.4 , 0, -12.5, Ball_Radius},{-0.4 , 0, -12.5, Ball_Radius},
    {-0.8 , 0, -13.3, Ball_Radius},{0.0 , 0, -13.3, Ball_Radius},
    {0.8 , 0, -13.3, Ball_Radius},{-0.4 , 0, -14.1, Ball_Radius},
    {0.4 , 0, -14.1, Ball_Radius},{0.0 , 0, -14.9, Ball_Radius}
};
/*ポケットの初期位置*/
float Pockets_set_position[POCKET_NUM][4] =
{
    {-WIDTH + 1.3, -0.3, -DISTANCE + 1.3, Ball_Radius * 1.2},{-WIDTH + 0.8, -0.3, -DISTANCE/2, Ball_Radius * 1.2},
    {-WIDTH  + 1.3, -0.3,  - 1.3, Ball_Radius * 1.2},{WIDTH - 1.3, -0.3,  - 1.3, Ball_Radius * 1.2},
    {WIDTH - 0.8, -0.3, -DISTANCE/2, Ball_Radius * 1.2},{WIDTH - 1.3, -0.3, -DISTANCE + 1.3, Ball_Radius * 1.2}
};



/*衝突計算式*/
void Col_cal(double balls_i_vx,double balls_i_vz,double balls_j_vx, double balls_j_vz, double kaku_ij)
{
    double sokuV_i = sqrt(balls_i_vx * balls_i_vx + balls_i_vz * balls_i_vz);   //自分のボールの速度
    double mykaku_i = atan2(balls_i_vx,balls_i_vz);                             //自分のボールの角度
    double kaku_SA_ij = kaku_ij - mykaku_i;
    double Ux_ij = sokuV_i * cos(kaku_SA_ij) * sin(kaku_ij);                    //衝突後の相手球のx速度
    double Uz_ij = sokuV_i * cos(kaku_SA_ij) * cos(kaku_ij);                    //衝突後の相手球のz速度
    double AsokuVx_i,AsokuVz_i;
    
    if(sin(kaku_SA_ij)<0)
    {
        AsokuVx_i = -sokuV_i * sin(kaku_SA_ij) * sin(kaku_ij + M_PI/2);         //衝突後の自球のx速度
        AsokuVz_i = -sokuV_i * sin(kaku_SA_ij) * cos(kaku_ij + M_PI/2);         //衝突後の自球のz速度
    }
    else
    {
        AsokuVx_i = sokuV_i * sin(kaku_SA_ij) * sin(kaku_ij - M_PI /2);         //衝突後の自球のx速度
        AsokuVz_i = sokuV_i * sin(kaku_SA_ij) * cos(kaku_ij - M_PI /2);         //衝突後の自球のz速度
    }
    Velocitys[0] = Ux_ij;
    Velocitys[1] = Uz_ij;
    Velocitys[2] = AsokuVx_i;
    Velocitys[3] = AsokuVz_i;
}

/*キューの描画*/
void Set_Que(double radius, double height, int sides,double ball_x,double ball_z)
{
    double step = M_PI * 2 / (double)sides;
    int i;
    glTranslated(0.0 + ball_x, 0.0, 1.5 + ball_z);
    glRotated(-15, 1.0, 0.0, 0.0);
    /* 上面 */
    glNormal3d(0.0, 1.0, 0.0);
    glBegin(GL_TRIANGLE_FAN);
        for (i = 0; i < sides; i++)
        {
            double t = step * (double)i;
            glVertex3d(radius * cos(t) ,
                       radius * sin(t) + 0.5 ,
                       0.0 );
        }
    glEnd();
    /* 底面 */
    glNormal3d(0.0, -1.0, 0.0);
    glBegin(GL_TRIANGLE_FAN);
        for (i = sides; --i >= 0;)
        {
            double t = step * (double)i;
            glVertex3d(radius * cos(t) ,
                       radius * sin(t) + 0.5 ,
                       height );
        }
    glEnd();
    /* 側面 */
    glBegin(GL_QUAD_STRIP);
        for (i = 0; i <= sides; i++)
        {
            double t = step * (double)i;
            double x = sin(t);
            double z = cos(t);
        
            glNormal3d(x, 0.0, z);

            glVertex3f(radius * z ,
                       radius * x + 0.5,
                       0.0 );
            glVertex3f(radius * z ,
                       radius * x + 0.5,
                       height );
        }
    glEnd();
    glRotated(15, 1.0, 0.0, 0.0);
    glTranslated(0.0 - ball_x, 0.0, -1.5 - ball_z);
}






















