using System;
using System.Drawing;
using System.Collections.Generic;
using System.ComponentModel.Design;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Windows.Forms.Design;

namespace GUIDesigner
{
    public class MouseMessageFilter : System.Windows.Forms.IMessageFilter
    {
        private const int WM_MOUSEMOVE = 0x0200;

        private ToolStripStatusLabel label;
        public static Control view;

        public MouseMessageFilter(ToolStripStatusLabel label)
        {
            this.label = label;
        }

        public bool PreFilterMessage(ref Message m)
        {
            if (m.Msg == WM_MOUSEMOVE)
            {
                if (view != null)
                {
                    Point mousePosition = view.PointToClient(Control.MousePosition);
                    if (view.ClientRectangle.Contains(mousePosition))
                        label.Text = "(" + mousePosition.X + ", " + mousePosition.Y + ")";
                    else
                        label.Text = "";
                }
            }
            return false;
        }
    }
}
