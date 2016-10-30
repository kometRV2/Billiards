//
//  Calc.cpp
//  Biliyard
//
//  Created by 大森誠 on 2016/10/25.
//  Copyright © 2016年 大森誠. All rights reserved.
//


#include <math.h>
#include "Calc.hpp"

/*球とポケットの各種光源の設定、環境光(amb)、拡散光(dif)、鏡面光(spe)*/
float amb[BALL_NUM + 1][4] = {
    {1.0,0.0,0.0,1.0},{0.0,0.0,1.0,1.0},{0.0,0.0,1.0,1.0},{0.0,0.0,1.0,1.0},{0.0,0.0,1.0,1.0},
    {0.0,0.0,1.0,1.0},{0.0,0.0,1.0,1.0},{0.0,0.0,1.0,1.0},{0.0,0.0,1.0,1.0},{0.0,0.0,1.0,1.0},{0.0,0.0,0.0,1.0}
};
float dif[BALL_NUM + 1][4] = {
    {1.0,1.0,1.0,1.0},{1.0,1.0,0.0,1.0},{0.0,0.0,0.5,1.0},{0.8,0.0,0.0,1.0},{0.3,0.0,0.7,1.0},
    {1.0,0.4,0.1,1.0},{0.0,0.8,0.0,1.0},{0.6,0.3,0.3,1.0},{0.0,0.0,0.0,1.0},{0.5,0.5,0.2,1.0},{0.0,0.0,0.0,1.0}
};
float spe[BALL_NUM + 1][4] = {
    {1.0, 1.0, 1.0, 1.0},{1.0, 1.0, 1.0, 1.0},{1.0, 1.0, 1.0, 1.0},{1.0, 1.0, 1.0, 1.0},{1.0, 1.0, 1.0, 1.0},
    {1.0,1.0,1.0,1.0},{1.0,1.0,1.0,1.0},{1.0,1.0,1.0,1.0},{1.0,1.0,1.0,1.0},{1.0,1.0,1.0,1.0},{0.0,0.0,0.0,1.0}
};
/*上に表示する次の球の色の設定(RGBA)*/
float Next_ball_color[9][4] ={
    { 1.0, 1.0, 0.0, 1.0 },{ 0.0, 0.0, 1.0, 1.0 },{ 1.0, 0.0, 0.0, 1.0 },
    { 0.3, 0.0, 0.7, 1.0 },{ 1.0, 0.4, 0.1, 1.0 },{ 0.0, 0.8, 0.0, 1.0 },
    { 0.6, 0.3, 0.3, 1.0 },{ 0.0, 0.0, 0.0, 1.0 },{ 0.5, 0.5, 0.2, 1.0 }
};

/*白球の初期位置*/
float start_ball_posx = 0.0;
float start_ball_posz = -3.0;
/*的球全ての初期位置*/
float balls_set_position[BALL_NUM][4] = {
    {start_ball_posx, Ball_Radius, start_ball_posz, Ball_Radius},{0.0 , Ball_Radius, -11.7, Ball_Radius},
    {0.4 , Ball_Radius, -12.5, Ball_Radius},{-0.4 , Ball_Radius, -12.5, Ball_Radius},
    {-0.8 , Ball_Radius, -13.3, Ball_Radius},{0.0 , Ball_Radius, -13.3, Ball_Radius},
    {0.8 , Ball_Radius, -13.3, Ball_Radius},{-0.4 , Ball_Radius, -14.1, Ball_Radius},
    {0.4 , Ball_Radius, -14.1, Ball_Radius},{0.0 , Ball_Radius, -14.9, Ball_Radius}
};
/*ポケットの初期位置*/
float pockets_set_position[POCKET_NUM][4] = {
    {-WIDTH + 1.3, -0.3, -DISTANCE + 1.3, Ball_Radius * 1.2},{-WIDTH + 0.8, -0.3, -DISTANCE/2, Ball_Radius * 1.2},
    {-WIDTH  + 1.3, -0.3,  - 1.3, Ball_Radius * 1.2},{WIDTH - 1.3, -0.3,  - 1.3, Ball_Radius * 1.2},
    {WIDTH - 0.8, -0.3, -DISTANCE/2, Ball_Radius * 1.2},{WIDTH - 1.3, -0.3, -DISTANCE + 1.3, Ball_Radius * 1.2}
};

double velocitys[4]; //下の衝突計算式での答えを格納する配列
//衝突計算式
void Col_cal(double balls_i_vx,double balls_i_vz,double balls_j_vx, double balls_j_vz, double kaku_ij)
{
    double sokuV_i = sqrt(balls_i_vx * balls_i_vx + balls_i_vz * balls_i_vz);   //自分のボールの速度
    double mykaku_i = atan2(balls_i_vx,balls_i_vz);                             //自分のボールの角度
    double kaku_SA_ij = kaku_ij - mykaku_i;
    double Ux_ij = sokuV_i * cos(kaku_SA_ij) * sin(kaku_ij);                    //衝突後の相手球のx速度
    double Uz_ij = sokuV_i * cos(kaku_SA_ij) * cos(kaku_ij);                    //衝突後の相手球のz速度
    double AsokuVx_i,AsokuVz_i;
    
    if(sin(kaku_SA_ij)<0){
        AsokuVx_i = -sokuV_i * sin(kaku_SA_ij) * sin(kaku_ij + M_PI/2);         //衝突後の自球のx速度
        AsokuVz_i = -sokuV_i * sin(kaku_SA_ij) * cos(kaku_ij + M_PI/2);         //衝突後の自球のz速度
    }else{
        AsokuVx_i = sokuV_i * sin(kaku_SA_ij) * sin(kaku_ij - M_PI /2);         //衝突後の自球のx速度
        AsokuVz_i = sokuV_i * sin(kaku_SA_ij) * cos(kaku_ij - M_PI /2);         //衝突後の自球のz速度
    }
    velocitys[0] = Ux_ij;
    velocitys[1] = Uz_ij;
    velocitys[2] = AsokuVx_i;
    velocitys[3] = AsokuVz_i;
}





