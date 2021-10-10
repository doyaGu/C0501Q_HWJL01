/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2011 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "SDL_config.h"

#include "SDL_events.h"
#include "../../events/SDL_touch_c.h"

#include "SDL_castor3touch.h"
#include "tslib.h"
#include <stdbool.h>
#include <string.h>
#include "ite/ith.h"

#define	MAX_FINGER_NUM	10

static struct tsdev *ts;
static const SDL_TouchID touchId = 0;
static SDL_FingerID fingerId;
static bool down[MAX_FINGER_NUM];
static bool mDown[MAX_FINGER_NUM];
static bool rawMode;
static float rawX, rawY;
static struct ts_sample c3samp[10];
static int	gLastId[10] = {0};
static int	gLastFingerNum = 0;
static int	gSdlFingerNum = 0;

static void lp_tp_module_identify_callback_default(char **module)
{
	*module = "NO_CALLBACK";
}
void (*lp_tp_module_identify_callback)(char **module) = lp_tp_module_identify_callback_default;


void Castor3_InitTouch(void)
{
	SDL_Touch touch;
	SDL_Touch* ptr;

#if defined(CFG_TOUCH_I2C0)
    ts = ts_open(":i2c0",0);
#elif defined(CFG_TOUCH_I2C1)
    ts = ts_open(":i2c1",0);
#elif defined(CFG_TOUCH_SPI)
    ts = ts_open(":spi",0);
#endif // CFG_TOUCH_I2C

	if (!ts) 
    {
		perror("ts_open");
	}

	if(CFG_TOUCH_MODULE_COUNT > 1)
    {
    	char *m;

    	//To indentify TP module
    	lp_tp_module_identify_callback(&m);
    	
    	if(strcmp(m,"NO_CALLBACK") == 0)
    	{
    		//NO callback function, please implement the lp_tp_module_identify_callback() in AP layer
    		//if CFG_TOUCH_MODULE_COUNT > 1, you have to implement the callback function for a correct TP module.
    		//OR set "CFG_TOUCH_MODULE" as only one module, if you didn't implement the callback function
    		printf("[ERROR] NO callback function for identifying the correct module\n");
    	}
    	else if(m == NULL)
    	{
    		printf("Can not identify the TP module. Please check the callback function.\n");
    	}
    	else
    	{
    		//Load TP module by module name
    		if(ts_load_module_raw(ts, m, NULL))
    		{
    			//incorrect TP module, please check if the module is matched.
    			perror("ts_load_module_raw");  
    		}
    		else
    		{
    			printf("Load TP module finished, module: %s, count=%d\n", m, CFG_TOUCH_MODULE_COUNT);
    		}
    	}
    }

	if (ts_config(ts)) 
    {
		perror("ts_config");
	}

	touch.id = touchId;
	touch.x_min = 0;
	touch.x_max = CFG_TOUCH_X_MAX_VALUE;
	touch.native_xres = touch.x_max - touch.x_min;
	touch.y_min = 0;
	touch.y_max = CFG_TOUCH_Y_MAX_VALUE;
	touch.native_yres = touch.y_max - touch.y_min;
	touch.pressure_min = 0;
	touch.pressure_max = 1;
	touch.native_pressureres = touch.pressure_max - touch.pressure_min;

    c3samp[0].next = (struct ts_sample*)&c3samp[1];
    //c3samp[1].next = (struct ts_sample*)&c3samp[2];
    //c3samp[2].next = (struct ts_sample*)&c3samp[3];
    //c3samp[3].next = (struct ts_sample*)&c3samp[4];
    //c3samp[4].next = NULL;	
	//printf("initial fingerId: %lld, c3samp = %x, %x\n", fingerId, &c3samp[0], sizeof(struct ts_sample));
    
	memset(down, 0, sizeof(down));

	if (SDL_AddTouch(&touch, "") < 0) 
    {
	    perror("SDL_AddTouch");
	}
	ptr = SDL_GetTouch(touchId);
	ptr->xres = ithLcdGetWidth();
	ptr->yres = ithLcdGetHeight();
    
    rawMode = false;
    rawX = rawY = 0.0f;
}

void Castor3_PumpTouchEvent(void)
{
    int ret;
    int k=0;
    int curFgr = 0;
    int diff = 0;

    if (rawMode)
	        ret = ts_read_raw(ts, &c3samp[0], 1);
    else
	        ret = ts_read(ts, &c3samp[0], 1);

	if (ret < 0) 
    {
		perror("ts_read");
	}

	if (ret != 1)
	{
		if(gLastFingerNum)	ret = gLastFingerNum;
			
		c3samp[0].finger = 0;
	}	
	
#ifdef	CFG_TOUCH_MULTI_FINGER
	curFgr = c3samp[0].finger;
	
	if(gLastFingerNum!=curFgr)
	{
		int m;

		diff = 1;
		
		if(gLastFingerNum>curFgr)
		{
			for(m=curFgr; m<gLastFingerNum; m++)	c3samp[m].pressure = 0;
		}
		
		if(!curFgr)
		{
			for(m=0; m<3; m++)	
			{
				c3samp[m].id = gLastId[m];
				gLastId[m] = 0;
			}
		}
	}
	
	if(gLastFingerNum>curFgr)	curFgr = gLastFingerNum;
#else
	curFgr = 1;
	
	if(c3samp[k].id != 0)
	{
		//workaround the S/W structure issue(force to keep id = 0, if only one finger)
		//printf("	id(%x) != 0:%x,%x\n", c3samp[k].id, k, down[c3samp[k].id]  );
		c3samp[k].id = 0;
	}
#endif
	
	while(curFgr)
	{
		if (c3samp[k].pressure) 
	    {
	        if (down[c3samp[k].id] == false)
	        {
	        	//printf("sdl_sfd[%d]:fid=%d\n",k,c3samp[k].id);
	        	gSdlFingerNum++;
			    SDL_SendFingerDown(touchId, (SDL_FingerID)c3samp[k].id, SDL_TRUE, (float)c3samp[k].x, (float)c3samp[k].y, 1);
			    if(k==0)
			    {
	            	rawX = (float)c3samp[k].x;
	            	rawY = (float)c3samp[k].y;
			    }

	            down[c3samp[k].id] = true;
	        }
	        else
	        {
	        	//printf("sdl_sfm[%d]:fid=%d\n",k,c3samp[k].id);
			    SDL_SendTouchMotion(touchId, (SDL_FingerID)c3samp[k].id, SDL_FALSE, (float)c3samp[k].x, (float)c3samp[k].y, 1);
		    }
	    }
	    else 
	    {
	    	if(down[c3samp[k].id]==true)
	    	{
	    		gSdlFingerNum--;
	    		//printf("	sdl_sfu[%d]:fid=%d, cf=%d, fn=%d\n",k,c3samp[k].id,curFgr,gSdlFingerNum);
				SDL_SendFingerDown(touchId, (SDL_FingerID)c3samp[k].id, SDL_FALSE, (float)c3samp[k].x, (float)c3samp[k].y, 1);	
				down[c3samp[k].id] = false;
	    	}
		}
		
		gLastId[k] = c3samp[k].id;
		
		if(++k >= curFgr)
		{
			if(diff)
			{
				int m;
				//for(m=0; m<5; m++)		printf("	smp1[%d]:id=%d, fgr=%d, dn=%d, (%d, %d, %d)\n",m, c3samp[m].id, c3samp[m].finger, down[m], c3samp[m].pressure, c3samp[m].x,c3samp[m].y);		
			}
			break;
		}
		
		if( (k>5) || (curFgr>5) )
		{
			if(diff)
			{
				int m;
				printf("\n ### k:%d, curFgr:%d\n",k,curFgr);
				for(m=0; m<5; m++)		printf("	smp2[%d]:id=%d, fgr=%d, dn=%d, (%d, %d, %d)\n",m, c3samp[m].id, c3samp[m].finger, down[m], c3samp[m].pressure, c3samp[m].x,c3samp[m].y);		
			}
			break;
		}
	}

#ifdef	CFG_TOUCH_MULTI_FINGER
	if(gLastFingerNum != c3samp[0].finger)	gLastFingerNum = c3samp[0].finger;
#endif
}

int Castor3_CalibrateTouch(float screenX[], float screenY[], float touchX[], float touchY[])
{
    int j;
	float n, x, y, x2, y2, xy, z, zx, zy;
	float det, a, b, c, e, f, i;
	float scaling = 65536.0f;
    int result[7];
    FILE* file;

    // Get sums for matrix
	n = x = y = x2 = y2 = xy = 0;
	for(j=0;j<5;j++) 
    {
		n += 1.0;
		x += touchX[j];
		y += touchY[j];
		x2 += (touchX[j]*touchX[j]);
		y2 += (touchY[j]*touchY[j]);
		xy += (touchX[j]*touchY[j]);
	}

    // Get determinant of matrix -- check if determinant is too small
	det = n*(x2*y2 - xy*xy) + x*(xy*y - x*y2) + y*(x*xy - y*x2);
	if(det < 0.1 && det > -0.1) 
    {
		printf("ts_calibrate: determinant is too small -- %f\n",det);
		return -1;
	}

    // Get elements of inverse matrix
	a = (x2*y2 - xy*xy)/det;
	b = (xy*y - x*y2)/det;
	c = (x*xy - y*x2)/det;
	e = (n*y2 - y*y)/det;
	f = (x*y - n*xy)/det;
	i = (n*x2 - x*x)/det;

    // Get sums for x calibration
	z = zx = zy = 0;
	for(j=0;j<5;j++) 
    {
		z += screenX[j];
		zx += screenX[j]*touchX[j];
		zy += screenX[j]*touchY[j];
	}

    // Now multiply out to get the calibration for framebuffer x coord
	result[0] = (int)((a*z + b*zx + c*zy)*(scaling));
	result[1] = (int)((b*z + e*zx + f*zy)*(scaling));
	result[2] = (int)((c*z + f*zx + i*zy)*(scaling));

	printf("%f %f %f\n",(a*z + b*zx + c*zy),
				(b*z + e*zx + f*zy),
				(c*z + f*zx + i*zy));

    // Get sums for y calibration
	z = zx = zy = 0;
	for(j=0;j<5;j++) 
    {
		z += screenY[j];
		zx += screenY[j]*touchX[j];
		zy += screenY[j]*touchY[j];
	}

    // Now multiply out to get the calibration for framebuffer y coord
	result[3] = (int)((a*z + b*zx + c*zy)*(scaling));
	result[4] = (int)((b*z + e*zx + f*zy)*(scaling));
	result[5] = (int)((c*z + f*zx + i*zy)*(scaling));

	printf("%f %f %f\n",(a*z + b*zx + c*zy),
				(b*z + e*zx + f*zy),
				(c*z + f*zx + i*zy));

    // If we got here, we're OK, so assign scaling to a[6] and return
	result[6] = (int)scaling;

	printf ("Calibration constants: ");
	for (j = 0; j < 7; j++)
        printf("%d ", result[j]);
	printf("\n");

    file = fopen(CFG_PUBLIC_DRIVE ":/pointercal", "w");
    if (file == NULL)
        return -2;

	j = fprintf(file,"%d %d %d %d %d %d %d %d %d",
	              result[1], result[2], result[0],
	              result[4], result[5], result[3], result[6],
	              ithLcdGetWidth(), ithLcdGetHeight());
	fclose(file);

    if (j == 0)
        return -3;

	if (ts_config(ts)) 
    {
		perror("ts_config");
		return -4;
	}

    return 0;
}

void Castor3_ChangeTouchMode(int raw)
{
    rawMode = raw ? true : false;
}

void Castor3_ReadTouchRawPosition(float* x, float* y)
{
    if (x)
        *x = rawX;

    if (y)
        *y = rawY;
}

void Castor3_QuitTouch(void)
{
    ts_close(ts);
}

/* vi: set ts=4 sw=4 expandtab: */
