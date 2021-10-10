using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace GUIDesigner
{
    class FileListBoxWidget : ListBox, IWidget
    {
        public FileListBoxWidget()
        {
            base.BorderStyle = System.Windows.Forms.BorderStyle.None;
            base.Margin = new Padding(0, 0, 0, 0);
            this.FocusColor = SystemColors.ActiveCaption;
            this.FocusFontColor = this.ForeColor;
            this.ReadFontColor = SystemColors.GrayText;
            this.BackAlpha = 255;
            this.TextAlign = ContentAlignment.TopLeft;
            this.Path = "A:/";
            this.ScrollDelay = ITU.ITU_SCROLL_DELAY;
            this.StopDelay = ITU.ITU_STOP_DELAY;
            this.ItemHeight = base.ItemHeight;
            this.BoldSize = 1;
            this.FontChanged += new EventHandler(FileListBoxWidget_FontChanged);

            this.Action01 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Load, "", "");
            this.Action02 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Load, "", "");
            this.Action03 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Load, "", "");
            this.Action04 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Load, "", "");
            this.Action05 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Load, "", "");
            this.Action06 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Load, "", "");
            this.Action07 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Load, "", "");
            this.Action08 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Load, "", "");
            this.Action09 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Load, "", "");
            this.Action10 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Load, "", "");
            this.Action11 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Load, "", "");
            this.Action12 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Load, "", "");
            this.Action13 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Load, "", "");
            this.Action14 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Load, "", "");
            this.Action15 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Load, "", "");

            this.FontChanged += new EventHandler(FileListBoxWidget_FontChanged);
        }

        void FileListBoxWidget_FontChanged(object sender, EventArgs e)
        {
            if (this.ItemHeight < base.ItemHeight)
                this.ItemHeight = base.ItemHeight;
        }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public BorderStyle BorderStyle { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual Cursor Cursor { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual RightToLeft RightToLeft { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool UseWaitCursor { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual bool AllowDrop { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public int ColumnWidth { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual ContextMenuStrip ContextMenuStrip { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool Enabled { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual DrawMode DrawMode { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public int HorizontalExtent { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]        
        new public bool HorizontalScrollbar { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public ImeMode ImeMode { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool IntegralHeight { get; set; }

        new public int ItemHeight { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool MultiColumn { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool ScrollAlwaysVisible { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual SelectionMode SelectionMode { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool Sorted { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool UseTabStops { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public string FormatString { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool FormattingEnabled { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public string AccessibleDescription { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public string AccessibleName { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public AccessibleRole AccessibleRole { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual AnchorStyles Anchor { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual DockStyle Dock { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public Padding Margin { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual Size MaximumSize { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual Size MinimumSize { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool CausesValidation { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public ControlBindingsCollection DataBindings { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public object DataSource { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public string DisplayMember { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public object Tag { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public string ValueMember { get; set; }

        public class StringListConverter : TypeConverter
        {
            // Convert from a string.
            public override object ConvertFrom(ITypeDescriptorContext context, System.Globalization.CultureInfo culture, object value)
            {
                if (value.GetType() == typeof(string))
                {
                    return value;
                }
                else
                {
                    return base.ConvertFrom(context, culture, value);
                }
            }

            public override bool
            GetStandardValuesSupported(ITypeDescriptorContext context)
            {
                return true; // display drop
            }
            public override bool
            GetStandardValuesExclusive(ITypeDescriptorContext context)
            {
                return true; // drop-down vs combo
            }
            public override StandardValuesCollection
            GetStandardValues(ITypeDescriptorContext context)
            {
                // note you can also look at context etc to build list
                return new StandardValuesCollection(NameCreationService.names.ToArray());
            }
        }

        public ContentAlignment TextAlign { get; set; }

        [TypeConverter(typeof(StringListConverter))]        
        public string PageIndexTarget { get; set; }

        [TypeConverter(typeof(StringListConverter))]        
        public string PageCountTarget { get; set; }

        public Color FocusColor { get; set; }
        public Color FocusFontColor { get; set; }
        public Color ReadFontColor { get; set; }
        public byte BackAlpha { get; set; }
        public string Path { get; set; }
        public int ScrollDelay { get; set; }
        public int StopDelay { get; set; }
        public int FontIndex { get; set; }
        public int BoldSize { get; set; }

        public enum WidgetEvent
        {
            Timer = 0,
            Load = 14,
            Select = 18,
            Delay0 = 25,
            Delay1 = 26,
            Delay2 = 27,
            Delay3 = 28,
            Delay4 = 29,
            Delay5 = 30,
            Delay6 = 31,
            Delay7 = 32,
            Custom0 = 100,
            Custom1 = 101,
            Custom2 = 102,
            Custom3 = 103,
            Custom4 = 104,
            Custom5 = 105,
            Custom6 = 106,
            Custom7 = 107,
            Custom8 = 108,
            Custom9 = 109,
            Custom10 = 110,
            Custom11 = 111,
            Custom12 = 112,
            Custom13 = 113,
            Custom14 = 114,
            Custom15 = 115,
            Custom16 = 116,
            Custom17 = 117,
            Custom18 = 118,
            Custom19 = 119,
            Custom20 = 120,
            Custom21 = 121,
            Custom22 = 122,
            Custom23 = 123,
            Custom24 = 124,
            Custom25 = 125,
            Custom26 = 126,
            Custom27 = 127,
            Custom28 = 128,
            Custom29 = 129,
            Custom30 = 130
        };

        public class WidgetActionTypeConverter : TypeConverter
        {
            public override bool CanConvertFrom(ITypeDescriptorContext context, Type sourceType)
            {
                if (sourceType == typeof(string)) return true;
                return base.CanConvertFrom(context, sourceType);
            }

            // Return true if we need to convert into a string.
            public override bool CanConvertTo(ITypeDescriptorContext context, Type destinationType)
            {
                if (destinationType == typeof(String)) return true;
                return base.CanConvertTo(context, destinationType);
            }

            // Convert from a string.
            public override object ConvertFrom(ITypeDescriptorContext context, CultureInfo culture, object value)
            {
                if (value.GetType() == typeof(string))
                {
                    // Split the string separated by commas.
                    string txt = (string)(value);
                    string[] fields = txt.Split(new char[] { ',' });

                    try
                    {
                        ITU.WidgetActionType a = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), fields[0]);
                        WidgetEvent e = WidgetEvent.Load;
                        try
                        {
                            e = (WidgetEvent)Enum.Parse(typeof(WidgetEvent), fields[1]);
                        }
                        catch
                        {
                            a = ITU.WidgetActionType.None;
                        }
                        return new WidgetAction(a, e, fields[2], fields[3]);
                    }
                    catch
                    {
                        throw new InvalidCastException(
                            "Cannot convert the string '" +
                            value.ToString() + "' into a Action");
                    }
                }
                else
                {
                    return base.ConvertFrom(context, culture, value);
                }
            }

            public override object ConvertTo(ITypeDescriptorContext context, System.Globalization.CultureInfo culture, object value, Type destinationType)
            {
                if (destinationType == typeof(string)) return value.ToString();
                return base.ConvertTo(context, culture, value, destinationType);
            }

            public override bool GetPropertiesSupported(ITypeDescriptorContext context)
            {
                return true;
            }

            public override PropertyDescriptorCollection GetProperties(ITypeDescriptorContext context, object value, Attribute[] attributes)
            {
                return TypeDescriptor.GetProperties(value);
            }
        }

        public class WidgetStringListConverter : TypeConverter
        {
            public override bool
            GetStandardValuesSupported(ITypeDescriptorContext context)
            {
                return true; // display drop
            }
            //public override bool
            //GetStandardValuesExclusive(ITypeDescriptorContext context)
            //{
            //    return true; // drop-down vs combo
            //}
            public override StandardValuesCollection
            GetStandardValues(ITypeDescriptorContext context)
            {
                List<string> names = new List<string>();
                WidgetAction a = (WidgetAction)context.Instance;

                foreach (HostSurface hs in HostSurface.hostSurfaces)
                {
                    Uitlity.GetWidgetNamesByActionType(hs.formDesignerHost.Container.Components, a.Action, names);
                }
                names.Sort();
                return new StandardValuesCollection(names.ToArray());
            }

            public override bool CanConvertFrom(System.ComponentModel.ITypeDescriptorContext context, System.Type sourceType)
            {
                if (sourceType == typeof(string))
                    return true;
                else
                    return base.CanConvertFrom(context, sourceType);
            }

            public override object ConvertFrom(System.ComponentModel.ITypeDescriptorContext context, System.Globalization.CultureInfo culture, object value)
            {
                if (value.GetType() == typeof(string))
                {
                    string s = value as string;
                    //NameCreationService.AddName(s);
                    return s;
                }
                else
                    return base.ConvertFrom(context, culture, value);
            }
        }

        [Serializable]
        [TypeConverter(typeof(WidgetActionTypeConverter))]
        public struct WidgetAction
        {
            public ITU.WidgetActionType Action { get; set; }
            public WidgetEvent Event { get; set; }

            private volatile string target;
            [TypeConverter(typeof(WidgetStringListConverter))]
            public String Target
            {
                get
                {
                    return target;
                }

                set
                {
                    if (Action == ITU.WidgetActionType.Function && ITU.enterKeyPressed && ITU.layerWidget != null && ITU.projectPath != null)
                    {
                        string funcName = value.Trim();

                        CodeGenerator.InvokeVisualStudio(funcName);

                        target = funcName;
                    }
                    else
                    {
                        target = value;
                    }
                }
            }

            public String Parameter { get; set; }

            public override string ToString()
            {
                return Action + "," + Event + "," + Target + "," + Parameter;
            }

            public WidgetAction(ITU.WidgetActionType action, WidgetEvent ev, String target, String param)
                : this()
            {
                Event = ev;
                Action = action;
                Target = target;
                Parameter = param;
            }
        }

        public WidgetAction Action01 { get; set; }
        public WidgetAction Action02 { get; set; }
        public WidgetAction Action03 { get; set; }
        public WidgetAction Action04 { get; set; }
        public WidgetAction Action05 { get; set; }
        public WidgetAction Action06 { get; set; }
        public WidgetAction Action07 { get; set; }
        public WidgetAction Action08 { get; set; }
        public WidgetAction Action09 { get; set; }
        public WidgetAction Action10 { get; set; }
        public WidgetAction Action11 { get; set; }
        public WidgetAction Action12 { get; set; }
        public WidgetAction Action13 { get; set; }
        public WidgetAction Action14 { get; set; }
        public WidgetAction Action15 { get; set; }
        
        protected override CreateParams CreateParams
        {
            get
            {
                CreateParams cp = base.CreateParams;
                cp.Style &= ~0x200000;  // Turn off WS_VSCROLL
                return cp;
            }
        }

        private bool hided = false;
        public bool Hided
        {
            get
            {
                return hided;
            }

            set
            {
                if (value)
                    Hide();
                else
                    Show();

                hided = value;
            }
        }

        public ITUWidget CreateITUWidget()
        {
            ITUFileListBox flistbox = new ITUFileListBox();

            flistbox.type = ITUWidgetType.ITU_FILELISTBOX;
            flistbox.name = this.Name;
            flistbox.flags |= this.TabStop ? ITU.ITU_TAPSTOP : 0;

            PropertyDescriptorCollection properties = TypeDescriptor.GetProperties(this);
            flistbox.visible = (bool)properties["Visible"].GetValue(this);

            flistbox.active = false;
            flistbox.dirty = false;
            flistbox.alpha = 255;
            flistbox.tabIndex = this.TabIndex;
            flistbox.rect.x = this.Location.X;
            flistbox.rect.y = this.Location.Y;
            flistbox.rect.width = this.Size.Width;
            flistbox.rect.height = this.Size.Height;
            flistbox.color.alpha = this.BackAlpha;
            flistbox.color.red = this.BackColor.R;
            flistbox.color.green = this.BackColor.G;
            flistbox.color.blue = this.BackColor.B;
            flistbox.bound.x = 0;
            flistbox.bound.y = 0;
            flistbox.bound.width = 0;
            flistbox.bound.height = 0;

            int count = this.Size.Height / base.ItemHeight;

            for (int i = 0; i < count; i++)
            {
                ITUScrollText text = new ITUScrollText();

                text.name = "";
                text.visible = true;
                text.active = false;
                text.dirty = false;
                text.alpha = 255;
                text.rect.x = 0;
                text.rect.y = 0;
                text.rect.width = this.Size.Width;
                text.rect.height = base.ItemHeight;
                text.color.alpha = this.ForeColor.A;
                text.color.red = this.ForeColor.R;
                text.color.green = this.ForeColor.G;
                text.color.blue = this.ForeColor.B;
                text.bound.x = 0;
                text.bound.y = 0;
                text.bound.width = 0;
                text.bound.height = 0;
                text.bgColor.alpha = 0;
                text.bgColor.red = 0;
                text.bgColor.green = 0;
                text.bgColor.blue = 0;
                text.fontWidth = (int)Math.Round(this.Font.Size * (this.Font.FontFamily.GetCellAscent(FontStyle.Regular) + this.Font.FontFamily.GetCellDescent(FontStyle.Regular)) / this.Font.FontFamily.GetEmHeight(FontStyle.Regular));
                text.fontHeight = text.fontWidth;
                text.width = this.Size.Width;
                text.fontIndex = this.FontIndex;

                if (base.Font.Bold)
                    text.textFlags |= ITUText.ITU_TEXT_BOLD;

                text.boldSize = this.BoldSize;

                switch (this.TextAlign)
                {
                    case ContentAlignment.BottomLeft:
                        text.layout = ITULayout.ITU_LAYOUT_BOTTOM_LEFT;
                        break;

                    case ContentAlignment.MiddleLeft:
                        text.layout = ITULayout.ITU_LAYOUT_MIDDLE_LEFT;
                        break;

                    case ContentAlignment.TopLeft:
                        text.layout = ITULayout.ITU_LAYOUT_TOP_LEFT;
                        break;

                    case ContentAlignment.BottomCenter:
                        text.layout = ITULayout.ITU_LAYOUT_BOTTOM_CENTER;
                        break;

                    case ContentAlignment.MiddleCenter:
                        text.layout = ITULayout.ITU_LAYOUT_MIDDLE_CENTER;
                        break;

                    case ContentAlignment.TopCenter:
                        text.layout = ITULayout.ITU_LAYOUT_TOP_CENTER;
                        break;

                    case ContentAlignment.BottomRight:
                        text.layout = ITULayout.ITU_LAYOUT_BOTTOM_RIGHT;
                        break;

                    case ContentAlignment.MiddleRight:
                        text.layout = ITULayout.ITU_LAYOUT_MIDDLE_RIGHT;
                        break;

                    case ContentAlignment.TopRight:
                        text.layout = ITULayout.ITU_LAYOUT_TOP_RIGHT;
                        break;

                    default:
                        text.layout = ITULayout.ITU_LAYOUT_DEFAULT;
                        break;
                }

                text.scrollDelay = this.ScrollDelay;
                text.stopDelay = this.StopDelay;
                text.scrollTextState = 1;

                WidgetNode node = new WidgetNode();
                node.widget = text;
                flistbox.items.Add(node);
            }
            flistbox.pageIndexName       = this.PageIndexTarget;
            flistbox.pageCountName       = this.PageCountTarget;
            flistbox.focusColor.alpha    = this.FocusColor.A;
            flistbox.focusColor.red      = this.FocusColor.R;
            flistbox.focusColor.green    = this.FocusColor.G;
            flistbox.focusColor.blue     = this.FocusColor.B;
            flistbox.focusFontColor.alpha = this.FocusFontColor.A;
            flistbox.focusFontColor.red = this.FocusFontColor.R;
            flistbox.focusFontColor.green = this.FocusFontColor.G;
            flistbox.focusFontColor.blue = this.FocusFontColor.B;
            flistbox.orgFontColor.alpha = this.ForeColor.A;
            flistbox.orgFontColor.red = this.ForeColor.R;
            flistbox.orgFontColor.green = this.ForeColor.G;
            flistbox.orgFontColor.blue = this.ForeColor.B;
            flistbox.readFontColor.alpha = this.ReadFontColor.A;
            flistbox.readFontColor.red = this.ReadFontColor.R;
            flistbox.readFontColor.green = this.ReadFontColor.G;
            flistbox.readFontColor.blue = this.ReadFontColor.B;
            flistbox.scrollDelay         = this.ScrollDelay;
            flistbox.stopDelay           = this.StopDelay;
            flistbox.path                = this.Path;

            flistbox.actions[0].action = (ITUActionType)this.Action01.Action;
            flistbox.actions[0].ev = (ITUEvent)this.Action01.Event;
            flistbox.actions[0].target = this.Action01.Target;
            flistbox.actions[0].param = this.Action01.Parameter;
            flistbox.actions[1].action = (ITUActionType)this.Action02.Action;
            flistbox.actions[1].ev = (ITUEvent)this.Action02.Event;
            flistbox.actions[1].target = this.Action02.Target;
            flistbox.actions[1].param = this.Action02.Parameter;
            flistbox.actions[2].action = (ITUActionType)this.Action03.Action;
            flistbox.actions[2].ev = (ITUEvent)this.Action03.Event;
            flistbox.actions[2].target = this.Action03.Target;
            flistbox.actions[2].param = this.Action03.Parameter;
            flistbox.actions[3].action = (ITUActionType)this.Action04.Action;
            flistbox.actions[3].ev = (ITUEvent)this.Action04.Event;
            flistbox.actions[3].target = this.Action04.Target;
            flistbox.actions[3].param = this.Action04.Parameter;
            flistbox.actions[4].action = (ITUActionType)this.Action05.Action;
            flistbox.actions[4].ev = (ITUEvent)this.Action05.Event;
            flistbox.actions[4].target = this.Action05.Target;
            flistbox.actions[4].param = this.Action05.Parameter;
            flistbox.actions[5].action = (ITUActionType)this.Action06.Action;
            flistbox.actions[5].ev = (ITUEvent)this.Action06.Event;
            flistbox.actions[5].target = this.Action06.Target;
            flistbox.actions[5].param = this.Action06.Parameter;
            flistbox.actions[6].action = (ITUActionType)this.Action07.Action;
            flistbox.actions[6].ev = (ITUEvent)this.Action07.Event;
            flistbox.actions[6].target = this.Action07.Target;
            flistbox.actions[6].param = this.Action07.Parameter;
            flistbox.actions[7].action = (ITUActionType)this.Action08.Action;
            flistbox.actions[7].ev = (ITUEvent)this.Action08.Event;
            flistbox.actions[7].target = this.Action08.Target;
            flistbox.actions[7].param = this.Action08.Parameter;
            flistbox.actions[8].action = (ITUActionType)this.Action09.Action;
            flistbox.actions[8].ev = (ITUEvent)this.Action09.Event;
            flistbox.actions[8].target = this.Action09.Target;
            flistbox.actions[8].param = this.Action09.Parameter;
            flistbox.actions[9].action = (ITUActionType)this.Action10.Action;
            flistbox.actions[9].ev = (ITUEvent)this.Action10.Event;
            flistbox.actions[9].target = this.Action10.Target;
            flistbox.actions[9].param = this.Action10.Parameter;
            flistbox.actions[10].action = (ITUActionType)this.Action11.Action;
            flistbox.actions[10].ev = (ITUEvent)this.Action11.Event;
            flistbox.actions[10].target = this.Action11.Target;
            flistbox.actions[10].param = this.Action11.Parameter;
            flistbox.actions[11].action = (ITUActionType)this.Action12.Action;
            flistbox.actions[11].ev = (ITUEvent)this.Action12.Event;
            flistbox.actions[11].target = this.Action12.Target;
            flistbox.actions[11].param = this.Action12.Parameter;
            flistbox.actions[12].action = (ITUActionType)this.Action13.Action;
            flistbox.actions[12].ev = (ITUEvent)this.Action13.Event;
            flistbox.actions[12].target = this.Action13.Target;
            flistbox.actions[12].param = this.Action13.Parameter;
            flistbox.actions[13].action = (ITUActionType)this.Action14.Action;
            flistbox.actions[13].ev = (ITUEvent)this.Action14.Event;
            flistbox.actions[13].target = this.Action14.Target;
            flistbox.actions[13].param = this.Action14.Parameter;
            flistbox.actions[14].action = (ITUActionType)this.Action15.Action;
            flistbox.actions[14].ev = (ITUEvent)this.Action15.Event;
            flistbox.actions[14].target = this.Action15.Target;
            flistbox.actions[14].param = this.Action15.Parameter;

            return flistbox;
        }

        public void SaveImages(String path)
        {
        }

        public void WriteFunctions(HashSet<string> functions)
        {
            if (this.Action01.Action == ITU.WidgetActionType.Function)
                functions.Add(this.Action01.Target);
            if (this.Action02.Action == ITU.WidgetActionType.Function)
                functions.Add(this.Action02.Target);
            if (this.Action03.Action == ITU.WidgetActionType.Function)
                functions.Add(this.Action03.Target);
            if (this.Action04.Action == ITU.WidgetActionType.Function)
                functions.Add(this.Action04.Target);
            if (this.Action05.Action == ITU.WidgetActionType.Function)
                functions.Add(this.Action05.Target);
            if (this.Action06.Action == ITU.WidgetActionType.Function)
                functions.Add(this.Action06.Target);
            if (this.Action07.Action == ITU.WidgetActionType.Function)
                functions.Add(this.Action07.Target);
            if (this.Action08.Action == ITU.WidgetActionType.Function)
                functions.Add(this.Action08.Target);
            if (this.Action09.Action == ITU.WidgetActionType.Function)
                functions.Add(this.Action09.Target);
            if (this.Action10.Action == ITU.WidgetActionType.Function)
                functions.Add(this.Action10.Target);
            if (this.Action11.Action == ITU.WidgetActionType.Function)
                functions.Add(this.Action11.Target);
            if (this.Action12.Action == ITU.WidgetActionType.Function)
                functions.Add(this.Action12.Target);
            if (this.Action13.Action == ITU.WidgetActionType.Function)
                functions.Add(this.Action13.Target);
            if (this.Action14.Action == ITU.WidgetActionType.Function)
                functions.Add(this.Action14.Target);
            if (this.Action15.Action == ITU.WidgetActionType.Function)
                functions.Add(this.Action15.Target);
        }
    }
}
