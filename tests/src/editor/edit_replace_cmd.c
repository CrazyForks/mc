/*
   src/editor - tests for edit_replace_cmd() function

   Copyright (C) 2025
   Free Software Foundation, Inc.

   Written by:
   Andrew Borodin <aborodin@vmail.ru>, 2025

   This file is part of the Midnight Commander.

   The Midnight Commander is free software: you can redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of the License,
   or (at your option) any later version.

   The Midnight Commander is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#define TEST_SUITE_NAME "/src/editor"

#include "tests/mctest.h"

#include <ctype.h>

#ifdef HAVE_CHARSET
#    include "lib/charsets.h"
#    include "src/selcodepage.h"
#endif
#include "src/editor/editwidget.h"
#include "src/editor/editmacros.h"  // edit_load_macro_cmd()
#include "src/editor/editsearch.h"

static WGroup owner;
static WEdit *test_edit;
static gboolean only_in_selection = FALSE;

static const char *test_regex_in = "qwe\n"     //
                                   "qwe\n"     //
                                   "qwe\n"     //
                                   "qwe\n"     //
                                   "qwe\n"     //
                                   "qwe\n"     //
                                   "qwe\n"     //
                                   "qwe\n";    //
static const char *test_regex_out = "Xqwe\n"   //
                                    "Xqwe\n"   //
                                    "Xqwe\n"   //
                                    "Xqwe\n"   //
                                    "Xqwe\n"   //
                                    "Xqwe\n"   //
                                    "Xqwe\n"   //
                                    "Xqwe\n";  //

static const char *test_regex_selected_in = "qwe\n"     //
                                            "qwe\n"     //
                                            "qwe\n"     // selected, mark1 = 8 (begin of line) or 9
                                            "qwe\n"     // selected
                                            "qwe\n"     // selected
                                            "qwe\n"     // mark2 = 20, begin of line
                                            "qwe\n"     //
                                            "qwe\n";    //
static const char *test1_regex_selected_out = "qwe\n"   //
                                              "qwe\n"   //
                                              "Xqwe\n"  // selected, mark1 = 8 (begin of line)
                                              "Xqwe\n"  // selected
                                              "Xqwe\n"  // selected
                                              "qwe\n"   // mark2 = 20, begin of line
                                              "qwe\n"   //
                                              "qwe\n";  //
static const char *test2_regex_selected_out = "qwe\n"   //
                                              "qwe\n"   //
                                              "qwe\n"   // selected, mark1 = 9 (not begin of line)
                                              "Xqwe\n"  // selected
                                              "Xqwe\n"  // selected
                                              "qwe\n"   // mark2 = 20, begin of line
                                              "qwe\n"   //
                                              "qwe\n";  //

static const char *replace_regex_from = "^.*";
static const char *replace_regex_to = "X\\0";

/* --------------------------------------------------------------------------------------------- */

void edit_dialog_replace_show (WEdit *edit, const char *search_default, const char *replace_default,
                               char **search_text, char **replace_text);
int edit_dialog_replace_prompt_show (WEdit *edit, char *from_text, char *to_text, int xpos,
                                     int ypos);

/* --------------------------------------------------------------------------------------------- */
/* @Mock */
void
message (int flags, const char *title, const char *text, ...)
{
    (void) flags;
    (void) title;
    (void) text;
}

/* --------------------------------------------------------------------------------------------- */
/* @Mock */
void
status_msg_init (status_msg_t *sm, const char *title, double delay, status_msg_cb init_cb,
                 status_msg_update_cb update_cb, status_msg_cb deinit_cb)
{
    (void) sm;
    (void) title;
    (void) delay;
    (void) init_cb;
    (void) update_cb;
    (void) deinit_cb;
}

/* --------------------------------------------------------------------------------------------- */
/* @Mock */
void
status_msg_deinit (status_msg_t *sm)
{
    (void) sm;
}

/* --------------------------------------------------------------------------------------------- */
/* @Mock */
mc_search_cbret_t
edit_search_update_callback (const void *user_data, off_t char_offset)
{
    (void) user_data;
    (void) char_offset;

    return MC_SEARCH_CB_OK;
}

/* --------------------------------------------------------------------------------------------- */
/* @Mock */
void
edit_dialog_replace_show (WEdit *edit, const char *search_default, const char *replace_default,
                          char **search_text, char **replace_text)
{
    (void) edit;
    (void) search_default;
    (void) replace_default;

    *search_text = g_strdup (replace_regex_from);
    *replace_text = g_strdup (replace_regex_to);

    edit_search_options.type = MC_SEARCH_T_REGEX;
    edit_search_options.only_in_selection = only_in_selection;
}

/* --------------------------------------------------------------------------------------------- */
/* @Mock */
int
edit_dialog_replace_prompt_show (WEdit *edit, char *from_text, char *to_text, int xpos, int ypos)
{
    (void) edit;
    (void) from_text;
    (void) to_text;
    (void) xpos;
    (void) ypos;

    return B_REPLACE_ALL;
}

/* --------------------------------------------------------------------------------------------- */
/* @Mock */
void
edit_load_syntax (WEdit *edit, GPtrArray *pnames, const char *type)
{
    (void) edit;
    (void) pnames;
    (void) type;
}

/* --------------------------------------------------------------------------------------------- */

/* @Mock */
int
edit_get_syntax_color (WEdit *edit, off_t byte_index)
{
    (void) edit;
    (void) byte_index;

    return 0;
}

/* --------------------------------------------------------------------------------------------- */

/* @Mock */
gboolean
edit_load_macro_cmd (WEdit *edit)
{
    (void) edit;

    return FALSE;
}

/* --------------------------------------------------------------------------------------------- */

/* @Before */
static void
setup (void)
{
    WRect r;

    str_init_strings (NULL);

#ifdef HAVE_CHARSET
    mc_global.sysconfig_dir = (char *) TEST_SHARE_DIR;
    load_codepages_list ();
#endif

    edit_options.filesize_threshold = (char *) "64M";

    rect_init (&r, 0, 0, 24, 80);
    test_edit = edit_init (NULL, &r, NULL);
    memset (&owner, 0, sizeof (owner));
    group_add_widget (&owner, WIDGET (test_edit));

    mc_global.source_codepage = 0;
    mc_global.display_codepage = 0;
#ifdef HAVE_CHARSET
    cp_source = "ASCII";
    cp_display = "ASCII";
#endif

    do_set_codepage (0);
    edit_set_codeset (test_edit);
}

/* --------------------------------------------------------------------------------------------- */

/* @After */
static void
teardown (void)
{
    edit_clean (test_edit);
    group_remove_widget (test_edit);
    g_free (test_edit);

#ifdef HAVE_CHARSET
    free_codepages_list ();
#endif

    str_uninit_strings ();
}

/* --------------------------------------------------------------------------------------------- */

static void
test_replace_check (const char *test_out)
{
    GString *actual_replaced_str;

    actual_replaced_str = g_string_new ("");

    for (off_t i = 0; i < test_edit->buffer.size; i++)
    {
        const int chr = edit_buffer_get_byte (&test_edit->buffer, i);

        g_string_append_c (actual_replaced_str, chr);
    }

    mctest_assert_str_eq (actual_replaced_str->str, test_out);
    g_string_free (actual_replaced_str, TRUE);
}

/* --------------------------------------------------------------------------------------------- */

START_TEST (test_replace_regex)
{
    // given
    only_in_selection = FALSE;
    test_edit->mark1 = 0;
    test_edit->mark2 = 0;

    for (const char *ti = test_regex_in; *ti != '\0'; ti++)
        edit_buffer_insert (&test_edit->buffer, *ti);

    // when
    edit_cursor_move (test_edit, 0);
    edit_replace_cmd (test_edit, FALSE);

    // then
    test_replace_check (test_regex_out);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

START_TEST (test1_replace_regex_in_selection)
{
    // given
    only_in_selection = TRUE;
    test_edit->mark1 = 8;
    test_edit->mark2 = 20;

    for (const char *ti = test_regex_selected_in; *ti != '\0'; ti++)
        edit_buffer_insert (&test_edit->buffer, *ti);

    // when
    edit_cursor_move (test_edit, 0);
    edit_replace_cmd (test_edit, FALSE);

    // then
    test_replace_check (test1_regex_selected_out);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

START_TEST (test2_replace_regex_in_selection)
{
    // given
    only_in_selection = TRUE;
    test_edit->mark1 = 9;
    test_edit->mark2 = 20;

    for (const char *ti = test_regex_selected_in; *ti != '\0'; ti++)
        edit_buffer_insert (&test_edit->buffer, *ti);

    // when
    edit_cursor_move (test_edit, 0);
    edit_replace_cmd (test_edit, FALSE);

    // then
    test_replace_check (test2_regex_selected_out);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

int
main (void)
{
    TCase *tc_core;

    tc_core = tcase_create ("Core");

    tcase_add_checked_fixture (tc_core, setup, teardown);

    // Add new tests here: ***************
    tcase_add_test (tc_core, test_replace_regex);
    tcase_add_test (tc_core, test1_replace_regex_in_selection);
    tcase_add_test (tc_core, test2_replace_regex_in_selection);
    // ***********************************

    return mctest_run_all (tc_core);
}

/* --------------------------------------------------------------------------------------------- */
