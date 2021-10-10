#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ite/ith.h"
#include "ite/itu.h"

extern ITUScene theScene;

extern unsigned char  jy_ime(unsigned char *input_jy_val,unsigned char * get_hanzi,unsigned short*hh);

static ITUTextBox* keyboardJYTextBox;
static ITUBackground* keyboardJYBackground;
static ITUButton* keyboardJY0Button;
static ITUButton* keyboardJY1Button;
static ITUButton* keyboardJY2Button;
static ITUButton* keyboardJY3Button;
static ITUButton* keyboardJY4Button;
static ITUButton* keyboardJY5Button;
static ITUButton* keyboardJY6Button;
static ITUButton* keyboardJY7Button;
static ITUButton* keyboardJY8Button;
static ITUButton* keyboardJY9Button;

static int JYIndex;
static int JYStringIndex;
static unsigned char ChsString[1200];
static unsigned char JYInputString[6];
static unsigned char* JYPageIndexes[20];
static int JYPageIndex;


bool KeyboardJYBackButtonOnPress(ITUWidget* widget, char* param)
{
    if (JYIndex >= 0)
    {
        char buf[512];

        strcpy(buf, ituTextGetString(keyboardJYTextBox));
        buf[JYIndex] = '\0';
        ituTextBoxSetString(keyboardJYTextBox, buf);
        JYIndex = -1;
        ituButtonSetString(keyboardJY0Button, NULL);
        ituButtonSetString(keyboardJY1Button, NULL);
        ituButtonSetString(keyboardJY2Button, NULL);
        ituButtonSetString(keyboardJY3Button, NULL);
        ituButtonSetString(keyboardJY4Button, NULL);
        ituButtonSetString(keyboardJY5Button, NULL);
        ituButtonSetString(keyboardJY6Button, NULL);
        ituButtonSetString(keyboardJY7Button, NULL);
        ituButtonSetString(keyboardJY8Button, NULL);
        ituButtonSetString(keyboardJY9Button, NULL);
    }
    else
        ituTextBoxBack(keyboardJYTextBox);
	return true;
}


bool KeyboardJYPageUpButtonOnPress(ITUWidget* widget, char* param)
{
    if (JYPageIndex > 0)
    {
        unsigned char* ptr;
        wchar_t wc;
        int i;

        ituButtonSetString(keyboardJY1Button, NULL);
        ituButtonSetString(keyboardJY2Button, NULL);
        ituButtonSetString(keyboardJY3Button, NULL);
        ituButtonSetString(keyboardJY4Button, NULL);
        ituButtonSetString(keyboardJY5Button, NULL);
        ituButtonSetString(keyboardJY6Button, NULL);
        ituButtonSetString(keyboardJY7Button, NULL);
        ituButtonSetString(keyboardJY8Button, NULL);
        ituButtonSetString(keyboardJY9Button, NULL);
        ituButtonSetString(keyboardJY0Button, NULL);

        i = 0;
        ptr = JYPageIndexes[JYPageIndex - 1];

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
                    btn = keyboardJY0Button;
                else if (i == 1)
                    btn = keyboardJY1Button;
                else if (i == 2)
                    btn = keyboardJY2Button;
                else if (i == 3)
                    btn = keyboardJY3Button;
                else if (i == 4)
                    btn = keyboardJY4Button;
                else if (i == 5)
                    btn = keyboardJY5Button;
                else if (i == 6)
                    btn = keyboardJY6Button;
                else if (i == 7)
                    btn = keyboardJY7Button;
                else if (i == 8)
                    btn = keyboardJY8Button;
                else if (i == 9)
                    btn = keyboardJY9Button;

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

        JYPageIndex--;
    }
    return true;
}

bool KeyboardJYPageDownButtonOnPress(ITUWidget* widget, char* param)
{
    if (JYPageIndex >= 0 && JYPageIndex < ITH_COUNT_OF(JYPageIndexes) - 1 && JYPageIndexes[JYPageIndex + 1])
    {
        unsigned char* ptr;
        wchar_t wc;
        int i;

        ituButtonSetString(keyboardJY0Button, NULL);
        ituButtonSetString(keyboardJY1Button, NULL);
        ituButtonSetString(keyboardJY2Button, NULL);
        ituButtonSetString(keyboardJY3Button, NULL);
        ituButtonSetString(keyboardJY4Button, NULL);
        ituButtonSetString(keyboardJY5Button, NULL);
        ituButtonSetString(keyboardJY6Button, NULL);
        ituButtonSetString(keyboardJY7Button, NULL);
        ituButtonSetString(keyboardJY8Button, NULL);
        ituButtonSetString(keyboardJY9Button, NULL);

        i = 0;
        ptr = JYPageIndexes[JYPageIndex + 1];

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
                    btn = keyboardJY0Button;
                else if (i == 1)
                    btn = keyboardJY1Button;
                else if (i == 2)
                    btn = keyboardJY2Button;
                else if (i == 3)
                    btn = keyboardJY3Button;
                else if (i == 4)
                    btn = keyboardJY4Button;
                else if (i == 5)
                    btn = keyboardJY5Button;
                else if (i == 6)
                    btn = keyboardJY6Button;
                else if (i == 7)
                    btn = keyboardJY7Button;
                else if (i == 8)
                    btn = keyboardJY8Button;
                else if (i == 9)
                    btn = keyboardJY9Button;

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

        JYPageIndex++;

        if (i >= 10)
        {
            JYPageIndexes[JYPageIndex + 1] = ptr;
        }
        else
        {
            JYPageIndexes[JYPageIndex + 1] = NULL;
        }
    }
	return true;
}

bool KeyboardJYNumButtonOnPress(ITUWidget* widget, char* param)
{
	if (JYIndex >= 0)
	{
		char buf[512];
		ITUButton* btn;
		switch (atoi(param))
		{
		case 0: btn = keyboardJY0Button; break;
		case 1: btn = keyboardJY1Button; break;
		case 2: btn = keyboardJY2Button; break;
		case 3: btn = keyboardJY3Button; break;
		case 4: btn = keyboardJY4Button; break;
		case 5: btn = keyboardJY5Button; break;
		case 6: btn = keyboardJY6Button; break;
		case 7: btn = keyboardJY7Button; break;
		case 8: btn = keyboardJY8Button; break;
		case 9: btn = keyboardJY9Button; break;
		}

		strcpy(buf, ituTextGetString(keyboardJYTextBox));
        buf[JYIndex] = '\0';
        strcat(buf, ituTextGetString(&btn->text));
        ituTextBoxSetString((ITUTextBox*)keyboardJYTextBox, buf);
		JYIndex = -1;
        ituButtonSetString(keyboardJY1Button, NULL);
        ituButtonSetString(keyboardJY2Button, NULL);
        ituButtonSetString(keyboardJY3Button, NULL);
        ituButtonSetString(keyboardJY4Button, NULL);
        ituButtonSetString(keyboardJY5Button, NULL);
        ituButtonSetString(keyboardJY6Button, NULL);
        ituButtonSetString(keyboardJY7Button, NULL);
        ituButtonSetString(keyboardJY8Button, NULL);
        ituButtonSetString(keyboardJY9Button, NULL);
        ituButtonSetString(keyboardJY0Button, NULL);
	}
	return true;
}

bool KeyboardJYCharButtonOnPress(ITUWidget* widget, char* param)
{
    ITUButton* btn = (ITUButton*) widget;
    char* input = ituTextGetString(&btn->text);
    char buf[512];
    char* str = ituTextGetString(keyboardJYTextBox);
    int count = str ? strlen(str) : 0;

    if (count < keyboardJYTextBox->maxLen)
    {
            int len;
            unsigned short hh = 0;

            if (JYIndex == -1)
            {
                JYIndex = count;
				memset(JYInputString, 0, 6);
            }
            //if (count - JYIndex < 6)
                ituTextBoxInput((ITUTextBox*)keyboardJYTextBox, input);

            strcat(JYInputString, param);
            len = strlen(JYInputString);
            memset(buf, ' ', 6);
            memcpy(buf, JYInputString, len);
            buf[6] = '\0';
            memset(ChsString, '\0', sizeof(ChsString));
            if (jy_ime(buf, ChsString, &hh))
            {
                unsigned char* ptr;
                wchar_t wc;
                int i;

                ituButtonSetString(keyboardJY1Button, NULL);
                ituButtonSetString(keyboardJY2Button, NULL);
                ituButtonSetString(keyboardJY3Button, NULL);
                ituButtonSetString(keyboardJY4Button, NULL);
                ituButtonSetString(keyboardJY5Button, NULL);
                ituButtonSetString(keyboardJY6Button, NULL);
                ituButtonSetString(keyboardJY7Button, NULL);
                ituButtonSetString(keyboardJY8Button, NULL);
                ituButtonSetString(keyboardJY9Button, NULL);
                ituButtonSetString(keyboardJY0Button, NULL);

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
                            btn = keyboardJY0Button;
                        else if (i == 1)
                            btn = keyboardJY1Button;
                        else if (i == 2)
                            btn = keyboardJY2Button;
                        else if (i == 3)
                            btn = keyboardJY3Button;
                        else if (i == 4)
                            btn = keyboardJY4Button;
                        else if (i == 5)
                            btn = keyboardJY5Button;
                        else if (i == 6)
                            btn = keyboardJY6Button;
                        else if (i == 7)
                            btn = keyboardJY7Button;
                        else if (i == 8)
                            btn = keyboardJY8Button;
                        else if (i == 9)
                            btn = keyboardJY9Button;

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

                JYPageIndex = 0;
                JYPageIndexes[0] = ChsString;

                if (i >= 10)
                {
                    JYPageIndexes[1] = ptr;
                }
                else
                {
                    JYPageIndexes[1] = NULL;
                }
            }
    }

	return true;
}

bool KeyboardJYOnEnter(ITUWidget* widget, char* param)
{
	if (!keyboardJYTextBox)
    {
        keyboardJYTextBox = ituSceneFindWidget(&theScene, "keyboardJYTextBox");
        assert(keyboardJYTextBox);

        keyboardJYBackground = ituSceneFindWidget(&theScene, "keyboardJYBackground");
        assert(keyboardJYBackground);

		keyboardJY0Button = ituSceneFindWidget(&theScene, "keyboardJY0Button");
        assert(keyboardJY0Button);

		keyboardJY1Button = ituSceneFindWidget(&theScene, "keyboardJY1Button");
        assert(keyboardJY1Button);

		keyboardJY2Button = ituSceneFindWidget(&theScene, "keyboardJY2Button");
        assert(keyboardJY2Button);

		keyboardJY3Button = ituSceneFindWidget(&theScene, "keyboardJY3Button");
        assert(keyboardJY3Button);

		keyboardJY4Button = ituSceneFindWidget(&theScene, "keyboardJY4Button");
        assert(keyboardJY4Button);

		keyboardJY5Button = ituSceneFindWidget(&theScene, "keyboardJY5Button");
        assert(keyboardJY5Button);

		keyboardJY6Button = ituSceneFindWidget(&theScene, "keyboardJY6Button");
        assert(keyboardJY6Button);

		keyboardJY7Button = ituSceneFindWidget(&theScene, "keyboardJY7Button");
        assert(keyboardJY7Button);

		keyboardJY8Button = ituSceneFindWidget(&theScene, "keyboardJY8Button");
        assert(keyboardJY8Button);

		keyboardJY9Button = ituSceneFindWidget(&theScene, "keyboardJY9Button");
        assert(keyboardJY9Button);
	}

    ituButtonSetString(keyboardJY0Button, NULL);
    ituButtonSetString(keyboardJY1Button, NULL);
    ituButtonSetString(keyboardJY2Button, NULL);
    ituButtonSetString(keyboardJY3Button, NULL);
    ituButtonSetString(keyboardJY4Button, NULL);
    ituButtonSetString(keyboardJY5Button, NULL);
    ituButtonSetString(keyboardJY6Button, NULL);
    ituButtonSetString(keyboardJY7Button, NULL);
    ituButtonSetString(keyboardJY8Button, NULL);
    ituButtonSetString(keyboardJY9Button, NULL);

    JYIndex = JYPageIndex = -1;
	return true;
}

