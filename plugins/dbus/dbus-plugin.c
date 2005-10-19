/* dbus-plugin - xchat plugin for remote access using DBUS
 * Copyright (C) 2005 Claessens Xavier
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Claessens Xavier
 * x_claessens@skynet.be
 */

#include <dbus/dbus-glib.h>
#include "xchat-plugin.h"
#include <glib/gi18n.h>


#define PNAME "xchat remote access"
#define PDESC "plugin for remote access using DBUS";
#define PVERSION "0.1"

#define DBUS_SERVICE "org.xchat.service"
#define DBUS_OBJECT "/org/xchat/RemoteObject"

static xchat_plugin *ph;

typedef struct RemoteObject RemoteObject;
typedef struct RemoteObjectClass RemoteObjectClass;

GType Remote_object_get_type (void);

struct RemoteObject
{
  GObject parent;
};

struct RemoteObjectClass
{
  GObjectClass parent;
};

#define REMOTE_TYPE_OBJECT              (remote_object_get_type ())
#define REMOTE_OBJECT(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), REMOTE_TYPE_OBJECT, RemoteObject))
#define REMOTE_OBJECT_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), REMOTE_TYPE_OBJECT, RemoteObjectClass))
#define REMOTE_IS_OBJECT(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), REMOTE_TYPE_OBJECT))
#define REMOTE_IS_OBJECT_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), REMOTE_TYPE_OBJECT))
#define REMOTE_OBJECT_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), REMOTE_TYPE_OBJECT, RemoteObjectClass))

G_DEFINE_TYPE(RemoteObject, remote_object, G_TYPE_OBJECT)

/* Available services */
static gboolean remote_object_command (RemoteObject *obj, const gchar *command, GError **error);
static gboolean remote_object_print (RemoteObject *obj, const gchar *text, GError **error);
static gboolean remote_object_set_context (RemoteObject *obj, const gchar *server, const gchar *channel, GError **error);
static gboolean remote_object_get_info (RemoteObject *obj, const gchar *id, gchar **ret_info, GError **error);
static gboolean remote_object_get_prefs (RemoteObject *obj, const gchar *name, int *ret_type, gchar **ret_str, int *ret_int, GError **error);

#include "dbus-plugin-glue.h"

static void
remote_object_init (RemoteObject *obj)
{
}

static void
remote_object_class_init (RemoteObjectClass *klass)
{
}

/* Implementation of services */

static gboolean
remote_object_command (RemoteObject *obj, const gchar *command, GError **error)
{
  xchat_command (ph, command);
  return TRUE;
}

static gboolean
remote_object_print (RemoteObject *obj, const gchar *text, GError **error)
{
  xchat_print (ph, text);
  return TRUE;
}

static gboolean
remote_object_set_context (RemoteObject *obj, const gchar *server, const gchar *channel, GError **error)
{
  xchat_context *context = NULL;
  if (server[0] == '\0')
    server = NULL;
  if (channel[0] == '\0')
    channel = NULL;
  context = xchat_find_context (ph, server, channel);
  xchat_set_context (ph, context);
  return TRUE;
}

static gboolean
remote_object_get_info (RemoteObject *obj, const gchar *id, gchar **ret_info, GError **error)
{
  const gchar *info = NULL;
  info = xchat_get_info (ph, id);
  *ret_info = g_strdup (info);
  return TRUE;
}

static gboolean
remote_object_get_prefs (RemoteObject *obj, const gchar *name, int *ret_type, gchar **ret_str, int *ret_int, GError **error)
{
  const gchar *str = NULL;
  int i = 0;
  *ret_type = xchat_get_prefs (ph, name, &str, &i);
  *ret_str = g_strdup (str);
  *ret_int = i;
  return TRUE;
}

/* DBUS stuffs */

static gboolean
init_dbus(void)
{
  DBusGConnection *bus;
  DBusGProxy *bus_proxy;
  GError *error = NULL;
  RemoteObject *obj;
  guint request_name_result;

  dbus_g_object_type_install_info (REMOTE_TYPE_OBJECT, &dbus_glib_remote_object_object_info);

  bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (!bus)
  {
    xchat_printf (ph, _("Couldn't connect to session bus : %s\n"), error->message);
    g_error_free (error);
    return FALSE;
  }

  bus_proxy = dbus_g_proxy_new_for_name (bus, "org.freedesktop.DBus",
                                         "/org/freedesktop/DBus",
                                         "org.freedesktop.DBus");

  if (!dbus_g_proxy_call (bus_proxy, "RequestName", &error,
                          G_TYPE_STRING, DBUS_SERVICE,
                          G_TYPE_UINT, DBUS_NAME_FLAG_PROHIBIT_REPLACEMENT,
                          G_TYPE_INVALID,
                          G_TYPE_UINT, &request_name_result,
                          G_TYPE_INVALID))
  {
    xchat_printf (ph, _("Failed to acquire "DBUS_SERVICE" : %s\n"), error->message);
    g_error_free (error);
    return FALSE;
  }

  obj = g_object_new (REMOTE_TYPE_OBJECT, NULL);
  dbus_g_connection_register_g_object (bus, DBUS_OBJECT, G_OBJECT (obj));
  return TRUE;
}

/* xchat_plugin stuffs */

void
xchat_plugin_get_info(char **name, char **desc, char **version, void **reserved)
{
  *name = PNAME;
  *desc = PDESC;
  *version = PVERSION;
}

int
xchat_plugin_init(xchat_plugin *plugin_handle,
                  char **plugin_name,
                  char **plugin_desc,
                  char **plugin_version,
                  char *arg)
{
  gboolean success;
  ph = plugin_handle;
  *plugin_name = PNAME;
  *plugin_desc = PDESC;
  *plugin_version = PVERSION;
  success = init_dbus();
  if (success)
    xchat_print(ph, _(PNAME " loaded successfully!\n"));
  return success;
}
