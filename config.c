/* See LICENSE file for license and copyright information */

#include "config.h"
#include "commands.h"
#include "completion.h"
#include "callbacks.h"
#include "shortcuts.h"
#include "zathura.h"

#include <girara/settings.h>
#include <girara/session.h>
#include <girara/shortcuts.h>
#include <girara/config.h>
#include <girara/commands.h>
#include <girara/gtk2-compat.h>

static void
cb_color_change(girara_session_t* session, const char* name, girara_setting_type_t UNUSED(type), void* value, void* UNUSED(data))
{
  g_return_if_fail(value != NULL);
  g_return_if_fail(session != NULL);
  g_return_if_fail(session->global.data != NULL);
  g_return_if_fail(name != NULL);
  zathura_t* zathura = session->global.data;

  char* string_value = (char*) value;
  if (g_strcmp0(name, "highlight-color") == 0) {
    gdk_color_parse(string_value, &(zathura->ui.colors.highlight_color));
  } else if (g_strcmp0(name, "highlight-active-color") == 0) {
    gdk_color_parse(string_value, &(zathura->ui.colors.highlight_color_active));
  } else if (g_strcmp0(name, "recolor-darkcolor") == 0) {
    gdk_color_parse(string_value, &(zathura->ui.colors.recolor_dark_color));
  } else if (g_strcmp0(name, "recolor-lightcolor") == 0) {
    gdk_color_parse(string_value, &(zathura->ui.colors.recolor_light_color));
  }

  /* TODO: cause a redraw here? */
}


void
config_load_default(zathura_t* zathura)
{
  if (zathura == NULL || zathura->ui.session == NULL) {
    return;
  }

  int int_value              = 0;
  float float_value          = 0;
  char* string_value         = NULL;
  bool bool_value            = false;
  girara_session_t* gsession = zathura->ui.session;

  /* mode settings */
  zathura->modes.normal     = gsession->modes.normal;
  zathura->modes.fullscreen = girara_mode_add(gsession, "fullscreen");
  zathura->modes.index      = girara_mode_add(gsession, "index");
  zathura->modes.insert     = girara_mode_add(gsession, "insert");

#define NORMAL zathura->modes.normal
#define INSERT zathura->modes.insert
#define INDEX zathura->modes.index
#define FULLSCREEN zathura->modes.fullscreen

  girara_mode_set(gsession, zathura->modes.normal);

  /* zathura settings */
  int_value = 10;
  girara_setting_add(gsession, "zoom-step",     &int_value,   INT,   false, "Zoom step",               NULL, NULL);
  int_value = 1;
  girara_setting_add(gsession, "page-padding",  &int_value,   INT,   true,  "Padding between pages",   NULL, NULL);
  int_value = 1;
  girara_setting_add(gsession, "pages-per-row", &int_value,   INT,   false, "Number of pages per row", cb_pages_per_row_value_changed, NULL);
  float_value = 40;
  girara_setting_add(gsession, "scroll-step",   &float_value, FLOAT, false, "Scroll step",             NULL, NULL);
  int_value = 10;
  girara_setting_add(gsession, "zoom-min",      &int_value,   INT,   false, "Zoom minimum", NULL, NULL);
  int_value = 1000;
  girara_setting_add(gsession, "zoom-max",      &int_value,   INT,   false, "Zoom maximum", NULL, NULL);

  girara_setting_add(gsession, "recolor-darkcolor",      NULL, STRING, false, "Recoloring (dark color)",         cb_color_change, NULL);
  girara_setting_set(gsession, "recolor-darkcolor",      "#FFFFFF");
  girara_setting_add(gsession, "recolor-lightcolor",     NULL, STRING, false, "Recoloring (light color)",        cb_color_change, NULL);
  girara_setting_set(gsession, "recolor-lightcolor",     "#000000");
  girara_setting_add(gsession, "highlight-color",        NULL, STRING, false, "Color for highlighting",          cb_color_change, NULL);
  girara_setting_set(gsession, "highlight-color",        "#9FBC00");
  girara_setting_add(gsession, "highlight-active-color", NULL, STRING, false, "Color for highlighting (active)", cb_color_change, NULL);
  girara_setting_set(gsession, "highlight-active-color", "#00BC00");

  float_value = 0.5;
  girara_setting_add(gsession, "highlight-transparency", &float_value, FLOAT,   false, "Transparency for highlighting", NULL, NULL);
  bool_value = true;
  girara_setting_add(gsession, "render-loading",         &bool_value,  BOOLEAN, false, "Render 'Loading ...'", NULL, NULL);
  string_value = "best-fit";
  girara_setting_add(gsession, "adjust-open",            string_value, STRING,  false, "Adjust to when opening file", NULL, NULL);
  bool_value = false;
  girara_setting_add(gsession, "show-hidden",            &bool_value,  BOOLEAN, false, "Show hidden files and directories", NULL, NULL);

  /* define default shortcuts */
  girara_shortcut_add(gsession, GDK_CONTROL_MASK, GDK_KEY_c,          NULL, sc_abort,                    0,          0,               NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Escape,     NULL, sc_abort,                    0,          0,               NULL);

  girara_shortcut_add(gsession, 0,                GDK_KEY_a,          NULL, sc_adjust_window,            NORMAL,     ADJUST_BESTFIT,  NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_s,          NULL, sc_adjust_window,            NORMAL,     ADJUST_WIDTH,    NULL);

  girara_shortcut_add(gsession, 0,                GDK_KEY_m,          NULL, sc_change_mode,              NORMAL,     ADD_MARKER,      NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_apostrophe, NULL, sc_change_mode,              NORMAL,     EVAL_MARKER,     NULL);

  girara_shortcut_add(gsession, 0,                GDK_KEY_slash,      NULL, sc_focus_inputbar,           NORMAL,     0,               &("/"));
  girara_shortcut_add(gsession, GDK_SHIFT_MASK,   GDK_KEY_slash,      NULL, sc_focus_inputbar,           NORMAL,     0,               &("/"));
  girara_shortcut_add(gsession, 0,                GDK_KEY_question,   NULL, sc_focus_inputbar,           NORMAL,     0,               &("?"));
  girara_shortcut_add(gsession, 0,                GDK_KEY_colon,      NULL, sc_focus_inputbar,           NORMAL,     0,               &(":"));
  girara_shortcut_add(gsession, 0,                GDK_KEY_o,          NULL, sc_focus_inputbar,           NORMAL,     0,               &(":open "));
  girara_shortcut_add(gsession, 0,                GDK_KEY_O,          NULL, sc_focus_inputbar,           NORMAL,     APPEND_FILEPATH, &(":open "));

  girara_shortcut_add(gsession, 0,                GDK_KEY_f,          NULL, sc_follow,                   NORMAL,     0,               NULL);

  girara_shortcut_add(gsession, 0,                0,                  "gg", sc_goto,                     NORMAL,     TOP,             NULL);
  girara_shortcut_add(gsession, 0,                0,                  "gg", sc_goto,                     FULLSCREEN, TOP,             NULL);
  girara_shortcut_add(gsession, 0,                0,                  "G",  sc_goto,                     NORMAL,     BOTTOM,          NULL);
  girara_shortcut_add(gsession, 0,                0,                  "G",  sc_goto,                     FULLSCREEN, BOTTOM,          NULL);

  girara_shortcut_add(gsession, 0,                GDK_KEY_J,          NULL, sc_navigate,                 NORMAL,     NEXT,            NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_K,          NULL, sc_navigate,                 NORMAL,     PREVIOUS,        NULL);
  girara_shortcut_add(gsession, GDK_MOD1_MASK,    GDK_KEY_Right,      NULL, sc_navigate,                 NORMAL,     NEXT,            NULL);
  girara_shortcut_add(gsession, GDK_MOD1_MASK,    GDK_KEY_Left,       NULL, sc_navigate,                 NORMAL,     PREVIOUS,        NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Page_Down,  NULL, sc_navigate,                 NORMAL,     NEXT,            NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Page_Up,    NULL, sc_navigate,                 NORMAL,     PREVIOUS,        NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_J,          NULL, sc_navigate,                 FULLSCREEN, NEXT,            NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_K,          NULL, sc_navigate,                 FULLSCREEN, PREVIOUS,        NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Left,       NULL, sc_navigate,                 FULLSCREEN, PREVIOUS,        NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Up,         NULL, sc_navigate,                 FULLSCREEN, PREVIOUS,        NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Down,       NULL, sc_navigate,                 FULLSCREEN, NEXT,            NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Right,      NULL, sc_navigate,                 FULLSCREEN, NEXT,            NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Page_Down,  NULL, sc_navigate,                 FULLSCREEN, NEXT,            NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Page_Up,    NULL, sc_navigate,                 FULLSCREEN, PREVIOUS,        NULL);

  girara_shortcut_add(gsession, 0,                GDK_KEY_k,          NULL, sc_navigate_index,           INDEX,      UP,              NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_j,          NULL, sc_navigate_index,           INDEX,      DOWN,            NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_h,          NULL, sc_navigate_index,           INDEX,      COLLAPSE,        NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_l,          NULL, sc_navigate_index,           INDEX,      EXPAND,          NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_L,          NULL, sc_navigate_index,           INDEX,      EXPAND_ALL,      NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_H,          NULL, sc_navigate_index,           INDEX,      COLLAPSE_ALL,    NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Up,         NULL, sc_navigate_index,           INDEX,      UP,              NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Down,       NULL, sc_navigate_index,           INDEX,      DOWN,            NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Left,       NULL, sc_navigate_index,           INDEX,      COLLAPSE,        NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Right,      NULL, sc_navigate_index,           INDEX,      EXPAND,          NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_space,      NULL, sc_navigate_index,           INDEX,      SELECT,          NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Return,     NULL, sc_navigate_index,           INDEX,      SELECT,          NULL);

  girara_shortcut_add(gsession, GDK_CONTROL_MASK, GDK_KEY_i,          NULL, sc_recolor,                  NORMAL,     0,               NULL);

  girara_shortcut_add(gsession, 0,                GDK_KEY_R,          NULL, sc_reload,                   NORMAL,     0,               NULL);

  girara_shortcut_add(gsession, 0,                GDK_KEY_r,          NULL, sc_rotate,                   NORMAL,     0,               NULL);

  girara_shortcut_add(gsession, 0,                GDK_KEY_h,          NULL, sc_scroll,                   NORMAL,     LEFT,            NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_j,          NULL, sc_scroll,                   NORMAL,     DOWN,            NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_k,          NULL, sc_scroll,                   NORMAL,     UP,              NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_l,          NULL, sc_scroll,                   NORMAL,     RIGHT,           NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Left,       NULL, sc_scroll,                   NORMAL,     LEFT,            NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Up,         NULL, sc_scroll,                   NORMAL,     UP,              NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Down,       NULL, sc_scroll,                   NORMAL,     DOWN,            NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Right,      NULL, sc_scroll,                   NORMAL,     RIGHT,           NULL);
  girara_shortcut_add(gsession, GDK_CONTROL_MASK, GDK_KEY_t,          NULL, sc_scroll,                   NORMAL,     HALF_LEFT,       NULL);
  girara_shortcut_add(gsession, GDK_CONTROL_MASK, GDK_KEY_d,          NULL, sc_scroll,                   NORMAL,     HALF_DOWN,       NULL);
  girara_shortcut_add(gsession, GDK_CONTROL_MASK, GDK_KEY_u,          NULL, sc_scroll,                   NORMAL,     HALF_UP,         NULL);
  girara_shortcut_add(gsession, GDK_CONTROL_MASK, GDK_KEY_y,          NULL, sc_scroll,                   NORMAL,     HALF_RIGHT,      NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_t,          NULL, sc_scroll,                   NORMAL,     FULL_LEFT,       NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_0,          NULL, sc_scroll,                   NORMAL,     FULL_LEFT,       NULL);
  girara_shortcut_add(gsession, GDK_CONTROL_MASK, GDK_KEY_f,          NULL, sc_scroll,                   NORMAL,     FULL_DOWN,       NULL);
  girara_shortcut_add(gsession, GDK_CONTROL_MASK, GDK_KEY_b,          NULL, sc_scroll,                   NORMAL,     FULL_UP,         NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_y,          NULL, sc_scroll,                   NORMAL,     FULL_RIGHT,      NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_dollar,     NULL, sc_scroll,                   NORMAL,     FULL_RIGHT,      NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_space,      NULL, sc_scroll,                   NORMAL,     FULL_DOWN,       NULL);
  girara_shortcut_add(gsession, GDK_SHIFT_MASK,   GDK_KEY_space,      NULL, sc_scroll,                   NORMAL,     FULL_UP,         NULL);

  girara_shortcut_add(gsession, 0,                GDK_KEY_n,          NULL, sc_search,                   NORMAL,     FORWARD,         NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_N,          NULL, sc_search,                   NORMAL,     BACKWARD,        NULL);

  girara_shortcut_add(gsession, 0,                GDK_KEY_Tab,        NULL, sc_toggle_index,             NORMAL,     0,               NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Tab,        NULL, sc_toggle_index,             INDEX,      0,               NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_F5,         NULL, sc_toggle_fullscreen,        NORMAL,     0,               NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_F5,         NULL, sc_toggle_fullscreen,        FULLSCREEN, 0,               NULL);
  girara_shortcut_add(gsession, GDK_CONTROL_MASK, GDK_KEY_n,          NULL, girara_sc_toggle_statusbar,  NORMAL,     0,               NULL);
  girara_shortcut_add(gsession, GDK_CONTROL_MASK, GDK_KEY_m,          NULL, girara_sc_toggle_inputbar,   NORMAL,     0,               NULL);

  girara_shortcut_add(gsession, 0,                GDK_KEY_q,          NULL, sc_quit,                     NORMAL,     0,               NULL);

  girara_shortcut_add(gsession, 0,                GDK_KEY_plus,       NULL, sc_zoom,                     NORMAL,     ZOOM_IN,         NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_plus,       NULL, sc_zoom,                     FULLSCREEN, ZOOM_IN,         NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_minus,      NULL, sc_zoom,                     NORMAL,     ZOOM_OUT,        NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_minus,      NULL, sc_zoom,                     FULLSCREEN, ZOOM_OUT,        NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_equal,      NULL, sc_zoom,                     NORMAL,     ZOOM_ORIGINAL,   NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_equal,      NULL, sc_zoom,                     FULLSCREEN, ZOOM_ORIGINAL,   NULL);
  girara_shortcut_add(gsession, 0,                0,                  "zI", sc_zoom,                     NORMAL,     ZOOM_IN,         NULL);
  girara_shortcut_add(gsession, 0,                0,                  "zI", sc_zoom,                     FULLSCREEN, ZOOM_IN,         NULL);
  girara_shortcut_add(gsession, 0,                0,                  "zO", sc_zoom,                     NORMAL,     ZOOM_OUT,        NULL);
  girara_shortcut_add(gsession, 0,                0,                  "zO", sc_zoom,                     FULLSCREEN, ZOOM_OUT,        NULL);
  girara_shortcut_add(gsession, 0,                0,                  "z0", sc_zoom,                     NORMAL,     ZOOM_ORIGINAL,   NULL);
  girara_shortcut_add(gsession, 0,                0,                  "z0", sc_zoom,                     FULLSCREEN, ZOOM_ORIGINAL,   NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_equal,      NULL, sc_zoom,                     NORMAL,     ZOOM_SPECIFIC,   NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_equal,      NULL, sc_zoom,                     FULLSCREEN, ZOOM_SPECIFIC,   NULL);

  /* mouse events */
  girara_mouse_event_add(gsession, 0, 0,                                   sc_mouse_scroll, NORMAL,     GIRARA_EVENT_SCROLL_UP,      UP,   NULL);
  girara_mouse_event_add(gsession, 0, 0,                                   sc_mouse_scroll, FULLSCREEN, GIRARA_EVENT_SCROLL_UP,      UP,   NULL);
  girara_mouse_event_add(gsession, 0, 0,                                   sc_mouse_scroll, NORMAL,     GIRARA_EVENT_SCROLL_DOWN,    DOWN, NULL);
  girara_mouse_event_add(gsession, 0, 0,                                   sc_mouse_scroll, FULLSCREEN, GIRARA_EVENT_SCROLL_DOWN,    DOWN, NULL);
  girara_mouse_event_add(gsession, GDK_CONTROL_MASK, 0,                    sc_mouse_zoom,   NORMAL,     GIRARA_EVENT_SCROLL_UP,      UP,   NULL);
  girara_mouse_event_add(gsession, GDK_CONTROL_MASK, 0,                    sc_mouse_zoom,   FULLSCREEN, GIRARA_EVENT_SCROLL_UP,      UP,   NULL);
  girara_mouse_event_add(gsession, GDK_CONTROL_MASK, 0,                    sc_mouse_zoom,   NORMAL,     GIRARA_EVENT_SCROLL_DOWN,    DOWN, NULL);
  girara_mouse_event_add(gsession, GDK_CONTROL_MASK, 0,                    sc_mouse_zoom,   FULLSCREEN, GIRARA_EVENT_SCROLL_DOWN,    DOWN, NULL);
  girara_mouse_event_add(gsession, 0,                GIRARA_MOUSE_BUTTON2, sc_mouse_scroll, NORMAL,     GIRARA_EVENT_BUTTON_PRESS,   0,    NULL);
  girara_mouse_event_add(gsession, GDK_BUTTON2_MASK, GIRARA_MOUSE_BUTTON2, sc_mouse_scroll, NORMAL,     GIRARA_EVENT_BUTTON_RELEASE, 0,    NULL);
  girara_mouse_event_add(gsession, GDK_BUTTON2_MASK, 0,                    sc_mouse_scroll, NORMAL,     GIRARA_EVENT_MOTION_NOTIFY,  0,    NULL);

  /* define default inputbar commands */
  girara_inputbar_command_add(gsession, "bmark",   NULL, cmd_bookmark_create, NULL,         "Add a bookmark");
  girara_inputbar_command_add(gsession, "bdelete", NULL, cmd_bookmark_delete, cc_bookmarks, "Delete a bookmark");
  girara_inputbar_command_add(gsession, "blist",   NULL, cmd_bookmark_open,   cc_bookmarks, "List all bookmarks");
  girara_inputbar_command_add(gsession, "close",   NULL, cmd_close,           NULL,         "Close current file");
  girara_inputbar_command_add(gsession, "info",    NULL, cmd_info,            NULL,         "Show file information");
  girara_inputbar_command_add(gsession, "help",    NULL, cmd_help,            NULL,         "Show help");
  girara_inputbar_command_add(gsession, "open",    "o",  cmd_open,            cc_open,      "Open document");
  girara_inputbar_command_add(gsession, "print",   NULL, cmd_print,           NULL,         "Print document");
  girara_inputbar_command_add(gsession, "write",   NULL, cmd_save,            NULL,         "Save document");
  girara_inputbar_command_add(gsession, "write!",  NULL, cmd_savef,           NULL,         "Save document (and force overwriting)");
  girara_inputbar_command_add(gsession, "export",  NULL, cmd_export,          cc_export,    "Save attachments");

  girara_special_command_add(gsession, '/', cmd_search, true, FORWARD,  NULL);
  girara_special_command_add(gsession, '?', cmd_search, true, BACKWARD, NULL);

  /* add shortcut mappings */
  girara_shortcut_mapping_add(gsession, "abort",             sc_abort);
  girara_shortcut_mapping_add(gsession, "adjust_window",     sc_adjust_window);
  girara_shortcut_mapping_add(gsession, "change_mode",       sc_change_mode);
  girara_shortcut_mapping_add(gsession, "follow",            sc_follow);
  girara_shortcut_mapping_add(gsession, "goto",              sc_goto);
  girara_shortcut_mapping_add(gsession, "navigate_index",    sc_navigate_index);
  girara_shortcut_mapping_add(gsession, "navigate",          sc_navigate);
  girara_shortcut_mapping_add(gsession, "quit",              sc_quit);
  girara_shortcut_mapping_add(gsession, "recolor",           sc_recolor);
  girara_shortcut_mapping_add(gsession, "reload",            sc_reload);
  girara_shortcut_mapping_add(gsession, "rotate",            sc_rotate);
  girara_shortcut_mapping_add(gsession, "scroll",            sc_scroll);
  girara_shortcut_mapping_add(gsession, "search",            sc_search);
  girara_shortcut_mapping_add(gsession, "toggle_fullscreen", sc_toggle_fullscreen);
  girara_shortcut_mapping_add(gsession, "toggle_index",      sc_toggle_index);
  girara_shortcut_mapping_add(gsession, "toggle_inputbar",   girara_sc_toggle_inputbar);
  girara_shortcut_mapping_add(gsession, "toggle_statusbar",  girara_sc_toggle_statusbar);
  girara_shortcut_mapping_add(gsession, "zoom",              sc_zoom);

  /* add argument mappings */
  girara_argument_mapping_add(gsession, "bottom",       BOTTOM);
  girara_argument_mapping_add(gsession, "default",      DEFAULT);
  girara_argument_mapping_add(gsession, "collapse",     COLLAPSE);
  girara_argument_mapping_add(gsession, "collapse-all", COLLAPSE_ALL);
  girara_argument_mapping_add(gsession, "down",         DOWN);
  girara_argument_mapping_add(gsession, "expand",       EXPAND);
  girara_argument_mapping_add(gsession, "expand-all",   EXPAND_ALL);
  girara_argument_mapping_add(gsession, "full-down",    FULL_DOWN);
  girara_argument_mapping_add(gsession, "full-up",      FULL_UP);
  girara_argument_mapping_add(gsession, "half-down",    HALF_DOWN);
  girara_argument_mapping_add(gsession, "half-up",      HALF_UP);
  girara_argument_mapping_add(gsession, "full-right",   FULL_RIGHT);
  girara_argument_mapping_add(gsession, "full-left",    FULL_LEFT);
  girara_argument_mapping_add(gsession, "half-right",   HALF_RIGHT);
  girara_argument_mapping_add(gsession, "half-left",    HALF_LEFT);
  girara_argument_mapping_add(gsession, "in",           ZOOM_IN);
  girara_argument_mapping_add(gsession, "left",         LEFT);
  girara_argument_mapping_add(gsession, "next",         NEXT);
  girara_argument_mapping_add(gsession, "out",          ZOOM_OUT);
  girara_argument_mapping_add(gsession, "previous",     PREVIOUS);
  girara_argument_mapping_add(gsession, "right",        RIGHT);
  girara_argument_mapping_add(gsession, "specific",     ZOOM_SPECIFIC);
  girara_argument_mapping_add(gsession, "top",          TOP);
  girara_argument_mapping_add(gsession, "up",           UP);
  girara_argument_mapping_add(gsession, "best-fit",     ADJUST_BESTFIT);
  girara_argument_mapping_add(gsession, "width",        ADJUST_WIDTH);
}

void
config_load_file(zathura_t* zathura, char* path)
{
  if (zathura == NULL || path == NULL) {
    return;
  }

  girara_config_parse(zathura->ui.session, path);
}