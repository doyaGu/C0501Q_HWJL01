#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ite/ith.h"
#include "ite/itu.h"

extern ITUScene theScene;

extern unsigned char  py_ime(unsigned char *input_py_val,unsigned char * get_hanzi,unsigned short*hh);

static ITUTextBox* keyboardPYTextBox;
static ITUBackground* keyboardPYBackground;
static ITUButton* keyboardPY0Button;
static ITUButton* keyboardPY1Button;
static ITUButton* keyboardPY2Button;
static ITUButton* keyboardPY3Button;
static ITUButton* keyboardPY4Button;
static ITUButton* keyboardPY5Button;
static ITUButton* keyboardPY6Button;
static ITUButton* keyboardPY7Button;
static ITUButton* keyboardPY8Button;
static ITUButton* keyboardPY9Button;

static int PYIndex;
static int PYStringIndex;
static unsigned char ChsString[1200];
static unsigned char PYInputString[6];
static unsigned char* PYPageIndexes[20];
static int PYPageIndex;


bool KeyboardPYBackButtonOnPress(ITUWidget* widget, char* param)
{
    if (PYIndex >= 0)
    {
        char buf[512];

        strcpy(buf, ituTextGetString(keyboardPYTextBox));
        buf[PYIndex] = '\0';
        ituTextBoxSetString(keyboardPYTextBox, buf);
        PYIndex = -1;
        ituButtonSetString(keyboardPY0Button, NULL);
        ituButtonSetString(keyboardPY1Button, NULL);
        ituButtonSetString(keyboardPY2Button, NULL);
        ituButtonSetString(keyboardPY3Button, NULL);
        ituButtonSetString(keyboardPY4Button, NULL);
        ituButtonSetString(keyboardPY5Button, NULL);
        ituButtonSetString(keyboardPY6Button, NULL);
        ituButtonSetString(keyboardPY7Button, NULL);
        ituButtonSetString(keyboardPY8Button, NULL);
        ituButtonSetString(keyboardPY9Button, NULL);
    }
    else
        ituTextBoxBack(keyboardPYTextBox);
	return true;
}


bool KeyboardPYPageUpButtonOnPress(ITUWidget* widget, char* param)
{
    if (PYPageIndex > 0)
    {
        unsigned char* ptr;
        wchar_t wc;
        int i;

        ituButtonSetString(keyboardPY1Button, NULL);
        ituButtonSetString(keyboardPY2Button, NULL);
        ituButtonSetString(keyboardPY3Button, NULL);
        ituButtonSetString(keyboardPY4Button, NULL);
        ituButtonSetString(keyboardPY5Button, NULL);
        ituButtonSetString(keyboardPY6Button, NULL);
        ituButtonSetString(keyboardPY7Button, NULL);
        ituButtonSetString(keyboardPY8Button, NULL);
        ituButtonSetString(keyboardPY9Button, NULL);
        ituButtonSetString(keyboardPY0Button, NULL);

        i = 0;
        ptr = PYPageIndexes[PYPageIndex - 1];

        do
        {
            int len = strlen(ptr);

            if (len > 0)
                len = mbtowc(&wc, ptr, len);

            if (len > 0)
            {
                ITUButton* btn = NULL;
                char buf[512];
                memcpy(buf, ptr, len);
                buf[len] = '\0';

                if (i == 0)
                    btn = keyboardPY0Button;
                else if (i == 1)
                    btn = keyboardPY1Button;
                else if (i == 2)
                    btn = keyboardPY2Button;
                else if (i == 3)
                    btn = keyboardPY3Button;
                else if (i == 4)
                    btn = keyboardPY4Button;
                else if (i == 5)
                    btn = keyboardPY5Button;
                else if (i == 6)
                    btn = keyboardPY6Button;
                else if (i == 7)
                    btn = keyboardPY7Button;
                else if (i == 8)
                    btn = keyboardPY8Button;
                else if (i == 9)
                    btn = keyboardPY9Button;

                if (btn)
                    ituButtonSetString(btn, buf);
            }
            else
            {
                break;
            }
            i++;
            ptr += len;

        } while (i < 10);

        PYPageIndex--;
    }
    return true;
}

bool KeyboardPYPageDownButtonOnPress(ITUWidget* widget, char* param)
{
    if (PYPageIndex >= 0 && PYPageIndex < ITH_COUNT_OF(PYPageIndexes) - 1 && PYPageIndexes[PYPageIndex + 1])
    {
        unsigned char* ptr;
        wchar_t wc;
        int i;

        ituButtonSetString(keyboardPY0Button, NULL);
        ituButtonSetString(keyboardPY1Button, NULL);
        ituButtonSetString(keyboardPY2Button, NULL);
        ituButtonSetString(keyboardPY3Button, NULL);
        ituButtonSetString(keyboardPY4Button, NULL);
        ituButtonSetString(keyboardPY5Button, NULL);
        ituButtonSetString(keyboardPY6Button, NULL);
        ituButtonSetString(keyboardPY7Button, NULL);
        ituButtonSetString(keyboardPY8Button, NULL);
        ituButtonSetString(keyboardPY9Button, NULL);

        i = 0;
        ptr = PYPageIndexes[PYPageIndex + 1];

        do
        {
            int len = strlen(ptr);

            if (len > 0)
                len = mbtowc(&wc, ptr, len);

            if (len > 0)
            {
                ITUButton* btn = NULL;
                char buf[512];
                memcpy(buf, ptr, len);
                buf[len] = '\0';

                if (i == 0)
                    btn = keyboardPY0Button;
                else if (i == 1)
                    btn = keyboardPY1Button;
                else if (i == 2)
                    btn = keyboardPY2Button;
                else if (i == 3)
                    btn = keyboardPY3Button;
                else if (i == 4)
                    btn = keyboardPY4Button;
                else if (i == 5)
                    btn = keyboardPY5Button;
                else if (i == 6)
                    btn = keyboardPY6Button;
                else if (i == 7)
                    btn = keyboardPY7Button;
                else if (i == 8)
                    btn = keyboardPY8Button;
                else if (i == 9)
                    btn = keyboardPY9Button;

                if (btn)
                    ituButtonSetString(btn, buf);
            }
            else
            {
                break;
            }
            i++;
            ptr += len;

        } while (i < 10);

        PYPageIndex++;

        if (i >= 10)
        {
            PYPageIndexes[PYPageIndex + 1] = ptr;
        }
        else
        {
            PYPageIndexes[PYPageIndex + 1] = NULL;
        }
    }
	return true;
}

bool KeyboardPYNumButtonOnPress(ITUWidget* widget, char* param)
{
	if (PYIndex >= 0)
	{
		char buf[512];
		ITUButton* btn;
		switch (atoi(param))
		{
		case 0: btn = keyboardPY0Button; break;
		case 1: btn = keyboardPY1Button; break;
		case 2: btn = keyboardPY2Button; break;
		case 3: btn = keyboardPY3Button; break;
		case 4: btn = keyboardPY4Button; break;
		case 5: btn = keyboardPY5Button; break;
		case 6: btn = keyboardPY6Button; break;
		case 7: btn = keyboardPY7Button; break;
		case 8: btn = keyboardPY8Button; break;
		case 9: btn = keyboardPY9Button; break;
		}

		strcpy(buf, ituTextGetString(keyboardPYTextBox));
        buf[PYIndex] = '\0';
        strcat(buf, ituTextGetString(&btn->text));
        ituTextBoxSetString((ITUTextBox*)keyboardPYTextBox, buf);
		PYIndex = -1;
        ituButtonSetString(keyboardPY1Button, NULL);
        ituButtonSetString(keyboardPY2Button, NULL);
        ituButtonSetString(keyboardPY3Button, NULL);
        ituButtonSetString(keyboardPY4Button, NULL);
        ituButtonSetString(keyboardPY5Button, NULL);
        ituButtonSetString(keyboardPY6Button, NULL);
        ituButtonSetString(keyboardPY7Button, NULL);
        ituButtonSetString(keyboardPY8Button, NULL);
        ituButtonSetString(keyboardPY9Button, NULL);
        ituButtonSetString(keyboardPY0Button, NULL);
	}
	return true;
}

bool KeyboardPYCharButtonOnPress(ITUWidget* widget, char* param)
{
    ITUButton* btn = (ITUButton*) widget;
    char* input = ituTextGetString(&btn->text);
    char buf[512];
    char* str = ituTextGetString(keyboardPYTextBox);
    int count = str ? strlen(str) : 0;

    if (count < keyboardPYTextBox->maxLen)
    {
            int len;
            unsigned short hh = 0;

            if (PYIndex == -1)
            {
                PYIndex = count;
				memset(PYInputString, 0, 6);
            }
            //if (count - PYIndex < 6)
                ituTextBoxInput((ITUTextBox*)keyboardPYTextBox, input);

            strcat(PYInputString, param);
            len = strlen(PYInputString);
            memset(buf, ' ', 6);
            memcpy(buf, PYInputString, len);
            buf[6] = '\0';
            memset(ChsString, '\0', sizeof(ChsString));
            if (py_ime(buf, ChsString, &hh))
            {
                unsigned char* ptr;
                wchar_t wc;
                int i;

                ituButtonSetString(keyboardPY1Button, NULL);
                ituButtonSetString(keyboardPY2Button, NULL);
                ituButtonSetString(keyboardPY3Button, NULL);
                ituButtonSetString(keyboardPY4Button, NULL);
                ituButtonSetString(keyboardPY5Button, NULL);
                ituButtonSetString(keyboardPY6Button, NULL);
                ituButtonSetString(keyboardPY7Button, NULL);
                ituButtonSetString(keyboardPY8Button, NULL);
                ituButtonSetString(keyboardPY9Button, NULL);
                ituButtonSetString(keyboardPY0Button, NULL);

                i = 0;
                ptr = ChsString;

                do
                {
                    len = strlen(ptr);

                    if (len > 0)
                        len = mbtowc(&wc, ptr, len);

                    if (len > 0)
                    {
                        ITUButton* btn = NULL;
                        memcpy(buf, ptr, len);
                        buf[len] = '\0';

                        if (i == 0)
                            btn = keyboardPY0Button;
                        else if (i == 1)
                            btn = keyboardPY1Button;
                        else if (i == 2)
                            btn = keyboardPY2Button;
                        else if (i == 3)
                            btn = keyboardPY3Button;
                        else if (i == 4)
                            btn = keyboardPY4Button;
                        else if (i == 5)
                            btn = keyboardPY5Button;
                        else if (i == 6)
                            btn = keyboardPY6Button;
                        else if (i == 7)
                            btn = keyboardPY7Button;
                        else if (i == 8)
                            btn = keyboardPY8Button;
                        else if (i == 9)
                            btn = keyboardPY9Button;

                        if (btn)
                            ituButtonSetString(btn, buf);
                    }
                    else
                    {
                        break;
                    }
                    i++;
                    ptr += len;

                } while (i < 10);

                PYPageIndex = 0;
                PYPageIndexes[0] = ChsString;

                if (i >= 10)
                {
                    PYPageIndexes[1] = ptr;
                }
                else
                {
                    PYPageIndexes[1] = NULL;
                }
            }
    }

	return true;
}

bool KeyboardPYOnEnter(ITUWidget* widget, char* param)
{
	if (!keyboardPYTextBox)
    {
        keyboardPYTextBox = ituSceneFindWidget(&theScene, "keyboardPYTextBox");
        assert(keyboardPYTextBox);

        keyboardPYBackground = ituSceneFindWidget(&theScene, "keyboardPYBackground");
        assert(keyboardPYBackground);

		keyboardPY0Button = ituSceneFindWidget(&theScene, "keyboardPY0Button");
        assert(keyboardPY0Button);

		keyboardPY1Button = ituSceneFindWidget(&theScene, "keyboardPY1Button");
        assert(keyboardPY1Button);

		keyboardPY2Button = ituSceneFindWidget(&theScene, "keyboardPY2Button");
        assert(keyboardPY2Button);

		keyboardPY3Button = ituSceneFindWidget(&theScene, "keyboardPY3Button");
        assert(keyboardPY3Button);

		keyboardPY4Button = ituSceneFindWidget(&theScene, "keyboardPY4Button");
        assert(keyboardPY4Button);

		keyboardPY5Button = ituSceneFindWidget(&theScene, "keyboardPY5Button");
        assert(keyboardPY5Button);

		keyboardPY6Button = ituSceneFindWidget(&theScene, "keyboardPY6Button");
        assert(keyboardPY6Button);

		keyboardPY7Button = ituSceneFindWidget(&theScene, "keyboardPY7Button");
        assert(keyboardPY7Button);

		keyboardPY8Button = ituSceneFindWidget(&theScene, "keyboardPY8Button");
        assert(keyboardPY8Button);

		keyboardPY9Button = ituSceneFindWidget(&theScene, "keyboardPY9Button");
        assert(keyboardPY9Button);
	}

    ituButtonSetString(keyboardPY0Button, NULL);
    ituButtonSetString(keyboardPY1Button, NULL);
    ituButtonSetString(keyboardPY2Button, NULL);
    ituButtonSetString(keyboardPY3Button, NULL);
    ituButtonSetString(keyboardPY4Button, NULL);
    ituButtonSetString(keyboardPY5Button, NULL);
    ituButtonSetString(keyboardPY6Button, NULL);
    ituButtonSetString(keyboardPY7Button, NULL);
    ituButtonSetString(keyboardPY8Button, NULL);
    ituButtonSetString(keyboardPY9Button, NULL);

    PYIndex = PYPageIndex = -1;
	return true;
}

