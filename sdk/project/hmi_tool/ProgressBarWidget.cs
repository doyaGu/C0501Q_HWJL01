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
    class ProgressBarWidget : Button, IWidget
    {
        public ProgressBarWidget()
        {
            base.BackColor = SystemColors.Window;
            base.BackgroundImageLayout = ImageLayout.None;
            this.Compress = true;
            this.External = false;
            this.ExternalImage = false;
            base.FlatAppearance.BorderColor = SystemColors.WindowFrame;
            base.FlatAppearance.MouseDownBackColor = SystemColors.Highlight;
            base.FlatAppearance.MouseOverBackColor = Color.Transparent;
            base.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            base.Margin = new Padding(0, 0, 0, 0);
            this.Maximum = 100;
            this.Step = 1;
            this.Orientation = ProgressBarWidgetLayout.Horizontal;
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
        new public virtual Cursor Cursor { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual Font Font { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual Color ForeColor { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public FlatStyle FlatStyle { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public Image Image { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public ContentAlignment ImageAlign { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public int ImageIndex { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public string ImageKey { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public ImageList ImageList { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual RightToLeft RightToLeft { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public string Text { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual ContentAlignment TextAlign { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public TextImageRelation TextImageRelation { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool UseMnemonic { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool UseVisualStyleBackColor { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool UseWaitCursor { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual bool AllowDrop { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool AutoEllipsis { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual ContextMenuStrip ContextMenuStrip { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual DialogResult DialogResult { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool Enabled { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public int TabIndex { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool TabStop { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool UseCompatibleTextRendering { get; set; }

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

        public ITU.WidgetPixelFormat PixelFormat { get; set; }
        public Boolean Compress { get; set; }
        public Boolean External { get; set; }
        public Boolean ExternalImage { get; set; }

        public enum ProgressBarWidgetLayout
        {
            Horizontal = 2,
            Vertical = 0
        };

        public ProgressBarWidgetLayout Orientation { get; set; }
        public int Minimum { get; set; }
        public int Maximum { get; set; }
        public int Step { get; set; }
        public int Value { get; set; }
        public Color GradientColor { get; set; }
        public ITU.WidgetGradientMode GradientMode { get; set; }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image BarImage { get; set; }

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
        public String ValueTarget { get; set; }

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
            ITUProgressBar bar = new ITUProgressBar();

            bar.name = this.Name;

            PropertyDescriptorCollection properties = TypeDescriptor.GetProperties(this);
            bar.visible = (bool)properties["Visible"].GetValue(this);

            bar.active = false;
            bar.dirty = false;
            bar.alpha = 255;
            bar.rect.x = this.Location.X;
            bar.rect.y = this.Location.Y;
            bar.rect.width = this.Size.Width;
            bar.rect.height = this.Size.Height;
            bar.color.alpha = this.BackColor.A;
            bar.color.red = this.BackColor.R;
            bar.color.green = this.BackColor.G;
            bar.color.blue = this.BackColor.B;
            bar.bound.x = 0;
            bar.bound.y = 0;
            bar.bound.width = 0;
            bar.bound.height = 0;

            if (this.ExternalImage)
                bar.flags |= ITU.ITU_EXTERNAL_IMAGE;

            if (this.BackgroundImage != null)
            {
                bar.staticSurf = ITU.CreateSurfaceNode(this.BackgroundImage as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    bar.flags |= ITU.ITU_EXTERNAL;
            }

            bar.fgColor.alpha = this.FlatAppearance.MouseDownBackColor.A;
            bar.fgColor.red = this.FlatAppearance.MouseDownBackColor.R;
            bar.fgColor.green = this.FlatAppearance.MouseDownBackColor.G;
            bar.fgColor.blue = this.FlatAppearance.MouseDownBackColor.B;

            bar.layout = (ITULayout)this.Orientation;
            bar.min = this.Minimum;
            bar.max = this.Maximum;
            bar.step = this.Step;
            bar.value = this.Value;
            bar.valueName = this.ValueTarget;

            bar.graidentMode = (uint)this.GradientMode;
            bar.gradientColor.alpha = this.GradientColor.A;
            bar.gradientColor.red = this.GradientColor.R;
            bar.gradientColor.green = this.GradientColor.G;
            bar.gradientColor.blue = this.GradientColor.B;

            if (this.BarImage != null)
            {
                bar.staticBarSurf = ITU.CreateSurfaceNode(this.BarImage as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    bar.flags |= ITU.ITU_EXTERNAL;
            }            

            return bar;
        }

        public void SaveImages(String path)
        {
            if (this.BackgroundImage != null)
            {
                Bitmap bitmap = this.BackgroundImage as Bitmap;
                String filePath = path + "\\" + this.Name + "_BackgroundImage.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }

            if (this.BarImage != null)
            {
                Bitmap bitmap = this.BarImage as Bitmap;
                String filePath = path + "\\" + this.Name + "_FocusImage.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }
        }

        public void WriteFunctions(HashSet<string> functions)
        {
        }
    }
}
