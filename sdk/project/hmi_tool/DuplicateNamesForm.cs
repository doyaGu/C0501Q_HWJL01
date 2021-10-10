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
    public partial class DuplicateNamesForm : Form
    {
        public DuplicateNamesForm()
        {
            InitializeComponent();
        }

        private void closeButton_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        public void AppendLine(string text)
        {
            this.resultTextBox.AppendText(text + "\n");
        }
    }
}
