//
//  Calc.hpp
//  Biliyard
//
//  Created by 大森誠 on 2016/10/25.
//  Copyright © 2016年 大森誠. All rights reserved.
//

#ifndef Calc_hpp
#define Calc_hpp

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define BALL_NUM 10                 //球の数、ナインボール
#define POCKET_NUM 6                 //ポケットの数
#define WIDTH 6                      // 台の横幅の２分の１
#define DISTANCE 20                  // 台の奥行きの長さ
#define Ball_Radius 0.4              // 球の半径

extern float Amb[BALL_NUM + 1][4];
extern float Dif[BALL_NUM + 1][4];
extern float Spe[BALL_NUM + 1][4];

extern float Next_ball_color[BALL_NUM - 1][4];

extern float Start_ball_posx ;
extern float Start_ball_posz ;

extern float Balls_set_position[BALL_NUM][4] ;
extern float Pockets_set_position[POCKET_NUM][4];

extern void Col_cal(double balls_i_vx,double balls_i_vz,double balls_j_vx, double balls_j_vz, double kaku_ij);

extern double Velocitys[4];


#endif /* Calc_hpp */


