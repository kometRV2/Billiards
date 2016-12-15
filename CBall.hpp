//
//  CBall.hpp
//  Biliyard
//
//  Created by 大森誠 on 2016/11/22.
//  Copyright © 2016年 大森誠. All rights reserved.
//

#ifndef CBall_h
#define CBall_h

#define TEXWIDTH  256       // テクスチャの幅
#define TEXHEIGHT 256       // テクスチャの高さ

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

static void init(void)
{
    /* テクスチャの読み込みに使う配列 */
    GLubyte udd_tex[TEXHEIGHT][TEXWIDTH][3];
    FILE *udd_fp;
    
    //絶対パス："/Users/oomorimakoto/Desktop/mokume.data"
    /* テクスチャ画像の読み込み */
    if ((udd_fp = fopen("mokume.data", "rb")) != NULL)
    {
        fread(udd_tex, sizeof udd_tex, 1, udd_fp);
        fclose(udd_fp);
        /* テクスチャの割り当て */
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXWIDTH, TEXHEIGHT, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, udd_tex);
    }
    else
    {
        perror("mokume.data");
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

/*初期化*/
void MyInit(char *progname)
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

void MyReshape(int width, int height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(70.0, (double)width / (double)height, 0.1, 50.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslated(0.0, 0.0, -5.0);
}

#endif /* CBall_h */
