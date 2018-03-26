//
//  main.cpp
//  OpenGL_test
//
//  Created by wyf on 2018/3/23.
//  Copyright © 2018年 wyf. All rights reserved.
//

#include <stdio.h>
#include "GLTools.h"
#include <glut/glut.h>
#include "GLMatrixStack.h"
#include "GLFrame.h"
#include "GLFrustum.h"
#include "GLGeometryTransform.h"
#include "StopWatch.h"

#include <math.h>
#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif


GLTriangleBatch toursBatch;
GLTriangleBatch sphereBatch;
GLBatch floorBatch;

GLMatrixStack modelViewMatrixStack;
GLMatrixStack projectStack;
GLFrustum viewFrustum;
GLFrame cameraFrame;
GLGeometryTransform transFormPipeline;


GLShaderManager shaderManager;

//窗口大小改变时接受新的宽度和高度，其中0,0代表窗口中视口的左下角坐标，w，h代表像素

void ChangeSize(int w,int h)
{
    
    glViewport(0,0, w, h);
    
    //cameraFrame.MoveForward(-15.0f);
    
    viewFrustum.SetPerspective(35.0f, float(w)/float(h), 1.0f, 100.0f);
    projectStack.LoadMatrix(viewFrustum.GetProjectionMatrix());
    
    transFormPipeline.SetMatrixStacks(modelViewMatrixStack, projectStack);
}

//为程序作一次性的设置

void SetupRC()

{
    //设置背影颜色
    
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glClearColor(0.0f,0.0f,0,1.0f);
    
    //初始化着色管理器
    
    shaderManager.InitializeStockShaders();
    gltMakeTorus(toursBatch, 0.4f, 0.15f, 30, 30);
    gltMakeSphere(sphereBatch, 0.1, 26, 13);
    
    //画地板
    floorBatch.Begin(GL_LINES, 324);
    
    for (GLfloat x = -20.0f; x < 20.0f; x += 0.5f) {
        floorBatch.Vertex3f(x, -0.5, -20.0);
        floorBatch.Vertex3f(x, -0.5, 20);
        
        floorBatch.Vertex3f(-20.0, -0.5f, x);
        floorBatch.Vertex3f(20.0f, -0.5f, x);
    }
    
    floorBatch.End();
}

//开始渲染

void RenderScene(void)
{
    
    static GLfloat vFloorColor[] = {0.0f,1.0f,0.0f,1.0f};
    static GLfloat vTrousColor[] = {1.0f,0.0f,0.0f,1.0f};
    
    static GLfloat vSphereColor[] = {0.0f,0.0,1.0f,0.0f};
    
    //基于时间动画
    static CStopWatch rotTime;
    float yRot = rotTime.GetElapsedSeconds() * 60.0f;
    
    
    //清除一个或一组特定的缓冲区
    
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    //设置一组浮点数来表示红色
    
    modelViewMatrixStack.PushMatrix();
    
    M3DMatrix44f cameraMatrix;
    cameraFrame.GetCameraMatrix(cameraMatrix);
    modelViewMatrixStack.PushMatrix(cameraMatrix);
    
    //绘制地板
    shaderManager.UseStockShader(GLT_SHADER_FLAT,transFormPipeline.GetModelViewProjectionMatrix(),vFloorColor);
    floorBatch.Draw();
    
    modelViewMatrixStack.Translate(0, 0, -2.5);
    modelViewMatrixStack.PushMatrix();
    
    
    modelViewMatrixStack.Rotate(yRot, 0, 1.0, 0);
    
    //绘制甜甜圈
    shaderManager.UseStockShader(GLT_SHADER_FLAT,transFormPipeline.GetModelViewProjectionMatrix(),vTrousColor);
    toursBatch.Draw();
    
    modelViewMatrixStack.PopMatrix();
    
    
    //球球
    modelViewMatrixStack.Rotate(yRot * -2.0f, 0, 1, 0);
    modelViewMatrixStack.Translate(0.5f, 0, 0);
    shaderManager.UseStockShader(GLT_SHADER_FLAT,transFormPipeline.GetModelViewProjectionMatrix(),vSphereColor);
    sphereBatch.Draw();
    
    modelViewMatrixStack.PopMatrix();
    modelViewMatrixStack.PopMatrix();
    
    //提交着色器
    
    
    //将在后台缓冲区进行渲染，然后在结束时交换到前台
    
    glutPostRedisplay();
    glutSwapBuffers();
    
}

void specialKeyClick(int key ,int x, int y)
{
    if (key == GLUT_KEY_UP) {

        cameraFrame.TranslateWorld(0, 0, -0.3);
    }
    
    if (key == GLUT_KEY_LEFT) {
        cameraFrame.RotateWorld(0.1, 0, 1, 0);
    }
    
    if (key == GLUT_KEY_RIGHT) {
        cameraFrame.RotateWorld(-0.1, 0, 1, 0);
    }
    
    if (key == GLUT_KEY_DOWN) {
        cameraFrame.TranslateWorld(0, 0, 0.3);
    }
    
}

int main(int argc,char* argv[])

{
    
    //设置当前工作目录，针对MAC OS X
    
    gltSetWorkingDirectory(argv[0]);
    
    //初始化GLUT库
    
    glutInit(&argc, argv);
    
    /*初始化双缓冲窗口，其中标志GLUT_DOUBLE、GLUT_RGBA、GLUT_DEPTH、GLUT_STENCIL分别指
     
     双缓冲窗口、RGBA颜色模式、深度测试、模板缓冲区*/
    
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH|GLUT_STENCIL);
    
    //GLUT窗口大小，标题窗口
    
    glutInitWindowSize(800,600);
    
    glutCreateWindow("Triangle");
    
    //注册回调函数
    
    glutReshapeFunc(ChangeSize);
    
    glutDisplayFunc(RenderScene);
    
    glutSpecialFunc(specialKeyClick);
    
    //驱动程序的初始化中没有出现任何问题。
    
    GLenum err = glewInit();
    
    if(GLEW_OK != err) {
        
        fprintf(stderr,"glew error:%s\n",glewGetErrorString(err));
        
        return 1;
        
    }
    
    //调用SetupRC
    
    SetupRC();
    
    glutMainLoop();
    
    return 0;
    
}
