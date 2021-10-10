namespace GUIDesigner
{
    partial class MainForm
    {
        /// <summary>
        /// 設計工具所需的變數。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 清除任何使用中的資源。
        /// </summary>
        /// <param name="disposing">如果應該處置 Managed 資源則為 true，否則為 false。</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form 設計工具產生的程式碼

        /// <summary>
        /// 此為設計工具支援所需的方法 - 請勿使用程式碼編輯器
        /// 修改這個方法的內容。
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.mainMenuStrip = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.newToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.templateToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveAsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.exportToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.exportImagesToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.createFunctionTableToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.createLayerCodeToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.createLayerCMakeToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.editToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.undoToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.redoToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
            this.cutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.copyToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.pasteToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.deleteToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator3 = new System.Windows.Forms.ToolStripSeparator();
            this.findToolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator5 = new System.Windows.Forms.ToolStripSeparator();
            this.importSpriteImagesToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.optionToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.compressionToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.jpegCompressionToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.jPEGQualityToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.bigEndianToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.projectDirectoryToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.duplicateNamesToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
            this.newLayerToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.removeLayerToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.enableExternalToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.disableExternalToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.emulatorToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.runToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.fontPathToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.testDataToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.aboutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.statusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.toolStripStatusLabel1 = new System.Windows.Forms.ToolStripStatusLabel();
            this.designerPropertyGrid = new System.Windows.Forms.PropertyGrid();
            this.splitContainer2 = new System.Windows.Forms.SplitContainer();
            this.toolTabControl = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.toolboxListBox = new System.Windows.Forms.ListBox();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.layerListBox = new System.Windows.Forms.ListBox();
            this.tabPage3 = new System.Windows.Forms.TabPage();
            this.widgetListBox = new System.Windows.Forms.ListBox();
            this.treeTabPage = new System.Windows.Forms.TabPage();
            this.widgetTreeView = new System.Windows.Forms.TreeView();
            this.widgetTreeContextMenuStrip = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.showToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.hideToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator4 = new System.Windows.Forms.ToolStripSeparator();
            this.moveUpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.moveDownToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.moveTopToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.moveBottomToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.splitContainer3 = new System.Windows.Forms.SplitContainer();
            this.layerTabControl = new GUIDesigner.DraggableTabControl();
            this.toolStrip1 = new System.Windows.Forms.ToolStrip();
            this.alignTopToolStripButton = new System.Windows.Forms.ToolStripButton();
            this.alignBottomToolStripButton = new System.Windows.Forms.ToolStripButton();
            this.alignLeftToolStripButton = new System.Windows.Forms.ToolStripButton();
            this.alignRightToolStripButton = new System.Windows.Forms.ToolStripButton();
            this.alignHorizontalCentersToolStripButton = new System.Windows.Forms.ToolStripButton();
            this.alignVerticalCentersToolStripButton = new System.Windows.Forms.ToolStripButton();
            this.centerHorizontallyToolStripButton = new System.Windows.Forms.ToolStripButton();
            this.centerVerticallyToolStripButton = new System.Windows.Forms.ToolStripButton();
            this.buildToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.buildToolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
            this.mainMenuStrip.SuspendLayout();
            this.statusStrip1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer2)).BeginInit();
            this.splitContainer2.Panel1.SuspendLayout();
            this.splitContainer2.Panel2.SuspendLayout();
            this.splitContainer2.SuspendLayout();
            this.toolTabControl.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.tabPage2.SuspendLayout();
            this.tabPage3.SuspendLayout();
            this.treeTabPage.SuspendLayout();
            this.widgetTreeContextMenuStrip.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer3)).BeginInit();
            this.splitContainer3.Panel1.SuspendLayout();
            this.splitContainer3.Panel2.SuspendLayout();
            this.splitContainer3.SuspendLayout();
            this.toolStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // mainMenuStrip
            // 
            this.mainMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.editToolStripMenuItem,
            this.optionToolStripMenuItem,
            this.toolStripMenuItem1,
            this.emulatorToolStripMenuItem,
            this.buildToolStripMenuItem,
            this.helpToolStripMenuItem});
            this.mainMenuStrip.Location = new System.Drawing.Point(0, 0);
            this.mainMenuStrip.Name = "mainMenuStrip";
            this.mainMenuStrip.Size = new System.Drawing.Size(1284, 24);
            this.mainMenuStrip.TabIndex = 2;
            this.mainMenuStrip.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.newToolStripMenuItem,
            this.templateToolStripMenuItem,
            this.openToolStripMenuItem,
            this.saveToolStripMenuItem,
            this.saveAsToolStripMenuItem,
            this.exportToolStripMenuItem,
            this.exportImagesToolStripMenuItem,
            this.createFunctionTableToolStripMenuItem,
            this.createLayerCodeToolStripMenuItem,
            this.createLayerCMakeToolStripMenuItem,
            this.exitToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(34, 20);
            this.fileToolStripMenuItem.Text = "&File";
            // 
            // newToolStripMenuItem
            // 
            this.newToolStripMenuItem.Name = "newToolStripMenuItem";
            this.newToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.N)));
            this.newToolStripMenuItem.Size = new System.Drawing.Size(182, 22);
            this.newToolStripMenuItem.Text = "&New";
            this.newToolStripMenuItem.Click += new System.EventHandler(this.newToolStripMenuItem_Click);
            // 
            // templateToolStripMenuItem
            // 
            this.templateToolStripMenuItem.Name = "templateToolStripMenuItem";
            this.templateToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.T)));
            this.templateToolStripMenuItem.Size = new System.Drawing.Size(182, 22);
            this.templateToolStripMenuItem.Text = "New &Template";
            this.templateToolStripMenuItem.Click += new System.EventHandler(this.templateToolStripMenuItem_Click);
            // 
            // openToolStripMenuItem
            // 
            this.openToolStripMenuItem.Name = "openToolStripMenuItem";
            this.openToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.O)));
            this.openToolStripMenuItem.Size = new System.Drawing.Size(182, 22);
            this.openToolStripMenuItem.Text = "&Open...";
            this.openToolStripMenuItem.Click += new System.EventHandler(this.openToolStripMenuItem_Click);
            // 
            // saveToolStripMenuItem
            // 
            this.saveToolStripMenuItem.Name = "saveToolStripMenuItem";
            this.saveToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.S)));
            this.saveToolStripMenuItem.Size = new System.Drawing.Size(182, 22);
            this.saveToolStripMenuItem.Text = "&Save";
            this.saveToolStripMenuItem.Click += new System.EventHandler(this.saveToolStripMenuItem_Click);
            // 
            // saveAsToolStripMenuItem
            // 
            this.saveAsToolStripMenuItem.Name = "saveAsToolStripMenuItem";
            this.saveAsToolStripMenuItem.Size = new System.Drawing.Size(182, 22);
            this.saveAsToolStripMenuItem.Text = "Save &As...";
            this.saveAsToolStripMenuItem.Click += new System.EventHandler(this.saveAsToolStripMenuItem_Click);
            // 
            // exportToolStripMenuItem
            // 
            this.exportToolStripMenuItem.Name = "exportToolStripMenuItem";
            this.exportToolStripMenuItem.Size = new System.Drawing.Size(182, 22);
            this.exportToolStripMenuItem.Text = "&Export...";
            this.exportToolStripMenuItem.Click += new System.EventHandler(this.exportToolStripMenuItem_Click);
            // 
            // exportImagesToolStripMenuItem
            // 
            this.exportImagesToolStripMenuItem.Name = "exportImagesToolStripMenuItem";
            this.exportImagesToolStripMenuItem.Size = new System.Drawing.Size(182, 22);
            this.exportImagesToolStripMenuItem.Text = "Export &Images...";
            this.exportImagesToolStripMenuItem.Click += new System.EventHandler(this.exportImagesToolStripMenuItem_Click);
            // 
            // createFunctionTableToolStripMenuItem
            // 
            this.createFunctionTableToolStripMenuItem.Name = "createFunctionTableToolStripMenuItem";
            this.createFunctionTableToolStripMenuItem.Size = new System.Drawing.Size(182, 22);
            this.createFunctionTableToolStripMenuItem.Text = "Create Function Table...";
            this.createFunctionTableToolStripMenuItem.Click += new System.EventHandler(this.createFunctionTableToolStripMenuItem_Click);
            // 
            // createLayerCodeToolStripMenuItem
            // 
            this.createLayerCodeToolStripMenuItem.Name = "createLayerCodeToolStripMenuItem";
            this.createLayerCodeToolStripMenuItem.Size = new System.Drawing.Size(182, 22);
            this.createLayerCodeToolStripMenuItem.Text = "Create Layer Code...";
            this.createLayerCodeToolStripMenuItem.Click += new System.EventHandler(this.createLayerCodeToolStripMenuItem_Click);
            // 
            // createLayerCMakeToolStripMenuItem
            // 
            this.createLayerCMakeToolStripMenuItem.Name = "createLayerCMakeToolStripMenuItem";
            this.createLayerCMakeToolStripMenuItem.Size = new System.Drawing.Size(182, 22);
            this.createLayerCMakeToolStripMenuItem.Text = "Create Layer CMake...";
            this.createLayerCMakeToolStripMenuItem.Click += new System.EventHandler(this.createLayerCMakeToolStripMenuItem_Click);
            // 
            // exitToolStripMenuItem
            // 
            this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
            this.exitToolStripMenuItem.Size = new System.Drawing.Size(182, 22);
            this.exitToolStripMenuItem.Text = "E&xit";
            this.exitToolStripMenuItem.Click += new System.EventHandler(this.exitToolStripMenuItem_Click);
            // 
            // editToolStripMenuItem
            // 
            this.editToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.undoToolStripMenuItem,
            this.redoToolStripMenuItem,
            this.toolStripSeparator2,
            this.cutToolStripMenuItem,
            this.copyToolStripMenuItem,
            this.pasteToolStripMenuItem,
            this.deleteToolStripMenuItem,
            this.toolStripSeparator3,
            this.findToolStripMenuItem1,
            this.toolStripSeparator5,
            this.importSpriteImagesToolStripMenuItem});
            this.editToolStripMenuItem.Name = "editToolStripMenuItem";
            this.editToolStripMenuItem.Size = new System.Drawing.Size(36, 20);
            this.editToolStripMenuItem.Text = "&Edit";
            // 
            // undoToolStripMenuItem
            // 
            this.undoToolStripMenuItem.Name = "undoToolStripMenuItem";
            this.undoToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.Z)));
            this.undoToolStripMenuItem.Size = new System.Drawing.Size(177, 22);
            this.undoToolStripMenuItem.Text = "&Undo";
            this.undoToolStripMenuItem.Click += new System.EventHandler(this.undoToolStripMenuItem_Click);
            // 
            // redoToolStripMenuItem
            // 
            this.redoToolStripMenuItem.Name = "redoToolStripMenuItem";
            this.redoToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.Y)));
            this.redoToolStripMenuItem.Size = new System.Drawing.Size(177, 22);
            this.redoToolStripMenuItem.Text = "&Redo";
            this.redoToolStripMenuItem.Click += new System.EventHandler(this.redoToolStripMenuItem_Click);
            // 
            // toolStripSeparator2
            // 
            this.toolStripSeparator2.Name = "toolStripSeparator2";
            this.toolStripSeparator2.Size = new System.Drawing.Size(174, 6);
            // 
            // cutToolStripMenuItem
            // 
            this.cutToolStripMenuItem.Name = "cutToolStripMenuItem";
            this.cutToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.X)));
            this.cutToolStripMenuItem.Size = new System.Drawing.Size(177, 22);
            this.cutToolStripMenuItem.Text = "Cu&t";
            this.cutToolStripMenuItem.Click += new System.EventHandler(this.cutToolStripMenuItem_Click);
            // 
            // copyToolStripMenuItem
            // 
            this.copyToolStripMenuItem.Name = "copyToolStripMenuItem";
            this.copyToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.C)));
            this.copyToolStripMenuItem.Size = new System.Drawing.Size(177, 22);
            this.copyToolStripMenuItem.Text = "&Copy";
            this.copyToolStripMenuItem.Click += new System.EventHandler(this.copyToolStripMenuItem_Click);
            // 
            // pasteToolStripMenuItem
            // 
            this.pasteToolStripMenuItem.Name = "pasteToolStripMenuItem";
            this.pasteToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.V)));
            this.pasteToolStripMenuItem.Size = new System.Drawing.Size(177, 22);
            this.pasteToolStripMenuItem.Text = "&Paste";
            this.pasteToolStripMenuItem.Click += new System.EventHandler(this.pasteToolStripMenuItem_Click);
            // 
            // deleteToolStripMenuItem
            // 
            this.deleteToolStripMenuItem.Name = "deleteToolStripMenuItem";
            this.deleteToolStripMenuItem.Size = new System.Drawing.Size(177, 22);
            this.deleteToolStripMenuItem.Text = "&Delete";
            this.deleteToolStripMenuItem.Click += new System.EventHandler(this.deleteToolStripMenuItem_Click);
            // 
            // toolStripSeparator3
            // 
            this.toolStripSeparator3.Name = "toolStripSeparator3";
            this.toolStripSeparator3.Size = new System.Drawing.Size(174, 6);
            // 
            // findToolStripMenuItem1
            // 
            this.findToolStripMenuItem1.Name = "findToolStripMenuItem1";
            this.findToolStripMenuItem1.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.F)));
            this.findToolStripMenuItem1.Size = new System.Drawing.Size(177, 22);
            this.findToolStripMenuItem1.Text = "&Find...";
            this.findToolStripMenuItem1.Click += new System.EventHandler(this.findToolStripMenuItem1_Click);
            // 
            // toolStripSeparator5
            // 
            this.toolStripSeparator5.Name = "toolStripSeparator5";
            this.toolStripSeparator5.Size = new System.Drawing.Size(174, 6);
            // 
            // importSpriteImagesToolStripMenuItem
            // 
            this.importSpriteImagesToolStripMenuItem.Name = "importSpriteImagesToolStripMenuItem";
            this.importSpriteImagesToolStripMenuItem.Size = new System.Drawing.Size(177, 22);
            this.importSpriteImagesToolStripMenuItem.Text = "Import Sprite Images...";
            this.importSpriteImagesToolStripMenuItem.Click += new System.EventHandler(this.importSpriteImagesToolStripMenuItem_Click);
            // 
            // optionToolStripMenuItem
            // 
            this.optionToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.compressionToolStripMenuItem,
            this.jpegCompressionToolStripMenuItem,
            this.jPEGQualityToolStripMenuItem,
            this.bigEndianToolStripMenuItem,
            this.projectDirectoryToolStripMenuItem,
            this.duplicateNamesToolStripMenuItem});
            this.optionToolStripMenuItem.Name = "optionToolStripMenuItem";
            this.optionToolStripMenuItem.Size = new System.Drawing.Size(49, 20);
            this.optionToolStripMenuItem.Text = "Option";
            // 
            // compressionToolStripMenuItem
            // 
            this.compressionToolStripMenuItem.Checked = true;
            this.compressionToolStripMenuItem.CheckOnClick = true;
            this.compressionToolStripMenuItem.CheckState = System.Windows.Forms.CheckState.Checked;
            this.compressionToolStripMenuItem.Name = "compressionToolStripMenuItem";
            this.compressionToolStripMenuItem.Size = new System.Drawing.Size(159, 22);
            this.compressionToolStripMenuItem.Text = "Compression";
            // 
            // jpegCompressionToolStripMenuItem
            // 
            this.jpegCompressionToolStripMenuItem.CheckOnClick = true;
            this.jpegCompressionToolStripMenuItem.Name = "jpegCompressionToolStripMenuItem";
            this.jpegCompressionToolStripMenuItem.Size = new System.Drawing.Size(159, 22);
            this.jpegCompressionToolStripMenuItem.Text = "JPEG Compression";
            // 
            // jPEGQualityToolStripMenuItem
            // 
            this.jPEGQualityToolStripMenuItem.Name = "jPEGQualityToolStripMenuItem";
            this.jPEGQualityToolStripMenuItem.Size = new System.Drawing.Size(159, 22);
            this.jPEGQualityToolStripMenuItem.Text = "JPEG Quality...";
            this.jPEGQualityToolStripMenuItem.Click += new System.EventHandler(this.jPEGQualityToolStripMenuItem_Click);
            // 
            // bigEndianToolStripMenuItem
            // 
            this.bigEndianToolStripMenuItem.CheckOnClick = true;
            this.bigEndianToolStripMenuItem.Name = "bigEndianToolStripMenuItem";
            this.bigEndianToolStripMenuItem.Size = new System.Drawing.Size(159, 22);
            this.bigEndianToolStripMenuItem.Text = "Big-Endian";
            // 
            // projectDirectoryToolStripMenuItem
            // 
            this.projectDirectoryToolStripMenuItem.Name = "projectDirectoryToolStripMenuItem";
            this.projectDirectoryToolStripMenuItem.Size = new System.Drawing.Size(159, 22);
            this.projectDirectoryToolStripMenuItem.Text = "Project Directory...";
            this.projectDirectoryToolStripMenuItem.Click += new System.EventHandler(this.projectDirectoryToolStripMenuItem_Click);
            // 
            // duplicateNamesToolStripMenuItem
            // 
            this.duplicateNamesToolStripMenuItem.Name = "duplicateNamesToolStripMenuItem";
            this.duplicateNamesToolStripMenuItem.Size = new System.Drawing.Size(159, 22);
            this.duplicateNamesToolStripMenuItem.Text = "Duplicate Names...";
            this.duplicateNamesToolStripMenuItem.Click += new System.EventHandler(this.duplicateNamesToolStripMenuItem_Click);
            // 
            // toolStripMenuItem1
            // 
            this.toolStripMenuItem1.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.newLayerToolStripMenuItem,
            this.removeLayerToolStripMenuItem,
            this.enableExternalToolStripMenuItem,
            this.disableExternalToolStripMenuItem});
            this.toolStripMenuItem1.Name = "toolStripMenuItem1";
            this.toolStripMenuItem1.Size = new System.Drawing.Size(44, 20);
            this.toolStripMenuItem1.Text = "Layer";
            // 
            // newLayerToolStripMenuItem
            // 
            this.newLayerToolStripMenuItem.Name = "newLayerToolStripMenuItem";
            this.newLayerToolStripMenuItem.Size = new System.Drawing.Size(146, 22);
            this.newLayerToolStripMenuItem.Text = "New";
            this.newLayerToolStripMenuItem.Click += new System.EventHandler(this.newLayerToolStripMenuItem_Click);
            // 
            // removeLayerToolStripMenuItem
            // 
            this.removeLayerToolStripMenuItem.Name = "removeLayerToolStripMenuItem";
            this.removeLayerToolStripMenuItem.Size = new System.Drawing.Size(146, 22);
            this.removeLayerToolStripMenuItem.Text = "Remove";
            this.removeLayerToolStripMenuItem.Click += new System.EventHandler(this.removeLayerToolStripMenuItem_Click);
            // 
            // enableExternalToolStripMenuItem
            // 
            this.enableExternalToolStripMenuItem.Name = "enableExternalToolStripMenuItem";
            this.enableExternalToolStripMenuItem.Size = new System.Drawing.Size(146, 22);
            this.enableExternalToolStripMenuItem.Text = "Enable External";
            this.enableExternalToolStripMenuItem.Click += new System.EventHandler(this.enableExternalToolStripMenuItem_Click);
            // 
            // disableExternalToolStripMenuItem
            // 
            this.disableExternalToolStripMenuItem.Name = "disableExternalToolStripMenuItem";
            this.disableExternalToolStripMenuItem.Size = new System.Drawing.Size(146, 22);
            this.disableExternalToolStripMenuItem.Text = "Disable External";
            this.disableExternalToolStripMenuItem.Click += new System.EventHandler(this.disableExternalToolStripMenuItem_Click);
            // 
            // emulatorToolStripMenuItem
            // 
            this.emulatorToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.runToolStripMenuItem,
            this.toolStripSeparator1,
            this.fontPathToolStripMenuItem,
            this.testDataToolStripMenuItem});
            this.emulatorToolStripMenuItem.Name = "emulatorToolStripMenuItem";
            this.emulatorToolStripMenuItem.Size = new System.Drawing.Size(60, 20);
            this.emulatorToolStripMenuItem.Text = "Emulator";
            // 
            // runToolStripMenuItem
            // 
            this.runToolStripMenuItem.Name = "runToolStripMenuItem";
            this.runToolStripMenuItem.ShortcutKeys = System.Windows.Forms.Keys.F5;
            this.runToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.runToolStripMenuItem.Text = "Run";
            this.runToolStripMenuItem.Click += new System.EventHandler(this.runToolStripMenuItem_Click);
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(149, 6);
            // 
            // fontPathToolStripMenuItem
            // 
            this.fontPathToolStripMenuItem.Name = "fontPathToolStripMenuItem";
            this.fontPathToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.fontPathToolStripMenuItem.Text = "Font Path...";
            this.fontPathToolStripMenuItem.Click += new System.EventHandler(this.fontPathToolStripMenuItem_Click);
            // 
            // testDataToolStripMenuItem
            // 
            this.testDataToolStripMenuItem.CheckOnClick = true;
            this.testDataToolStripMenuItem.Name = "testDataToolStripMenuItem";
            this.testDataToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.testDataToolStripMenuItem.Text = "Test Data";
            // 
            // helpToolStripMenuItem
            // 
            this.helpToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.aboutToolStripMenuItem});
            this.helpToolStripMenuItem.Name = "helpToolStripMenuItem";
            this.helpToolStripMenuItem.Size = new System.Drawing.Size(39, 20);
            this.helpToolStripMenuItem.Text = "&Help";
            // 
            // aboutToolStripMenuItem
            // 
            this.aboutToolStripMenuItem.Name = "aboutToolStripMenuItem";
            this.aboutToolStripMenuItem.Size = new System.Drawing.Size(99, 22);
            this.aboutToolStripMenuItem.Text = "&About";
            this.aboutToolStripMenuItem.Click += new System.EventHandler(this.aboutToolStripMenuItem_Click);
            // 
            // statusStrip1
            // 
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.statusLabel,
            this.toolStripStatusLabel1});
            this.statusStrip1.Location = new System.Drawing.Point(0, 712);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Size = new System.Drawing.Size(1284, 22);
            this.statusStrip1.TabIndex = 3;
            this.statusStrip1.Text = "statusStrip1";
            // 
            // statusLabel
            // 
            this.statusLabel.Name = "statusLabel";
            this.statusLabel.Size = new System.Drawing.Size(0, 17);
            // 
            // toolStripStatusLabel1
            // 
            this.toolStripStatusLabel1.Name = "toolStripStatusLabel1";
            this.toolStripStatusLabel1.Size = new System.Drawing.Size(0, 17);
            // 
            // designerPropertyGrid
            // 
            this.designerPropertyGrid.Dock = System.Windows.Forms.DockStyle.Fill;
            this.designerPropertyGrid.Font = new System.Drawing.Font("Calibri", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.designerPropertyGrid.Location = new System.Drawing.Point(0, 0);
            this.designerPropertyGrid.Name = "designerPropertyGrid";
            this.designerPropertyGrid.Size = new System.Drawing.Size(283, 668);
            this.designerPropertyGrid.TabIndex = 0;
            // 
            // splitContainer2
            // 
            this.splitContainer2.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.splitContainer2.Location = new System.Drawing.Point(0, 48);
            this.splitContainer2.Name = "splitContainer2";
            // 
            // splitContainer2.Panel1
            // 
            this.splitContainer2.Panel1.Controls.Add(this.toolTabControl);
            // 
            // splitContainer2.Panel2
            // 
            this.splitContainer2.Panel2.Controls.Add(this.splitContainer3);
            this.splitContainer2.Size = new System.Drawing.Size(1284, 668);
            this.splitContainer2.SplitterDistance = 156;
            this.splitContainer2.TabIndex = 6;
            // 
            // toolTabControl
            // 
            this.toolTabControl.Controls.Add(this.tabPage1);
            this.toolTabControl.Controls.Add(this.tabPage2);
            this.toolTabControl.Controls.Add(this.tabPage3);
            this.toolTabControl.Controls.Add(this.treeTabPage);
            this.toolTabControl.Dock = System.Windows.Forms.DockStyle.Fill;
            this.toolTabControl.Location = new System.Drawing.Point(0, 0);
            this.toolTabControl.Multiline = true;
            this.toolTabControl.Name = "toolTabControl";
            this.toolTabControl.SelectedIndex = 0;
            this.toolTabControl.Size = new System.Drawing.Size(156, 668);
            this.toolTabControl.TabIndex = 6;
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.toolboxListBox);
            this.tabPage1.Location = new System.Drawing.Point(4, 38);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage1.Size = new System.Drawing.Size(148, 626);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "Toolbox";
            this.tabPage1.UseVisualStyleBackColor = true;
            // 
            // toolboxListBox
            // 
            this.toolboxListBox.Dock = System.Windows.Forms.DockStyle.Fill;
            this.toolboxListBox.Font = new System.Drawing.Font("Calibri", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.toolboxListBox.FormattingEnabled = true;
            this.toolboxListBox.ItemHeight = 18;
            this.toolboxListBox.Location = new System.Drawing.Point(3, 3);
            this.toolboxListBox.Name = "toolboxListBox";
            this.toolboxListBox.Size = new System.Drawing.Size(142, 620);
            this.toolboxListBox.Sorted = true;
            this.toolboxListBox.TabIndex = 1;
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.layerListBox);
            this.tabPage2.Location = new System.Drawing.Point(4, 38);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage2.Size = new System.Drawing.Size(148, 626);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "Layers";
            this.tabPage2.UseVisualStyleBackColor = true;
            // 
            // layerListBox
            // 
            this.layerListBox.Dock = System.Windows.Forms.DockStyle.Fill;
            this.layerListBox.Font = new System.Drawing.Font("Calibri", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.layerListBox.FormattingEnabled = true;
            this.layerListBox.ItemHeight = 18;
            this.layerListBox.Location = new System.Drawing.Point(3, 3);
            this.layerListBox.Name = "layerListBox";
            this.layerListBox.Size = new System.Drawing.Size(142, 620);
            this.layerListBox.Sorted = true;
            this.layerListBox.TabIndex = 0;
            // 
            // tabPage3
            // 
            this.tabPage3.Controls.Add(this.widgetListBox);
            this.tabPage3.Location = new System.Drawing.Point(4, 38);
            this.tabPage3.Name = "tabPage3";
            this.tabPage3.Size = new System.Drawing.Size(148, 626);
            this.tabPage3.TabIndex = 2;
            this.tabPage3.Text = "Widgets";
            this.tabPage3.UseVisualStyleBackColor = true;
            // 
            // widgetListBox
            // 
            this.widgetListBox.Dock = System.Windows.Forms.DockStyle.Fill;
            this.widgetListBox.Font = new System.Drawing.Font("Calibri", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.widgetListBox.FormattingEnabled = true;
            this.widgetListBox.ItemHeight = 18;
            this.widgetListBox.Location = new System.Drawing.Point(0, 0);
            this.widgetListBox.Name = "widgetListBox";
            this.widgetListBox.Size = new System.Drawing.Size(148, 626);
            this.widgetListBox.Sorted = true;
            this.widgetListBox.TabIndex = 0;
            // 
            // treeTabPage
            // 
            this.treeTabPage.Controls.Add(this.widgetTreeView);
            this.treeTabPage.Location = new System.Drawing.Point(4, 38);
            this.treeTabPage.Name = "treeTabPage";
            this.treeTabPage.Padding = new System.Windows.Forms.Padding(3);
            this.treeTabPage.Size = new System.Drawing.Size(148, 626);
            this.treeTabPage.TabIndex = 3;
            this.treeTabPage.Text = "Tree";
            this.treeTabPage.UseVisualStyleBackColor = true;
            // 
            // widgetTreeView
            // 
            this.widgetTreeView.ContextMenuStrip = this.widgetTreeContextMenuStrip;
            this.widgetTreeView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.widgetTreeView.Font = new System.Drawing.Font("Calibri", 11.25F);
            this.widgetTreeView.Location = new System.Drawing.Point(3, 3);
            this.widgetTreeView.Name = "widgetTreeView";
            this.widgetTreeView.Size = new System.Drawing.Size(142, 620);
            this.widgetTreeView.TabIndex = 0;
            // 
            // widgetTreeContextMenuStrip
            // 
            this.widgetTreeContextMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.showToolStripMenuItem,
            this.hideToolStripMenuItem,
            this.toolStripSeparator4,
            this.moveUpToolStripMenuItem,
            this.moveDownToolStripMenuItem,
            this.moveTopToolStripMenuItem,
            this.moveBottomToolStripMenuItem});
            this.widgetTreeContextMenuStrip.Name = "contextMenuStrip1";
            this.widgetTreeContextMenuStrip.Size = new System.Drawing.Size(155, 142);
            // 
            // showToolStripMenuItem
            // 
            this.showToolStripMenuItem.Name = "showToolStripMenuItem";
            this.showToolStripMenuItem.Size = new System.Drawing.Size(154, 22);
            this.showToolStripMenuItem.Text = "Show";
            this.showToolStripMenuItem.Click += new System.EventHandler(this.showToolStripMenuItem_Click);
            // 
            // hideToolStripMenuItem
            // 
            this.hideToolStripMenuItem.Name = "hideToolStripMenuItem";
            this.hideToolStripMenuItem.Size = new System.Drawing.Size(154, 22);
            this.hideToolStripMenuItem.Text = "Hide";
            this.hideToolStripMenuItem.Click += new System.EventHandler(this.hideToolStripMenuItem_Click);
            // 
            // toolStripSeparator4
            // 
            this.toolStripSeparator4.Name = "toolStripSeparator4";
            this.toolStripSeparator4.Size = new System.Drawing.Size(151, 6);
            // 
            // moveUpToolStripMenuItem
            // 
            this.moveUpToolStripMenuItem.Name = "moveUpToolStripMenuItem";
            this.moveUpToolStripMenuItem.Size = new System.Drawing.Size(154, 22);
            this.moveUpToolStripMenuItem.Text = "Move Up";
            this.moveUpToolStripMenuItem.Click += new System.EventHandler(this.moveUpToolStripMenuItem_Click);
            // 
            // moveDownToolStripMenuItem
            // 
            this.moveDownToolStripMenuItem.Name = "moveDownToolStripMenuItem";
            this.moveDownToolStripMenuItem.Size = new System.Drawing.Size(154, 22);
            this.moveDownToolStripMenuItem.Text = "Move Down";
            this.moveDownToolStripMenuItem.Click += new System.EventHandler(this.moveDownToolStripMenuItem_Click);
            // 
            // moveTopToolStripMenuItem
            // 
            this.moveTopToolStripMenuItem.Name = "moveTopToolStripMenuItem";
            this.moveTopToolStripMenuItem.Size = new System.Drawing.Size(154, 22);
            this.moveTopToolStripMenuItem.Text = "Move Top";
            this.moveTopToolStripMenuItem.Click += new System.EventHandler(this.moveTopToolStripMenuItem_Click);
            // 
            // moveBottomToolStripMenuItem
            // 
            this.moveBottomToolStripMenuItem.Name = "moveBottomToolStripMenuItem";
            this.moveBottomToolStripMenuItem.Size = new System.Drawing.Size(154, 22);
            this.moveBottomToolStripMenuItem.Text = "Move Bottom";
            this.moveBottomToolStripMenuItem.Click += new System.EventHandler(this.moveBottomToolStripMenuItem_Click);
            // 
            // splitContainer3
            // 
            this.splitContainer3.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer3.Location = new System.Drawing.Point(0, 0);
            this.splitContainer3.Name = "splitContainer3";
            // 
            // splitContainer3.Panel1
            // 
            this.splitContainer3.Panel1.Controls.Add(this.layerTabControl);
            // 
            // splitContainer3.Panel2
            // 
            this.splitContainer3.Panel2.Controls.Add(this.designerPropertyGrid);
            this.splitContainer3.Size = new System.Drawing.Size(1124, 668);
            this.splitContainer3.SplitterDistance = 837;
            this.splitContainer3.TabIndex = 0;
            // 
            // layerTabControl
            // 
            this.layerTabControl.AllowDrop = true;
            this.layerTabControl.Dock = System.Windows.Forms.DockStyle.Fill;
            this.layerTabControl.Font = new System.Drawing.Font("Calibri", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.layerTabControl.Location = new System.Drawing.Point(0, 0);
            this.layerTabControl.Name = "layerTabControl";
            this.layerTabControl.SelectedIndex = 0;
            this.layerTabControl.Size = new System.Drawing.Size(837, 668);
            this.layerTabControl.TabIndex = 1;
            // 
            // toolStrip1
            // 
            this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.alignTopToolStripButton,
            this.alignBottomToolStripButton,
            this.alignLeftToolStripButton,
            this.alignRightToolStripButton,
            this.alignHorizontalCentersToolStripButton,
            this.alignVerticalCentersToolStripButton,
            this.centerHorizontallyToolStripButton,
            this.centerVerticallyToolStripButton});
            this.toolStrip1.Location = new System.Drawing.Point(0, 24);
            this.toolStrip1.Name = "toolStrip1";
            this.toolStrip1.Size = new System.Drawing.Size(1284, 25);
            this.toolStrip1.TabIndex = 7;
            this.toolStrip1.Text = "toolStrip1";
            // 
            // alignTopToolStripButton
            // 
            this.alignTopToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.alignTopToolStripButton.Image = ((System.Drawing.Image)(resources.GetObject("alignTopToolStripButton.Image")));
            this.alignTopToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.alignTopToolStripButton.Name = "alignTopToolStripButton";
            this.alignTopToolStripButton.Size = new System.Drawing.Size(23, 22);
            this.alignTopToolStripButton.Text = "AlignTop";
            this.alignTopToolStripButton.Click += new System.EventHandler(this.alignTopToolStripButton_Click);
            // 
            // alignBottomToolStripButton
            // 
            this.alignBottomToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.alignBottomToolStripButton.Image = ((System.Drawing.Image)(resources.GetObject("alignBottomToolStripButton.Image")));
            this.alignBottomToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.alignBottomToolStripButton.Name = "alignBottomToolStripButton";
            this.alignBottomToolStripButton.Size = new System.Drawing.Size(23, 22);
            this.alignBottomToolStripButton.Text = "AlignBottom";
            this.alignBottomToolStripButton.Click += new System.EventHandler(this.alignBottomToolStripButton_Click);
            // 
            // alignLeftToolStripButton
            // 
            this.alignLeftToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.alignLeftToolStripButton.Image = ((System.Drawing.Image)(resources.GetObject("alignLeftToolStripButton.Image")));
            this.alignLeftToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.alignLeftToolStripButton.Name = "alignLeftToolStripButton";
            this.alignLeftToolStripButton.Size = new System.Drawing.Size(23, 22);
            this.alignLeftToolStripButton.Text = "AlignLeft";
            this.alignLeftToolStripButton.Click += new System.EventHandler(this.alignLeftToolStripButton_Click);
            // 
            // alignRightToolStripButton
            // 
            this.alignRightToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.alignRightToolStripButton.Image = ((System.Drawing.Image)(resources.GetObject("alignRightToolStripButton.Image")));
            this.alignRightToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.alignRightToolStripButton.Name = "alignRightToolStripButton";
            this.alignRightToolStripButton.Size = new System.Drawing.Size(23, 22);
            this.alignRightToolStripButton.Text = "AlignRight";
            this.alignRightToolStripButton.Click += new System.EventHandler(this.alignRightToolStripButton_Click);
            // 
            // alignHorizontalCentersToolStripButton
            // 
            this.alignHorizontalCentersToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.alignHorizontalCentersToolStripButton.Image = ((System.Drawing.Image)(resources.GetObject("alignHorizontalCentersToolStripButton.Image")));
            this.alignHorizontalCentersToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.alignHorizontalCentersToolStripButton.Name = "alignHorizontalCentersToolStripButton";
            this.alignHorizontalCentersToolStripButton.Size = new System.Drawing.Size(23, 22);
            this.alignHorizontalCentersToolStripButton.Text = "AlignHorizontalCenters";
            this.alignHorizontalCentersToolStripButton.Click += new System.EventHandler(this.alignHorizontalCentersToolStripButton_Click);
            // 
            // alignVerticalCentersToolStripButton
            // 
            this.alignVerticalCentersToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.alignVerticalCentersToolStripButton.Image = ((System.Drawing.Image)(resources.GetObject("alignVerticalCentersToolStripButton.Image")));
            this.alignVerticalCentersToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.alignVerticalCentersToolStripButton.Name = "alignVerticalCentersToolStripButton";
            this.alignVerticalCentersToolStripButton.Size = new System.Drawing.Size(23, 22);
            this.alignVerticalCentersToolStripButton.Text = "AlignVerticalCenters";
            this.alignVerticalCentersToolStripButton.Click += new System.EventHandler(this.alignVerticalCentersToolStripButton_Click);
            // 
            // centerHorizontallyToolStripButton
            // 
            this.centerHorizontallyToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.centerHorizontallyToolStripButton.Image = ((System.Drawing.Image)(resources.GetObject("centerHorizontallyToolStripButton.Image")));
            this.centerHorizontallyToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.centerHorizontallyToolStripButton.Name = "centerHorizontallyToolStripButton";
            this.centerHorizontallyToolStripButton.Size = new System.Drawing.Size(23, 22);
            this.centerHorizontallyToolStripButton.Text = "CenterHorizontally";
            this.centerHorizontallyToolStripButton.Click += new System.EventHandler(this.centerHorizontallyToolStripButton_Click);
            // 
            // centerVerticallyToolStripButton
            // 
            this.centerVerticallyToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.centerVerticallyToolStripButton.Image = ((System.Drawing.Image)(resources.GetObject("centerVerticallyToolStripButton.Image")));
            this.centerVerticallyToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.centerVerticallyToolStripButton.Name = "centerVerticallyToolStripButton";
            this.centerVerticallyToolStripButton.Size = new System.Drawing.Size(23, 22);
            this.centerVerticallyToolStripButton.Text = "CenterVertically";
            this.centerVerticallyToolStripButton.Click += new System.EventHandler(this.centerVerticallyToolStripButton_Click);
            // 
            // buildToolStripMenuItem
            // 
            this.buildToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.buildToolStripMenuItem1});
            this.buildToolStripMenuItem.Name = "buildToolStripMenuItem";
            this.buildToolStripMenuItem.Size = new System.Drawing.Size(43, 20);
            this.buildToolStripMenuItem.Text = "Build";
            // 
            // buildToolStripMenuItem1
            // 
            this.buildToolStripMenuItem1.Name = "buildToolStripMenuItem1";
            this.buildToolStripMenuItem1.Size = new System.Drawing.Size(152, 22);
            this.buildToolStripMenuItem1.Text = "Build...";
            this.buildToolStripMenuItem1.Click += new System.EventHandler(this.buildToolStripMenuItem1_Click);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1284, 734);
            this.Controls.Add(this.toolStrip1);
            this.Controls.Add(this.splitContainer2);
            this.Controls.Add(this.mainMenuStrip);
            this.Controls.Add(this.statusStrip1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MainMenuStrip = this.mainMenuStrip;
            this.Name = "MainForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "GUI Designer";
            this.WindowState = System.Windows.Forms.FormWindowState.Minimized;
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainForm_FormClosing);
            this.Shown += new System.EventHandler(this.MainForm_Shown);
            this.mainMenuStrip.ResumeLayout(false);
            this.mainMenuStrip.PerformLayout();
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            this.splitContainer2.Panel1.ResumeLayout(false);
            this.splitContainer2.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer2)).EndInit();
            this.splitContainer2.ResumeLayout(false);
            this.toolTabControl.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.tabPage2.ResumeLayout(false);
            this.tabPage3.ResumeLayout(false);
            this.treeTabPage.ResumeLayout(false);
            this.widgetTreeContextMenuStrip.ResumeLayout(false);
            this.splitContainer3.Panel1.ResumeLayout(false);
            this.splitContainer3.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer3)).EndInit();
            this.splitContainer3.ResumeLayout(false);
            this.toolStrip1.ResumeLayout(false);
            this.toolStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        //Bless added
        private int max_regen_item;
        private string itu_regen_ini;
        private System.Xml.XmlDocument itu_doc;
        private System.Xml.XmlNode itu_node;
        private System.Windows.Forms.MenuStrip mainMenuStrip;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem openToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem saveToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem saveAsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem helpToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem aboutToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem editToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem cutToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem copyToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem pasteToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem deleteToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem optionToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem compressionToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem emulatorToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem runToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
        private System.Windows.Forms.ToolStripMenuItem fontPathToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem newLayerToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem removeLayerToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem undoToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem redoToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
        private System.Windows.Forms.StatusStrip statusStrip1;
        private System.Windows.Forms.ToolStripStatusLabel statusLabel;
        private System.Windows.Forms.ToolStripMenuItem exportToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem testDataToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem newToolStripMenuItem;
        private System.Windows.Forms.PropertyGrid designerPropertyGrid;
        private System.Windows.Forms.SplitContainer splitContainer2;
        private System.Windows.Forms.TabControl toolTabControl;
        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.ListBox toolboxListBox;
        private System.Windows.Forms.TabPage tabPage2;
        private System.Windows.Forms.ListBox layerListBox;
        private System.Windows.Forms.TabPage tabPage3;
        private System.Windows.Forms.ListBox widgetListBox;
        private System.Windows.Forms.SplitContainer splitContainer3;
        private System.Windows.Forms.ToolStripMenuItem exportImagesToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem jpegCompressionToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem bigEndianToolStripMenuItem;
        private System.Windows.Forms.TabPage treeTabPage;
        private System.Windows.Forms.TreeView widgetTreeView;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator3;
        private System.Windows.Forms.ToolStripMenuItem importSpriteImagesToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem enableExternalToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem disableExternalToolStripMenuItem;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel1;
        private System.Windows.Forms.ToolStripMenuItem createFunctionTableToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem projectDirectoryToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem createLayerCodeToolStripMenuItem;
        private System.Windows.Forms.ToolStrip toolStrip1;
        private System.Windows.Forms.ToolStripButton alignBottomToolStripButton;
        private System.Windows.Forms.ToolStripButton alignTopToolStripButton;
        private System.Windows.Forms.ToolStripButton alignLeftToolStripButton;
        private System.Windows.Forms.ToolStripButton alignRightToolStripButton;
        private System.Windows.Forms.ToolStripButton alignHorizontalCentersToolStripButton;
        private System.Windows.Forms.ToolStripButton alignVerticalCentersToolStripButton;
        private System.Windows.Forms.ToolStripButton centerHorizontallyToolStripButton;
        private System.Windows.Forms.ToolStripButton centerVerticallyToolStripButton;
        private System.Windows.Forms.ContextMenuStrip widgetTreeContextMenuStrip;
        private System.Windows.Forms.ToolStripMenuItem showToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem hideToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator4;
        private System.Windows.Forms.ToolStripMenuItem moveUpToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem moveDownToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem findToolStripMenuItem1;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator5;
        private DraggableTabControl layerTabControl;
        private System.Windows.Forms.ToolStripMenuItem duplicateNamesToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem createLayerCMakeToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem moveTopToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem moveBottomToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem templateToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem jPEGQualityToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem buildToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem buildToolStripMenuItem1;

    }
}

