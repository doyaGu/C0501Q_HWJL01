#include <assert.h>
#include <malloc.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"

static const char popupButtonName[] = "ITUPopupButton";

bool ituPopupButtonClone(ITUWidget* widget, ITUWidget** cloned)
{
    ITUPopupButton* popupButton = (ITUPopupButton*)widget;
    assert(widget);
    assert(cloned);
    ITU_ASSERT_THREAD();

    if (*cloned == NULL)
    {
        ITUWidget* newWidget = malloc(sizeof(ITUPopupButton));
        if (newWidget == NULL)
            return false;

        memcpy(newWidget, widget, sizeof(ITUPopupButton));
        newWidget->tree.child = newWidget->tree.parent = newWidget->tree.sibling = NULL;
        *cloned = newWidget;
    }

    return ituButtonClone(widget, cloned);
}

bool ituPopupButtonUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUButton* btn = (ITUButton*) widget;
    ITUPopupButton* popupButton = (ITUPopupButton*) widget;
	ITUText* text = &(btn->text);
    ITUEvent response_key = widget->flags & ITU_RESPONSE_TO_UP_KEY ? ITU_EVENT_MOUSEUP : ITU_EVENT_MOUSEDOWN;
    assert(popupButton);

    if (ev == ITU_EVENT_MOUSEDOWN || ev == ITU_EVENT_MOUSEUP)
    {
        result |= ituIconUpdate(widget, ev, arg1, arg2, arg3);

        if (ituWidgetIsEnabled(widget) && popupButton->frame == 0 && !result)
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if (ituWidgetIsInside(widget, x, y))
            {
                if (ev == ITU_EVENT_MOUSEDOWN)
                    ituButtonSetPressed(btn, true);

                if (ev == response_key && btn->pressed)
                {
                    memcpy(&popupButton->orgRect, &widget->rect, sizeof (ITURectangle));
                    popupButton->orgAlpha = widget->alpha;

                    widget->rect.x = popupButton->orgRect.x - popupButton->orgRect.width * popupButton->incPercent / (popupButton->totalframe * 100);
                    widget->rect.y = popupButton->orgRect.y - popupButton->orgRect.height * popupButton->incPercent / (popupButton->totalframe * 100);
                    widget->rect.width = popupButton->orgRect.width + popupButton->orgRect.width * popupButton->incPercent / (popupButton->totalframe * 100 / 2);
                    widget->rect.height = popupButton->orgRect.height + popupButton->orgRect.height * popupButton->incPercent / (popupButton->totalframe * 100 / 2);

					//Bless
					popupButton->orgFontSize = text->fontHeight;

                    widget->alpha = popupButton->orgAlpha * popupButton->alphaPercent / 100;

                    ituFocusWidget(widget);
                    ituWidgetUpdate(widget, ITU_EVENT_LAYOUT, 0, 0, 0);

                    popupButton->frame = 1;
                }
                result |= widget->dirty;
            }
        }
        if (ev == ITU_EVENT_MOUSEUP)
            ituButtonSetPressed(btn, false);

        return widget->visible ? result : false;
    }

    result |= ituButtonUpdate(widget, ev, arg1, arg2, arg3);

    if (ev == ITU_EVENT_TIMER)
    {
        if (popupButton->frame > 0)
        {
            if (popupButton->frame >= popupButton->totalframe - 1)
            {
                memcpy(&widget->rect, &popupButton->orgRect, sizeof (ITURectangle));
                widget->alpha = popupButton->orgAlpha;

                popupButton->frame = 0;
				ituTextSetFontSize(text, popupButton->orgFontSize);

                result |= ituExecActions((ITUWidget*)popupButton, popupButton->btn.actions, ITU_EVENT_PRESS, 0);
            }
            else
            {
                int ratio, step;

                popupButton->frame++;
                
                ratio = popupButton->totalframe * 100 / 2;
                step = (popupButton->frame > popupButton->totalframe / 2) ? (popupButton->totalframe - popupButton->frame) : popupButton->frame;

                widget->rect.x = popupButton->orgRect.x - popupButton->orgRect.width * step * popupButton->incPercent / (popupButton->totalframe * 100);
                widget->rect.y = popupButton->orgRect.y - popupButton->orgRect.height * step * popupButton->incPercent / (popupButton->totalframe * 100);
                widget->rect.width = popupButton->orgRect.width + popupButton->orgRect.width * step * popupButton->incPercent / (popupButton->totalframe * 100 / 2);
                widget->rect.height = popupButton->orgRect.height + popupButton->orgRect.height * step * popupButton->incPercent / (popupButton->totalframe * 100 / 2);

                widget->alpha = popupButton->orgAlpha * popupButton->alphaPercent / 100;

				if (popupButton->textScaleFactor > 0)
					ituTextSetFontSize(text, popupButton->orgFontSize * widget->rect.height / popupButton->orgRect.height * (popupButton->textScaleFactor + 100) / 100);
            }
            ituWidgetUpdate(widget, ITU_EVENT_LAYOUT, 0, 0, 0);
            result = widget->dirty = true;
        }
    }
    return widget->visible ? result : false;
}

void ituPopupButtonInit(ITUPopupButton* popupButton)
{
    assert(popupButton);
    ITU_ASSERT_THREAD();

    memset(popupButton, 0, sizeof (ITUPopupButton));

    ituButtonInit(&popupButton->btn);

    ituWidgetSetType(popupButton, ITU_POPUPBUTTON);
    ituWidgetSetName(popupButton, popupButtonName);
    ituWidgetSetClone(popupButton, ituPopupButtonClone);
    ituWidgetSetUpdate(popupButton, ituPopupButtonUpdate);
}

void ituPopupButtonLoad(ITUPopupButton* popupButton, uint32_t base)
{
    assert(popupButton);

	if (popupButton->orgFontSize < 0)
		popupButton->orgFontSize = 0;
	else if (popupButton->orgFontSize > 50)
		popupButton->orgFontSize = 50;

    ituButtonLoad(&popupButton->btn, base);
    ituWidgetSetClone(popupButton, ituPopupButtonClone);
    ituWidgetSetUpdate(popupButton, ituPopupButtonUpdate);
}
