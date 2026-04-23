/*
 * Minimal NM VPN editor plugin for ZeroTier
 * Shows network-id and moon-id fields in the connection settings dialog.
 * Build: gcc -shared -fPIC -o libnm-vpn-plugin-zerotier-editor.so editor.c \
 *        $(pkg-config --cflags --libs libnm gtk4)
 */
#include <gtk/gtk.h>
#include <NetworkManager.h>
#define ZT_SERVICE "org.freedesktop.NetworkManager.zerotier"

typedef struct {
    GObject parent;
    GtkWidget *nwid_entry;
    GtkWidget *moon_entry;
} ZtEditor;

typedef struct { GObjectClass parent; } ZtEditorClass;

static void zt_editor_vpn_iface_init(NMVpnEditorInterface *iface);
G_DEFINE_TYPE_WITH_CODE(ZtEditor, zt_editor, G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE(NM_TYPE_VPN_EDITOR, zt_editor_vpn_iface_init))

static void changed_cb(GtkEditable *e, gpointer user_data) {
    g_signal_emit_by_name(user_data, "changed");
}

static GObject *zt_editor_get_widget(NMVpnEditor *editor) {
    ZtEditor *self = (ZtEditor *)editor;
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 12);

    GtkWidget *l1 = gtk_label_new("Network ID:");
    gtk_widget_set_halign(l1, GTK_ALIGN_END);
    self->nwid_entry = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(self->nwid_entry), 16);
    gtk_widget_set_hexpand(self->nwid_entry, TRUE);
    g_signal_connect(self->nwid_entry, "changed", G_CALLBACK(changed_cb), self);

    GtkWidget *l2 = gtk_label_new("Moon ID:");
    gtk_widget_set_halign(l2, GTK_ALIGN_END);
    self->moon_entry = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(self->moon_entry), 10);
    gtk_widget_set_hexpand(self->moon_entry, TRUE);
    g_signal_connect(self->moon_entry, "changed", G_CALLBACK(changed_cb), self);

    gtk_grid_attach(GTK_GRID(grid), l1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), self->nwid_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), l2, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), self->moon_entry, 1, 1, 1, 1);

    gtk_widget_set_visible(grid, TRUE);
    return G_OBJECT(grid);
}

static gboolean zt_editor_update(NMVpnEditor *editor, NMConnection *conn, GError **err) {
    ZtEditor *self = (ZtEditor *)editor;
    NMSettingVpn *vpn = nm_connection_get_setting_vpn(conn);
    if (!vpn) {
        vpn = (NMSettingVpn *)nm_setting_vpn_new();
        g_object_set(vpn, NM_SETTING_VPN_SERVICE_TYPE, ZT_SERVICE, NULL);
        nm_connection_add_setting(conn, NM_SETTING(vpn));
    }
    const char *nwid = gtk_editable_get_text(GTK_EDITABLE(self->nwid_entry));
    const char *moon = gtk_editable_get_text(GTK_EDITABLE(self->moon_entry));
    nm_setting_vpn_add_data_item(vpn, "network-id", nwid ? nwid : "");
    nm_setting_vpn_add_data_item(vpn, "moon-id", moon ? moon : "");
    return TRUE;
}

static void zt_editor_vpn_iface_init(NMVpnEditorInterface *iface) {
    iface->get_widget = zt_editor_get_widget;
    iface->update_connection = zt_editor_update;
}

static void zt_editor_init(ZtEditor *self) {}
static void zt_editor_class_init(ZtEditorClass *klass) {}

/* ── Plugin factory ── */

typedef struct {
    GObject parent;
} ZtPlugin;
typedef struct { GObjectClass parent; } ZtPluginClass;

static void zt_plugin_iface_init(NMVpnEditorPluginInterface *iface);
G_DEFINE_TYPE_WITH_CODE(ZtPlugin, zt_plugin, G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE(NM_TYPE_VPN_EDITOR_PLUGIN, zt_plugin_iface_init))

static NMVpnEditor *zt_plugin_get_editor(NMVpnEditorPlugin *plugin,
                                          NMConnection *conn, GError **err) {
    ZtEditor *ed = g_object_new(zt_editor_get_type(), NULL);
    NMSettingVpn *vpn = nm_connection_get_setting_vpn(conn);
    if (vpn) {
        const char *nwid = nm_setting_vpn_get_data_item(vpn, "network-id");
        const char *moon = nm_setting_vpn_get_data_item(vpn, "moon-id");
        if (nwid) gtk_editable_set_text(GTK_EDITABLE(ed->nwid_entry), nwid);
        if (moon) gtk_editable_set_text(GTK_EDITABLE(ed->moon_entry), moon);
    }
    return NM_VPN_EDITOR(ed);
}

static guint32 zt_plugin_get_capabilities(NMVpnEditorPlugin *p) { return 0; }
static char *zt_plugin_get_service(NMVpnEditorPlugin *p) { return g_strdup(ZT_SERVICE); }

static void zt_plugin_iface_init(NMVpnEditorPluginInterface *iface) {
    iface->get_editor = zt_plugin_get_editor;
    iface->get_capabilities = zt_plugin_get_capabilities;
}

static void zt_plugin_init(ZtPlugin *self) {}
static void zt_plugin_class_init(ZtPluginClass *klass) {}

G_MODULE_EXPORT NMVpnEditorPlugin *nm_vpn_editor_plugin_factory(GError **err) {
    return NM_VPN_EDITOR_PLUGIN(g_object_new(zt_plugin_get_type(), NULL));
}
