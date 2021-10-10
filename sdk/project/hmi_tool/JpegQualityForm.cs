using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace GUIDesigner
{
    public partial class JpegQualityForm : Form
    {
        public JpegQualityForm()
        {
            InitializeComponent();

            this.qualityNumericUpDown.Value = ITU.jpegQuality;
        }

        private void okButton_Click(object sender, EventArgs e)
        {
            ITU.jpegQuality = (int)this.qualityNumericUpDown.Value;
            this.Close();
        }
    }
}
