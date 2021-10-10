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
    class AudioWidget : FontDialog, IWidget
    {
        public AudioWidget()
        {
            this.FilePath = "A:/audio.mp3";
            this.Volume = 100;
            this.VolumeEnable = true;
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
        new public bool AllowScriptChange { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool AllowSimulations { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool AllowVectorFonts { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool AllowVerticalFonts { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public Color Color { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool FixedPitchOnly { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public Font Font { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool FontMustExist { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public int MaxSize { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public int MinSize { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool ScriptsOnly { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool ShowApply { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool ShowColor { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool ShowEffects { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool ShowHelp { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public object Tag { get; set; }

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

        public string Name
        {
            get
            {
                return this.Site.Name;
            }

            set
            {
            }
        }

        public bool Repeat { get; set; }
        public bool Playing { get; set; }
        public string FilePath { get; set; }
        public bool VolumeEnable { get; set; }
        public int Volume { get; set; }

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
            ITUAudio audio = new ITUAudio();

            audio.name = this.Site.Name;

            audio.visible = true;
            audio.active = false;
            audio.dirty = false;
            audio.alpha = 0;
            audio.rect.x = 0;
            audio.rect.y = 0;
            audio.rect.width = 0;
            audio.rect.height = 0;
            audio.color.alpha = 0;
            audio.color.red = 0;
            audio.color.green = 0;
            audio.color.blue = 0;
            audio.bound.x = 0;
            audio.bound.y = 0;
            audio.bound.width = 0;
            audio.bound.height = 0;

            audio.audioFlags = 0;

            if (this.Repeat)
                audio.audioFlags |= ITUAudio.ITU_AUDIO_REPEAT;

            if (this.Playing)
                audio.audioFlags |= ITUAudio.ITU_AUDIO_PLAYING;

            if (this.VolumeEnable)
                audio.audioFlags |= ITUAudio.ITU_AUDIO_VOLUME;

            audio.filePath = this.FilePath;
            audio.volume = this.Volume;

            audio.actions[0].action = (ITUActionType)this.Action01.Action;
            audio.actions[0].ev = (ITUEvent)this.Action01.Event;
            audio.actions[0].target = this.Action01.Target;
            audio.actions[0].param = this.Action01.Parameter;
            audio.actions[1].action = (ITUActionType)this.Action02.Action;
            audio.actions[1].ev = (ITUEvent)this.Action02.Event;
            audio.actions[1].target = this.Action02.Target;
            audio.actions[1].param = this.Action02.Parameter;
            audio.actions[2].action = (ITUActionType)this.Action03.Action;
            audio.actions[2].ev = (ITUEvent)this.Action03.Event;
            audio.actions[2].target = this.Action03.Target;
            audio.actions[2].param = this.Action03.Parameter;
            audio.actions[3].action = (ITUActionType)this.Action04.Action;
            audio.actions[3].ev = (ITUEvent)this.Action04.Event;
            audio.actions[3].target = this.Action04.Target;
            audio.actions[3].param = this.Action04.Parameter;
            audio.actions[4].action = (ITUActionType)this.Action05.Action;
            audio.actions[4].ev = (ITUEvent)this.Action05.Event;
            audio.actions[4].target = this.Action05.Target;
            audio.actions[4].param = this.Action05.Parameter;
            audio.actions[5].action = (ITUActionType)this.Action06.Action;
            audio.actions[5].ev = (ITUEvent)this.Action06.Event;
            audio.actions[5].target = this.Action06.Target;
            audio.actions[5].param = this.Action06.Parameter;
            audio.actions[6].action = (ITUActionType)this.Action07.Action;
            audio.actions[6].ev = (ITUEvent)this.Action07.Event;
            audio.actions[6].target = this.Action07.Target;
            audio.actions[6].param = this.Action07.Parameter;
            audio.actions[7].action = (ITUActionType)this.Action08.Action;
            audio.actions[7].ev = (ITUEvent)this.Action08.Event;
            audio.actions[7].target = this.Action08.Target;
            audio.actions[7].param = this.Action08.Parameter;
            audio.actions[8].action = (ITUActionType)this.Action09.Action;
            audio.actions[8].ev = (ITUEvent)this.Action09.Event;
            audio.actions[8].target = this.Action09.Target;
            audio.actions[8].param = this.Action09.Parameter;
            audio.actions[9].action = (ITUActionType)this.Action10.Action;
            audio.actions[9].ev = (ITUEvent)this.Action10.Event;
            audio.actions[9].target = this.Action10.Target;
            audio.actions[9].param = this.Action10.Parameter;
            audio.actions[10].action = (ITUActionType)this.Action11.Action;
            audio.actions[10].ev = (ITUEvent)this.Action11.Event;
            audio.actions[10].target = this.Action11.Target;
            audio.actions[10].param = this.Action11.Parameter;
            audio.actions[11].action = (ITUActionType)this.Action12.Action;
            audio.actions[11].ev = (ITUEvent)this.Action12.Event;
            audio.actions[11].target = this.Action12.Target;
            audio.actions[11].param = this.Action12.Parameter;
            audio.actions[12].action = (ITUActionType)this.Action13.Action;
            audio.actions[12].ev = (ITUEvent)this.Action13.Event;
            audio.actions[12].target = this.Action13.Target;
            audio.actions[12].param = this.Action13.Parameter;
            audio.actions[13].action = (ITUActionType)this.Action14.Action;
            audio.actions[13].ev = (ITUEvent)this.Action14.Event;
            audio.actions[13].target = this.Action14.Target;
            audio.actions[13].param = this.Action14.Parameter;
            audio.actions[14].action = (ITUActionType)this.Action15.Action;
            audio.actions[14].ev = (ITUEvent)this.Action15.Event;
            audio.actions[14].target = this.Action15.Target;
            audio.actions[14].param = this.Action15.Parameter;

            return audio;
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
