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
    class DrawPenWidget : Panel, IWidget
    {
        public DrawPenWidget()
        {
            base.Margin = new Padding(0, 0, 0, 0);

            this.Visible = true;
            //this.Hided = false;
            this.Pressure = -1;
            this.PenColor = Color.Black;
            this.Action01 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Stopped, "", "");
            this.Action02 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Stopped, "", "");
            this.Action03 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Stopped, "", "");
            this.Action04 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Stopped, "", "");
            this.Action05 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Stopped, "", "");
            this.Action06 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Stopped, "", "");
            this.Action07 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Stopped, "", "");
            this.Action08 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Stopped, "", "");
            this.Action09 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Stopped, "", "");
            this.Action10 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Stopped, "", "");
            this.Action11 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Stopped, "", "");
            this.Action12 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Stopped, "", "");
            this.Action13 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Stopped, "", "");
            this.Action14 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Stopped, "", "");
            this.Action15 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Stopped, "", "");
        }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual Color BackColor { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual Image BackgroundImage { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual ImageLayout BackgroundImageLayout { get; set; }

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
        public bool Visible
        {
            get
            {
                return true;
            }

            set
            {
            }
        }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public bool Hided
        {
            get
            {
                return false;
            }

            set
            {
            }
        }

        public int Pressure { get; set; }
        public Color PenColor { get; set; }

        public enum WidgetEvent
        {
            Stopped = 33
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
                        WidgetEvent e = WidgetEvent.Stopped;
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

        public ITUWidget CreateITUWidget()
        {
            ITUDrawPen drawpen = new ITUDrawPen();

            drawpen.name = this.Site.Name;

            drawpen.visible = true;
            drawpen.active = false;
            drawpen.dirty = false;
            drawpen.alpha = 255;
            drawpen.rect.x = this.Location.X;
            drawpen.rect.y = this.Location.Y;
            drawpen.rect.width = this.Size.Width;
            drawpen.rect.height = this.Size.Height;
            //drawpen.color.alpha = 0;
            //drawpen.color.red = 0;
            //drawpen.color.green = 0;
            //drawpen.color.blue = 0;
            drawpen.bound.x = 0;
            drawpen.bound.y = 0;
            drawpen.bound.width = 0;
            drawpen.bound.height = 0;

            drawpen.drawpenFlags = 0;
            drawpen.surf = 0;
            drawpen.screensurf = 0;
            drawpen.bitsurf = 0;
            drawpen.cursorsurf = 0;
            drawpen.pensurf = 0;

            drawpen.pressure = this.Pressure;
            drawpen.pressure_rawdata = 0;
            drawpen.hasmove = 0;
            drawpen.px0 = 0;
            drawpen.py0 = 0;
            drawpen.px1 = 0;
            drawpen.py1 = 0;
            drawpen.px2 = 0;
            drawpen.py2 = 0;
            drawpen.px3 = 0;
            drawpen.py3 = 0;
            drawpen.last_pen_x = 0;
            drawpen.last_pen_y = 0;
            drawpen.pen_x = 0;
            drawpen.pen_y = 0;
            drawpen.usb_status = 0;
            drawpen.usb_working = 0;
            drawpen.usb_ready_to_rw = 0;
            drawpen.usb_lock = 0;
            drawpen.usb_thread = 0;
            drawpen.lastcolor.alpha = this.PenColor.A;
            drawpen.lastcolor.red = this.PenColor.R;
            drawpen.lastcolor.green = this.PenColor.G;
            drawpen.lastcolor.blue = this.PenColor.B;
            drawpen.pencolor.alpha = this.PenColor.A;
            drawpen.pencolor.red = this.PenColor.R;
            drawpen.pencolor.green = this.PenColor.G;
            drawpen.pencolor.blue = this.PenColor.B;

            drawpen.actions[0].action = (ITUActionType)this.Action01.Action;
            drawpen.actions[0].ev = (ITUEvent)this.Action01.Event;
            drawpen.actions[0].target = this.Action01.Target;
            drawpen.actions[0].param = this.Action01.Parameter;
            drawpen.actions[1].action = (ITUActionType)this.Action02.Action;
            drawpen.actions[1].ev = (ITUEvent)this.Action02.Event;
            drawpen.actions[1].target = this.Action02.Target;
            drawpen.actions[1].param = this.Action02.Parameter;
            drawpen.actions[2].action = (ITUActionType)this.Action03.Action;
            drawpen.actions[2].ev = (ITUEvent)this.Action03.Event;
            drawpen.actions[2].target = this.Action03.Target;
            drawpen.actions[2].param = this.Action03.Parameter;
            drawpen.actions[3].action = (ITUActionType)this.Action04.Action;
            drawpen.actions[3].ev = (ITUEvent)this.Action04.Event;
            drawpen.actions[3].target = this.Action04.Target;
            drawpen.actions[3].param = this.Action04.Parameter;
            drawpen.actions[4].action = (ITUActionType)this.Action05.Action;
            drawpen.actions[4].ev = (ITUEvent)this.Action05.Event;
            drawpen.actions[4].target = this.Action05.Target;
            drawpen.actions[4].param = this.Action05.Parameter;
            drawpen.actions[5].action = (ITUActionType)this.Action06.Action;
            drawpen.actions[5].ev = (ITUEvent)this.Action06.Event;
            drawpen.actions[5].target = this.Action06.Target;
            drawpen.actions[5].param = this.Action06.Parameter;
            drawpen.actions[6].action = (ITUActionType)this.Action07.Action;
            drawpen.actions[6].ev = (ITUEvent)this.Action07.Event;
            drawpen.actions[6].target = this.Action07.Target;
            drawpen.actions[6].param = this.Action07.Parameter;
            drawpen.actions[7].action = (ITUActionType)this.Action08.Action;
            drawpen.actions[7].ev = (ITUEvent)this.Action08.Event;
            drawpen.actions[7].target = this.Action08.Target;
            drawpen.actions[7].param = this.Action08.Parameter;
            drawpen.actions[8].action = (ITUActionType)this.Action09.Action;
            drawpen.actions[8].ev = (ITUEvent)this.Action09.Event;
            drawpen.actions[8].target = this.Action09.Target;
            drawpen.actions[8].param = this.Action09.Parameter;
            drawpen.actions[9].action = (ITUActionType)this.Action10.Action;
            drawpen.actions[9].ev = (ITUEvent)this.Action10.Event;
            drawpen.actions[9].target = this.Action10.Target;
            drawpen.actions[9].param = this.Action10.Parameter;
            drawpen.actions[10].action = (ITUActionType)this.Action11.Action;
            drawpen.actions[10].ev = (ITUEvent)this.Action11.Event;
            drawpen.actions[10].target = this.Action11.Target;
            drawpen.actions[10].param = this.Action11.Parameter;
            drawpen.actions[11].action = (ITUActionType)this.Action12.Action;
            drawpen.actions[11].ev = (ITUEvent)this.Action12.Event;
            drawpen.actions[11].target = this.Action12.Target;
            drawpen.actions[11].param = this.Action12.Parameter;
            drawpen.actions[12].action = (ITUActionType)this.Action13.Action;
            drawpen.actions[12].ev = (ITUEvent)this.Action13.Event;
            drawpen.actions[12].target = this.Action13.Target;
            drawpen.actions[12].param = this.Action13.Parameter;
            drawpen.actions[13].action = (ITUActionType)this.Action14.Action;
            drawpen.actions[13].ev = (ITUEvent)this.Action14.Event;
            drawpen.actions[13].target = this.Action14.Target;
            drawpen.actions[13].param = this.Action14.Parameter;
            drawpen.actions[14].action = (ITUActionType)this.Action15.Action;
            drawpen.actions[14].ev = (ITUEvent)this.Action15.Event;
            drawpen.actions[14].target = this.Action15.Target;
            drawpen.actions[14].param = this.Action15.Parameter;

            return drawpen;
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
