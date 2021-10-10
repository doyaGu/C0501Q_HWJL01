/*
 * Copyright (c) 2011 ITE technology Corp. All Rights Reserved.
 */
/** @file
 * create brush object.
 *
 * @author James Lin
 * @version 1.0
 */

#include "m2d/m2d_graphics.h"
#include "m2d/m2d_dispctxt.h"
#include "m2d/m2d_engine.h"
#include "m2d/m2d_line.h"
#include "m2d/m2d_rotate.h"

#if 1

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

//=============================================================================
//                              Function Definition
//=============================================================================
/**
 * Draw a line from current position up to the specified point.
 *
 * @param display       display context handle.
 * @param endX          x-coordinate of ending point.
 * @param endY          y-coordinate of ending point.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
MMP_RESULT
mmpM2dDrawLine(
    MMP_M2D_SURFACE destDisplay,
    MMP_INT         endX,
    MMP_INT         endY)
{
    MMP_BOOL      result     = MMP_FALSE;
    MMP_UINT8     *lcdBuffer = NULL;
    MMP_UINT32    i          = 0;
    MMP_UINT32    penWidth   = ((M2D_SURFACE *)destDisplay)->pen->penWidth;
    MMP_M2D_POINT oriPos     = ((M2D_SURFACE *)destDisplay)->currPos;

    MMP_INT32     lineSlope  = 0;
    MMP_INT32     lineDirection;

    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(destDisplay))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    if (IS_INVALIDATE_IMAGE_FORMAT(destDisplay))
    {
        return MMP_RESULT_ERROR_DEST_DISP_FORMAT;
    }

    // Calculate line slope
    if ((endX - ((M2D_SURFACE *)destDisplay)->currPos.x) != 0)
    {
        if (endY > ((M2D_SURFACE *)destDisplay)->currPos.y
         && endX > ((M2D_SURFACE *)destDisplay)->currPos.x)
        {
            // lineSlope > 0
            lineSlope = 1;
        }
        else if (endY < ((M2D_SURFACE *)destDisplay)->currPos.y
              && endX < ((M2D_SURFACE *)destDisplay)->currPos.x)
        {
            // lineSlope > 0
            lineSlope = 1;
        }
        else
        {
            // lineSlope < 0
            lineSlope = -1;
        }
    } // End of lineSlope

    // Check line direction
    if (endX == ((M2D_SURFACE *)destDisplay)->currPos.x)
    {
        lineDirection = LINE_VERTICAL;
    }
    else if (endY == ((M2D_SURFACE *)destDisplay)->currPos.y)
    {
        lineDirection = LINE_HORIZONTAL;
    }
    else if (lineSlope > 0)
    {
        lineDirection = LINE_LEFT_BOTTOM;
    }
    else if (lineSlope <= 0)
    {
        lineDirection = LINE_RIGHT_BOTTOM;
    }//End of Check line direction

    for (i = 0; i < penWidth; i++)
    {
        if (i != 0)
        {
            switch (lineDirection)
            {
            case LINE_VERTICAL:
                ((M2D_SURFACE *)destDisplay)->currPos.x += 1;
                endX                                    += 1;
                break;
            case LINE_HORIZONTAL:
                ((M2D_SURFACE *)destDisplay)->currPos.y += 1;
                endY                                    += 1;
                break;
            case LINE_RIGHT_BOTTOM:
                if (i % 2 == 1) //singular
                {
                    ((M2D_SURFACE *)destDisplay)->currPos.y += 1;
                    endY                                    += 1;
                }
                else
                {
                    ((M2D_SURFACE *)destDisplay)->currPos.x += 1;
                    endX                                    += 1;
                }//End of if(i%2==1)
                break;
            case LINE_LEFT_BOTTOM:
                if (i % 2 == 1) //singular
                {
                    ((M2D_SURFACE *)destDisplay)->currPos.x -= 1;
                    endX                                    -= 1;
                }
                else
                {
                    ((M2D_SURFACE *)destDisplay)->currPos.y += 1;
                    endY                                    += 1;
                } //End of if(i%2==1)
                break;
            }     // End of switch(lineDirection)
        }         //End of if(i != 0)

        //HW
        result = m2dDrawLine(
            (M2D_SURFACE *)destDisplay,
            ((M2D_SURFACE *)destDisplay)->currPos.x,
            ((M2D_SURFACE *)destDisplay)->currPos.y,
            endX,
            endY,
            ((M2D_SURFACE *)destDisplay)->pen->penColor,
            ((M2D_SURFACE *)destDisplay)->pen->penStyle,
            ((M2D_SURFACE *)destDisplay)->pen->penWidth);

        if (result == MMP_FALSE)
            return MMP_RESULT_ERROR;
    }//End of for(i=0;i<penWidth;i++)
    ((M2D_SURFACE *)destDisplay)->currPos = oriPos;

    return MMP_RESULT_SUCCESS;
}

    #ifndef SHRINK_CODE_SIZE
/**
 * Draw a series of lines by connecting the points in the specified point
 * array.
 *
 * @param display       display context handle.
 * @param points        array of endpoints.
 * @param pointCount    the amount of the points in array.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
MMP_RESULT
mmpM2dDrawPolyLines(
    MMP_M2D_SURFACE destDisplay,
    MMP_M2D_POINT   *points,
    MMP_UINT        pointCount)
{
    MMP_UINT8 *lcdBuffer = NULL;

    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(destDisplay))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    if (IS_INVALIDATE_IMAGE_FORMAT(destDisplay))
    {
        return MMP_RESULT_ERROR_DEST_DISP_FORMAT;
    }

    //HW
    if (M2D_DrawPolyLines((M2D_SURFACE *)destDisplay,
                          points,
                          pointCount)
        != MMP_TRUE)
    {
        return MMP_RESULT_ERROR;
    }

    return MMP_RESULT_SUCCESS;
}
    #endif

/**
 * Draw a hollow rectangle by the current pen.
 *
 * @param display   display context handle.
 * @param startX    x-coordinate of the upper-left corner of the rectangle.
 * @param startY    y-coordinate of the upper-left corner of the rectangle.
 * @param width     width of the rectangle.
 * @param height    height of the rectangle.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
MMP_RESULT
mmpM2dDrawRectangle(
    MMP_M2D_SURFACE display,
    MMP_INT         startX,
    MMP_INT         startY,
    MMP_UINT        width,
    MMP_UINT        height)
{
    MMP_UINT32    i        = 0;
    MMP_INT32     penWidth = 0;
    MMP_M2D_POINT points[4];
    MMP_M2D_POINT oriPos   = ((M2D_SURFACE *)display)->currPos;

    penWidth                            = ((M2D_SURFACE *)display)->pen->penWidth;

    points[0].x                         = startX + width;
    points[0].y                         = startY;
    points[1].x                         = startX + width;
    points[1].y                         = startY + height;
    points[2].x                         = startX;
    points[2].y                         = startY + height;
    points[3].x                         = startX;
    points[3].y                         = startY;

    ((M2D_SURFACE *)display)->currPos.x = startX;
    ((M2D_SURFACE *)display)->currPos.y = startY;

    for (i = 0; i < 4; i++)
    {
        if (i != 0)
        {
            ((M2D_SURFACE *)display)->currPos.x = points[i - 1].x;
            if (i == 2)
                ((M2D_SURFACE *)display)->currPos.y = points[i - 1].y - penWidth + 1;
            else
                ((M2D_SURFACE *)display)->currPos.y = points[i - 1].y;
        } // End of if(i!=0)

        if (i == 1)
        {
            if (mmpM2dDrawLine(display,
                               points[i].x,
                               points[i].y + 1)
                != MMP_RESULT_SUCCESS)
            {
                return MMP_RESULT_ERROR;
            }
        }
        else if (i == 2)
        {
            if (mmpM2dDrawLine(display,
                               points[i].x,
                               points[i].y - penWidth + 1)
                != MMP_RESULT_SUCCESS)
            {
                return MMP_RESULT_ERROR;
            }
        }
        else
        {
            if (mmpM2dDrawLine(display,
                               points[i].x,
                               points[i].y)
                != MMP_RESULT_SUCCESS)
            {
                return MMP_RESULT_ERROR;
            }
        } // End of if(i==1 || i==2)
    }     // End of for(i=0;i<4;i++)
    ((M2D_SURFACE *)display)->currPos = oriPos;

    return MMP_RESULT_SUCCESS;
}//End of mmpM2dDrawRectangle()

    #ifndef SHRINK_CODE_SIZE
/**
 * Draw a series of hollow rectangles in the specified rectangle array by the
 * current pen.
 *
 * @param display       display context handle.
 * @param rects         array of the rectangles.
 * @param rectCount     the amount of the rectangles in array.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
MMP_RESULT
mmpM2dDrawPolyRectangle(
    MMP_M2D_SURFACE display,
    MMP_M2D_RECT    *rects,
    MMP_UINT        rectCount)
{
    MMP_UINT32 i;
    MMP_INT32  startX, startY;
    MMP_UINT32 width, height;

    for (i = 0; i < rectCount; i++)
    {
        startX = rects[i].left;
        startY = rects[i].top;

        width  = rects[i].right - rects[i].left;
        height = rects[i].bottom - rects[i].top;

        if (mmpM2dDrawRectangle(display,
                                startX,
                                startY,
                                width,
                                height)
            != MMP_RESULT_SUCCESS)
        {
            return MMP_RESULT_ERROR;
        }
    }

    return MMP_RESULT_SUCCESS;
}//End of mmpDrawPolyRectangle()
    #endif

    #ifndef SHRINK_CODE_SIZE
/**
 * Draw a series of lines by connecting the points in the specified point
 * array.
 *
 * @param display       display context handle.
 * @param points        array of endpoints.
 * @param pointCount    the amount of the points in array.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
MMP_BOOL
M2D_DrawPolyLines(
    M2D_SURFACE   *display,
    MMP_M2D_POINT *points,
    MMP_UINT      pointCount)
{
    MMP_BOOL      result        = MMP_FALSE;
    MMP_UINT32    i, j;
    MMP_UINT32    penWidth      = display->pen->penWidth;
    MMP_M2D_POINT oriPos        = display->currPos;
    MMP_INT32     endX, endY;

    MMP_INT32     lineSlope     = 0;
    MMP_INT32     lineDirection = 0;

    for (i = 0; i < pointCount; i++)
    {
        if (i != 0)
        {
            display->isResetLineStyle = 1;
            display->currPos          = points[i - 1];
        }
        else
        {
            display->isResetLineStyle = 0;
        }
        endX = points[i].x;
        endY = points[i].y;

        // Calculate line slope
        if (endX - display->currPos.x != 0)
        {
            if (endY > display->currPos.y && endX > display->currPos.x)
            {
                // lineSlope > 0
                lineSlope = 1;
            }
            else if (endY < display->currPos.y && endX < display->currPos.x)
            {
                // lineSlope > 0
                lineSlope = 1;
            }
            else
            {
                // lineSlope < 0
                lineSlope = -1;
            }
        } // End of lineSlope

        // Check line direction
        if (endX == display->currPos.x)
        {
            lineDirection = LINE_VERTICAL;
        }
        else if (endY == display->currPos.y)
        {
            lineDirection = LINE_HORIZONTAL;
        }
        else if (lineSlope > 0)
        {
            lineDirection = LINE_LEFT_BOTTOM;
        }
        else if (lineSlope < 0)
        {
            lineDirection = LINE_RIGHT_BOTTOM;
        }//End of Check line direction

        // If pen width > 1
        for (j = 0; j < penWidth; j++)
        {
            if (j != 0)
            {
                //Set new x,y coodinates
                switch (lineDirection)
                {
                case LINE_VERTICAL:
                    display->currPos.x += 1;
                    endX               += 1;
                    break;
                case LINE_HORIZONTAL:
                    display->currPos.y += 1;
                    endY               += 1;
                    break;
                case LINE_RIGHT_BOTTOM:
                    if (j % 2 == 1)
                    {
                        display->currPos.y += 1;
                        endY               += 1;
                    }
                    else
                    {
                        display->currPos.x += 1;
                        endX               += 1;
                    }//End of if(j%2==1)
                    break;
                case LINE_LEFT_BOTTOM:
                    if (j % 2 == 1)
                    {
                        display->currPos.x -= 1;
                        endX               -= 1;
                    }
                    else
                    {
                        display->currPos.y += 1;
                        endY               += 1;
                    } //End of if(j%2==1)
                    break;
                }     // End of switch(lineDirection)
            }         //End of if(j != 0)

            //HW
            result = m2dDrawLine(
                (M2D_SURFACE *)display,
                ((M2D_SURFACE *)display)->currPos.x,
                ((M2D_SURFACE *)display)->currPos.y,
                endX,
                endY,
                ((M2D_SURFACE *)display)->pen->penColor,
                ((M2D_SURFACE *)display)->pen->penStyle,
                ((M2D_SURFACE *)display)->pen->penWidth);

            if (result == MMP_FALSE)
                return MMP_FALSE;
        } //End of for(j=0;j<penWidth;j++)
    }     //End of for(i=0;i<pointCount;i++)

    display->currPos = oriPos;

    return MMP_TRUE;
}//End of M2D_DrawPolySolidLines()
    #endif

#endif