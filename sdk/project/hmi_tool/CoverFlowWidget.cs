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
    class CoverFlowWidget : Panel, IWidget
    {
        public CoverFlowWidget()
        {
            base.Margin = new Padding(0, 0, 0, 0);
            this.FocusIndex = 1;
            this.TotalFrame = 10;
            this.BounceRatio = 10;
            this.Cycle = false;
            this.Draggable = true;
            this.Touchable = true;
            this.EnableAllItems = false;
            this.HasLongPress = false;
            this.Orientation = CoverFlowWidgetLayout.Horizontal;
            this.BoundaryAlign = false;
            this.SlideMaxCount = 2;
            this.StopAnywhere = false;
            this.Overlapsize = 0;

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
        }

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

        public enum CoverFlowWidgetLayout
        {
            Horizontal = 2,
            Vertical = 0
        };

        public int FocusIndex { get; set; }
        public int TotalFrame { get; set; }
        public int BounceRatio { get; set; }
        private bool cycle = false;
        public bool Cycle
        {
            get { return cycle; }
            set
            {
                if (value)
                {
                    overlapsize = 0;
                }
                cycle = value;
            }
        }

        private bool saw = false;
        public bool StopAnywhere 
        {
            get { return saw; }
            set
            {
                if ((this.Cycle) || (!this.BoundaryAlign))
                {
                    saw = false;
                }
                else
                {
                    saw = value;
                }
            }
        }
        private int overlapsize = 0;
        public int Overlapsize
        {
            get { return overlapsize; }
            set
            {
                if ((this.Cycle) || (!this.BoundaryAlign))
                {
                    overlapsize = 0;
                }
                else if (value < 0)
                {
                    overlapsize = 0;
                }
                else
                {
                    overlapsize = value;
                }
            }
        }
        public bool Draggable { get; set; }
        public bool Touchable { get; set; }
        public bool EnableAllItems { get; set; }
        public bool HasLongPress { get; set; }
        public CoverFlowWidgetLayout Orientation { get; set; }
        private bool boundaryalign = false;
        public bool BoundaryAlign
        {
            get { return boundaryalign; }
            set
            {
                if (!value)
                {
                    overlapsize = 0;
                }
                boundaryalign = value;
            }
        }
        public int SlideMaxCount { get; set; }

        public enum WidgetEvent
        {
            SlideLeft = 10,
            SlideUp = 11,
            SlideRight = 12,
            SlideDown = 13,
            Changed = 20,
            Delay0 = 25,
            Delay1 = 26,
            Delay2 = 27,
            Delay3 = 28,
            Delay4 = 29,
            Delay5 = 30,
            Delay6 = 31,
            Delay7 = 32
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
            ITUCoverFlow coverFlow = new ITUCoverFlow();

            coverFlow.type = ITUWidgetType.ITU_COVERFLOW;
            coverFlow.name = this.Name;
            coverFlow.flags |= this.TabStop ? ITU.ITU_TAPSTOP : 0;
            coverFlow.flags |= this.HasLongPress ? ITU.ITU_HAS_LONG_PRESS : 0;
            coverFlow.flags |= this.Touchable ? ITU.ITU_TOUCHABLE : 0;

            PropertyDescriptorCollection properties = TypeDescriptor.GetProperties(this);
            coverFlow.visible = (bool)properties["Visible"].GetValue(this);

            coverFlow.active = false;
            coverFlow.dirty = false;
            coverFlow.alpha = 255;
            coverFlow.tabIndex = this.TabIndex;
            coverFlow.rect.x = this.Location.X;
            coverFlow.rect.y = this.Location.Y;
            coverFlow.rect.width = this.Size.Width;
            coverFlow.rect.height = this.Size.Height;
            coverFlow.color.alpha = this.BackColor.A;
            coverFlow.color.red = this.BackColor.R;
            coverFlow.color.green = this.BackColor.G;
            coverFlow.color.blue = this.BackColor.B;
            coverFlow.bound.x = 0;
            coverFlow.bound.y = 0;
            coverFlow.bound.width = 0;
            coverFlow.bound.height = 0;

            coverFlow.layout = ITULayout.ITU_LAYOUT_LEFT;
            coverFlow.focusIndex = this.FocusIndex;
            coverFlow.totalframe = this.TotalFrame;
            coverFlow.bounceRatio = this.BounceRatio;
            coverFlow.boundaryAlign = (this.BoundaryAlign) ? (1) : (0);
            coverFlow.slideCount = 0;
            coverFlow.slideMaxCount = this.SlideMaxCount;
            coverFlow.overlapsize = this.Overlapsize;

            if (this.Cycle)
                coverFlow.coverFlowFlags |= ITUCoverFlow.ITU_COVERFLOW_CYCLE;

            if (this.Draggable)
                coverFlow.flags |= ITU.ITU_DRAGGABLE;

            if (this.EnableAllItems)
                coverFlow.coverFlowFlags |= ITUCoverFlow.ITU_COVERFLOW_ENABLE_ALL;

            if (this.Orientation == CoverFlowWidgetLayout.Vertical)
            {
                coverFlow.layout = ITULayout.ITU_LAYOUT_UP;
                coverFlow.coverFlowFlags |= ITUCoverFlow.ITU_COVERFLOW_VERTICAL;
            }

            if (saw)
                coverFlow.coverFlowFlags |= ITUCoverFlow.ITU_COVERFLOW_ANYSTOP;


            coverFlow.actions[0].action = (ITUActionType)this.Action01.Action;
            coverFlow.actions[0].ev = (ITUEvent)this.Action01.Event;
            coverFlow.actions[0].target = this.Action01.Target;
            coverFlow.actions[0].param = this.Action01.Parameter;
            coverFlow.actions[1].action = (ITUActionType)this.Action02.Action;
            coverFlow.actions[1].ev = (ITUEvent)this.Action02.Event;
            coverFlow.actions[1].target = this.Action02.Target;
            coverFlow.actions[1].param = this.Action02.Parameter;
            coverFlow.actions[2].action = (ITUActionType)this.Action03.Action;
            coverFlow.actions[2].ev = (ITUEvent)this.Action03.Event;
            coverFlow.actions[2].target = this.Action03.Target;
            coverFlow.actions[2].param = this.Action03.Parameter;
            coverFlow.actions[3].action = (ITUActionType)this.Action04.Action;
            coverFlow.actions[3].ev = (ITUEvent)this.Action04.Event;
            coverFlow.actions[3].target = this.Action04.Target;
            coverFlow.actions[3].param = this.Action04.Parameter;
            coverFlow.actions[4].action = (ITUActionType)this.Action05.Action;
            coverFlow.actions[4].ev = (ITUEvent)this.Action05.Event;
            coverFlow.actions[4].target = this.Action05.Target;
            coverFlow.actions[4].param = this.Action05.Parameter;
            coverFlow.actions[5].action = (ITUActionType)this.Action06.Action;
            coverFlow.actions[5].ev = (ITUEvent)this.Action06.Event;
            coverFlow.actions[5].target = this.Action06.Target;
            coverFlow.actions[5].param = this.Action06.Parameter;
            coverFlow.actions[6].action = (ITUActionType)this.Action07.Action;
            coverFlow.actions[6].ev = (ITUEvent)this.Action07.Event;
            coverFlow.actions[6].target = this.Action07.Target;
            coverFlow.actions[6].param = this.Action07.Parameter;
            coverFlow.actions[7].action = (ITUActionType)this.Action08.Action;
            coverFlow.actions[7].ev = (ITUEvent)this.Action08.Event;
            coverFlow.actions[7].target = this.Action08.Target;
            coverFlow.actions[7].param = this.Action08.Parameter;
            coverFlow.actions[8].action = (ITUActionType)this.Action09.Action;
            coverFlow.actions[8].ev = (ITUEvent)this.Action09.Event;
            coverFlow.actions[8].target = this.Action09.Target;
            coverFlow.actions[8].param = this.Action09.Parameter;
            coverFlow.actions[9].action = (ITUActionType)this.Action10.Action;
            coverFlow.actions[9].ev = (ITUEvent)this.Action10.Event;
            coverFlow.actions[9].target = this.Action10.Target;
            coverFlow.actions[9].param = this.Action10.Parameter;
            coverFlow.actions[10].action = (ITUActionType)this.Action11.Action;
            coverFlow.actions[10].ev = (ITUEvent)this.Action11.Event;
            coverFlow.actions[10].target = this.Action11.Target;
            coverFlow.actions[10].param = this.Action11.Parameter;
            coverFlow.actions[11].action = (ITUActionType)this.Action12.Action;
            coverFlow.actions[11].ev = (ITUEvent)this.Action12.Event;
            coverFlow.actions[11].target = this.Action12.Target;
            coverFlow.actions[11].param = this.Action12.Parameter;
            coverFlow.actions[12].action = (ITUActionType)this.Action13.Action;
            coverFlow.actions[12].ev = (ITUEvent)this.Action13.Event;
            coverFlow.actions[12].target = this.Action13.Target;
            coverFlow.actions[12].param = this.Action13.Parameter;
            coverFlow.actions[13].action = (ITUActionType)this.Action14.Action;
            coverFlow.actions[13].ev = (ITUEvent)this.Action14.Event;
            coverFlow.actions[13].target = this.Action14.Target;
            coverFlow.actions[13].param = this.Action14.Parameter;
            coverFlow.actions[14].action = (ITUActionType)this.Action15.Action;
            coverFlow.actions[14].ev = (ITUEvent)this.Action15.Event;
            coverFlow.actions[14].target = this.Action15.Target;
            coverFlow.actions[14].param = this.Action15.Parameter;

            return coverFlow;
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
