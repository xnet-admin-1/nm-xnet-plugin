/*
 * NM VPN editor plugin for XNet
 * Build: gcc -shared -fPIC -o libnm-vpn-plugin-xnet-editor.so editor.c \
 *        $(pkg-config --cflags --libs libnm gtk4)
 */
#include <gtk/gtk.h>
#include <NetworkManager.h>

#define ZT_SERVICE "org.freedesktop.NetworkManager.xnet"

/* ── Editor ── */

typedef struct { GObject parent; GtkWidget *widget, *nwid; } ZtEditor;
typedef struct { GObjectClass parent; } ZtEditorClass;
static void zt_ed_iface_init(NMVpnEditorInterface *iface);
G_DEFINE_TYPE_WITH_CODE(ZtEditor, zt_editor, G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE(NM_TYPE_VPN_EDITOR, zt_ed_iface_init))

static void changed_cb(GtkEditable *e, gpointer d) { g_signal_emit_by_name(d, "changed"); }

static GObject *zt_ed_get_widget(NMVpnEditor *ed) {
    return G_OBJECT(((ZtEditor *)ed)->widget);
}

static gboolean zt_ed_update(NMVpnEditor *ed, NMConnection *conn, GError **err) {
    ZtEditor *s = (ZtEditor *)ed;
    NMSettingVpn *vpn = nm_connection_get_setting_vpn(conn);
    if (!vpn) {
        vpn = (NMSettingVpn *)nm_setting_vpn_new();
        g_object_set(vpn, NM_SETTING_VPN_SERVICE_TYPE, ZT_SERVICE, NULL);
        nm_connection_add_setting(conn, NM_SETTING(vpn));
    }
    const char *n = gtk_editable_get_text(GTK_EDITABLE(s->nwid));
    nm_setting_vpn_add_data_item(vpn, "network-id", n ? n : "");
    return TRUE;
}

static void zt_editor_init(ZtEditor *s) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_margin_start(box, 12);
    gtk_widget_set_margin_end(box, 12);
    gtk_widget_set_margin_top(box, 12);
    gtk_widget_set_margin_bottom(box, 12);

    GtkWidget *l = gtk_label_new("Network ID:");
    s->nwid = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(s->nwid), 16);
    gtk_widget_set_hexpand(s->nwid, TRUE);
    g_signal_connect(s->nwid, "changed", G_CALLBACK(changed_cb), s);

    gtk_box_append(GTK_BOX(box), l);
    gtk_box_append(GTK_BOX(box), s->nwid);

    s->widget = box;
    g_object_ref_sink(s->widget);
}

static void zt_editor_finalize(GObject *obj) {
    g_clear_object(&((ZtEditor *)obj)->widget);
    G_OBJECT_CLASS(zt_editor_parent_class)->finalize(obj);
}

static void zt_ed_iface_init(NMVpnEditorInterface *i) {
    i->get_widget = zt_ed_get_widget;
    i->update_connection = zt_ed_update;
}
static void zt_editor_class_init(ZtEditorClass *k) {
    G_OBJECT_CLASS(k)->finalize = zt_editor_finalize;
}

/* ── Plugin ── */

typedef struct { GObject parent; } ZtPlugin;
typedef struct { GObjectClass parent; } ZtPluginClass;
enum { PROP_0, PROP_NAME, PROP_DESC, PROP_SERVICE };
static void zt_pl_iface_init(NMVpnEditorPluginInterface *iface);
G_DEFINE_TYPE_WITH_CODE(ZtPlugin, zt_plugin, G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE(NM_TYPE_VPN_EDITOR_PLUGIN, zt_pl_iface_init))

static NMVpnEditor *zt_pl_get_editor(NMVpnEditorPlugin *p, NMConnection *c, GError **e) {
    ZtEditor *ed = g_object_new(zt_editor_get_type(), NULL);
    NMSettingVpn *vpn = nm_connection_get_setting_vpn(c);
    if (vpn) {
        const char *n = nm_setting_vpn_get_data_item(vpn, "network-id");
        if (n) gtk_editable_set_text(GTK_EDITABLE(ed->nwid), n);
    }
    return NM_VPN_EDITOR(ed);
}

static guint32 zt_pl_get_caps(NMVpnEditorPlugin *p) { return 0; }

static void zt_pl_get_prop(GObject *obj, guint id, GValue *val, GParamSpec *ps) {
    switch (id) {
    case PROP_NAME:    g_value_set_string(val, "XNet"); break;
    case PROP_DESC:    g_value_set_string(val, "XNet VPN connection"); break;
    case PROP_SERVICE: g_value_set_string(val, ZT_SERVICE); break;
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, id, ps);
    }
}

static void zt_pl_iface_init(NMVpnEditorPluginInterface *i) {
    i->get_editor = zt_pl_get_editor;
    i->get_capabilities = zt_pl_get_caps;
}
static void zt_plugin_init(ZtPlugin *s) {}
static void zt_plugin_class_init(ZtPluginClass *k) {
    GObjectClass *oc = G_OBJECT_CLASS(k);
    oc->get_property = zt_pl_get_prop;
    g_object_class_override_property(oc, PROP_NAME, NM_VPN_EDITOR_PLUGIN_NAME);
    g_object_class_override_property(oc, PROP_DESC, NM_VPN_EDITOR_PLUGIN_DESCRIPTION);
    g_object_class_override_property(oc, PROP_SERVICE, NM_VPN_EDITOR_PLUGIN_SERVICE);
}

G_MODULE_EXPORT NMVpnEditorPlugin *nm_vpn_editor_plugin_factory(GError **e) {
    return NM_VPN_EDITOR_PLUGIN(g_object_new(zt_plugin_get_type(), NULL));
}
