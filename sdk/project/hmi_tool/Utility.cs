using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Windows.Forms;
using System.Security.Cryptography;

namespace GUIDesigner
{
    public class Uitlity
    {
        public static void GetWidgetNamesByActionType(ComponentCollection components, ITU.WidgetActionType actionType, List<string> names)
        {
            foreach (IComponent comp in components)
            {
                if (comp is IWidget)
                {
                    switch (actionType)
                    {
                        case ITU.WidgetActionType.Show:
                        case ITU.WidgetActionType.Hide:
                        case ITU.WidgetActionType.Focus:
                            if (comp is Control)
                            {
                                names.Add((comp as IWidget).Name);
                            }
                            break;

                        case ITU.WidgetActionType.Previous:
                        case ITU.WidgetActionType.Next:
                            if (comp is CalendarWidget ||
                                comp is CoverFlowWidget ||
                                comp is ImageCoverFlowWidget ||
                                comp is ScaleCoverFlowWidget ||
                                comp is ListBoxWidget ||
                                comp is FileListBoxWidget ||
                                comp is IconListBoxWidget ||
                                comp is MediaFileListBoxWidget ||
                                comp is ScrollListBoxWidget ||
                                comp is ScrollMediaFileListBoxWidget ||
                                comp is ScrollIconListBoxWidget ||
                                comp is PageFlowWidget ||
                                comp is WheelWidget)
                            {
                                names.Add((comp as IWidget).Name);
                            }
                            break;

                        case ITU.WidgetActionType.Back:
                            if (comp is AnimationWidget ||
                                comp is CalendarWidget ||
                                comp is FileListBoxWidget ||
                                comp is MediaFileListBoxWidget ||
                                comp is TextBoxWidget)
                            {
                                names.Add((comp as Control).Name);
                            }
                            break;

                        case ITU.WidgetActionType.Play:
                        case ITU.WidgetActionType.Stop:
                            if (comp is AnimationWidget ||
                                comp is SpriteWidget ||
                                comp is VideoWidget ||
                                comp is AudioWidget ||
                                comp is SlideshowWidget)
                            {
                                names.Add((comp as IWidget).Name);
                            }
                            break;

                        case ITU.WidgetActionType.Goto:
                            if (comp is AnimationWidget ||
                                comp is CircleProgressBarWidget ||
                                comp is CoverFlowWidget ||
                                comp is ImageCoverFlowWidget ||
                                comp is ScaleCoverFlowWidget ||
                                comp is LayerWidget ||
                                comp is ListBoxWidget ||
                                comp is FileListBoxWidget ||
                                comp is IconListBoxWidget ||
                                comp is MediaFileListBoxWidget ||
                                comp is ScrollListBoxWidget ||
                                comp is ScrollMediaFileListBoxWidget ||
                                comp is ScrollIconListBoxWidget ||
                                comp is PageFlowWidget ||
                                comp is ProgressBarWidget ||
                                comp is SpriteWidget ||
                                comp is TrackBarWidget ||
                                comp is VideoWidget ||
                                comp is WheelWidget ||
                                comp is SlideshowWidget ||
                                comp is DrawPenWidget)
                            {
                                names.Add((comp as IWidget).Name);
                            }
                            break;

                        case ITU.WidgetActionType.Language:
                            if (comp is TextWidget ||
                                comp is TextBoxWidget ||
                                comp is ScrollTextWidget)
                            {
                                names.Add((comp as IWidget).Name);
                            }
                            break;

                        case ITU.WidgetActionType.Input:
                            if (comp is TextWidget ||
                                comp is TextBoxWidget ||
                                comp is ScrollTextWidget ||
                                comp is KeyboardWidget)
                            {
                                names.Add((comp as IWidget).Name);
                            }
                            break;

                        case ITU.WidgetActionType.Bind:
                            if (comp is TextBoxWidget ||
                                comp is ScrollListBoxWidget ||
                                comp is ScrollMediaFileListBoxWidget ||
                                comp is ScrollIconListBoxWidget ||
                                comp is TableListBoxWidget ||
                                comp is TableIconListBoxWidget ||
                                comp is KeyboardWidget ||
                                comp is DrawPenWidget)
                            {
                                names.Add((comp as IWidget).Name);
                            }
                            break;

                        case ITU.WidgetActionType.Clear:
                            if (comp is TextBoxWidget ||
                                comp is DrawPenWidget)
                            {
                                names.Add((comp as IWidget).Name);
                            }
                            break;

                        case ITU.WidgetActionType.Enable:
                        case ITU.WidgetActionType.Disable:
                            if (comp is Control ||
                                comp is AudioWidget)
                            {
                                names.Add((comp as IWidget).Name);
                            }
                            break;

                        case ITU.WidgetActionType.Check:
                        case ITU.WidgetActionType.Uncheck:
                            if (comp is CheckBoxWidget ||
                                comp is RadioBoxWidget ||
                                comp is PopupRadioBoxWidget ||
                                comp is ListBoxWidget ||
                                comp is FileListBoxWidget ||
                                comp is IconListBoxWidget ||
                                comp is MediaFileListBoxWidget ||
                                comp is ScrollListBoxWidget ||
                                comp is ScrollMediaFileListBoxWidget ||
                                comp is ScrollIconListBoxWidget ||
                                comp is TableListBoxWidget ||
                                comp is TableIconListBoxWidget)
                            {
                                names.Add((comp as IWidget).Name);
                            }
                            break;

                        case ITU.WidgetActionType.Reload:
                            if (comp is AnimationWidget ||
                                comp is ContainerWidget ||
                                comp is ImageCoverFlowWidget ||
                                comp is ScrollBarWidget ||
                                comp is ListBoxWidget ||
                                comp is FileListBoxWidget ||
                                comp is IconListBoxWidget ||
                                comp is MediaFileListBoxWidget ||
                                comp is ScrollListBoxWidget ||
                                comp is ScrollMediaFileListBoxWidget ||
                                comp is ScrollIconListBoxWidget ||
                                comp is TableListBoxWidget ||
                                comp is TableIconListBoxWidget)
                            {
                                names.Add((comp as IWidget).Name);
                            }
                            break;

                        case ITU.WidgetActionType.DoDelay0:
                        case ITU.WidgetActionType.DoDelay1:
                        case ITU.WidgetActionType.DoDelay2:
                        case ITU.WidgetActionType.DoDelay3:
                        case ITU.WidgetActionType.DoDelay4:
                        case ITU.WidgetActionType.DoDelay5:
                        case ITU.WidgetActionType.DoDelay6:
                        case ITU.WidgetActionType.DoDelay7:
                            if (comp is BackgroundButtonWidget ||
                                comp is ButtonWidget ||
                                comp is CheckBoxWidget ||
                                comp is RadioBoxWidget ||
                                comp is PopupRadioBoxWidget ||
                                comp is CoverFlowWidget ||
                                comp is ImageCoverFlowWidget ||
                                comp is ScaleCoverFlowWidget ||
                                comp is ListBoxWidget ||
                                comp is FileListBoxWidget ||
                                comp is IconListBoxWidget ||
                                comp is MediaFileListBoxWidget ||
                                comp is ScrollListBoxWidget ||
                                comp is ScrollMediaFileListBoxWidget ||
                                comp is ScrollIconListBoxWidget ||
                                comp is TableListBoxWidget ||
                                comp is TableIconListBoxWidget ||
                                comp is PageFlowWidget ||
                                comp is ScrollBarWidget ||
                                comp is TextBoxWidget ||
                                comp is TrackBarWidget ||
                                comp is DragIconWidget ||
                                comp is DrawPenWidget ||
                                comp is WheelWidget)
                            {
                                names.Add((comp as IWidget).Name);
                            }
                            break;

                        case ITU.WidgetActionType.LoadExternal:
                        case ITU.WidgetActionType.ReleaseExternal:
                        case ITU.WidgetActionType.LoadFont:
                            if (comp is LayerWidget)
                            {
                                names.Add((comp as IWidget).Name);
                            }
                            break;

                        default:
                            break;
                    }
                }
            }
        }
    }
}
