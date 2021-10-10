using System;
using System.Collections.Generic;
using System.ComponentModel.Design;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Windows.Forms.Design;

namespace GUIDesigner
{
    public class KeystrokMessageFilter : System.Windows.Forms.IMessageFilter
    {
        public static IDesignerHost host;

        public bool PreFilterMessage(ref Message m)
        {
            if (host == null)
                return false;

            if (m.Msg == 256 /*0x0100 WM_KEYDOWN*/)
            {
                IMenuCommandService mcs = host.GetService(typeof(IMenuCommandService)) as IMenuCommandService;

                switch (((int)m.WParam) | ((int)Control.ModifierKeys))
                {
                case (int)Keys.Up: mcs.GlobalInvoke(MenuCommands.KeyMoveUp);
                    return true;

                case (int)Keys.Down: mcs.GlobalInvoke(MenuCommands.KeyMoveDown);
                    return true;

                case (int)Keys.Right: mcs.GlobalInvoke(MenuCommands.KeyMoveRight);
                    return true;

                case (int)Keys.Left: mcs.GlobalInvoke(MenuCommands.KeyMoveLeft);
                    return true;

                case (int)Keys.Enter:
                    ITU.enterKeyPressed = true;
                    return false;

                case (int)(Keys.Control | Keys.Up): mcs.GlobalInvoke(MenuCommands.KeyNudgeUp);
                    return true;

                case (int)(Keys.Control | Keys.Down): mcs.GlobalInvoke(MenuCommands.KeyNudgeDown);
                    return true;

                case (int)(Keys.Control | Keys.Right): mcs.GlobalInvoke(MenuCommands.KeyNudgeRight);
                    return true;

                case (int)(Keys.Control | Keys.Left): mcs.GlobalInvoke(MenuCommands.KeyNudgeLeft);
                    return true;

                case (int)(Keys.Shift | Keys.Up): mcs.GlobalInvoke(MenuCommands.KeySizeHeightDecrease);
                    return true;

                case (int)(Keys.Shift | Keys.Down): mcs.GlobalInvoke(MenuCommands.KeySizeHeightIncrease);
                    return true;

                case (int)(Keys.Shift | Keys.Right): mcs.GlobalInvoke(MenuCommands.KeySizeWidthIncrease);
                    return true;

                case (int)(Keys.Shift | Keys.Left): mcs.GlobalInvoke(MenuCommands.KeySizeWidthDecrease);
                    return true;

                case (int)(Keys.Control | Keys.Shift | Keys.Up): mcs.GlobalInvoke(MenuCommands.KeyNudgeHeightDecrease);
                    return true;

                case (int)(Keys.Control | Keys.Shift | Keys.Down): mcs.GlobalInvoke(MenuCommands.KeyNudgeHeightIncrease);
                    return true;

                case (int)(Keys.Control | Keys.Shift | Keys.Right): mcs.GlobalInvoke(MenuCommands.KeyNudgeWidthIncrease);
                    return true;

                case (int)(Keys.ControlKey | Keys.Shift | Keys.Left): mcs.GlobalInvoke(MenuCommands.KeyNudgeWidthDecrease);
                    return true;
                }
            }
            else if (m.Msg == 257 /*0x0101 WM_KEYUP*/)
            {
                switch (((int)m.WParam) | ((int)Control.ModifierKeys))
                {
                    case (int)Keys.Enter:
                        ITU.enterKeyPressed = false;
                        return false;
                }
            }
            return false;
        }
    }
}
