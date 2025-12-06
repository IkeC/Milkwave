using MilkwaveRemote.Helper;

namespace MilkwaveRemote {
  partial class MilkwaveInfoForm {
    /// <summary>
    /// Required designer variable.
    /// </summary>
    private System.ComponentModel.IContainer components = null;

    private DarkModeCS dm;

    /// <summary>
    /// Clean up any resources being used.
    /// </summary>
    /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
    protected override void Dispose(bool disposing) {
      if (disposing && (components != null)) {
        components.Dispose();
      }
      base.Dispose(disposing);
    }

    #region Windows Form Designer generated code

    /// <summary>
    /// Required method for Designer support - do not modify
    /// the contents of this method with the code editor.
    /// </summary>
    private void InitializeComponent() {
      System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MilkwaveInfoForm));
      textBox = new RichTextBox();
      btnClose = new Button();
      SuspendLayout();
      // 
      // textBox
      // 
      textBox.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      textBox.Font = new Font("Segoe UI", 10.8F, FontStyle.Regular, GraphicsUnit.Point, 0);
      textBox.Location = new Point(10, 9);
      textBox.Margin = new Padding(3, 2, 3, 2);
      textBox.Name = "textBox";
      textBox.ReadOnly = true;
      textBox.Size = new Size(480, 292);
      textBox.TabIndex = 0;
      textBox.Text = "Hello";
      textBox.LinkClicked += textBox_LinkClicked;
      textBox.MouseClick += textBox_MouseClick;
      textBox.MouseLeave += textBox_MouseLeave;
      textBox.MouseMove += textBox_MouseMove;
      // 
      // btnClose
      // 
      btnClose.Anchor = AnchorStyles.Bottom | AnchorStyles.Right;
      btnClose.Location = new Point(408, 278);
      btnClose.Margin = new Padding(3, 2, 3, 2);
      btnClose.Name = "btnClose";
      btnClose.Size = new Size(82, 22);
      btnClose.TabIndex = 1;
      btnClose.Text = "Close";
      btnClose.UseVisualStyleBackColor = true;
      btnClose.Click += btnClose_Click;
      // 
      // MilkwaveInfoForm
      // 
      AutoScaleDimensions = new SizeF(7F, 15F);
      AutoScaleMode = AutoScaleMode.Font;
      ClientSize = new Size(500, 309);
      Controls.Add(btnClose);
      Controls.Add(textBox);
      Icon = (Icon)resources.GetObject("$this.Icon");
      Margin = new Padding(3, 2, 3, 2);
      MaximizeBox = false;
      MinimizeBox = false;
      Name = "MilkwaveInfoForm";
      StartPosition = FormStartPosition.CenterParent;
      Text = "Milkwave";
      Shown += MilkwaveInfoForm_Shown;
      ResumeLayout(false);
    }

    #endregion

    private RichTextBox textBox;
    private Button btnClose;
  }
}