//                       YaST2-GTK                                //
// YaST webpage - http://developer.novell.com/wiki/index.php/YaST //

/* YGtkExtEntry widget */
// check the header file for information about this widget

#include "ygtkfindentry.h"
#include <gtk/gtk.h>

G_DEFINE_TYPE (YGtkExtEntry, ygtk_ext_entry, GTK_TYPE_ENTRY)

static void ygtk_ext_entry_init (YGtkExtEntry *entry)
{
}

static void ygtk_ext_entry_destroy (GtkObject *object)
{
	GTK_OBJECT_CLASS (ygtk_ext_entry_parent_class)->destroy (object);

	YGtkExtEntry *entry = YGTK_EXT_ENTRY (object);
	ygtk_ext_entry_set_border_window_size (entry, YGTK_EXT_ENTRY_LEFT_WIN, 0);
	ygtk_ext_entry_set_border_window_size (entry, YGTK_EXT_ENTRY_RIGHT_WIN, 0);
}

static void ygtk_ext_entry_map (GtkWidget *widget)
{
	if (GTK_WIDGET_REALIZED (widget) && !GTK_WIDGET_MAPPED (widget)) {
		GTK_WIDGET_CLASS (ygtk_ext_entry_parent_class)->map (widget);

		YGtkExtEntry *entry = YGTK_EXT_ENTRY (widget);
		if (entry->left_window)
			gdk_window_show (entry->left_window);
		if (entry->right_window)
			gdk_window_show (entry->right_window);
	}
}

static void ygtk_ext_entry_unmap (GtkWidget *widget)
{
	if (GTK_WIDGET_MAPPED (widget)) {
		YGtkExtEntry *entry = YGTK_EXT_ENTRY (widget);
		if (entry->left_window)
			gdk_window_hide (entry->left_window);
		if (entry->right_window)
			gdk_window_hide (entry->right_window);

		GTK_WIDGET_CLASS (ygtk_ext_entry_parent_class)->unmap (widget);
	}
}

static void ygtk_ext_entry_sync_color (YGtkExtEntry *entry)
{
	/* We don't use gtk_style_set_background() since we want to reflect
	   gtk_widget_modify_base() color, if changed. */

	GtkWidget *widget = GTK_WIDGET (entry);
	GdkColor color = widget->style->base [GTK_STATE_NORMAL];
	gdk_rgb_find_color (gtk_widget_get_colormap (widget), &color);

	if (entry->left_window)
		gdk_window_set_background (entry->left_window, &color);
	if (entry->right_window)
		gdk_window_set_background (entry->right_window, &color);
}

static void ygtk_ext_entry_style_set (GtkWidget *widget, GtkStyle *prev_style)
{
	GTK_WIDGET_CLASS (ygtk_ext_entry_parent_class)->style_set (widget, prev_style);
	ygtk_ext_entry_sync_color (YGTK_EXT_ENTRY (widget));
}

GdkWindow *ygtk_ext_entry_get_window (YGtkExtEntry *entry,
                                      YGtkExtEntryWindowType type)
{
	switch (type) {
		case YGTK_EXT_ENTRY_WIDGET_WIN:
			return gtk_widget_get_parent_window (GTK_WIDGET (entry));
		case YGTK_EXT_ENTRY_TEXT_WIN:
			return GTK_ENTRY (entry)->text_area;
		case YGTK_EXT_ENTRY_LEFT_WIN:
			return entry->left_window;
		case YGTK_EXT_ENTRY_RIGHT_WIN:
			return entry->right_window;
	}
	return NULL;
}

void ygtk_ext_entry_set_border_window_size (YGtkExtEntry *entry,
                                            YGtkExtEntryWindowType type, gint size)
{
	GtkWidget *widget = GTK_WIDGET (entry);
	g_assert (type == YGTK_EXT_ENTRY_LEFT_WIN || type == YGTK_EXT_ENTRY_RIGHT_WIN);

	GdkWindow **window;
	if (type == YGTK_EXT_ENTRY_LEFT_WIN)
		window = &entry->left_window;
	else
		window = &entry->right_window;

	if (size) {
		if (!*window) {
			// must be realized to create a window
			GdkWindowAttr attributes;
			gint attributes_mask;
			attributes.window_type = GDK_WINDOW_CHILD;
			attributes.wclass = GDK_INPUT_OUTPUT;
			attributes.visual = gtk_widget_get_visual (widget);
			attributes.colormap = gtk_widget_get_colormap (widget);
			attributes.event_mask = gtk_widget_get_events (widget);
			attributes.event_mask |= GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK 
				| GDK_BUTTON_RELEASE_MASK | GDK_LEAVE_NOTIFY_MASK
				| GDK_ENTER_NOTIFY_MASK | GDK_POINTER_MOTION_MASK
				| GDK_POINTER_MOTION_HINT_MASK;
			attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
			// this will be tuned on size_allocate
			if (type == YGTK_EXT_ENTRY_LEFT_WIN)
				attributes.x = 0;
			else
				attributes.x = widget->allocation.width - size;
			attributes.y = widget->allocation.y;
			attributes.width = size;
			attributes.height = widget->allocation.height;

			*window = gdk_window_new (widget->window,
			                          &attributes, attributes_mask);
			gdk_window_set_user_data (*window, widget);
			// set background style
			ygtk_ext_entry_sync_color (entry);
	
			if (GTK_WIDGET_MAPPED (widget))
				gdk_window_show (*window);
			gtk_widget_queue_resize (widget);
		}
		else {
			gint width, height;
			gdk_drawable_get_size (GDK_DRAWABLE (*window), &width, &height);
			if (width != size) {
				gdk_window_resize (*window, size, height);
				gtk_widget_queue_resize (widget);
			}
		}
	}
	else {  // remove the window
		if (*window) {
			gdk_window_set_user_data (*window, NULL);
			gdk_window_destroy (*window);
			*window = NULL;
			gtk_widget_queue_resize (widget);
		}
    }
}

gint ygtk_ext_entry_get_border_window_size (YGtkExtEntry *entry,
                                            YGtkExtEntryWindowType type)
{
	g_assert (type == YGTK_EXT_ENTRY_LEFT_WIN || type == YGTK_EXT_ENTRY_RIGHT_WIN);
	GdkWindow *window = ygtk_ext_entry_get_window (entry, type);
	gint size = 0;
	if (window)
		gdk_drawable_get_size (GDK_DRAWABLE (window), &size, NULL);
	return size;
}

static void ygtk_ext_entry_size_request (GtkWidget *widget, GtkRequisition *req)
{
	GTK_WIDGET_CLASS (ygtk_ext_entry_parent_class)->size_request (widget, req);

	YGtkExtEntry *entry = YGTK_EXT_ENTRY (widget);
	req->width += ygtk_ext_entry_get_border_window_size (entry,
	                                                     YGTK_EXT_ENTRY_LEFT_WIN);
	req->width += ygtk_ext_entry_get_border_window_size (entry,
	                                                     YGTK_EXT_ENTRY_RIGHT_WIN);
}

static void ygtk_ext_entry_size_allocate (GtkWidget *widget,
                                          GtkAllocation *allocation)
{
	if (!GTK_WIDGET_REALIZED (widget))
		return;

	GTK_WIDGET_CLASS (ygtk_ext_entry_parent_class)->size_allocate
	                                                    (widget, allocation);
	YGtkExtEntry *entry = YGTK_EXT_ENTRY (widget);

	gint left_border, right_border;
	left_border = ygtk_ext_entry_get_border_window_size (entry,
	                                                     YGTK_EXT_ENTRY_LEFT_WIN);
	right_border = ygtk_ext_entry_get_border_window_size (entry,
	                                                      YGTK_EXT_ENTRY_RIGHT_WIN);

	GdkWindow *window;
	gint _x, _y, _w, _h, x, w;

	// text window
	window = ygtk_ext_entry_get_window (entry, YGTK_EXT_ENTRY_TEXT_WIN);
	gdk_window_get_geometry (window, &_x, &_y, &_w, &_h, NULL);
	if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_LTR)
		x = _x + left_border;
	else
		x = _x + right_border;
	w = _w - (left_border + right_border);
	gdk_window_move_resize (window, x, _y, w, _h);

	// left window
	window = ygtk_ext_entry_get_window (entry, YGTK_EXT_ENTRY_LEFT_WIN);
	if (window) {
		if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_LTR)
			x = _x;
		else
			x = _x + _w - left_border;
		w = left_border;
		gdk_window_move_resize (window, x, _y, w, _h);
	}

	// right window
	window = ygtk_ext_entry_get_window (entry, YGTK_EXT_ENTRY_RIGHT_WIN);
	if (window) {
		if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_LTR)
			x = _x + _w - right_border;
		else
			x = _x;
		w = right_border;
		gdk_window_move_resize (window, x, _y, w, _h);
	}
}

GtkWidget *ygtk_ext_entry_new()
{ return g_object_new (YGTK_TYPE_EXT_ENTRY, NULL); }

static void ygtk_ext_entry_class_init (YGtkExtEntryClass *klass)
{
	GtkWidgetClass *gtkwidget_class = GTK_WIDGET_CLASS (klass);
	gtkwidget_class->map = ygtk_ext_entry_map;
	gtkwidget_class->unmap = ygtk_ext_entry_unmap;
	gtkwidget_class->style_set = ygtk_ext_entry_style_set;
	gtkwidget_class->size_request = ygtk_ext_entry_size_request;
	gtkwidget_class->size_allocate = ygtk_ext_entry_size_allocate;

	GtkObjectClass *gtkobject_class = GTK_OBJECT_CLASS (klass);
	gtkobject_class->destroy = ygtk_ext_entry_destroy;
}

/* YGtkFindEntry widget */
// check the header file for information about this widget

#include <string.h>

static void ygtk_find_entry_editable_init (GtkEditableClass *iface);

G_DEFINE_TYPE_WITH_CODE (YGtkFindEntry, ygtk_find_entry, YGTK_TYPE_EXT_ENTRY,
	G_IMPLEMENT_INTERFACE (GTK_TYPE_EDITABLE, ygtk_find_entry_editable_init))

static void ygtk_find_entry_init (YGtkFindEntry *entry)
{
}

static void ygtk_find_entry_realize (GtkWidget *widget)
{
	GTK_WIDGET_CLASS (ygtk_find_entry_parent_class)->realize (widget);

	YGtkExtEntry *eentry = YGTK_EXT_ENTRY (widget);
	YGtkFindEntry *fentry = YGTK_FIND_ENTRY (widget);
	fentry->find_icon = gtk_widget_render_icon (widget, GTK_STOCK_FIND,
	                                            GTK_ICON_SIZE_MENU, NULL);
	fentry->clear_icon = gtk_widget_render_icon (widget, GTK_STOCK_CLEAR,
	                                             GTK_ICON_SIZE_MENU, NULL);

	ygtk_ext_entry_set_border_window_size (eentry,
		YGTK_EXT_ENTRY_LEFT_WIN, gdk_pixbuf_get_width (fentry->find_icon) + 4);
	ygtk_ext_entry_set_border_window_size (eentry,
		YGTK_EXT_ENTRY_RIGHT_WIN, gdk_pixbuf_get_width (fentry->clear_icon) + 4);

	GdkDisplay *display = gtk_widget_get_display (widget);
	GdkCursor *cursor = gdk_cursor_new_for_display (display, GDK_HAND1);

	gdk_window_set_cursor (eentry->left_window, cursor);
	gdk_window_set_cursor (eentry->right_window, cursor);
	gdk_cursor_unref (cursor);
}

static void ygtk_find_entry_map (GtkWidget *widget)
{
	if (GTK_WIDGET_REALIZED (widget) && !GTK_WIDGET_MAPPED (widget)) {
		GTK_WIDGET_CLASS (ygtk_find_entry_parent_class)->map (widget);
		// only show clear icon when the entry has text
		GdkWindow *clear_win = YGTK_EXT_ENTRY (widget)->right_window;
		if (clear_win)
			gdk_window_hide (clear_win);
	}
}

static gboolean ygtk_find_entry_expose (GtkWidget *widget, GdkEventExpose *event)
{
	YGtkExtEntry *eentry = YGTK_EXT_ENTRY (widget);
	YGtkFindEntry *fentry = YGTK_FIND_ENTRY (widget);
	if (event->window == eentry->left_window)
		gdk_draw_pixbuf (GDK_DRAWABLE (eentry->left_window),
		                 widget->style->fg_gc[0],
		                 fentry->find_icon, 0, 0, 2, 0, -1, -1,
		                 GDK_RGB_DITHER_NONE, 0, 0);
	else if (event->window == eentry->right_window)
		gdk_draw_pixbuf (GDK_DRAWABLE (eentry->right_window),
		                 widget->style->fg_gc[0],
		                 fentry->clear_icon, 0, 0, 2, 0, -1, -1,
		                 GDK_RGB_DITHER_NONE, 0, 0);
	else
		GTK_WIDGET_CLASS (ygtk_find_entry_parent_class)->expose_event (widget, event);
	return TRUE;
}

static gboolean ygtk_find_entry_button_press_event (GtkWidget *widget,
                                                    GdkEventButton *event)
{
	YGtkFindEntry *fentry = YGTK_FIND_ENTRY (widget);
	YGtkExtEntry *eentry = YGTK_EXT_ENTRY (widget);
	if (event->window == eentry->left_window) {
		// If the entry has an associated context menu, use it.
		// Otherwise, find icon selects entry's text.
		if (fentry->context_menu)
			gtk_menu_popup (fentry->context_menu, NULL, NULL, NULL,
			                NULL, event->button, event->time);
		else {
			gtk_widget_grab_focus (widget);
			gtk_editable_select_region (GTK_EDITABLE (widget), 0, -1);
		}
	}
	else if (event->window == eentry->right_window)
		gtk_editable_delete_text (GTK_EDITABLE (widget), 0, -1);
	else
		return GTK_WIDGET_CLASS (ygtk_find_entry_parent_class)->button_press_event
		                                                            (widget, event);
	return TRUE;
}

static void ygtk_find_entry_insert_text (GtkEditable *editable,
	const gchar *new_text, gint new_text_len, gint *pos)
{
	GtkEditableClass *parent_editable_iface = g_type_interface_peek
		(ygtk_find_entry_parent_class, GTK_TYPE_EDITABLE);
	parent_editable_iface->insert_text (editable, new_text, new_text_len, pos);

	GdkWindow *clear_win = YGTK_EXT_ENTRY (editable)->right_window;
	if (clear_win)
		gdk_window_show (clear_win);
}

static void ygtk_find_entry_delete_text (GtkEditable *editable, gint start_pos,
                                         gint end_pos)
{
	GtkEditableClass *parent_editable_iface = g_type_interface_peek
		(ygtk_find_entry_parent_class, GTK_TYPE_EDITABLE);
	parent_editable_iface->delete_text (editable, start_pos, end_pos);

	int has_text = strlen (gtk_entry_get_text (GTK_ENTRY (editable)));
	if (!has_text) {
		/* Set or delete text may be called while the widget has not yet been
		   realized. */
		GdkWindow *clear_win = YGTK_EXT_ENTRY (editable)->right_window;
		if (clear_win)
			gdk_window_hide (clear_win);
	}
}

void ygtk_find_entry_attach_menu (YGtkFindEntry *entry, GtkMenu *menu)
{
	if (entry->context_menu)
		gtk_menu_detach (entry->context_menu);
	entry->context_menu = menu;
	if (menu)
		gtk_menu_attach_to_widget (menu, GTK_WIDGET (entry), NULL);
}

GtkWidget *ygtk_find_entry_new (gboolean will_use_find_icon)
{ return g_object_new (YGTK_TYPE_FIND_ENTRY, NULL); }

static void ygtk_find_entry_class_init (YGtkFindEntryClass *klass)
{
	GtkWidgetClass *gtkwidget_class = GTK_WIDGET_CLASS (klass);
	gtkwidget_class->realize = ygtk_find_entry_realize;
	gtkwidget_class->map = ygtk_find_entry_map;
	gtkwidget_class->expose_event = ygtk_find_entry_expose;
	gtkwidget_class->button_press_event = ygtk_find_entry_button_press_event;
}

static void ygtk_find_entry_editable_init (GtkEditableClass *iface)
{
  iface->insert_text = ygtk_find_entry_insert_text;
  iface->delete_text = ygtk_find_entry_delete_text;
}