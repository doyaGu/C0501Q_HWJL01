namespace hmi_mcu
{
    partial class MainForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.commandListBox = new System.Windows.Forms.ListBox();
            this.execButton = new System.Windows.Forms.Button();
            this.portComboBox = new System.Windows.Forms.ComboBox();
            this.portStateButton = new System.Windows.Forms.Button();
            this.recvTextBox = new System.Windows.Forms.TextBox();
            this.paramListBox = new System.Windows.Forms.ListBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // commandListBox
            // 
            this.commandListBox.FormattingEnabled = true;
            this.commandListBox.ItemHeight = 12;
            this.commandListBox.Items.AddRange(new object[] {
            "tap power ",
            "tap auto ",
            "tap cool ",
            "tap Dry ",
            "tap Fan",
            "tap Heat ",
            "drag 調溫bar up ",
            "drag 調溫bar down ",
            "tap 風量 hi",
            "tap 風量 mid",
            "tap 風量 low",
            "tap 風量 auto",
            "Home page up",
            "Home page down",
            "tap airconditioner",
            "tap multimedia"});
            this.commandListBox.Location = new System.Drawing.Point(13, 130);
            this.commandListBox.Name = "commandListBox";
            this.commandListBox.Size = new System.Drawing.Size(120, 88);
            this.commandListBox.TabIndex = 0;
            // 
            // execButton
            // 
            this.execButton.Location = new System.Drawing.Point(28, 292);
            this.execButton.Name = "execButton";
            this.execButton.Size = new System.Drawing.Size(75, 23);
            this.execButton.TabIndex = 1;
            this.execButton.Text = "Execute";
            this.execButton.UseVisualStyleBackColor = true;
            this.execButton.Click += new System.EventHandler(this.execButton_Click);
            // 
            // portComboBox
            // 
            this.portComboBox.FormattingEnabled = true;
            this.portComboBox.Location = new System.Drawing.Point(11, 33);
            this.portComboBox.Name = "portComboBox";
            this.portComboBox.Size = new System.Drawing.Size(121, 20);
            this.portComboBox.TabIndex = 2;
            // 
            // portStateButton
            // 
            this.portStateButton.Location = new System.Drawing.Point(28, 70);
            this.portStateButton.Name = "portStateButton";
            this.portStateButton.Size = new System.Drawing.Size(75, 23);
            this.portStateButton.TabIndex = 3;
            this.portStateButton.Text = "Connect";
            this.portStateButton.UseVisualStyleBackColor = true;
            this.portStateButton.Click += new System.EventHandler(this.portStateButton_Click);
            // 
            // recvTextBox
            // 
            this.recvTextBox.Location = new System.Drawing.Point(139, 10);
            this.recvTextBox.MaxLength = 2147483647;
            this.recvTextBox.Multiline = true;
            this.recvTextBox.Name = "recvTextBox";
            this.recvTextBox.ReadOnly = true;
            this.recvTextBox.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.recvTextBox.Size = new System.Drawing.Size(345, 344);
            this.recvTextBox.TabIndex = 4;
            // 
            // paramListBox
            // 
            this.paramListBox.FormattingEnabled = true;
            this.paramListBox.ItemHeight = 12;
            this.paramListBox.Items.AddRange(new object[] {
            "Off",
            "On"});
            this.paramListBox.Location = new System.Drawing.Point(11, 246);
            this.paramListBox.Name = "paramListBox";
            this.paramListBox.Size = new System.Drawing.Size(120, 28);
            this.paramListBox.TabIndex = 5;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(13, 13);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(57, 12);
            this.label1.TabIndex = 6;
            this.label1.Text = "COM Ports";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(13, 115);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(58, 12);
            this.label2.TabIndex = 7;
            this.label2.Text = "Commands";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(13, 231);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(55, 12);
            this.label3.TabIndex = 8;
            this.label3.Text = "Parameters";
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(496, 366);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.paramListBox);
            this.Controls.Add(this.recvTextBox);
            this.Controls.Add(this.portStateButton);
            this.Controls.Add(this.portComboBox);
            this.Controls.Add(this.execButton);
            this.Controls.Add(this.commandListBox);
            this.Name = "MainForm";
            this.Text = "HMI MCU";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ListBox commandListBox;
        private System.Windows.Forms.Button execButton;
        private System.Windows.Forms.ComboBox portComboBox;
        private System.Windows.Forms.Button portStateButton;
        private System.Windows.Forms.TextBox recvTextBox;
        private System.Windows.Forms.ListBox paramListBox;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
    }
}

