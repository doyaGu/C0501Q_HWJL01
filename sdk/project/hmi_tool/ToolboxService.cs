using System;
using System.Collections.Generic;
using System.Drawing.Design;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace GUIDesigner
{
    class ToolboxService : IToolboxService
    {
        public ListBox toolboxListBox { get; set; }

        public void AddCreator(ToolboxItemCreatorCallback creator, string format, System.ComponentModel.Design.IDesignerHost host)
        {
            throw new NotImplementedException();
        }

        public void AddCreator(ToolboxItemCreatorCallback creator, string format)
        {
            throw new NotImplementedException();
        }

        public void AddLinkedToolboxItem(ToolboxItem toolboxItem, string category, System.ComponentModel.Design.IDesignerHost host)
        {
            throw new NotImplementedException();
        }

        public void AddLinkedToolboxItem(ToolboxItem toolboxItem, System.ComponentModel.Design.IDesignerHost host)
        {
            throw new NotImplementedException();
        }

        public void AddToolboxItem(ToolboxItem toolboxItem, string category)
        {
            throw new NotImplementedException();
        }

        public void AddToolboxItem(ToolboxItem toolboxItem)
        {
            throw new NotImplementedException();
        }

        public CategoryNameCollection CategoryNames
        {
            get { throw new NotImplementedException(); }
        }

        public ToolboxItem DeserializeToolboxItem(object serializedObject, System.ComponentModel.Design.IDesignerHost host)
        {
            return this.DeserializeToolboxItem(serializedObject);
        }

        public ToolboxItem DeserializeToolboxItem(object serializedObject)
        {
            return ((DataObject)serializedObject).GetData(typeof(ToolboxItem)) as ToolboxItem;
        }

        public ToolboxItem GetSelectedToolboxItem(System.ComponentModel.Design.IDesignerHost host)
        {
            return GetSelectedToolboxItem();
        }

        public ToolboxItem GetSelectedToolboxItem()
        {
            if (toolboxListBox.SelectedIndex <= 0)
                return null;
            else
                return toolboxListBox.SelectedItem as ToolboxItem;
        }

        public ToolboxItemCollection GetToolboxItems(string category, System.ComponentModel.Design.IDesignerHost host)
        {
            throw new NotImplementedException();
        }

        public ToolboxItemCollection GetToolboxItems(string category)
        {
            throw new NotImplementedException();
        }

        public ToolboxItemCollection GetToolboxItems(System.ComponentModel.Design.IDesignerHost host)
        {
            throw new NotImplementedException();
        }

        public ToolboxItemCollection GetToolboxItems()
        {
            throw new NotImplementedException();
        }

        public bool IsSupported(object serializedObject, System.Collections.ICollection filterAttributes)
        {
            return true;
        }

        public bool IsSupported(object serializedObject, System.ComponentModel.Design.IDesignerHost host)
        {
            return true;
        }

        public bool IsToolboxItem(object serializedObject, System.ComponentModel.Design.IDesignerHost host)
        {
            throw new NotImplementedException();
        }

        public bool IsToolboxItem(object serializedObject)
        {
            throw new NotImplementedException();
        }

        public void Refresh()
        {
            throw new NotImplementedException();
        }

        public void RemoveCreator(string format, System.ComponentModel.Design.IDesignerHost host)
        {
            throw new NotImplementedException();
        }

        public void RemoveCreator(string format)
        {
            throw new NotImplementedException();
        }

        public void RemoveToolboxItem(ToolboxItem toolboxItem, string category)
        {
            throw new NotImplementedException();
        }

        public void RemoveToolboxItem(ToolboxItem toolboxItem)
        {
            throw new NotImplementedException();
        }

        public string SelectedCategory
        {
            get
            {
                throw new NotImplementedException();
            }
            set
            {
                throw new NotImplementedException();
            }
        }

        public void SelectedToolboxItemUsed()
        {
            toolboxListBox.SelectedItem = null;
        }

        public object SerializeToolboxItem(ToolboxItem toolboxItem)
        {
            throw new NotImplementedException();
        }

        public bool SetCursor()
        {
            if (toolboxListBox.SelectedIndex <= 0)
                Cursor.Current = Cursors.Default;
            else
                Cursor.Current = Cursors.Cross;

            return true;
        }

        public void SetSelectedToolboxItem(ToolboxItem toolboxItem)
        {
            throw new NotImplementedException();
        }
    }
}
