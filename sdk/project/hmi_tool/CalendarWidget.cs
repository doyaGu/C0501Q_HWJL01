using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Drawing.Design;
using System.Drawing.Imaging;

namespace GUIDesigner
{
    class CalendarWidget : Panel, IWidget
    {
        public CalendarWidget()
        {
            base.Margin = new Padding(0, 0, 0, 0);
            this.MinimumYear = 2000;
            this.MaximumYear = 2500;
            this.Compress = true;
            this.External = false;
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

        public ITU.WidgetPixelFormat PixelFormat { get; set; }
        public Boolean Compress { get; set; }
        public Boolean External { get; set; }

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

        public enum CalendarWidgetCalendarType
        {
            AD = 0,
            ChineseLunar = 1
        };

        public CalendarWidgetCalendarType CalendarType { get; set; }

        [TypeConverter(typeof(StringListConverter))]
        public String SundayTarget { get; set; }

        [TypeConverter(typeof(StringListConverter))]
        public String MondayTarget { get; set; }

        [TypeConverter(typeof(StringListConverter))]
        public String TuesdayTarget { get; set; }

        [TypeConverter(typeof(StringListConverter))]
        public String WednesdayTarget { get; set; }

        [TypeConverter(typeof(StringListConverter))]
        public String ThursdayTarget { get; set; }

        [TypeConverter(typeof(StringListConverter))]
        public String FridayTarget { get; set; }

        [TypeConverter(typeof(StringListConverter))]
        public String SaturdayTarget { get; set; }

        [TypeConverter(typeof(StringListConverter))]
        public String YearTarget { get; set; }

        [TypeConverter(typeof(StringListConverter))]
        public String MonthTarget { get; set; }

        [TypeConverter(typeof(StringListConverter))]
        public String DayTarget { get; set; }

        public int MinimumYear { get; set; }
        public int MaximumYear { get; set; }
        public int Year { get; set; }
        public int Month { get; set; }
        public int Day { get; set; }

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
            ITUCalendar cal = new ITUCalendar();

            cal.name = this.Name;

            PropertyDescriptorCollection properties = TypeDescriptor.GetProperties(this);
            cal.visible = (bool)properties["Visible"].GetValue(this);

            cal.active = false;
            cal.dirty = false;
            cal.alpha = 255;
            cal.rect.x = this.Location.X;
            cal.rect.y = this.Location.Y;
            cal.rect.width = this.Size.Width;
            cal.rect.height = this.Size.Height;
            cal.color.alpha = this.BackColor.A;
            cal.color.red = this.BackColor.R;
            cal.color.green = this.BackColor.G;
            cal.color.blue = this.BackColor.B;
            cal.bound.x = 0;
            cal.bound.y = 0;
            cal.bound.width = 0;
            cal.bound.height = 0;

            if (this.BackgroundImage != null)
            {
                cal.staticSurf = ITU.CreateSurfaceNode(this.BackgroundImage as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    cal.flags |= ITU.ITU_EXTERNAL;
            }

            cal.caltype = (int)this.CalendarType;
            cal.sunName = this.SundayTarget;
            cal.monName = this.MondayTarget;
            cal.tueName = this.TuesdayTarget;
            cal.wedName = this.WednesdayTarget;
            cal.thuName = this.ThursdayTarget;
            cal.friName = this.FridayTarget;
            cal.satName = this.SaturdayTarget;
            cal.minYear = this.MinimumYear;
            cal.maxYear = this.MaximumYear;
            cal.year = this.Year;
            cal.yearName = this.YearTarget;
            cal.month = this.Month;
            cal.monthName = this.MonthTarget;
            cal.day = this.Day;
            cal.dayName = this.DayTarget;

            return cal;
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
        }
    }
}
