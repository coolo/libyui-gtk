/********************************************************************
 *           YaST2-GTK - http://en.opensuse.org/YaST2-GTK           *
 ********************************************************************/

/* YGtkFieldEntry is an extension of GtkEntry with the added
   functionality of being able to define fields (useful for when
   you need the user to set a IP address or time/date). The number
   of fields, their individual range and separation character
   is all customizable.
*/

#ifndef YGTK_FIELD_ENTRY_H
#define YGTK_FIELD_ENTRY_H

#include <gtk/gtk.h>
G_BEGIN_DECLS

#define YGTK_TYPE_FIELD_ENTRY            (ygtk_field_entry_get_type ())
#define YGTK_FIELD_ENTRY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                          YGTK_TYPE_FIELD_ENTRY, YGtkFieldEntry))
#define YGTK_FIELD_ENTRY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  \
                                          YGTK_TYPE_FIELD_ENTRY, YGtkFieldEntryClass))
#define IS_YGTK_FIELD_ENTRY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                                          YGTK_TYPE_FIELD_ENTRY))
#define IS_YGTK_FIELD_ENTRY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  \
                                          YGTK_TYPE_FIELD_ENTRY))
#define YGTK_FIELD_ENTRY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  \
                                          YGTK_TYPE_FIELD_ENTRY, YGtkFieldEntryClass))

typedef struct _YGtkFieldEntry
{
	GtkHBox parent;
} YGtkFieldEntry;

typedef struct _YGtkFieldEntryClass
{
	GtkHBoxClass parent_class;

	void (* filter_entry_changed) (YGtkFieldEntry *entry, gint field_nb);
} YGtkFieldEntryClass;

GtkWidget* ygtk_field_entry_new      (void);
GType      ygtk_field_entry_get_type (void) G_GNUC_CONST;

// if this is the first field, separator will be ignored.
guint      ygtk_field_entry_add_field         (YGtkFieldEntry *entry, gchar separator);
// max_length can be 0 to disable it. valids_chars can be NULL to disable it.
void       ygtk_field_entry_setup_field       (YGtkFieldEntry *entry, guint index,
                                               gint max_length, const gchar *valid_chars);

const gchar *ygtk_field_entry_get_field_text   (YGtkFieldEntry *entry, guint index);
void         ygtk_field_entry_set_field_text   (YGtkFieldEntry *entry, guint index,
                                                const gchar *text);

GtkEntry    *ygtk_field_entry_get_field_widget (YGtkFieldEntry *entry, guint index);
gboolean     ygtk_field_entry_set_focus        (YGtkFieldEntry *entry);

G_END_DECLS
#endif /*YGTK_FIELD_ENTRY_H*/

