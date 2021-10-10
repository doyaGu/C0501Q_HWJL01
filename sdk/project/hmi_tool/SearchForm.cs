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
    public partial class SearchForm : Form
    {
        public SearchForm()
        {
            InitializeComponent();

            inputTextBox.KeyPress += new KeyPressEventHandler(inputTextBox_KeyPress);
            this.ActiveControl = inputTextBox;
        }

        void inputTextBox_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (e.KeyChar == Convert.ToChar(13))
            {
                MainForm.mainForm.SearchWidget(this.inputTextBox.Text);
                this.Close();
            }
        }

        private void searchButton_Click(object sender, EventArgs e)
        {
            MainForm.mainForm.SearchWidget(this.inputTextBox.Text);
            this.Close();
        }

        private void inputTextBox_TextChanged(object sender, EventArgs e)
        {

        }

        private void label1_Click(object sender, EventArgs e)
        {

        }
    }
}
