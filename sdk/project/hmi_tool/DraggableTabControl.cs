using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Windows.Forms;

namespace GUIDesigner
{
	/// <summary>
	/// Summary description for DraggableTabPage.
	/// </summary>
	public class DraggableTabControl : System.Windows.Forms.TabControl
	{
		/// <summary> 
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

        public ListBox layerListBox = null;
        public ListBox widgetListBox = null;
        public TreeView widgetTreeView = null;

		public DraggableTabControl()
		{
			// This call is required by the Windows.Forms Form Designer.
			InitializeComponent();
			AllowDrop = true;

			// TODO: Add any initialization after the InitForm call

		}

		/// <summary> 
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if(components != null)
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Component Designer generated code
		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
		}
		#endregion

        protected override void OnDragDrop(System.Windows.Forms.DragEventArgs e)
        {
            base.OnDragDrop(e);

            Point pt = new Point(e.X, e.Y);
            //We need client coordinates.
            pt = PointToClient(pt);

            //Get the tab we are hovering over.
            TabPage hover_tab = GetTabPageByTab(pt);

            //Make sure we are on a tab.
            if (hover_tab != null)
            {
                //Make sure there is a TabPage being dragged.
                if (e.Data.GetDataPresent(typeof(TabPage)))
                {
                    e.Effect = DragDropEffects.Move;
                    TabPage drag_tab = (TabPage)e.Data.GetData(typeof(TabPage));

                    int item_drag_index = FindIndex(drag_tab);
                    int drop_location_index = FindIndex(hover_tab);

                    HostSurface drag_surf = HostSurface.hostSurfaces[item_drag_index];

                    //Don't do anything if we are hovering over ourself.
                    if (item_drag_index != drop_location_index)
                    {
                        ArrayList pages = new ArrayList();
                        ArrayList surfaces = new ArrayList();

                        //Put all tab pages into an array.
                        for (int i = 0; i < TabPages.Count; i++)
                        {
                            //Except the one we are dragging.
                            if (i != item_drag_index)
                            {
                                pages.Add(TabPages[i]);
                                surfaces.Add(HostSurface.hostSurfaces[i]);
                            }
                        }

                        //Now put the one we are dragging it at the proper location.
                        pages.Insert(drop_location_index, drag_tab);
                        surfaces.Insert(drop_location_index, drag_surf);

                        //Make them all go away for a nanosec.
                        TabPages.Clear();
                        HostSurface.hostSurfaces.Clear();

                        //Add them all back in.
                        TabPages.AddRange((TabPage[])pages.ToArray(typeof(TabPage)));
                        foreach (HostSurface surf in surfaces)
                        {
                            HostSurface.hostSurfaces.Add(surf);
                        }

                        //Make sure the drag tab is selected.
                        SelectedTab = drag_tab;

                        ITU.modified = true;
                    }
                }
            }
            else
            {
                e.Effect = DragDropEffects.None;
            }        
         }

		protected override void OnDragOver(System.Windows.Forms.DragEventArgs e)
		{
            base.OnDragOver(e);

			Point pt = new Point(e.X, e.Y);
			//We need client coordinates.
			pt = PointToClient(pt);
			
			//Get the tab we are hovering over.
			TabPage hover_tab = GetTabPageByTab(pt);

			//Make sure we are on a tab.
			if(hover_tab != null)
			{
				//Make sure there is a TabPage being dragged.
				if(e.Data.GetDataPresent(typeof(TabPage)))
				{
					e.Effect = DragDropEffects.Move;
				}
			}
			else
			{
				e.Effect = DragDropEffects.None;
			}
 		}

        private void AddWidgetItem(Control container, ListBox.ObjectCollection items)
        {
            foreach (Control c in container.Controls)
            {
                if (c is IWidget)
                    items.Add(c.Name);

                AddWidgetItem(c, items);
            }
        }

        private void AddWidgetTree(Control container, TreeNodeCollection nodes, bool inverse)
        {
            foreach (Control c in container.Controls)
            {
                if (c is IWidget)
                {
                    TreeNode node;

                    if (inverse)
                        node = nodes.Insert(0, c.Name, c.Name);
                    else
                        node = nodes.Add(c.Name, c.Name);

                    node.Tag = c;
                    AddWidgetTree(c, node.Nodes, inverse);
                }
                else
                    AddWidgetTree(c, nodes, inverse);
            }
        }

		protected override void OnMouseDown(MouseEventArgs e)
		{
			base.OnMouseDown(e);

			Point pt = new Point(e.X, e.Y);
			TabPage tp = GetTabPageByTab(pt);
			
			if(tp != null)
			{
				DoDragDrop(tp, DragDropEffects.All);

                layerListBox.SelectedItem = tp.Text;
                widgetListBox.Items.Clear();
                AddWidgetItem(tp, widgetListBox.Items);

                widgetTreeView.Nodes.Clear();
                AddWidgetTree(tp, widgetTreeView.Nodes, true);
			}
		}

		/// <summary>
		/// Finds the TabPage whose tab is contains the given point.
		/// </summary>
		/// <param name="pt">The point (given in client coordinates) to look for a TabPage.</param>
		/// <returns>The TabPage whose tab is at the given point (null if there isn't one).</returns>
		private TabPage GetTabPageByTab(Point pt)
		{
			TabPage tp = null;

			for(int i = 0; i < TabPages.Count; i++)
			{
				if(GetTabRect(i).Contains(pt))
				{
					tp = TabPages[i];
					break;
				}
			}
			
			return tp;
		}

		/// <summary>
		/// Loops over all the TabPages to find the index of the given TabPage.
		/// </summary>
		/// <param name="page">The TabPage we want the index for.</param>
		/// <returns>The index of the given TabPage(-1 if it isn't found.)</returns>
		private int FindIndex(TabPage page)
		{
			for(int i = 0; i < TabPages.Count; i++)
			{
				if(TabPages[i] == page)
					return i;
			}

			return -1;
		}
	}
}
