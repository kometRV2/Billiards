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

#define BALL_NUM 10                 //球の数、ナインボール
#define POCKET_NUM 6                 //ポケットの数
#define WIDTH 6                      // 台の横幅の２分の１
#define DISTANCE 20                  // 台の奥行きの長さ
#define Ball_Radius 0.4              // 球の半径

extern float amb[BALL_NUM + 1][4];
extern float dif[BALL_NUM + 1][4];
extern float spe[BALL_NUM + 1][4];

extern float Next_ball_color[BALL_NUM - 1][4];

extern float start_ball_posx ;
extern float start_ball_posz ;

extern float balls_set_position[BALL_NUM][4] ;
extern float pockets_set_position[POCKET_NUM][4];

extern void Col_cal(double balls_i_vx,double balls_i_vz,double balls_j_vx, double balls_j_vz, double kaku_ij);

extern double velocitys[4];


#endif /* Calc_hpp */


