#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "ite/ith.h"

extern unsigned char  py_ime(unsigned char *input_py_val,unsigned char * get_hanzi,unsigned short*hh);

static ITULayer* mainMenuLayer;
static ITUTextBox* keyboard2TextBox;
static ITUKeyboard* keyboard2Keyboard;
static ITUBackground* keyboard2EnUpperBackground;
static ITUBackground* keyboard2EnLowerBackground;
static ITUBackground* keyboard2ChsBackground;
static ITUBackground* keyboard2SymbolBackground;
static ITUButton* keyboard2Chs1Button;
static ITUButton* keyboard2Chs2Button;
static ITUButton* keyboard2Chs3Button;
static ITUButton* keyboard2Chs4Button;
static ITUButton* keyboard2Chs5Button;
static ITUButton* keyboard2Chs6Button;
static ITUButton* keyboard2Chs7Button;
static ITUButton* keyboard2Chs8Button;
static ITUButton* keyboard2Chs9Button;
static ITUButton* keyboard2Chs0Button;
static ITUButton* keyboard2SymbolButton;
static ITUButton* keyboard2EnUpperButton;
static ITUButton* keyboard2EnLowerButton;
static ITUButton* keyboard2ChsButton;

static int keyboard2ChsIndex;
static unsigned char keyboard2ChsString[1200];
static unsigned char* keyboard2ChsPageIndexes[20];
static int keyboard2ChsPageIndex;

bool Keyboard2EnterButtonOnPress(ITUWidget* widget, char* param)
{
    ituTextBoxSetString(keyboard2TextBox, NULL);
	return true;
}

bool Keyboard2BackButtonOnPress(ITUWidget* widget, char* param)
{
    if (keyboard2ChsIndex >= 0)
    {
        char buf[512];

        strcpy(buf, ituTextGetString(keyboard2Keyboard->target));
        buf[keyboard2ChsIndex] = '\0';
        ituTextBoxSetString((ITUTextBox*)keyboard2Keyboard->target, buf);
        keyboard2ChsIndex = -1;
        ituButtonSetString(keyboard2Chs1Button, NULL);
        ituButtonSetString(keyboard2Chs2Button, NULL);
        ituButtonSetString(keyboard2Chs3Button, NULL);
        ituButtonSetString(keyboard2Chs4Button, NULL);
        ituButtonSetString(keyboard2Chs5Button, NULL);
        ituButtonSetString(keyboard2Chs6Button, NULL);
        ituButtonSetString(keyboard2Chs7Button, NULL);
        ituButtonSetString(keyboard2Chs8Button, NULL);
        ituButtonSetString(keyboard2Chs9Button, NULL);
        ituButtonSetString(keyboard2Chs0Button, NULL);
    }
    else
    {
        ituTextBoxBack((ITUTextBox*)keyboard2Keyboard->target);
    }
    keyboard2ChsPageIndex = -1;
	return true;
}

bool Keyboard2EnUpperButtonOnPress(ITUWidget* widget, char* param)
{
    keyboard2ChsPageIndex = -1;
	return true;
}

bool Keyboard2ChsButtonOnPress(ITUWidget* widget, char* param)
{
    ituButtonSetString(keyboard2Chs1Button, NULL);
    ituButtonSetString(keyboard2Chs2Button, NULL);
    ituButtonSetString(keyboard2Chs3Button, NULL);
    ituButtonSetString(keyboard2Chs4Button, NULL);
    ituButtonSetString(keyboard2Chs5Button, NULL);
    ituButtonSetString(keyboard2Chs6Button, NULL);
    ituButtonSetString(keyboard2Chs7Button, NULL);
    ituButtonSetString(keyboard2Chs8Button, NULL);
    ituButtonSetString(keyboard2Chs9Button, NULL);
    ituButtonSetString(keyboard2Chs0Button, NULL);
	return true;
}

bool Keyboard2PageUpButtonOnPress(ITUWidget* widget, char* param)
{
    if (keyboard2ChsPageIndex > 0)
    {
        unsigned char* ptr;
        wchar_t wc;
        int i;

        ituButtonSetString(keyboard2Chs1Button, NULL);
        ituButtonSetString(keyboard2Chs2Button, NULL);
        ituButtonSetString(keyboard2Chs3Button, NULL);
        ituButtonSetString(keyboard2Chs4Button, NULL);
        ituButtonSetString(keyboard2Chs5Button, NULL);
        ituButtonSetString(keyboard2Chs6Button, NULL);
        ituButtonSetString(keyboard2Chs7Button, NULL);
        ituButtonSetString(keyboard2Chs8Button, NULL);
        ituButtonSetString(keyboard2Chs9Button, NULL);
        ituButtonSetString(keyboard2Chs0Button, NULL);

        i = 0;
        ptr = keyboard2ChsPageIndexes[keyboard2ChsPageIndex - 1];

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
                    btn = keyboard2Chs1Button;
                else if (i == 1)
                    btn = keyboard2Chs2Button;
                else if (i == 2)
                    btn = keyboard2Chs3Button;
                else if (i == 3)
                    btn = keyboard2Chs4Button;
                else if (i == 4)
                    btn = keyboard2Chs5Button;
                else if (i == 5)
                    btn = keyboard2Chs6Button;
                else if (i == 6)
                    btn = keyboard2Chs7Button;
                else if (i == 7)
                    btn = keyboard2Chs8Button;
                else if (i == 8)
                    btn = keyboard2Chs9Button;
                else if (i == 9)
                    btn = keyboard2Chs0Button;

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

        keyboard2ChsPageIndex--;
    }
    return true;
}

bool Keyboard2PageDownButtonOnPress(ITUWidget* widget, char* param)
{
    if (keyboard2ChsPageIndex >= 0 && keyboard2ChsPageIndex < ITH_COUNT_OF(keyboard2ChsPageIndexes) - 1 && keyboard2ChsPageIndexes[keyboard2ChsPageIndex + 1])
    {
        unsigned char* ptr;
        wchar_t wc;
        int i;

        ituButtonSetString(keyboard2Chs1Button, NULL);
        ituButtonSetString(keyboard2Chs2Button, NULL);
        ituButtonSetString(keyboard2Chs3Button, NULL);
        ituButtonSetString(keyboard2Chs4Button, NULL);
        ituButtonSetString(keyboard2Chs5Button, NULL);
        ituButtonSetString(keyboard2Chs6Button, NULL);
        ituButtonSetString(keyboard2Chs7Button, NULL);
        ituButtonSetString(keyboard2Chs8Button, NULL);
        ituButtonSetString(keyboard2Chs9Button, NULL);
        ituButtonSetString(keyboard2Chs0Button, NULL);

        i = 0;
        ptr = keyboard2ChsPageIndexes[keyboard2ChsPageIndex + 1];

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
                    btn = keyboard2Chs1Button;
                else if (i == 1)
                    btn = keyboard2Chs2Button;
                else if (i == 2)
                    btn = keyboard2Chs3Button;
                else if (i == 3)
                    btn = keyboard2Chs4Button;
                else if (i == 4)
                    btn = keyboard2Chs5Button;
                else if (i == 5)
                    btn = keyboard2Chs6Button;
                else if (i == 6)
                    btn = keyboard2Chs7Button;
                else if (i == 7)
                    btn = keyboard2Chs8Button;
                else if (i == 8)
                    btn = keyboard2Chs9Button;
                else if (i == 9)
                    btn = keyboard2Chs0Button;

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

        keyboard2ChsPageIndex++;

        if (i >= 10)
        {
            keyboard2ChsPageIndexes[keyboard2ChsPageIndex + 1] = ptr;
        }
        else
        {
            keyboard2ChsPageIndexes[keyboard2ChsPageIndex + 1] = NULL;
        }
    }
	return true;
}

bool Keyboard2ChsCharButtonOnPress(ITUWidget* widget, char* param)
{
    ITUButton* btn = (ITUButton*) widget;
    char* input = ituTextGetString(&btn->text);
    char buf[512];

    if (*input >= 'a' && *input <= 'z')
    {
        char* str = ituTextGetString(keyboard2Keyboard->target);
        int count = str ? strlen(str) : 0;

        if (count < keyboard2TextBox->maxLen)
        {
            int len;
            unsigned short hh = 0;

            if (keyboard2ChsIndex == -1)
            {
                keyboard2ChsIndex = count;
            }
            if (count - keyboard2ChsIndex < 6)
                ituTextBoxInput((ITUTextBox*)keyboard2Keyboard->target, input);

            str = ituTextGetString(keyboard2Keyboard->target);
            len = strlen(&str[keyboard2ChsIndex]);
            memset(buf, ' ', 6);
            memcpy(buf, &str[keyboard2ChsIndex], len);
            buf[6] = '\0';
            memset(keyboard2ChsString, '\0', sizeof(keyboard2ChsString));
            if (py_ime(buf, keyboard2ChsString, &hh))
            {
                unsigned char* ptr;
                wchar_t wc;
                int i;

                ituButtonSetString(keyboard2Chs1Button, NULL);
                ituButtonSetString(keyboard2Chs2Button, NULL);
                ituButtonSetString(keyboard2Chs3Button, NULL);
                ituButtonSetString(keyboard2Chs4Button, NULL);
                ituButtonSetString(keyboard2Chs5Button, NULL);
                ituButtonSetString(keyboard2Chs6Button, NULL);
                ituButtonSetString(keyboard2Chs7Button, NULL);
                ituButtonSetString(keyboard2Chs8Button, NULL);
                ituButtonSetString(keyboard2Chs9Button, NULL);
                ituButtonSetString(keyboard2Chs0Button, NULL);

                i = 0;
                ptr = keyboard2ChsString;

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
                            btn = keyboard2Chs1Button;
                        else if (i == 1)
                            btn = keyboard2Chs2Button;
                        else if (i == 2)
                            btn = keyboard2Chs3Button;
                        else if (i == 3)
                            btn = keyboard2Chs4Button;
                        else if (i == 4)
                            btn = keyboard2Chs5Button;
                        else if (i == 5)
                            btn = keyboard2Chs6Button;
                        else if (i == 6)
                            btn = keyboard2Chs7Button;
                        else if (i == 7)
                            btn = keyboard2Chs8Button;
                        else if (i == 8)
                            btn = keyboard2Chs9Button;
                        else if (i == 9)
                            btn = keyboard2Chs0Button;

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

                keyboard2ChsPageIndex = 0;
                keyboard2ChsPageIndexes[0] = keyboard2ChsString;

                if (i >= 10)
                {
                    keyboard2ChsPageIndexes[1] = ptr;
                }
                else
                {
                    keyboard2ChsPageIndexes[1] = NULL;
                }
            }
        }
    }
    else
    {
        if (keyboard2ChsIndex >= 0)
        {
            strcpy(buf, ituTextGetString(keyboard2Keyboard->target));
            buf[keyboard2ChsIndex] = '\0';
            strcat(buf, ituTextGetString(&btn->text));
            ituTextBoxSetString((ITUTextBox*)keyboard2Keyboard->target, buf);
            keyboard2ChsIndex = -1;
            ituButtonSetString(keyboard2Chs1Button, NULL);
            ituButtonSetString(keyboard2Chs2Button, NULL);
            ituButtonSetString(keyboard2Chs3Button, NULL);
            ituButtonSetString(keyboard2Chs4Button, NULL);
            ituButtonSetString(keyboard2Chs5Button, NULL);
            ituButtonSetString(keyboard2Chs6Button, NULL);
            ituButtonSetString(keyboard2Chs7Button, NULL);
            ituButtonSetString(keyboard2Chs8Button, NULL);
            ituButtonSetString(keyboard2Chs9Button, NULL);
            ituButtonSetString(keyboard2Chs0Button, NULL);
        }
    }
	return true;
}

bool Keyboard2OnEnter(ITUWidget* widget, char* param)
{
	if (!mainMenuLayer)
    {
        mainMenuLayer = ituSceneFindWidget(&theScene, "mainMenuLayer");
        assert(mainMenuLayer);
            
        keyboard2TextBox = ituSceneFindWidget(&theScene, "keyboard2TextBox");
        assert(keyboard2TextBox);

        keyboard2Keyboard = ituSceneFindWidget(&theScene, "keyboard2Keyboard");
        assert(keyboard2Keyboard);
        keyboard2Keyboard->target = (ITUWidget*)keyboard2TextBox;

        keyboard2EnUpperBackground = ituSceneFindWidget(&theScene, "keyboard2EnUpperBackground");
        assert(keyboard2EnUpperBackground);

        keyboard2EnLowerBackground = ituSceneFindWidget(&theScene, "keyboard2EnLowerBackground");
        assert(keyboard2EnLowerBackground);

        keyboard2ChsBackground = ituSceneFindWidget(&theScene, "keyboard2ChsBackground");
        assert(keyboard2ChsBackground);

        keyboard2SymbolBackground = ituSceneFindWidget(&theScene, "keyboard2SymbolBackground");
        assert(keyboard2SymbolBackground);

        keyboard2SymbolButton = ituSceneFindWidget(&theScene, "keyboard2SymbolButton");
        assert(keyboard2SymbolButton);

        keyboard2EnUpperButton = ituSceneFindWidget(&theScene, "keyboard2EnUpperButton");
        assert(keyboard2EnUpperButton);

        keyboard2EnLowerButton = ituSceneFindWidget(&theScene, "keyboard2EnLowerButton");
        assert(keyboard2EnLowerButton);

        keyboard2ChsButton = ituSceneFindWidget(&theScene, "keyboard2ChsButton");
        assert(keyboard2ChsButton);

		keyboard2Chs1Button = ituSceneFindWidget(&theScene, "keyboard2Chs1Button");
        assert(keyboard2Chs1Button);

		keyboard2Chs2Button = ituSceneFindWidget(&theScene, "keyboard2Chs2Button");
        assert(keyboard2Chs2Button);

		keyboard2Chs3Button = ituSceneFindWidget(&theScene, "keyboard2Chs3Button");
        assert(keyboard2Chs3Button);

		keyboard2Chs4Button = ituSceneFindWidget(&theScene, "keyboard2Chs4Button");
        assert(keyboard2Chs4Button);

		keyboard2Chs5Button = ituSceneFindWidget(&theScene, "keyboard2Chs5Button");
        assert(keyboard2Chs5Button);

		keyboard2Chs6Button = ituSceneFindWidget(&theScene, "keyboard2Chs6Button");
        assert(keyboard2Chs6Button);

		keyboard2Chs7Button = ituSceneFindWidget(&theScene, "keyboard2Chs7Button");
        assert(keyboard2Chs7Button);

		keyboard2Chs8Button = ituSceneFindWidget(&theScene, "keyboard2Chs8Button");
        assert(keyboard2Chs8Button);

		keyboard2Chs9Button = ituSceneFindWidget(&theScene, "keyboard2Chs9Button");
        assert(keyboard2Chs9Button);

		keyboard2Chs0Button = ituSceneFindWidget(&theScene, "keyboard2Chs0Button");
        assert(keyboard2Chs0Button);
	}

    ituWidgetSetVisible(keyboard2EnUpperBackground, false);
    ituWidgetSetVisible(keyboard2EnLowerBackground, true);
    ituWidgetSetVisible(keyboard2ChsBackground, false);
    ituWidgetSetVisible(keyboard2SymbolBackground, false);
    ituWidgetSetVisible(keyboard2EnUpperButton, false);
    ituWidgetSetVisible(keyboard2EnLowerButton, true);
    ituWidgetSetVisible(keyboard2ChsButton, false);
    ituButtonSetString(keyboard2Chs1Button, NULL);
    ituButtonSetString(keyboard2Chs2Button, NULL);
    ituButtonSetString(keyboard2Chs3Button, NULL);
    ituButtonSetString(keyboard2Chs4Button, NULL);
    ituButtonSetString(keyboard2Chs5Button, NULL);
    ituButtonSetString(keyboard2Chs6Button, NULL);
    ituButtonSetString(keyboard2Chs7Button, NULL);
    ituButtonSetString(keyboard2Chs8Button, NULL);
    ituButtonSetString(keyboard2Chs9Button, NULL);
    ituButtonSetString(keyboard2Chs0Button, NULL);

    keyboard2ChsIndex = keyboard2ChsPageIndex = -1;

	return true;
}

void Keyboard2Reset(void)
{
    mainMenuLayer = NULL;
}
