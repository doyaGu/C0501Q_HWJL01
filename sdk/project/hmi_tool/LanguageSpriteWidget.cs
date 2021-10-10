using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace GUIDesigner
{
    class LanguageSpriteWidget : Panel, IWidget
    {
        public LanguageSpriteWidget()
        {
            base.Margin = new Padding(0, 0, 0, 0);
            this.LanguageIndex = 0;
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

        public int LanguageIndex { get; set; }

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
            ITULanguageSprite ls = new ITULanguageSprite();

            ls.name = this.Name;

            PropertyDescriptorCollection properties = TypeDescriptor.GetProperties(this);
            ls.visible = (bool)properties["Visible"].GetValue(this);

            ls.active = false;
            ls.dirty = false;
            ls.alpha = 255;
            ls.rect.x = this.Location.X;
            ls.rect.y = this.Location.Y;
            ls.rect.width = this.Size.Width;
            ls.rect.height = this.Size.Height;
            ls.color.alpha = 255;
            ls.color.red = 0;
            ls.color.green = 0;
            ls.color.blue = 0;
            ls.bound.x = 0;
            ls.bound.y = 0;
            ls.bound.width = 0;
            ls.bound.height = 0;

            ls.frame = this.LanguageIndex;

            ls.actions[0].action = ITUActionType.ITU_ACTION_NONE;
            ls.actions[0].ev = ITUEvent.ITU_EVENT_PRESS;
            ls.actions[0].target = "";
            ls.actions[0].param = "";
            ls.actions[1].action = ITUActionType.ITU_ACTION_NONE;
            ls.actions[1].ev = ITUEvent.ITU_EVENT_PRESS;
            ls.actions[1].target = "";
            ls.actions[1].param = "";
            ls.actions[2].action = ITUActionType.ITU_ACTION_NONE;
            ls.actions[2].ev = ITUEvent.ITU_EVENT_PRESS;
            ls.actions[2].target = "";
            ls.actions[2].param = "";
            ls.actions[3].action = ITUActionType.ITU_ACTION_NONE;
            ls.actions[3].ev = ITUEvent.ITU_EVENT_PRESS;
            ls.actions[3].target = "";
            ls.actions[3].param = "";
            ls.actions[4].action = ITUActionType.ITU_ACTION_NONE;
            ls.actions[4].ev = ITUEvent.ITU_EVENT_PRESS;
            ls.actions[4].target = "";
            ls.actions[4].param = "";
            ls.actions[5].action = ITUActionType.ITU_ACTION_NONE;
            ls.actions[5].ev = ITUEvent.ITU_EVENT_PRESS;
            ls.actions[5].target = "";
            ls.actions[5].param = "";
            ls.actions[6].action = ITUActionType.ITU_ACTION_NONE;
            ls.actions[6].ev = ITUEvent.ITU_EVENT_PRESS;
            ls.actions[6].target = "";
            ls.actions[6].param = "";
            ls.actions[7].action = ITUActionType.ITU_ACTION_NONE;
            ls.actions[7].ev = ITUEvent.ITU_EVENT_PRESS;
            ls.actions[7].target = "";
            ls.actions[7].param = "";
            ls.actions[8].action = ITUActionType.ITU_ACTION_NONE;
            ls.actions[8].ev = ITUEvent.ITU_EVENT_PRESS;
            ls.actions[8].target = "";
            ls.actions[8].param = "";
            ls.actions[9].action = ITUActionType.ITU_ACTION_NONE;
            ls.actions[9].ev = ITUEvent.ITU_EVENT_PRESS;
            ls.actions[9].target = "";
            ls.actions[9].param = "";
            ls.actions[10].action = ITUActionType.ITU_ACTION_NONE;
            ls.actions[10].ev = ITUEvent.ITU_EVENT_PRESS;
            ls.actions[10].target = "";
            ls.actions[10].param = "";
            ls.actions[11].action = ITUActionType.ITU_ACTION_NONE;
            ls.actions[11].ev = ITUEvent.ITU_EVENT_PRESS;
            ls.actions[11].target = "";
            ls.actions[11].param = "";
            ls.actions[12].action = ITUActionType.ITU_ACTION_NONE;
            ls.actions[12].ev = ITUEvent.ITU_EVENT_PRESS;
            ls.actions[12].target = "";
            ls.actions[12].param = "";
            ls.actions[13].action = ITUActionType.ITU_ACTION_NONE;
            ls.actions[13].ev = ITUEvent.ITU_EVENT_PRESS;
            ls.actions[13].target = "";
            ls.actions[13].param = "";
            ls.actions[14].action = ITUActionType.ITU_ACTION_NONE;
            ls.actions[14].ev = ITUEvent.ITU_EVENT_PRESS;
            ls.actions[14].target = "";
            ls.actions[14].param = "";

            return ls;
        }

        public void SaveImages(String path)
        {
        }

        public void WriteFunctions(HashSet<string> functions)
        {
        }
    }
}
