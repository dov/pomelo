/* Generated by GOB (v2.0.20)   (do not edit directly) */

/* End world hunger, donate to the World Food Programme, http://www.wfp.org */

#define GOB_VERSION_MAJOR 2
#define GOB_VERSION_MINOR 0
#define GOB_VERSION_PATCHLEVEL 20

#define selfp (self->_priv)

#include <string.h> /* memset() */

#include "giv-settings-editor.h"

#include "giv-settings-editor-private.h"

#ifdef G_LIKELY
#define ___GOB_LIKELY(expr) G_LIKELY(expr)
#define ___GOB_UNLIKELY(expr) G_UNLIKELY(expr)
#else /* ! G_LIKELY */
#define ___GOB_LIKELY(expr) (expr)
#define ___GOB_UNLIKELY(expr) (expr)
#endif /* G_LIKELY */

#line 16 "src/giv-settings-editor.gob"

static GtkWidget *hig_like_frame_new(const gchar *label);
static GtkWidget *entry_new_from_double(double init_val);
static GtkWidget *entry_new_from_string(const gchar *text);
static GtkWidget *check_button_new_from_bool(bool init_state);
static GtkWidget *label_left_new(const gchar *label);
 

#line 35 "giv-settings-editor.cc"
/* self casting macros */
#define SELF(x) GIV_SETTINGS_EDITOR(x)
#define SELF_CONST(x) GIV_SETTINGS_EDITOR_CONST(x)
#define IS_SELF(x) GIV_IS_SETTINGS_EDITOR(x)
#define TYPE_SELF GIV_TYPE_SETTINGS_EDITOR
#define SELF_CLASS(x) GIV_SETTINGS_EDITOR_CLASS(x)

#define SELF_GET_CLASS(x) GIV_SETTINGS_EDITOR_GET_CLASS(x)

/* self typedefs */
typedef GivSettingsEditor Self;
typedef GivSettingsEditorClass SelfClass;

/* here are local prototypes */
static void giv_settings_editor_init (GivSettingsEditor * o) G_GNUC_UNUSED;
static void giv_settings_editor_class_init (GivSettingsEditorClass * c) G_GNUC_UNUSED;

/* pointer to the class of our parent */
static GtkDialogClass *parent_class = NULL;

/* Short form macros */
#define self_new giv_settings_editor_new
#define self_get_bool giv_settings_editor_get_bool
#define self_apply giv_settings_editor_apply
GType
giv_settings_editor_get_type (void)
{
	static GType type = 0;

	if ___GOB_UNLIKELY(type == 0) {
		static const GTypeInfo info = {
			sizeof (GivSettingsEditorClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) giv_settings_editor_class_init,
			(GClassFinalizeFunc) NULL,
			NULL /* class_data */,
			sizeof (GivSettingsEditor),
			0 /* n_preallocs */,
			(GInstanceInitFunc) giv_settings_editor_init,
			NULL
		};

		type = g_type_register_static (GTK_TYPE_DIALOG, "GivSettingsEditor", &info, (GTypeFlags)0);
	}

	return type;
}

/* a macro for creating a new object of our type */
#define GET_NEW ((GivSettingsEditor *)g_object_new(giv_settings_editor_get_type(), NULL))

/* a function for creating a new object of our type */
#include <stdarg.h>
static GivSettingsEditor * GET_NEW_VARG (const char *first, ...) G_GNUC_UNUSED;
static GivSettingsEditor *
GET_NEW_VARG (const char *first, ...)
{
	GivSettingsEditor *ret;
	va_list ap;
	va_start (ap, first);
	ret = (GivSettingsEditor *)g_object_new_valist (giv_settings_editor_get_type (), first, ap);
	va_end (ap);
	return ret;
}


static void
___finalize(GObject *obj_self)
{
#define __GOB_FUNCTION__ "Giv:Settings:Editor::finalize"
	GivSettingsEditor *self G_GNUC_UNUSED = GIV_SETTINGS_EDITOR (obj_self);
	gpointer priv G_GNUC_UNUSED = self->_priv;
#define giv_settings (self->_priv->giv_settings)
#define VAR giv_settings
	{
#line 30 "src/giv-settings-editor.gob"
	 if (giv_settings) g_object_unref(giv_settings); }
#line 114 "giv-settings-editor.cc"
	memset(&(giv_settings), 0, sizeof(giv_settings));
#undef VAR
#undef giv_settings
	if(G_OBJECT_CLASS(parent_class)->finalize) \
		(* G_OBJECT_CLASS(parent_class)->finalize)(obj_self);
}
#undef __GOB_FUNCTION__

static void 
giv_settings_editor_init (GivSettingsEditor * o)
{
#define __GOB_FUNCTION__ "Giv:Settings:Editor::init"
	o->_priv = G_TYPE_INSTANCE_GET_PRIVATE(o,GIV_TYPE_SETTINGS_EDITOR,GivSettingsEditorPrivate);
#line 25 "src/giv-settings-editor.gob"
	o->_priv->giv_settings = NULL;
#line 130 "giv-settings-editor.cc"
#line 30 "src/giv-settings-editor.gob"
	o->_priv->w_button_same_file_type = NULL;
#line 133 "giv-settings-editor.cc"
#line 30 "src/giv-settings-editor.gob"
	o->_priv->w_button_default_show_subpixel = NULL;
#line 136 "giv-settings-editor.cc"
#line 30 "src/giv-settings-editor.gob"
	o->_priv->w_button_default_auto_resize = NULL;
#line 139 "giv-settings-editor.cc"
#line 30 "src/giv-settings-editor.gob"
	o->_priv->w_button_default_auto_contrast = NULL;
#line 142 "giv-settings-editor.cc"
}
#undef __GOB_FUNCTION__
static void 
giv_settings_editor_class_init (GivSettingsEditorClass * c)
{
#define __GOB_FUNCTION__ "Giv:Settings:Editor::class_init"
	GObjectClass *g_object_class G_GNUC_UNUSED = (GObjectClass*) c;

	g_type_class_add_private(c,sizeof(GivSettingsEditorPrivate));

	parent_class = (GtkDialogClass *)g_type_class_ref (GTK_TYPE_DIALOG);

	g_object_class->finalize = ___finalize;
}
#undef __GOB_FUNCTION__



#line 36 "src/giv-settings-editor.gob"
GtkWidget * 
giv_settings_editor_new (GivSettings * giv_settings)
{
#line 165 "giv-settings-editor.cc"
#define __GOB_FUNCTION__ "Giv:Settings:Editor::new"
{
#line 38 "src/giv-settings-editor.gob"
	
        GivSettingsEditor *self = GET_NEW;

        selfp->giv_settings = G_OBJECT(giv_settings);
        g_object_ref(selfp->giv_settings);

        // Use HIG recommendation using frames without border.
        GtkWidget *w_frame = hig_like_frame_new("File Browsing");

        gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(self))),
                           w_frame, FALSE, FALSE, 0);

        GtkWidget *w_table = gtk_table_new(8,8,FALSE);
        gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(self))),
                           w_table, FALSE, FALSE, 0);
        
        // Whether to hide names
        int row = 0;
        gtk_table_attach(GTK_TABLE(w_table),
                         label_left_new("Same type?"),
                         1, 2,
                         row, row+1,
                         GtkAttachOptions(GTK_FILL|GTK_EXPAND),
                         GtkAttachOptions(0),
                         0,0);
        
        selfp->w_button_same_file_type = check_button_new_from_bool(giv_settings->do_same_filetype_on_next);
        g_object_set(selfp->w_button_same_file_type,
                     "tooltip-markup",
                     "When moving to next and previous file, only visit files of the same file type",
                     NULL);

        gtk_table_attach(GTK_TABLE(w_table),
                         selfp->w_button_same_file_type,
                         2, 3,
                         row, row+1,
                         GtkAttachOptions(GTK_FILL|GTK_EXPAND),
                         GtkAttachOptions(0),
                         0,0);

        // Default sub-pixel
        row++;
        gtk_table_attach(GTK_TABLE(w_table),
                         label_left_new("Default subpixel"),
                         1, 2,
                         row, row+1,
                         GtkAttachOptions(GTK_FILL|GTK_EXPAND),
                         GtkAttachOptions(0),
                         0,0);
        
        selfp->w_button_default_show_subpixel = check_button_new_from_bool(giv_settings->default_show_subpixel);
        g_object_set(selfp->w_button_default_show_subpixel,
                     "tooltip-markup",
                     "Whether to turn on sub pixel display by default",
                     NULL);

        gtk_table_attach(GTK_TABLE(w_table),
                         selfp->w_button_default_show_subpixel,
                         2, 3,
                         row, row+1,
                         GtkAttachOptions(GTK_FILL|GTK_EXPAND),
                         GtkAttachOptions(0),
                         0,0);

        // Default auto_resize
        row++;
        gtk_table_attach(GTK_TABLE(w_table),
                         label_left_new("Default auto-resize"),
                         1, 2,
                         row, row+1,
                         GtkAttachOptions(GTK_FILL|GTK_EXPAND),
                         GtkAttachOptions(0),
                         0,0);
        
        selfp->w_button_default_auto_resize = check_button_new_from_bool(giv_settings->default_auto_resize);
        g_object_set(selfp->w_button_default_auto_resize,
                     "tooltip-markup",
                     "Whether to do auto resize by default",
                     NULL);

        gtk_table_attach(GTK_TABLE(w_table),
                         selfp->w_button_default_auto_resize,
                         2, 3,
                         row, row+1,
                         GtkAttachOptions(GTK_FILL|GTK_EXPAND),
                         GtkAttachOptions(0),
                         0,0);

        // Default auto_contrast
        row++;
        gtk_table_attach(GTK_TABLE(w_table),
                         label_left_new("Default auto-contrast"),
                         1, 2,
                         row, row+1,
                         GtkAttachOptions(GTK_FILL|GTK_EXPAND),
                         GtkAttachOptions(0),
                         0,0);
        
        selfp->w_button_default_auto_contrast = check_button_new_from_bool(giv_settings->default_auto_contrast);
        g_object_set(selfp->w_button_default_auto_contrast,
                     "tooltip-markup",
                     "Whether to do auto resize by default",
                     NULL);

        gtk_table_attach(GTK_TABLE(w_table),
                         selfp->w_button_default_auto_contrast,
                         2, 3,
                         row, row+1,
                         GtkAttachOptions(GTK_FILL|GTK_EXPAND),
                         GtkAttachOptions(0),
                         0,0);


        // Buttons
        gtk_dialog_add_button(GTK_DIALOG(self),
                              GTK_STOCK_APPLY,
                              GTK_RESPONSE_APPLY);
        gtk_dialog_add_button(GTK_DIALOG(self),
                              GTK_STOCK_CANCEL,
                              GTK_RESPONSE_CANCEL);
        gtk_dialog_add_button(GTK_DIALOG(self),
                              GTK_STOCK_OK,
                              GTK_RESPONSE_ACCEPT);

        return GTK_WIDGET(self);
    }}
#line 295 "giv-settings-editor.cc"
#undef __GOB_FUNCTION__

#line 165 "src/giv-settings-editor.gob"
bool 
giv_settings_editor_get_bool (GivSettingsEditor * self, const char * label)
{
#line 302 "giv-settings-editor.cc"
#define __GOB_FUNCTION__ "Giv:Settings:Editor::get_bool"
#line 165 "src/giv-settings-editor.gob"
	g_return_val_if_fail (self != NULL, (bool )0);
#line 165 "src/giv-settings-editor.gob"
	g_return_val_if_fail (GIV_IS_SETTINGS_EDITOR (self), (bool )0);
#line 308 "giv-settings-editor.cc"
{
#line 167 "src/giv-settings-editor.gob"
	
        GtkWidget *w_toggle = GTK_WIDGET(g_object_get_data(G_OBJECT(self), label));
        if (!w_toggle)
            return FALSE;
        int state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w_toggle));
        return state;
    }}
#line 318 "giv-settings-editor.cc"
#undef __GOB_FUNCTION__

#line 175 "src/giv-settings-editor.gob"
void 
giv_settings_editor_apply (GivSettingsEditor * self)
{
#line 325 "giv-settings-editor.cc"
#define __GOB_FUNCTION__ "Giv:Settings:Editor::apply"
#line 175 "src/giv-settings-editor.gob"
	g_return_if_fail (self != NULL);
#line 175 "src/giv-settings-editor.gob"
	g_return_if_fail (GIV_IS_SETTINGS_EDITOR (self));
#line 331 "giv-settings-editor.cc"
{
#line 176 "src/giv-settings-editor.gob"
	
        GIV_SETTINGS(selfp->giv_settings)->do_same_filetype_on_next
            = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(selfp->w_button_same_file_type));
        GIV_SETTINGS(selfp->giv_settings)->default_show_subpixel
            = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(selfp->w_button_default_show_subpixel));
        GIV_SETTINGS(selfp->giv_settings)->default_auto_resize
            = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(selfp->w_button_default_auto_resize));
        GIV_SETTINGS(selfp->giv_settings)->default_auto_contrast
            = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(selfp->w_button_default_auto_contrast));
    }}
#line 344 "giv-settings-editor.cc"
#undef __GOB_FUNCTION__

#line 189 "src/giv-settings-editor.gob"

static GtkWidget *hig_like_frame_new(const gchar *label)
{
    GtkWidget *w_frame = gtk_frame_new(NULL);
    GtkWidget *w_label = gtk_label_new(NULL);
    gchar*markup = g_strdup_printf("<b>%s</b>", label);

    gtk_label_set_markup(GTK_LABEL(w_label),
                         markup);
    g_free(markup);
    gtk_frame_set_label_widget(GTK_FRAME(w_frame),
                               w_label);
    // Follow the HIG recommendation
    gtk_frame_set_shadow_type(GTK_FRAME(w_frame), GTK_SHADOW_NONE);

    return w_frame;
}


static GtkWidget *check_button_new_from_bool(bool init_state)
{
    GtkWidget *check_button = gtk_check_button_new();

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button),
                                 init_state);

    return check_button;
}
 
static GtkWidget *label_left_new(const gchar *label)
{
    gchar *markup = g_strdup_printf("%s:", label);
    GtkWidget *w_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(w_label), markup);
    g_free(markup);
    gtk_misc_set_alignment(GTK_MISC(w_label), 0, 0.5);
    return w_label;
}
 

#line 388 "giv-settings-editor.cc"
