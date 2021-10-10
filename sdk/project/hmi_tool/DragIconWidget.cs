using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Globalization;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows.Forms;
using System.Drawing.Design;
using System.Drawing.Imaging;

namespace GUIDesigner
{
    class DragIconWidget : PictureBox, IWidget
    {
        protected override void OnPaintBackground(PaintEventArgs e)
        {
            if (this.GradientMode == ITU.WidgetGradientMode.None)
                base.OnPaintBackground(e);
            else
            {
                Rectangle rc = new Rectangle(0, 0, this.ClientSize.Width, this.ClientSize.Height);
                using (LinearGradientBrush brush = new LinearGradientBrush(rc, this.BackColor, this.GradientColor, (LinearGradientMode)(this.GradientMode - 1)))
                {
                    e.Graphics.FillRectangle(brush, rc);
                }
            }
        }

        public DragIconWidget()
        {
            SetStyle(ControlStyles.SupportsTransparentBackColor, true);
            base.BackgroundImageLayout = ImageLayout.Center;
            base.SizeMode = PictureBoxSizeMode.CenterImage;
            this.ImagePixelFormat = ITU.WidgetPixelFormat.ARGB8888;
            this.BackImagePixelFormat = ITU.WidgetPixelFormat.ARGB8888;
            this.Compress = false;
            this.aaf = 0;
            //this.EffectSteps = 10;
            //this.ShowDelay = 0;
            //this.HideDelay = -1;
            this.TransferAlpha = false;
            this.Action01 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Press, "", "");
            this.Action02 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Press, "", "");
            this.Action03 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Press, "", "");
            this.Action04 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Press, "", "");
            this.Action05 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Press, "", "");
            this.Action06 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Press, "", "");
            this.Action07 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Press, "", "");
            this.Action08 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Press, "", "");
            this.Action09 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Press, "", "");
            this.Action10 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Press, "", "");
            this.Action11 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Press, "", "");
            this.Action12 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Press, "", "");
            this.Action13 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Press, "", "");
            this.Action14 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Press, "", "");
            this.Action15 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Press, "", "");
        }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        new public Image Image
        {
            get { return base.Image; }
            set { base.Image = value; }
        }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        public override Image BackgroundImage
        {
            get { return base.BackgroundImage; }
            set { base.BackgroundImage = value; }
        }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual ImageLayout BackgroundImageLayout { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual ImageLayout SizeMode { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public BorderStyle BorderStyle { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual Cursor Cursor { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual Font Font { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual Color ForeColor { get; set; }

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
        new public virtual ContextMenuStrip ContextMenuStrip { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool Enabled { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public ImeMode ImeMode { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public int TabIndex { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool TabStop { get; set; }

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
        new public virtual bool AutoScroll { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public Size AutoScrollMargin { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public Size AutoScrollMinSize { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public override bool AutoSize { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual AutoSizeMode AutoSizeMode { get; set; }

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
        new public Padding Padding { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool CausesValidation { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public ControlBindingsCollection DataBindings { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public object Tag { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool WaitOnLoad { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public Image ErrorImage { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public string ImageLocation { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public Image InitialImage { get; set; }

        public ITU.WidgetPixelFormat ImagePixelFormat { get; set; }
        public ITU.WidgetPixelFormat BackImagePixelFormat { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public Boolean Compress { get; set; }
        //public Boolean External { get; set; }

        //public int EffectSteps { get; set; }
        //public int ShowDelay { get; set; }
        //public int HideDelay { get; set; }
        //public ContainerWidget.ContainerWidgetEffect ShowingEffect { get; set; }
        //public ContainerWidget.ContainerWidgetEffect HidingEffect { get; set; }
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public bool TransferAlpha { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public Color GradientColor { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public ITU.WidgetGradientMode GradientMode { get; set; }

        private int aaf;

        public int ActiveAniFrame 
        { 
            get { return aaf; }
            set 
            {
                if (value < 0)
                {
                    aaf = 0;
                }
                else if (value > 20)
                {
                    aaf = 20;
                }
                else
                {
                    aaf = value;
                }
            }
        }

        public enum WidgetEvent
        {
            Press = 1,
            MouseDown = 6,
            MouseUp = 7,
            MouseLongPress = 21,
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
                        WidgetEvent e = WidgetEvent.Press;
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

        private bool hided = false;

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
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
            ITUDragIcon widget = new ITUDragIcon();

            widget.name = this.Name;

            PropertyDescriptorCollection properties = TypeDescriptor.GetProperties(this);
            widget.visible = (bool)properties["Visible"].GetValue(this);

            widget.active = false;
            widget.dirty = false;
            widget.alpha = 255;
            widget.rect.x = this.Location.X;
            widget.rect.y = this.Location.Y;
            widget.rect.width = this.Size.Width;
            widget.rect.height = this.Size.Height;
            widget.color.alpha = this.BackColor.A;
            widget.color.red = this.BackColor.R;
            widget.color.green = this.BackColor.G;
            widget.color.blue = this.BackColor.B;
            widget.bound.x = 0;
            widget.bound.y = 0;
            widget.bound.width = 0;
            widget.bound.height = 0;
            widget.touchX = 0;
            widget.touchY = 0;
            widget.working = 0;
            widget.moveX = 0;
            widget.moveY = 0;
            widget.frame = 0;
            widget.totalframe = this.aaf;
            widget.pw = 25;
            widget.ph = 25;
            widget.tmp1 = 0;
            widget.tmp2 = 0;
            //widget.showDelay = this.ShowDelay;
            //widget.hideDelay = this.HideDelay;
            widget.flags |= this.TransferAlpha ? ITU.ITU_TRANSFER_ALPHA : 0;
            //widget.effectSteps = this.EffectSteps;
            //widget.effects[(int)ITUState.ITU_STATE_SHOWING] = (ITUEffectType)this.ShowingEffect;
            //widget.effects[(int)ITUState.ITU_STATE_HIDING] = (ITUEffectType)this.HidingEffect;

            if (this.BackgroundImage != null)
            {
                widget.staticBackSurf = ITU.CreateSurfaceNode(this.BackgroundImage as Bitmap, this.BackImagePixelFormat, false, false);
            }

            if (this.Image != null)
            {
                widget.staticIconSurf = ITU.CreateSurfaceNode(this.Image as Bitmap, this.ImagePixelFormat, false, false);
            }

            //widget.surfIcon = null;
            
            //widget.graidentMode = (uint)this.GradientMode;
            //widget.gradientColor.alpha = this.GradientColor.A;
            //widget.gradientColor.red = this.GradientColor.R;
            //widget.gradientColor.green = this.GradientColor.G;
            //widget.gradientColor.blue = this.GradientColor.B;

            widget.actions[0].action = (ITUActionType)this.Action01.Action;
            widget.actions[0].ev = (ITUEvent)this.Action01.Event;
            widget.actions[0].target = this.Action01.Target;
            widget.actions[0].param = this.Action01.Parameter;
            widget.actions[1].action = (ITUActionType)this.Action02.Action;
            widget.actions[1].ev = (ITUEvent)this.Action02.Event;
            widget.actions[1].target = this.Action02.Target;
            widget.actions[1].param = this.Action02.Parameter;
            widget.actions[2].action = (ITUActionType)this.Action03.Action;
            widget.actions[2].ev = (ITUEvent)this.Action03.Event;
            widget.actions[2].target = this.Action03.Target;
            widget.actions[2].param = this.Action03.Parameter;
            widget.actions[3].action = (ITUActionType)this.Action04.Action;
            widget.actions[3].ev = (ITUEvent)this.Action04.Event;
            widget.actions[3].target = this.Action04.Target;
            widget.actions[3].param = this.Action04.Parameter;
            widget.actions[4].action = (ITUActionType)this.Action05.Action;
            widget.actions[4].ev = (ITUEvent)this.Action05.Event;
            widget.actions[4].target = this.Action05.Target;
            widget.actions[4].param = this.Action05.Parameter;
            widget.actions[5].action = (ITUActionType)this.Action06.Action;
            widget.actions[5].ev = (ITUEvent)this.Action06.Event;
            widget.actions[5].target = this.Action06.Target;
            widget.actions[5].param = this.Action06.Parameter;
            widget.actions[6].action = (ITUActionType)this.Action07.Action;
            widget.actions[6].ev = (ITUEvent)this.Action07.Event;
            widget.actions[6].target = this.Action07.Target;
            widget.actions[6].param = this.Action07.Parameter;
            widget.actions[7].action = (ITUActionType)this.Action08.Action;
            widget.actions[7].ev = (ITUEvent)this.Action08.Event;
            widget.actions[7].target = this.Action08.Target;
            widget.actions[7].param = this.Action08.Parameter;
            widget.actions[8].action = (ITUActionType)this.Action09.Action;
            widget.actions[8].ev = (ITUEvent)this.Action09.Event;
            widget.actions[8].target = this.Action09.Target;
            widget.actions[8].param = this.Action09.Parameter;
            widget.actions[9].action = (ITUActionType)this.Action10.Action;
            widget.actions[9].ev = (ITUEvent)this.Action10.Event;
            widget.actions[9].target = this.Action10.Target;
            widget.actions[9].param = this.Action10.Parameter;
            widget.actions[10].action = (ITUActionType)this.Action11.Action;
            widget.actions[10].ev = (ITUEvent)this.Action11.Event;
            widget.actions[10].target = this.Action11.Target;
            widget.actions[10].param = this.Action11.Parameter;
            widget.actions[11].action = (ITUActionType)this.Action12.Action;
            widget.actions[11].ev = (ITUEvent)this.Action12.Event;
            widget.actions[11].target = this.Action12.Target;
            widget.actions[11].param = this.Action12.Parameter;
            widget.actions[12].action = (ITUActionType)this.Action13.Action;
            widget.actions[12].ev = (ITUEvent)this.Action13.Event;
            widget.actions[12].target = this.Action13.Target;
            widget.actions[12].param = this.Action13.Parameter;
            widget.actions[13].action = (ITUActionType)this.Action14.Action;
            widget.actions[13].ev = (ITUEvent)this.Action14.Event;
            widget.actions[13].target = this.Action14.Target;
            widget.actions[13].param = this.Action14.Parameter;
            widget.actions[14].action = (ITUActionType)this.Action15.Action;
            widget.actions[14].ev = (ITUEvent)this.Action15.Event;
            widget.actions[14].target = this.Action15.Target;
            widget.actions[14].param = this.Action15.Parameter;

            return widget;
        }

        public void SaveImages(String path)
        {
            if (this.BackgroundImage != null)
            {
                Bitmap bitmap = this.BackgroundImage as Bitmap;
                String filePath = path + "\\" + this.Name + "_BackgroundImage.png";
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
