/*****************************************************************************
*                                                                            *
*  OpenNI 1.x Alpha                                                          *
*  Copyright (C) 2012 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of OpenNI.                                              *
*                                                                            *
*  Licensed under the Apache License, Version 2.0 (the "License");           *
*  you may not use this file except in compliance with the License.          *
*  You may obtain a copy of the License at                                   *
*                                                                            *
*      http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*                                                                            *
*****************************************************************************/
//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnOS.h>
#include <iostream>
#if (XN_PLATFORM == XN_PLATFORM_MACOSX)
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif
#include <math.h>

#include <XnCppWrapper.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace xn;
using namespace std;

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
#define SAMPLE_XML_PATH "../../Config/SamplesConfig.xml"

#define GL_WIN_SIZE_X 1280
#define GL_WIN_SIZE_Y 1024

#define DISPLAY_MODE_OVERLAY	1
#define DISPLAY_MODE_DEPTH		2
#define DISPLAY_MODE_IMAGE		3
#define DEFAULT_DISPLAY_MODE	DISPLAY_MODE_DEPTH

//color threshold for yellow ball
#define RED_THR_MAX 255
#define RED_THR_MIN 190
#define GREEN_THR_MAX 255
#define GREEN_THR_MIN 190
#define BLUE_THR_MAX 100
#define BLUE_THR_MIN 0

//---------------------------------------------------------------------------
// Globals
//---------------------------------------------------------------------------
float* g_pDepthHist;
XnRGB24Pixel* g_pTexMap = NULL;
unsigned int g_nTexMapX = 0;
unsigned int g_nTexMapY = 0;
XnDepthPixel g_nZRes;
unsigned int g_nViewState = DEFAULT_DISPLAY_MODE;

Context g_context;
ScriptNode g_scriptNode;
DepthGenerator g_depth;
ImageGenerator g_image;
DepthMetaData g_depthMD;
ImageMetaData g_imageMD;

// ball position
XnPoint3D ball_point2D, ball_point3D;

//socket
int sd;
struct sockaddr_in addr;

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
/*
void glutIdle (void)
{
	// Display the frame
	glutPostRedisplay();
}
*/
 /*
void glutDisplay (void)
{
	XnStatus rc = XN_STATUS_OK;

	// Read a new frame
	rc = g_context.WaitAnyUpdateAll();
	if (rc != XN_STATUS_OK)
	{
		printf("Read failed: %s\n", xnGetStatusString(rc));
		return;
	}

	g_depth.GetMetaData(g_depthMD);
	g_image.GetMetaData(g_imageMD);

	const XnDepthPixel* pDepth = g_depthMD.Data();

	// Copied from SimpleViewer
	// Clear the OpenGL buffers
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Setup the OpenGL viewpoint
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, GL_WIN_SIZE_X, GL_WIN_SIZE_Y, 0, -1.0, 1.0);
	
	// Calculate the accumulative histogram (the yellow display...)
	xnOSMemSet(g_pDepthHist, 0, g_nZRes*sizeof(float));

	unsigned int nNumberOfPoints = 0;
	for (XnUInt y = 0; y < g_depthMD.YRes(); ++y)
	{
		for (XnUInt x = 0; x < g_depthMD.XRes(); ++x, ++pDepth)
		{
			if (*pDepth != 0)
			{
				g_pDepthHist[*pDepth]++;
				nNumberOfPoints++;
			}
		}
	}
	for (int nIndex=1; nIndex<g_nZRes; nIndex++)
	{
		g_pDepthHist[nIndex] += g_pDepthHist[nIndex-1];
	}
	if (nNumberOfPoints)
	{
		for (int nIndex=1; nIndex<g_nZRes; nIndex++)
		{
			g_pDepthHist[nIndex] = (unsigned int)(256 * (1.0f - (g_pDepthHist[nIndex] / nNumberOfPoints)));
		}
	}

	xnOSMemSet(g_pTexMap, 0, g_nTexMapX*g_nTexMapY*sizeof(XnRGB24Pixel));

	// check if we need to draw image frame to texture
	if (g_nViewState == DISPLAY_MODE_OVERLAY ||
	    g_nViewState == DISPLAY_MODE_IMAGE || true)
	{
		const XnRGB24Pixel* pImageRow = g_imageMD.RGB24Data();
		XnRGB24Pixel* pTexRow = g_pTexMap + g_imageMD.YOffset() * g_nTexMapX;
		XnFloat ball_cnt = 0;
		// init ball pos
		XnFloat xCord = 0;
		XnFloat yCord = 0;
		//ball_point2D.X = 0;
		//ball_point2D.Y = 0;

		for (XnUInt y = 0; y < g_imageMD.YRes(); ++y)
		{
			const XnRGB24Pixel* pImage = pImageRow;
			XnRGB24Pixel* pTex = pTexRow + g_imageMD.XOffset();

			for (XnUInt x = 0; x < g_imageMD.XRes(); ++x, ++pImage, ++pTex)
			{
				*pTex = *pImage;
				
				// COLOR FILTER
				if((pTex->nRed > RED_THR_MIN) && (pTex->nRed < RED_THR_MAX) && (pTex->nGreen > GREEN_THR_MIN) && (pTex->nGreen < GREEN_THR_MAX) && (pTex->nBlue > BLUE_THR_MIN) && (pTex->nBlue < BLUE_THR_MAX)) {

				  //std::cout <<"2D X = " <<ball_point2D.X << std::endl;
				  
				  //YELLOW BALL
				  //ball_point2D.X += x;
				  //ball_point2D.Y += y;
				  xCord += x;
				  yCord += y;
				  ball_cnt += 1;

				}else{
				  pTex->nRed = 0;
				  pTex->nGreen = 0;
				  pTex->nBlue = 0;
				}
			}
			pImageRow += g_imageMD.XRes();
			pTexRow += g_nTexMapX;
		}
			cout<<"ball_point2D.X="<<xCord<<endl;
			if(ball_cnt !=0){
			  cout << xCord/ ball_cnt << endl;
			  ball_point2D.X = xCord/ ball_cnt;
			  ball_point2D.Y = yCord/ ball_cnt;
			}
			cout<<"ball position (x,y) = ("<<ball_point2D.X<<","<<ball_point2D.Y<<")"<<endl;
			cout <<"ball_cnt = "<<ball_cnt<<endl;	
	}

	// check if we need to draw depth frame to texture
	if (g_nViewState == DISPLAY_MODE_OVERLAY ||
		g_nViewState == DISPLAY_MODE_DEPTH || true)
	{
	  cout << "true" << endl;
		const XnDepthPixel* pDepthRow = g_depthMD.Data();
		XnRGB24Pixel* pTexRow = g_pTexMap + g_depthMD.YOffset() * g_nTexMapX;

		cout << "start calc" << endl;
		ball_point2D.Z = pDepthRow[(int)ball_point2D.Y*640+(int)ball_point2D.X];
		cout << "ball_point2D.Z= " << ball_point2D.Z << endl;
		g_depth.ConvertProjectiveToRealWorld(1, &ball_point2D, &ball_point3D);
		cout<<ball_point3D.X<<" "<<ball_point3D.Y<<" "<<ball_point3D.Z<<endl;

		for (XnUInt y = 0; y < g_depthMD.YRes(); ++y)
		{
			const XnDepthPixel* pDepth = pDepthRow;
			XnRGB24Pixel* pTex = pTexRow + g_depthMD.XOffset();

			for (XnUInt x = 0; x < g_depthMD.XRes(); ++x, ++pDepth, ++pTex)
			{
				if (*pDepth != 0)
				{
					int nHistValue = g_pDepthHist[*pDepth];
					pTex->nRed = nHistValue;
					pTex->nGreen = nHistValue;
					pTex->nBlue = 0;
				}
			}

			pDepthRow += g_depthMD.XRes();
			pTexRow += g_nTexMapX;
		}

	}

	// Create the OpenGL texture map
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, g_nTexMapX, g_nTexMapY, 0, GL_RGB, GL_UNSIGNED_BYTE, g_pTexMap);

	// Display the OpenGL texture map
	glColor4f(1,1,1,1);
	//glBegin(GL_QUADS);
	int nXRes = g_depthMD.FullXRes();
	int nYRes = g_depthMD.FullYRes();

	// upper left
	glTexCoord2f(0, 0);
	glVertex2f(0, 0);
	// upper right
	glTexCoord2f((float)nXRes/(float)g_nTexMapX, 0);
	glVertex2f(GL_WIN_SIZE_X, 0);
	// bottom right
	glTexCoord2f((float)nXRes/(float)g_nTexMapX, (float)nYRes/(float)g_nTexMapY);
	glVertex2f(GL_WIN_SIZE_X, GL_WIN_SIZE_Y);
	// bottom left
	glTexCoord2f(0, (float)nYRes/(float)g_nTexMapY);
	glVertex2f(0, GL_WIN_SIZE_Y);
	glEnd();

	// Swap the OpenGL display buffers
	glutSwapBuffers();


	char message[256];
	sprintf(message,"%f,%f,%f",ball_point3D.X,ball_point3D.Y,ball_point3D.Z);
	if(send(sd, message, 64, 0) < 0) {
	  perror("send");
	  //return -1;
	}
}
*/
/*
void glutKeyboard (unsigned char key, int x, int y)
{
	switch (key)
	{
		case 27:
			exit (1);
		case '1':
			g_nViewState = DISPLAY_MODE_OVERLAY;
			g_depth.GetAlternativeViewPointCap().SetViewPoint(g_image);
			break;
		case '2':
			g_nViewState = DISPLAY_MODE_DEPTH;
			g_depth.GetAlternativeViewPointCap().ResetViewPoint();
			break;
		case '3':
			g_nViewState = DISPLAY_MODE_IMAGE;
			g_depth.GetAlternativeViewPointCap().ResetViewPoint();
			break;
		case 'm':
			g_context.SetGlobalMirror(!g_context.GetGlobalMirror());
			break;
	}
}
*/

int main(int argc, char* argv[])
{
	XnStatus rc;

	EnumerationErrors errors;
	rc = g_context.InitFromXmlFile(SAMPLE_XML_PATH, g_scriptNode, &errors);
	if (rc == XN_STATUS_NO_NODE_PRESENT)
	{
		XnChar strError[1024];
		errors.ToString(strError, 1024);
		printf("%s\n", strError);
		return (rc);
	}
	else if (rc != XN_STATUS_OK)
	{
		printf("Open failed: %s\n", xnGetStatusString(rc));
		return (rc);
	}

	rc = g_context.FindExistingNode(XN_NODE_TYPE_DEPTH, g_depth);
	if (rc != XN_STATUS_OK)
	{
		printf("No depth node exists! Check your XML.");
		return 1;
	}

	rc = g_context.FindExistingNode(XN_NODE_TYPE_IMAGE, g_image);
	if (rc != XN_STATUS_OK)
	{
		printf("No image node exists! Check your XML.");
		return 1;
	}

	g_depth.GetMetaData(g_depthMD);
	g_image.GetMetaData(g_imageMD);

	// Hybrid mode isn't supported in this sample
	if (g_imageMD.FullXRes() != g_depthMD.FullXRes() || g_imageMD.FullYRes() != g_depthMD.FullYRes())
	{
		printf ("The device depth and image resolution must be equal!\n");
		return 1;
	}

	// RGB is the only image format supported.
	if (g_imageMD.PixelFormat() != XN_PIXEL_FORMAT_RGB24)
	{
		printf("The device image format must be RGB24\n");
		return 1;
	}

	// Texture map init
	g_nTexMapX = (((unsigned short)(g_depthMD.FullXRes()-1) / 512) + 1) * 512;
	g_nTexMapY = (((unsigned short)(g_depthMD.FullYRes()-1) / 512) + 1) * 512;
	g_pTexMap = (XnRGB24Pixel*)malloc(g_nTexMapX * g_nTexMapY * sizeof(XnRGB24Pixel));

	g_nZRes = g_depthMD.ZRes();
	g_pDepthHist = (float*)malloc(g_nZRes * sizeof(float));

	/*
	// OpenGL init
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(GL_WIN_SIZE_X*0.1, GL_WIN_SIZE_Y*0.1);
	glutCreateWindow ("OpenNI Simple Viewer");
	//glutFullScreen();

	//glutSetCursor(GLUT_CURSOR_NONE);
	glutKeyboardFunc(glutKeyboard);
	glutDisplayFunc(glutDisplay);
	glutIdleFunc(glutIdle);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	*/

	// socket init

	// IPv4 TCP のソケットを作成する
	if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	  perror("socket");
	  return -1;
	}

	// 送信先アドレスとポート番号を設定する(コマンドライン引数でIPを渡す)
	addr.sin_family = AF_INET;
	addr.sin_port = htons(22222);
	addr.sin_addr.s_addr = inet_addr(argv[1]);

	// サーバ接続（TCP の場合は、接続を確立する必要がある）
	connect(sd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));

	// Per frame code is in glutDisplay
	//glutMainLoop();

	while(1) {
	  XnStatus rc = XN_STATUS_OK;

	// Read a new frame
	rc = g_context.WaitAnyUpdateAll();
	if (rc != XN_STATUS_OK)
	{
		printf("Read failed: %s\n", xnGetStatusString(rc));
		//return;
	}

	g_depth.GetMetaData(g_depthMD);
	g_image.GetMetaData(g_imageMD);

	const XnDepthPixel* pDepth = g_depthMD.Data();

	// Calculate the accumulative histogram (the yellow display...)
	xnOSMemSet(g_pDepthHist, 0, g_nZRes*sizeof(float));

	unsigned int nNumberOfPoints = 0;
	for (XnUInt y = 0; y < g_depthMD.YRes(); ++y)
	{
		for (XnUInt x = 0; x < g_depthMD.XRes(); ++x, ++pDepth)
		{
			if (*pDepth != 0)
			{
				g_pDepthHist[*pDepth]++;
				nNumberOfPoints++;
			}
		}
	}
	for (int nIndex=1; nIndex<g_nZRes; nIndex++)
	{
		g_pDepthHist[nIndex] += g_pDepthHist[nIndex-1];
	}
	if (nNumberOfPoints)
	{
		for (int nIndex=1; nIndex<g_nZRes; nIndex++)
		{
			g_pDepthHist[nIndex] = (unsigned int)(256 * (1.0f - (g_pDepthHist[nIndex] / nNumberOfPoints)));
		}
	}

	xnOSMemSet(g_pTexMap, 0, g_nTexMapX*g_nTexMapY*sizeof(XnRGB24Pixel));

	// check if we need to draw image frame to texture
	if (g_nViewState == DISPLAY_MODE_OVERLAY ||
	    g_nViewState == DISPLAY_MODE_IMAGE || true)
	{
		const XnRGB24Pixel* pImageRow = g_imageMD.RGB24Data();
		XnRGB24Pixel* pTexRow = g_pTexMap + g_imageMD.YOffset() * g_nTexMapX;
		XnFloat ball_cnt = 0;
		// init ball pos
		XnFloat xCord = 0;
		XnFloat yCord = 0;
		//ball_point2D.X = 0;
		//ball_point2D.Y = 0;

		for (XnUInt y = 0; y < g_imageMD.YRes(); ++y)
		{
			const XnRGB24Pixel* pImage = pImageRow;
			XnRGB24Pixel* pTex = pTexRow + g_imageMD.XOffset();

			for (XnUInt x = 0; x < g_imageMD.XRes(); ++x, ++pImage, ++pTex)
			{
				*pTex = *pImage;
				
				// COLOR FILTER
				if((pTex->nRed > RED_THR_MIN) && (pTex->nRed < RED_THR_MAX) && (pTex->nGreen > GREEN_THR_MIN) && (pTex->nGreen < GREEN_THR_MAX) && (pTex->nBlue > BLUE_THR_MIN) && (pTex->nBlue < BLUE_THR_MAX)) {

				  //std::cout <<"2D X = " <<ball_point2D.X << std::endl;
				  
				  //YELLOW BALL
				  //ball_point2D.X += x;
				  //ball_point2D.Y += y;
				  xCord += x;
				  yCord += y;
				  ball_cnt += 1;

				}else{
				  pTex->nRed = 0;
				  pTex->nGreen = 0;
				  pTex->nBlue = 0;
				}
			}
			pImageRow += g_imageMD.XRes();
			pTexRow += g_nTexMapX;
		}
			cout<<"ball_point2D.X="<<xCord<<endl;
			if(ball_cnt !=0){
			  cout << xCord/ ball_cnt << endl;
			  ball_point2D.X = xCord/ ball_cnt;
			  ball_point2D.Y = yCord/ ball_cnt;
			}
			cout<<"ball position (x,y) = ("<<ball_point2D.X<<","<<ball_point2D.Y<<")"<<endl;
			cout <<"ball_cnt = "<<ball_cnt<<endl;	
	}

	// check if we need to draw depth frame to texture
	if (g_nViewState == DISPLAY_MODE_OVERLAY ||
		g_nViewState == DISPLAY_MODE_DEPTH || true)
	{
	  cout << "true" << endl;
		const XnDepthPixel* pDepthRow = g_depthMD.Data();
		XnRGB24Pixel* pTexRow = g_pTexMap + g_depthMD.YOffset() * g_nTexMapX;

		cout << "start calc" << endl;
		ball_point2D.Z = pDepthRow[(int)ball_point2D.Y*640+(int)ball_point2D.X];
		cout << "ball_point2D.Z= " << ball_point2D.Z << endl;
		g_depth.ConvertProjectiveToRealWorld(1, &ball_point2D, &ball_point3D);
		cout<<ball_point3D.X<<" "<<ball_point3D.Y<<" "<<ball_point3D.Z<<endl;

		for (XnUInt y = 0; y < g_depthMD.YRes(); ++y)
		{
			const XnDepthPixel* pDepth = pDepthRow;
			XnRGB24Pixel* pTex = pTexRow + g_depthMD.XOffset();

			for (XnUInt x = 0; x < g_depthMD.XRes(); ++x, ++pDepth, ++pTex)
			{
				if (*pDepth != 0)
				{
					int nHistValue = g_pDepthHist[*pDepth];
					pTex->nRed = nHistValue;
					pTex->nGreen = nHistValue;
					pTex->nBlue = 0;
				}
			}

			pDepthRow += g_depthMD.XRes();
			pTexRow += g_nTexMapX;
		}

	}

	//glBegin(GL_QUADS);
	int nXRes = g_depthMD.FullXRes();
	int nYRes = g_depthMD.FullYRes();

	char message[256];
	sprintf(message,"%f,%f,%f",ball_point3D.X,ball_point3D.Y,ball_point3D.Z);
	if(send(sd, message, 64, 0) < 0) {
	  perror("send");
	  //return -1;
	}

	}

	close(sd);

	return 0;
}
