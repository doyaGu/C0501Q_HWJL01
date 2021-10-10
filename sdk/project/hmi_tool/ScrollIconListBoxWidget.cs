using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Drawing.Design;
using System.Drawing.Imaging;

namespace GUIDesigner
{
    class ScrollIconListBoxWidget : ListBox, IWidget
    {
        public ScrollIconListBoxWidget()
        {
            base.BorderStyle = System.Windows.Forms.BorderStyle.None;
            base.Margin = new Padding(0, 0, 0, 0);
            this.FocusColor = SystemColors.ActiveCaption;
            this.BackAlpha = 255;
            this.ItemHeight = base.ItemHeight;
            this.Compress = true;
            this.External = false;
            this.TotalFrame = 10;
            this.Draggable = true;
            this.HasLongPress = false;
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

            this.FontChanged += new EventHandler(ScrollIconListBoxWidget_FontChanged);
        }

        void ScrollIconListBoxWidget_FontChanged(object sender, EventArgs e)
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

        [TypeConverter(typeof(StringListConverter))]
        public string PageIndexTarget { get; set; }

        [TypeConverter(typeof(StringListConverter))]
        public string PageCountTarget { get; set; }

        public Color FocusColor { get; set; }
        public byte BackAlpha { get; set; }
        public int TotalFrame { get; set; }
        public bool Draggable { get; set; }
        public bool HasLongPress { get; set; }
        public int FontIndex { get; set; }

        public enum WidgetEvent
        {
            Timer = 0,
            SlideUp = 11,
            SlideDown = 13,
            Load = 14,
            Select = 18,
            MouseLongPress = 21,
            Sync = 22,
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

        public ITU.WidgetPixelFormat PixelFormat { get; set; }
        public Boolean Compress { get; set; }
        public Boolean External { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image Image0 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image Image1 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image Image2 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image Image3 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image Image4 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image Image5 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image Image6 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image Image7 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image Image8 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image Image9 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image Image10 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image Image11 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image Image12 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image Image13 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image Image14 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image Image15 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image FocusImage0 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image FocusImage1 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image FocusImage2 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image FocusImage3 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image FocusImage4 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image FocusImage5 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image FocusImage6 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image FocusImage7 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image FocusImage8 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image FocusImage9 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image FocusImage10 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image FocusImage11 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image FocusImage12 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image FocusImage13 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image FocusImage14 { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image FocusImage15 { get; set; }

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
            ITUScrollIconListBox silistbox = new ITUScrollIconListBox();

            silistbox.type = ITUWidgetType.ITU_SCROLLICONLISTBOX;
            silistbox.name = this.Name;
            silistbox.flags |= this.TabStop ? ITU.ITU_TAPSTOP : 0;
            silistbox.flags |= this.HasLongPress ? ITU.ITU_HAS_LONG_PRESS : 0;

            PropertyDescriptorCollection properties = TypeDescriptor.GetProperties(this);
            silistbox.visible = (bool)properties["Visible"].GetValue(this);

            silistbox.active = false;
            silistbox.dirty = false;
            silistbox.alpha = 255;
            silistbox.tabIndex = this.TabIndex;
            silistbox.rect.x = this.Location.X;
            silistbox.rect.y = this.Location.Y;
            silistbox.rect.width = this.Size.Width;
            silistbox.rect.height = this.Size.Height;
            silistbox.color.alpha = this.BackAlpha;
            silistbox.color.red = this.BackColor.R;
            silistbox.color.green = this.BackColor.G;
            silistbox.color.blue = this.BackColor.B;
            silistbox.bound.x = 0;
            silistbox.bound.y = 0;
            silistbox.bound.width = 0;
            silistbox.bound.height = 0;

            for (int i = 0; i < 3; i++)
            {
                foreach (object item in this.Items)
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
                    text.rect.height = this.ItemHeight;
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
                    text.fontIndex = this.FontIndex;

                    string[] texts = new string[] { item.ToString() };
                    text.stringSet = ITU.CreateStringSetNode(texts);

                    text.width = this.Size.Width;
                    text.scrollDelay = ITU.ITU_SCROLL_DELAY;
                    text.stopDelay = ITU.ITU_STOP_DELAY;

                    WidgetNode node = new WidgetNode();
                    node.widget = text;
                    silistbox.items.Add(node);
                }
            }
            silistbox.pageIndexName = this.PageIndexTarget;
            silistbox.pageCountName = this.PageCountTarget;
            silistbox.focusColor.alpha = this.FocusColor.A;
            silistbox.focusColor.red = this.FocusColor.R;
            silistbox.focusColor.green = this.FocusColor.G;
            silistbox.focusColor.blue = this.FocusColor.B;
            silistbox.focusFontColor.alpha = 255;
            silistbox.focusFontColor.red = 0;
            silistbox.focusFontColor.green = 0;
            silistbox.focusFontColor.blue = 0;
            silistbox.orgFontColor.alpha = this.ForeColor.A;
            silistbox.orgFontColor.red = this.ForeColor.R;
            silistbox.orgFontColor.green = this.ForeColor.G;
            silistbox.orgFontColor.blue = this.ForeColor.B;
            silistbox.scrollDelay = ITU.ITU_SCROLL_DELAY;
            silistbox.stopDelay = ITU.ITU_STOP_DELAY;
            silistbox.totalframe = this.TotalFrame;

            if (this.Draggable)
                silistbox.flags |= ITU.ITU_DRAGGABLE;

            if (this.Image0 != null)
            {
                silistbox.staticSurfArray[0] = ITU.CreateSurfaceNode(this.Image0 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }
            if (this.Image1 != null)
            {
                silistbox.staticSurfArray[1] = ITU.CreateSurfaceNode(this.Image1 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }
            if (this.Image2 != null)
            {
                silistbox.staticSurfArray[2] = ITU.CreateSurfaceNode(this.Image2 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }
            if (this.Image3 != null)
            {
                silistbox.staticSurfArray[3] = ITU.CreateSurfaceNode(this.Image3 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }
            if (this.Image4 != null)
            {
                silistbox.staticSurfArray[4] = ITU.CreateSurfaceNode(this.Image4 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }
            if (this.Image5 != null)
            {
                silistbox.staticSurfArray[5] = ITU.CreateSurfaceNode(this.Image5 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }
            if (this.Image6 != null)
            {
                silistbox.staticSurfArray[6] = ITU.CreateSurfaceNode(this.Image6 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }
            if (this.Image7 != null)
            {
                silistbox.staticSurfArray[7] = ITU.CreateSurfaceNode(this.Image7 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }
            if (this.Image8 != null)
            {
                silistbox.staticSurfArray[8] = ITU.CreateSurfaceNode(this.Image8 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }
            if (this.Image9 != null)
            {
                silistbox.staticSurfArray[9] = ITU.CreateSurfaceNode(this.Image9 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }
            if (this.Image10 != null)
            {
                silistbox.staticSurfArray[10] = ITU.CreateSurfaceNode(this.Image10 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }
            if (this.Image11 != null)
            {
                silistbox.staticSurfArray[11] = ITU.CreateSurfaceNode(this.Image11 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }
            if (this.Image12 != null)
            {
                silistbox.staticSurfArray[12] = ITU.CreateSurfaceNode(this.Image12 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }
            if (this.Image13 != null)
            {
                silistbox.staticSurfArray[13] = ITU.CreateSurfaceNode(this.Image13 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }
            if (this.Image14 != null)
            {
                silistbox.staticSurfArray[14] = ITU.CreateSurfaceNode(this.Image14 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }
            if (this.Image15 != null)
            {
                silistbox.staticSurfArray[15] = ITU.CreateSurfaceNode(this.Image15 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }

            if (this.FocusImage0 != null)
            {
                silistbox.focusStaticSurfArray[0] = ITU.CreateSurfaceNode(this.FocusImage0 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }

            if (this.FocusImage1 != null)
            {
                silistbox.focusStaticSurfArray[1] = ITU.CreateSurfaceNode(this.FocusImage1 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }

            if (this.FocusImage2 != null)
            {
                silistbox.focusStaticSurfArray[2] = ITU.CreateSurfaceNode(this.FocusImage2 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }

            if (this.FocusImage3 != null)
            {
                silistbox.focusStaticSurfArray[3] = ITU.CreateSurfaceNode(this.FocusImage3 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }

            if (this.FocusImage4 != null)
            {
                silistbox.focusStaticSurfArray[4] = ITU.CreateSurfaceNode(this.FocusImage4 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }

            if (this.FocusImage5 != null)
            {
                silistbox.focusStaticSurfArray[5] = ITU.CreateSurfaceNode(this.FocusImage5 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }

            if (this.FocusImage6 != null)
            {
                silistbox.focusStaticSurfArray[6] = ITU.CreateSurfaceNode(this.FocusImage6 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }

            if (this.FocusImage7 != null)
            {
                silistbox.focusStaticSurfArray[7] = ITU.CreateSurfaceNode(this.FocusImage7 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }

            if (this.FocusImage8 != null)
            {
                silistbox.focusStaticSurfArray[8] = ITU.CreateSurfaceNode(this.FocusImage8 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }

            if (this.FocusImage9 != null)
            {
                silistbox.focusStaticSurfArray[9] = ITU.CreateSurfaceNode(this.FocusImage9 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }

            if (this.FocusImage10 != null)
            {
                silistbox.focusStaticSurfArray[10] = ITU.CreateSurfaceNode(this.FocusImage10 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }

            if (this.FocusImage11 != null)
            {
                silistbox.focusStaticSurfArray[11] = ITU.CreateSurfaceNode(this.FocusImage11 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }

            if (this.FocusImage12 != null)
            {
                silistbox.focusStaticSurfArray[12] = ITU.CreateSurfaceNode(this.FocusImage12 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }

            if (this.FocusImage13 != null)
            {
                silistbox.focusStaticSurfArray[13] = ITU.CreateSurfaceNode(this.FocusImage13 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }

            if (this.FocusImage14 != null)
            {
                silistbox.focusStaticSurfArray[14] = ITU.CreateSurfaceNode(this.FocusImage14 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }

            if (this.FocusImage15 != null)
            {
                silistbox.focusStaticSurfArray[15] = ITU.CreateSurfaceNode(this.FocusImage15 as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    silistbox.flags |= ITU.ITU_EXTERNAL;
            }

            silistbox.actions[0].action = (ITUActionType)this.Action01.Action;
            silistbox.actions[0].ev = (ITUEvent)this.Action01.Event;
            silistbox.actions[0].target = this.Action01.Target;
            silistbox.actions[0].param = this.Action01.Parameter;
            silistbox.actions[1].action = (ITUActionType)this.Action02.Action;
            silistbox.actions[1].ev = (ITUEvent)this.Action02.Event;
            silistbox.actions[1].target = this.Action02.Target;
            silistbox.actions[1].param = this.Action02.Parameter;
            silistbox.actions[2].action = (ITUActionType)this.Action03.Action;
            silistbox.actions[2].ev = (ITUEvent)this.Action03.Event;
            silistbox.actions[2].target = this.Action03.Target;
            silistbox.actions[2].param = this.Action03.Parameter;
            silistbox.actions[3].action = (ITUActionType)this.Action04.Action;
            silistbox.actions[3].ev = (ITUEvent)this.Action04.Event;
            silistbox.actions[3].target = this.Action04.Target;
            silistbox.actions[3].param = this.Action04.Parameter;
            silistbox.actions[4].action = (ITUActionType)this.Action05.Action;
            silistbox.actions[4].ev = (ITUEvent)this.Action05.Event;
            silistbox.actions[4].target = this.Action05.Target;
            silistbox.actions[4].param = this.Action05.Parameter;
            silistbox.actions[5].action = (ITUActionType)this.Action06.Action;
            silistbox.actions[5].ev = (ITUEvent)this.Action06.Event;
            silistbox.actions[5].target = this.Action06.Target;
            silistbox.actions[5].param = this.Action06.Parameter;
            silistbox.actions[6].action = (ITUActionType)this.Action07.Action;
            silistbox.actions[6].ev = (ITUEvent)this.Action07.Event;
            silistbox.actions[6].target = this.Action07.Target;
            silistbox.actions[6].param = this.Action07.Parameter;
            silistbox.actions[7].action = (ITUActionType)this.Action08.Action;
            silistbox.actions[7].ev = (ITUEvent)this.Action08.Event;
            silistbox.actions[7].target = this.Action08.Target;
            silistbox.actions[7].param = this.Action08.Parameter;
            silistbox.actions[8].action = (ITUActionType)this.Action09.Action;
            silistbox.actions[8].ev = (ITUEvent)this.Action09.Event;
            silistbox.actions[8].target = this.Action09.Target;
            silistbox.actions[8].param = this.Action09.Parameter;
            silistbox.actions[9].action = (ITUActionType)this.Action10.Action;
            silistbox.actions[9].ev = (ITUEvent)this.Action10.Event;
            silistbox.actions[9].target = this.Action10.Target;
            silistbox.actions[9].param = this.Action10.Parameter;
            silistbox.actions[10].action = (ITUActionType)this.Action11.Action;
            silistbox.actions[10].ev = (ITUEvent)this.Action11.Event;
            silistbox.actions[10].target = this.Action11.Target;
            silistbox.actions[10].param = this.Action11.Parameter;
            silistbox.actions[11].action = (ITUActionType)this.Action12.Action;
            silistbox.actions[11].ev = (ITUEvent)this.Action12.Event;
            silistbox.actions[11].target = this.Action12.Target;
            silistbox.actions[11].param = this.Action12.Parameter;
            silistbox.actions[12].action = (ITUActionType)this.Action13.Action;
            silistbox.actions[12].ev = (ITUEvent)this.Action13.Event;
            silistbox.actions[12].target = this.Action13.Target;
            silistbox.actions[12].param = this.Action13.Parameter;
            silistbox.actions[13].action = (ITUActionType)this.Action14.Action;
            silistbox.actions[13].ev = (ITUEvent)this.Action14.Event;
            silistbox.actions[13].target = this.Action14.Target;
            silistbox.actions[13].param = this.Action14.Parameter;
            silistbox.actions[14].action = (ITUActionType)this.Action15.Action;
            silistbox.actions[14].ev = (ITUEvent)this.Action15.Event;
            silistbox.actions[14].target = this.Action15.Target;
            silistbox.actions[14].param = this.Action15.Parameter;

            return silistbox;
        }

        public void SaveImages(String path)
        {
            if (this.Image0 != null)
            {
                Bitmap bitmap = this.Image0 as Bitmap;
                String filePath = path + "\\" + this.Name + "_Image0.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.Image1 != null)
            {
                Bitmap bitmap = this.Image1 as Bitmap;
                String filePath = path + "\\" + this.Name + "_Image1.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.Image2 != null)
            {
                Bitmap bitmap = this.Image2 as Bitmap;
                String filePath = path + "\\" + this.Name + "_Image2.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.Image3 != null)
            {
                Bitmap bitmap = this.Image3 as Bitmap;
                String filePath = path + "\\" + this.Name + "_Image3.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.Image4 != null)
            {
                Bitmap bitmap = this.Image4 as Bitmap;
                String filePath = path + "\\" + this.Name + "_Image4.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.Image5 != null)
            {
                Bitmap bitmap = this.Image5 as Bitmap;
                String filePath = path + "\\" + this.Name + "_Image5.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.Image6 != null)
            {
                Bitmap bitmap = this.Image6 as Bitmap;
                String filePath = path + "\\" + this.Name + "_Image6.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.Image7 != null)
            {
                Bitmap bitmap = this.Image7 as Bitmap;
                String filePath = path + "\\" + this.Name + "_Image7.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.Image8 != null)
            {
                Bitmap bitmap = this.Image8 as Bitmap;
                String filePath = path + "\\" + this.Name + "_Image8.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.Image9 != null)
            {
                Bitmap bitmap = this.Image9 as Bitmap;
                String filePath = path + "\\" + this.Name + "_Image9.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.Image10 != null)
            {
                Bitmap bitmap = this.Image10 as Bitmap;
                String filePath = path + "\\" + this.Name + "_Image10.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.Image11 != null)
            {
                Bitmap bitmap = this.Image11 as Bitmap;
                String filePath = path + "\\" + this.Name + "_Image11.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.Image12 != null)
            {
                Bitmap bitmap = this.Image12 as Bitmap;
                String filePath = path + "\\" + this.Name + "_Image12.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.Image13 != null)
            {
                Bitmap bitmap = this.Image13 as Bitmap;
                String filePath = path + "\\" + this.Name + "_Image13.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.Image14 != null)
            {
                Bitmap bitmap = this.Image14 as Bitmap;
                String filePath = path + "\\" + this.Name + "_Image14.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.Image15 != null)
            {
                Bitmap bitmap = this.Image15 as Bitmap;
                String filePath = path + "\\" + this.Name + "_Image15.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.FocusImage0 != null)
            {
                Bitmap bitmap = this.FocusImage0 as Bitmap;
                String filePath = path + "\\" + this.Name + "_FocusImage0.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.FocusImage1 != null)
            {
                Bitmap bitmap = this.FocusImage1 as Bitmap;
                String filePath = path + "\\" + this.Name + "_FocusImage1.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.FocusImage2 != null)
            {
                Bitmap bitmap = this.FocusImage2 as Bitmap;
                String filePath = path + "\\" + this.Name + "_FocusImage2.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.FocusImage3 != null)
            {
                Bitmap bitmap = this.FocusImage3 as Bitmap;
                String filePath = path + "\\" + this.Name + "_FocusImage3.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.FocusImage4 != null)
            {
                Bitmap bitmap = this.FocusImage4 as Bitmap;
                String filePath = path + "\\" + this.Name + "_FocusImage4.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.FocusImage5 != null)
            {
                Bitmap bitmap = this.FocusImage5 as Bitmap;
                String filePath = path + "\\" + this.Name + "_FocusImage5.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.FocusImage6 != null)
            {
                Bitmap bitmap = this.FocusImage6 as Bitmap;
                String filePath = path + "\\" + this.Name + "_FocusImage6.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.FocusImage7 != null)
            {
                Bitmap bitmap = this.FocusImage7 as Bitmap;
                String filePath = path + "\\" + this.Name + "_FocusImage7.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.FocusImage8 != null)
            {
                Bitmap bitmap = this.FocusImage8 as Bitmap;
                String filePath = path + "\\" + this.Name + "_FocusImage8.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.FocusImage9 != null)
            {
                Bitmap bitmap = this.FocusImage9 as Bitmap;
                String filePath = path + "\\" + this.Name + "_FocusImage9.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.FocusImage10 != null)
            {
                Bitmap bitmap = this.FocusImage10 as Bitmap;
                String filePath = path + "\\" + this.Name + "_FocusImage10.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.FocusImage11 != null)
            {
                Bitmap bitmap = this.FocusImage11 as Bitmap;
                String filePath = path + "\\" + this.Name + "_FocusImage11.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.FocusImage12 != null)
            {
                Bitmap bitmap = this.FocusImage12 as Bitmap;
                String filePath = path + "\\" + this.Name + "_FocusImage12.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.FocusImage13 != null)
            {
                Bitmap bitmap = this.FocusImage13 as Bitmap;
                String filePath = path + "\\" + this.Name + "_FocusImage13.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.FocusImage14 != null)
            {
                Bitmap bitmap = this.FocusImage14 as Bitmap;
                String filePath = path + "\\" + this.Name + "_FocusImage14.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.FocusImage15 != null)
            {
                Bitmap bitmap = this.FocusImage15 as Bitmap;
                String filePath = path + "\\" + this.Name + "_FocusImage15.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }
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
