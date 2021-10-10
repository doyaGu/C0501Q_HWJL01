using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Drawing.Design;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace GUIDesigner
{
    class WheelWidget : ListBox, IWidget
    {
        public WheelWidget()
        {
            base.BorderStyle = System.Windows.Forms.BorderStyle.None;
            base.Margin = new Padding(0, 0, 0, 0);
            this.FocusColor = SystemColors.ActiveCaption;
            this.FocusIndex = 1;
            //this.ItemCount = 3;
            this.Cycle = false;
            //this.Speed = 2;
            //this.SlideCount = 3;
            this.BackAlpha = 255;
            this.TextAlign = ContentAlignment.MiddleCenter;
            this.ItemHeight = base.ItemHeight;
            this.TotalFrame = 10;
            this.Draggable = true;
            this.Touchable = true;
            this.FontSquare = false;
            this.FocusFontHeight = (int)this.Font.SizeInPoints;
            this.BoldSize = 1;
            this.Items1 = new ObjectCollection(new ListBox());
            this.Items2 = new ObjectCollection(new ListBox());
            this.Items3 = new ObjectCollection(new ListBox());
            this.Items4 = new ObjectCollection(new ListBox());
            this.Items5 = new ObjectCollection(new ListBox());
            this.Items6 = new ObjectCollection(new ListBox());
            this.Items7 = new ObjectCollection(new ListBox());
            this.Action01 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Changed, "", "");
            this.Action02 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Changed, "", "");
            this.Action03 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Changed, "", "");
            this.Action04 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Changed, "", "");
            this.Action05 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Changed, "", "");
            this.Action06 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Changed, "", "");
            this.Action07 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Changed, "", "");
            this.Action08 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Changed, "", "");
            this.Action09 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Changed, "", "");
            this.Action10 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Changed, "", "");
            this.Action11 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Changed, "", "");
            this.Action12 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Changed, "", "");
            this.Action13 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Changed, "", "");
            this.Action14 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Changed, "", "");
            this.Action15 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Changed, "", "");

            this.FontChanged += new EventHandler(WheelWidget_FontChanged);
        }

        void WheelWidget_FontChanged(object sender, EventArgs e)
        {
            if (this.ItemHeight < base.ItemHeight)
                this.ItemHeight = base.ItemHeight;

            if (this.FocusFontHeight < (int)this.Font.SizeInPoints)
                this.FocusFontHeight = (int)this.Font.SizeInPoints;
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

        public ContentAlignment TextAlign { get; set; }

        public Color FocusColor { get; set; }
        public int FocusIndex { get; set; }
        //public int ItemCount { get; set; }
        public int TotalFrame { get; set; }
        public byte BackAlpha { get; set; }
        public bool Draggable { get; set; }
        public int FocusFontHeight { get; set; }
        public int FontIndex { get; set; }
        public int BoldSize { get; set; }
        public bool Cycle { get; set; }
        //public int Speed { get; set; }
        //public int SlideCount { get; set; }
        public bool FontSquare{ get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items1 { get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items2 { get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items3 { get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items4 { get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items5 { get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items6 { get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items7 { get; set; }

        public enum WidgetEvent
        {
            Changed = 20,
            Delay0 = 25,
            Delay1 = 26,
            Delay2 = 27,
            Delay3 = 28,
            Delay4 = 29,
            Delay5 = 30,
            Delay6 = 31,
            Delay7 = 32,
            Custom = 100
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
                        WidgetEvent e = WidgetEvent.Changed;
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

        public bool Touchable { get; set; }

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
            ITUWheel wheel = new ITUWheel();

            wheel.type = ITUWidgetType.ITU_WHEEL;
            wheel.name = this.Name;
            wheel.flags |= this.TabStop ? ITU.ITU_TAPSTOP : 0;
            wheel.flags |= this.Touchable ? ITU.ITU_TOUCHABLE : 0;

            PropertyDescriptorCollection properties = TypeDescriptor.GetProperties(this);
            wheel.visible = (bool)properties["Visible"].GetValue(this);

            wheel.active = false;
            wheel.dirty = false;
            wheel.alpha = 255;
            wheel.tabIndex = this.TabIndex;
            wheel.rect.x = this.Location.X;
            wheel.rect.y = this.Location.Y;
            wheel.rect.width = this.Size.Width;
            wheel.rect.height = this.Size.Height;
            wheel.color.alpha = this.BackAlpha;
            wheel.color.red = this.BackColor.R;
            wheel.color.green = this.BackColor.G;
            wheel.color.blue = this.BackColor.B;
            wheel.bound.x = 0;
            wheel.bound.y = 0;
            wheel.bound.width = 0;
            wheel.bound.height = 0;

            /*
            for (int i = 0; i < this.ItemCount / 2; i++)
            {
                ITUText text = new ITUText();

                text.flags = ITU.ITU_CLIP_DISABLED;
                text.name = "";
                text.visible = true;
                text.active = false;
                text.dirty = text.visible ? true : false;
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
                text.fontHeight = (int)this.Font.SizeInPoints;
                text.fontIndex = this.FontIndex;

                string[] texts = new string[] { "" };
                text.stringSet = ITU.CreateStringSetNode(texts);

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
                WidgetNode node = new WidgetNode();
                node.widget = text;
                wheel.items.Add(node);
            }
            */

            int index = 0;
            foreach (object item in this.Items)
            {
                ITUText text = new ITUText();

                text.flags = ITU.ITU_CLIP_DISABLED;
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

                if (base.Font.Bold)
                    text.textFlags |= ITUText.ITU_TEXT_BOLD;

                text.boldSize = this.BoldSize;

                string[] texts = new string[8];

                texts[0] = item.ToString();
                
                if (this.Items1.Count > index)
                    texts[1] = this.Items1[index].ToString();

                if (this.Items2.Count > index)
                    texts[2] = this.Items2[index].ToString();

                if (this.Items3.Count > index)
                    texts[3] = this.Items3[index].ToString();

                if (this.Items4.Count > index)
                    texts[4] = this.Items4[index].ToString();

                if (this.Items5.Count > index)
                    texts[5] = this.Items5[index].ToString();

                if (this.Items6.Count > index)
                    texts[6] = this.Items6[index].ToString();

                if (this.Items7.Count > index)
                    texts[7] = this.Items7[index].ToString();

                text.stringSet = ITU.CreateStringSetNode(texts);

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
                WidgetNode node = new WidgetNode();
                node.widget = text;
                wheel.items.Add(node);
                index++;
            }
            /*
            for (int i = 0; i < this.ItemCount / 2; i++)
            {
                ITUText text = new ITUText();

                text.flags = ITU.ITU_CLIP_DISABLED;
                text.name = "";
                text.visible = true;
                text.active = false;
                text.dirty = text.visible ? true : false;
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
                text.fontHeight = (int)this.Font.SizeInPoints;
                text.fontIndex = this.FontIndex;

                string[] texts = new string[] { "" };
                text.stringSet = ITU.CreateStringSetNode(texts);

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
                WidgetNode node = new WidgetNode();
                node.widget = text;
                wheel.items.Add(node);
            }
            */
            wheel.focusColor.alpha    = this.FocusColor.A;
            wheel.focusColor.red      = this.FocusColor.R;
            wheel.focusColor.green    = this.FocusColor.G;
            wheel.focusColor.blue     = this.FocusColor.B;
            wheel.normalColor.alpha   = this.ForeColor.A;
            wheel.normalColor.red     = this.ForeColor.R;
            wheel.normalColor.green   = this.ForeColor.G;
            wheel.normalColor.blue    = this.ForeColor.B;

            wheel.tempy = 0;
            wheel.shift_one = 0;
            wheel.sliding = 0;
            wheel.scal = 0;
            wheel.moving_step = 0;
            wheel.inside = 0;
            wheel.slide_step = 2;  //this.Speed;
            wheel.slide_itemcount = 0; // this.SlideCount;
            wheel.idle = 0;
            wheel.focusIndex = this.FocusIndex;
            wheel.itemCount = 7; // this.ItemCount;
            wheel.totalframe = this.TotalFrame;



            if (this.Draggable)
                wheel.flags |= ITU.ITU_DRAGGABLE;

            wheel.fontHeight = (int)this.Font.SizeInPoints;
            wheel.focusFontHeight = this.FocusFontHeight;

            if (this.Cycle)
                wheel.cycle_tor = 1;
            else
                wheel.cycle_tor = 0;

            wheel.cycle_arr_count = 0;
            wheel.maxci = 0;
            wheel.minci = 0;
            wheel.layout_ci = 0;
            wheel.fix_count = 0;
            wheel.focus_c = 0;
            wheel.focus_dev = 0;

            if (this.FontSquare)
                wheel.fontsquare = 1;
            else
                wheel.fontsquare = 0;

            for (int i = 0; i < ITU.ITU_WHEEL_CYCLE_ARR_LIMIT; i++)
            {
                wheel.cycle_arr[i] = 0;
            }

            wheel.actions[0].action = (ITUActionType)this.Action01.Action;
            wheel.actions[0].ev = (ITUEvent)this.Action01.Event;
            wheel.actions[0].target = this.Action01.Target;
            wheel.actions[0].param = this.Action01.Parameter;
            wheel.actions[1].action = (ITUActionType)this.Action02.Action;
            wheel.actions[1].ev = (ITUEvent)this.Action02.Event;
            wheel.actions[1].target = this.Action02.Target;
            wheel.actions[1].param = this.Action02.Parameter;
            wheel.actions[2].action = (ITUActionType)this.Action03.Action;
            wheel.actions[2].ev = (ITUEvent)this.Action03.Event;
            wheel.actions[2].target = this.Action03.Target;
            wheel.actions[2].param = this.Action03.Parameter;
            wheel.actions[3].action = (ITUActionType)this.Action04.Action;
            wheel.actions[3].ev = (ITUEvent)this.Action04.Event;
            wheel.actions[3].target = this.Action04.Target;
            wheel.actions[3].param = this.Action04.Parameter;
            wheel.actions[4].action = (ITUActionType)this.Action05.Action;
            wheel.actions[4].ev = (ITUEvent)this.Action05.Event;
            wheel.actions[4].target = this.Action05.Target;
            wheel.actions[4].param = this.Action05.Parameter;
            wheel.actions[5].action = (ITUActionType)this.Action06.Action;
            wheel.actions[5].ev = (ITUEvent)this.Action06.Event;
            wheel.actions[5].target = this.Action06.Target;
            wheel.actions[5].param = this.Action06.Parameter;
            wheel.actions[6].action = (ITUActionType)this.Action07.Action;
            wheel.actions[6].ev = (ITUEvent)this.Action07.Event;
            wheel.actions[6].target = this.Action07.Target;
            wheel.actions[6].param = this.Action07.Parameter;
            wheel.actions[7].action = (ITUActionType)this.Action08.Action;
            wheel.actions[7].ev = (ITUEvent)this.Action08.Event;
            wheel.actions[7].target = this.Action08.Target;
            wheel.actions[7].param = this.Action08.Parameter;
            wheel.actions[8].action = (ITUActionType)this.Action09.Action;
            wheel.actions[8].ev = (ITUEvent)this.Action09.Event;
            wheel.actions[8].target = this.Action09.Target;
            wheel.actions[8].param = this.Action09.Parameter;
            wheel.actions[9].action = (ITUActionType)this.Action10.Action;
            wheel.actions[9].ev = (ITUEvent)this.Action10.Event;
            wheel.actions[9].target = this.Action10.Target;
            wheel.actions[9].param = this.Action10.Parameter;
            wheel.actions[10].action = (ITUActionType)this.Action11.Action;
            wheel.actions[10].ev = (ITUEvent)this.Action11.Event;
            wheel.actions[10].target = this.Action11.Target;
            wheel.actions[10].param = this.Action11.Parameter;
            wheel.actions[11].action = (ITUActionType)this.Action12.Action;
            wheel.actions[11].ev = (ITUEvent)this.Action12.Event;
            wheel.actions[11].target = this.Action12.Target;
            wheel.actions[11].param = this.Action12.Parameter;
            wheel.actions[12].action = (ITUActionType)this.Action13.Action;
            wheel.actions[12].ev = (ITUEvent)this.Action13.Event;
            wheel.actions[12].target = this.Action13.Target;
            wheel.actions[12].param = this.Action13.Parameter;
            wheel.actions[13].action = (ITUActionType)this.Action14.Action;
            wheel.actions[13].ev = (ITUEvent)this.Action14.Event;
            wheel.actions[13].target = this.Action14.Target;
            wheel.actions[13].param = this.Action14.Parameter;
            wheel.actions[14].action = (ITUActionType)this.Action15.Action;
            wheel.actions[14].ev = (ITUEvent)this.Action15.Event;
            wheel.actions[14].target = this.Action15.Target;
            wheel.actions[14].param = this.Action15.Parameter;

            return wheel;
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
