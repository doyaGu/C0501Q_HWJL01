using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows.Forms;
using System.Drawing.Design;
using System.Drawing.Imaging;

namespace GUIDesigner
{
    class BackgroundWidget : Panel, IWidget
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
        
        public BackgroundWidget()
        {
            SetStyle(ControlStyles.SupportsTransparentBackColor, true);
            base.BackgroundImageLayout = ImageLayout.None;
            this.Compress = true;
            this.External = false;
            this.ExternalImage = false;
            this.EffectSteps = 10;
            this.ShowDelay = 0;
            this.HideDelay = -1;
            this.AlwaysVisible = false;
            this.TransferAlpha = false;
            this.Stretch = false;
            this.TransformType = IconWidget.WidgetTransformType.None;
            this.TransformX = 100;
            this.TransformY = 100;
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

        public ITU.WidgetPixelFormat PixelFormat { get; set; }
        public Boolean Compress { get; set; }
        public Boolean External { get; set; }
        public Boolean ExternalImage { get; set; }

        public int EffectSteps { get; set; }
        public int ShowDelay { get; set; }
        public int HideDelay { get; set; }
        public bool AlwaysVisible { get; set; }
        public bool TransferAlpha { get; set; }
        public ContainerWidget.ContainerWidgetEffect ShowingEffect { get; set; }
        public ContainerWidget.ContainerWidgetEffect HidingEffect { get; set; }
        public Color GradientColor { get; set; }
        public ITU.WidgetGradientMode GradientMode { get; set; }

        public Boolean Stretch
        {
            get { return base.BackgroundImageLayout == ImageLayout.Stretch; }
            set { base.BackgroundImageLayout = value ? ImageLayout.Stretch : ImageLayout.None; }
        }

        public int Angle { get; set; }

        public IconWidget.WidgetTransformType TransformType { get; set; }

        private int transformX = 0;
        public int TransformX
        {
            get
            {
                return transformX;
            }

            set
            {
                if (value < 0)
                    transformX = 0;
                else if (value > ITU.ITU_TRANSFORM_MAX_VALUE)
                    transformX = ITU.ITU_TRANSFORM_MAX_VALUE;
                else
                    transformX = value;
            }
        }

        private int transformY = 0;
        public int TransformY
        {
            get
            {
                return transformY;
            }

            set
            {
                if (value < 0)
                    transformY = 0;
                else if (value > ITU.ITU_TRANSFORM_MAX_VALUE)
                    transformY = ITU.ITU_TRANSFORM_MAX_VALUE;
                else
                    transformY = value;
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
            PropertyDescriptorCollection properties = TypeDescriptor.GetProperties(this);
            ITUBackground widget = new ITUBackground();
            
            widget.name = this.Name;

            if (!(bool)properties["Enabled"].GetValue(this))
                widget.flags &= ~ITU.ITU_ENABLED;

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
            widget.showDelay = this.ShowDelay;
            widget.hideDelay = this.HideDelay;
            widget.flags |= this.AlwaysVisible ? ITU.ITU_ALWAYS_VISIBLE : 0;
            widget.flags |= this.TransferAlpha ? ITU.ITU_TRANSFER_ALPHA : 0;
            widget.effectSteps = this.EffectSteps;
            widget.effects[(int)ITUState.ITU_STATE_SHOWING] = (ITUEffectType)this.ShowingEffect;
            widget.effects[(int)ITUState.ITU_STATE_HIDING] = (ITUEffectType)this.HidingEffect;

            if (this.Stretch)
            {
                widget.flags |= ITU.ITU_STRETCH;
            }
            widget.angle = this.Angle;

            widget.transformType = (ITUTransformType)this.TransformType;
            widget.transformX = this.TransformX;
            widget.transformY = this.TransformY;

            if (this.ExternalImage)
                widget.flags |= ITU.ITU_EXTERNAL_IMAGE;

            if (this.BackgroundImage != null)
            {
                widget.staticSurf = ITU.CreateSurfaceNode(this.BackgroundImage as Bitmap, this.PixelFormat, this.Compress, this.External);
                if (this.External)
                    widget.flags |= ITU.ITU_EXTERNAL;
            }
            
            widget.graidentMode = (uint)this.GradientMode;
            widget.gradientColor.alpha = this.GradientColor.A;
            widget.gradientColor.red = this.GradientColor.R;
            widget.gradientColor.green = this.GradientColor.G;
            widget.gradientColor.blue = this.GradientColor.B;

            return widget;
        }

        public void SaveImages(String path)
        {
            if (this.BackgroundImage != null)
            {
                Bitmap bitmap = this.BackgroundImage as Bitmap;
                String filePath = path + "\\" + this.Name.Replace('/', '_') + "_BackgroundImage.png";
                bitmap.Save(filePath, ImageFormat.Png);
            }
        }

        public void WriteFunctions(HashSet<string> functions)
        {
        }
    }
}
