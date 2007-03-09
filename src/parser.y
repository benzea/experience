/*  eXperience GTK engine: parser.y
 *  
 *  Copyright (C) 2004-2005  Benjamin Berg <benjamin@sipsolutions.net>
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

%{

#include "parser.h"
#include <gdk/gdk.h>
#include <glib.h>
#include <glib-object.h>
#include <stdio.h>
#include "parsetree.h"
#include "group.h"
#include "image.h"
#include "fill.h"
#include "group_drawable.h"
#include "match.h"
#include "filter.h"
#include "dynamic_color.h"

parsestate * pst;

//offset of g_scanner token values to the yacc ones.
#define SCANNER_TOKEN_OFFSET G_TOKEN_LAST

%}

%union {
  float v_float;
  int v_int;
  char* v_str;
  gboolean v_bool;
  GdkColor v_color;
  eXperienceDynamicColor v_dynamic_color;
  eXperiencePercent   v_percent;
  
  GValue * v_g_value;
  GValueArray * v_g_value_array;
  
  guint v_uint;
}

%token <v_float>	FLOAT
%token <v_int>		INT
%token				IMAGE_LITERAL
%token				GROUP_LITERAL
%token				ICONS_LITERAL
%token <v_str>		STRING

%token				FILE_LITERAL
%token				BORDER_LITERAL
%token				REPEAT_LITERAL
%token				STRETCH_LITERAL

%token				STATE_LITERAL
%token				PADDING_LITERAL
%token				Y_POS_LITERAL
%token				X_POS_LITERAL
%token				POS_LITERAL
%token				TILE_LITERAL

%token				FUNCTION_LITERAL
%token				DETAIL_LITERAL
%token				PROGRAM_LITERAL
%token				HAS_DEFAULT_LITERAL
%token				HAS_FOCUS_LITERAL
%token				SHADOW_LITERAL
%token				ARROW_DIRECTION_LITERAL
%token				ORIENTATION_LITERAL
%token				GAP_SIZE_LITERAL
%token				GAP_SIDE_LITERAL
%token				EXPANDER_STYLE_LITERAL
%token				WINDOW_EDGE_LITERAL
%token				CONTINUE_SIDE_LITERAL
%token				NONE_LITERAL
%token				BOTH_LITERAL
%token				SINGLE_LITERAL

%token				SATURATION_LITERAL
%token				PIXELATE_LITERAL
%token				BRIGHTNESS_LITERAL
%token				OPACITY_LITERAL

%token				CLEAR_AREA_LITERAL

%token				DONT_DRAW_LITERAL
%token				DONT_CLIP_LITERAL
%token				DONT_INHERIT_LITERAL

%token				LINE_WIDTH_LITERAL

%token				RECOLOR_LITERAL
%token				RECOLOR_GRADIENT_LITERAL

%token				CENTER_LITERAL
%token				DRAW_COMPONENTS_LITERAL

%token				INTERPOLATION_TYPE_LITERAL

%token				ROUNDING_LITERAL
%token				DIRECTION__LITERAL

%token				MIRROR_LITERAL
%token				ROTATE_LITERAL
%token				TEXT_DIRECTION_LITERAL
%token				PROPERTY_LITERAL

%token				WIDTH_LITERAL
%token				HEIGHT_LITERAL

%token				INNER_PADDING_LITERAL
%token				DRAW_ENTIRE_ONLY_LITERAL

%token				FILL_LITERAL
%token				COLOR_LITERAL

%token				WS_LITERAL
%token				IS_LITERAL
%token				PX_LITERAL

%token	<v_uint>	FUNCTION_IDENTIFIER
%token	<v_uint>	STATE_IDENTIFIER
%token	<v_uint>	SHADOW_IDENTIFIER
%token	<v_uint>	ARROW_DIRECTION_IDENTIFIER
%token	<v_uint>	ORIENTATION_IDENTIFIER
%token	<v_uint>	POSITION_IDENTIFIER
%token	<v_uint>	EXPANDER_STYLE_IDENTIFIER
%token	<v_uint>	WINDOW_EDGE_IDENTIFIER
%token	<v_uint>	DRAW_COMPONENTS_IDENTIFIER

%token	<v_uint>	INTERPOLATION_TYPE_IDENTIFIER

%token	<v_uint>	STYLE_COLOR_IDENTIFIER
%token	<v_uint>	ROUNDING_TYPE_IDENTIFIER

%token	<v_uint>	ROTATE_IDENTIFIER
%token	<v_uint>	TEXT_DIRECTION_IDENTIFIER

%token <v_bool>		BOOLEAN
%token <v_uint>		LEFT_RIGHT

%token				YY_ALWAYS_KEEP_LAST

%type <v_uint>		function
%type <v_uint>		functions
%type <v_uint>		text_direction
%type <v_uint>		text_directions
%type <v_g_value> gvalue
%type <v_g_value_array> gvalues
%type <v_uint>		states
%type <v_uint>		shadows
%type <v_uint>		shadow
%type <v_uint>		arrow_directions
%type <v_uint>		arrow_direction
%type <v_uint>		orientations
%type <v_uint>		positions
%type <v_uint>		position
%type <v_uint>		expander_styles
%type <v_uint>		window_edges
%type <v_uint>		continue_side
%type <v_uint>		continue_sides
%type <v_uint>		draw_component
%type <v_uint>		draw_components
%type <v_uint>		filter_mirror

%type <v_float>		float
%type <v_float>		ufloat
%type <v_int>		uint
%type <v_int>		int
%type <v_color>		color
%type <v_dynamic_color>	dynamic_color

%type <v_percent> percent_number
%type <v_percent> percent_calc

%{

eXperienceDynamicColor dynamic_color_black = {GET_SET_COLOR, {0,0,0,0}, {0,0} };

static int experience_yyparse (void);
static int experience_yylex (void);
static int experience_yyerror (char*s);

#define PST ((parsestate*)pst)
#define CURRENT_DRAWABLE (PST != NULL ? PST->current_drawable : NULL)
#define CURRENT_IMAGE ((eXperienceImage*)CURRENT_DRAWABLE)
#define CURRENT_GROUP (PST != NULL ? PST->current_group : NULL)
#define CURRENT_FILTER (PST != NULL ? PST->current_filter : NULL)
#define CURRENT_FILL (PST != NULL ? (eXperienceFill*) PST->current_drawable : NULL)
#define CURRENT_GROUP_DRAWABLE (PST != NULL ? (eXperienceGroupDrawable*) PST->current_drawable : NULL)
#define CURRENT_GROUP_MATCH (CURRENT_GROUP != NULL ? &CURRENT_GROUP->match : NULL)

%}

%% /* Grammar rules and actions follow */
start:		  definitions '}'			{ PST->should_end = 1; }
definitions:	  definition
				| definition definitions

definition:	  group
			| icons_def

/* group definition */
group:		  GROUP_LITERAL group_naming '{' group_defs '}'	{ parse_end_group (pst); }
			| GROUP_LITERAL group_naming_i					{ parse_end_group (pst); }

group_naming:								{ parse_begin_group (pst, NULL, NULL, FALSE); }
				| STRING					{ parse_begin_group (pst, $1, NULL, FALSE); }
				| group_naming_i
group_naming_i:	  STRING '=' STRING			{ parse_begin_group (pst, $1, $3, FALSE); }
				| '=' STRING				{ parse_begin_group (pst, NULL, $2, FALSE); }

group_defs:		  group_def
				| group_def group_defs
group_def:		  match_def
				| group_property
				| drawable
				| all_filter

/* group image */

drawable:			  group_image_begin group_image_defs '}'		{ parse_end_drawable (pst); }
					| fill_begin fill_defs '}'						{ parse_end_drawable (pst); }
					| group_drawable_begin group_drawable_defs '}'	{ parse_end_drawable (pst); }

group_image_begin:	  IMAGE_LITERAL uint '{'				{ parse_begin_drawable (pst, $2, experience_image_class); }
group_image_defs:	  group_image_def
					| group_image_def group_image_defs
group_image_def:	  drawable_property
					| image_property;

fill_begin:			  FILL_LITERAL uint '{'					{ parse_begin_drawable (pst, $2, experience_fill_class); }
fill_defs:			  fill_def
					| fill_def fill_defs
fill_def:			  drawable_property
					| fill_property

group_drawable_begin: GROUP_LITERAL uint '{'				{ parse_begin_drawable (pst, $2, experience_group_drawable_class); }
group_drawable_defs:  group_drawable_def
					| group_drawable_def group_drawable_defs
group_drawable_def:	  drawable_property
					| GROUP_LITERAL '=' STRING				{ experience_group_drawable_set_group_name (CURRENT_GROUP_DRAWABLE, $3); }
/* icons */

icons_def:		  icons_begin icon_state_defs  '}'	{ parse_end_icons (pst); }
icons_begin:	  ICONS_LITERAL '{'					{ parse_begin_icons (pst); }

icon_state_defs:  icon_state_def
				| icon_state_def icon_state_defs
icon_state_def:		  icon_state_begin '{' all_filters '}'	{ parse_end_icons_state (pst); }
icon_state_begin:	  STATE_IDENTIFIER							{ parse_begin_icons_state (pst, $1); }

/* group properties */
group_property:	  CLEAR_AREA_LITERAL '=' BOOLEAN		{ experience_group_set_clear_area  (CURRENT_GROUP, $3); }
				| DONT_CLIP_LITERAL '=' BOOLEAN				{ experience_group_set_dont_clip (CURRENT_GROUP, $3); }
				| PADDING_LITERAL '=' '{' int ',' int ',' int ',' int '}'	{ experience_group_set_padding (CURRENT_GROUP, $4, $6, $8, $10); }
				| GAP_SIZE_LITERAL '=' uint					{ experience_group_set_gap_size (CURRENT_GROUP, $3); }

/* match definitions */

match_def:	  STATE_LITERAL          '=' states				{ experience_match_set_states          (CURRENT_GROUP_MATCH, $3); }
			| DETAIL_LITERAL         '=' details
			| PROGRAM_LITERAL        '=' programs
			| SHADOW_LITERAL         '=' shadows			{ experience_match_set_shadows         (CURRENT_GROUP_MATCH, $3); }
			| ARROW_DIRECTION_LITERAL'=' arrow_directions	{ experience_match_set_arrow_directions(CURRENT_GROUP_MATCH, $3); }
			| ORIENTATION_LITERAL    '=' orientations		{ experience_match_set_orientations    (CURRENT_GROUP_MATCH, $3); }
			| GAP_SIDE_LITERAL       '=' positions			{ experience_match_set_gap_sides       (CURRENT_GROUP_MATCH, $3); }
			| EXPANDER_STYLE_LITERAL '=' expander_styles	{ experience_match_set_expander_styles (CURRENT_GROUP_MATCH, $3); }
			| WINDOW_EDGE_LITERAL    '=' window_edges		{ experience_match_set_window_edges    (CURRENT_GROUP_MATCH, $3); }
			| CONTINUE_SIDE_LITERAL  '=' continue_sides		{ experience_match_set_continue_sides  (CURRENT_GROUP_MATCH, $3); }
			| FUNCTION_LITERAL       '=' functions			{ experience_match_set_functions       (CURRENT_GROUP_MATCH, $3); }
			| TEXT_DIRECTION_LITERAL '=' text_directions	{ experience_match_set_text_directions (CURRENT_GROUP_MATCH, $3); }
			| PROPERTY_LITERAL STRING '=' gvalues			{ experience_match_set_property        (CURRENT_GROUP_MATCH, $2, $4); }
			/* deprecated */
			| HAS_DEFAULT_LITERAL    '=' gvalues			{ experience_match_set_property        (CURRENT_GROUP_MATCH, g_strdup ("has-default"), $3); }
			| HAS_FOCUS_LITERAL      '=' gvalues			{ experience_match_set_property        (CURRENT_GROUP_MATCH, g_strdup ("has-focus"), $3); }

/*--*/
details:		  detail
				| detail ',' details
detail:			  STRING										{ experience_match_add_detail (CURRENT_GROUP_MATCH, $1); }
/*--*/
programs:		  program
				| program ',' programs
program:		  STRING										{ experience_match_add_program_name (CURRENT_GROUP_MATCH, $1); }
/*--*/
states:			  STATE_IDENTIFIER								{ $$ = 1 << $1; }
				| STATE_IDENTIFIER ',' states					{ $$ = 1 << $1 | $3; }
/*--*/
function:		  FUNCTION_IDENTIFIER						{ $$ = $1; }
				| SHADOW_LITERAL							{ $$ = FUNCTION_SHADOW; }

functions:		  function							{ $$ = $1; }
				| function ',' functions			{ $$ = $1 | $3; }

/*--*/

text_direction:		  TEXT_DIRECTION_IDENTIFIER				{ $$ = 1 << $1; }
					| NONE_LITERAL							{ $$ = 1 << GTK_TEXT_DIR_NONE; }
text_directions:	  text_direction						{ $$ = $1; }
					| text_direction ',' text_directions	{ $$ = $1 | $3; }

/*--*/

gvalue:			  BOOLEAN								{ $$ = g_new0 (GValue, 1); g_value_init ($$, G_TYPE_BOOLEAN); g_value_set_boolean ($$, $1); }
				| float									{ $$ = g_new0 (GValue, 1); g_value_init ($$, G_TYPE_FLOAT);   g_value_set_float   ($$, $1); }
				| STRING								{ $$ = g_new0 (GValue, 1); g_value_init ($$, G_TYPE_STRING);  g_value_take_string ($$, $1); }
gvalues:		  gvalue								{ $$ = g_value_array_new(1); g_value_array_append ($$, $1); g_value_unset ($1); g_free ($1); }
				| gvalues ',' gvalue					{ g_value_array_append ($1, $3); g_value_unset ($3); g_free ($3); $$ = $1; }
				| DONT_INHERIT_LITERAL					{ $$ = NULL; }

/*--*/
shadows:		  shadow								{ $$ = 1 << $1; }
				| shadow ',' shadows					{ $$ = 1 << $1 | $3; }
shadow:			  SHADOW_IDENTIFIER						{ $$ = $1; }
				| NONE_LITERAL							{ $$ = GTK_SHADOW_NONE; }
/*--*/
arrow_directions: arrow_direction						{ $$ = $1; }
				| arrow_direction ',' arrow_directions	{ $$ = $1 | $3; }
arrow_direction:  ARROW_DIRECTION_IDENTIFIER	{ $$ = 1 << $1; }
				| LEFT_RIGHT			{ $$ = 1 << ($1 + GTK_ARROW_LEFT); }
/*--*/
orientations:	 ORIENTATION_IDENTIFIER							{ $$ = 1 << $1; }
				| ORIENTATION_IDENTIFIER ',' orientations		{ $$ = 1 << $1 | $3; }
/*--*/
positions:		  position							{ $$ = 1 << $1; }
				| position ',' positions				{ $$ = 1 << $1 | $3; }
position:	  POSITION_IDENTIFIER		{ $$ = $1; }
				| LEFT_RIGHT			{ $$ = $1 + GTK_POS_LEFT; }
				| NONE_LITERAL		{ $$ = POS_NONE; }
/*--*/
expander_styles:  EXPANDER_STYLE_IDENTIFIER						{ $$ = 1 << $1; }
				| EXPANDER_STYLE_IDENTIFIER ',' expander_styles	{ $$ = 1 << $1 | $3; }
/*--*/
window_edges:	  WINDOW_EDGE_IDENTIFIER						{ $$ = 1 << $1; }
				| WINDOW_EDGE_IDENTIFIER ',' window_edges		{ $$ = 1 << $1 | $3; }

/*--*/

continue_side:	  NONE_LITERAL		{ $$ = EXPERIENCE_CONTINUE_SIDE_NONE; }
				| LEFT_RIGHT		{ $$ = 1 << ($1 + 1); }
				| BOTH_LITERAL		{ $$ = EXPERIENCE_CONTINUE_SIDE_BOTH; }
				| SINGLE_LITERAL	{ $$ = EXPERIENCE_CONTINUE_SIDE_SINGLE; }

continue_sides:	  continue_side							{ $$ = $1; }
				| continue_side ',' continue_sides 		{ $$ = $1 | $3; }

/* image properties */
drawable_property: 	  padding_def
					| all_filter
					| DONT_INHERIT_LITERAL '=' BOOLEAN		{ experience_drawable_set_dont_inherit (CURRENT_DRAWABLE, $3); }
					| DONT_DRAW_LITERAL    '=' BOOLEAN		{ experience_drawable_set_dont_draw (CURRENT_DRAWABLE, $3); }
					| position_def
					| repeat_def
					| inner_padding_def
					| size_def
					| DONT_CLIP_LITERAL '=' BOOLEAN						{ experience_drawable_set_dont_clip (CURRENT_DRAWABLE, $3); }
					| DRAW_ENTIRE_ONLY_LITERAL '=' BOOLEAN				{ experience_drawable_set_draw_entire_only (CURRENT_DRAWABLE, $3); }
					| ROUNDING_LITERAL '=' ROUNDING_TYPE_IDENTIFIER		{ experience_drawable_set_rounding (CURRENT_DRAWABLE, $3); }

image_property:		  file_def
					| border_def
					| draw_components_def
					| interpolation_type_def

/*--*/

fill_property:		  COLOR_LITERAL '=' dynamic_color	{ experience_fill_set_color (CURRENT_FILL, $3); }

position_def:	  POS_LITERAL   '=' percent_calc	{ experience_drawable_set_pos (CURRENT_DRAWABLE, $3, ORIENTATION_VERTICAL | ORIENTATION_HORIZONTAL); }
				| X_POS_LITERAL '=' percent_calc	{ experience_drawable_set_pos (CURRENT_DRAWABLE, $3, ORIENTATION_HORIZONTAL); }
				| Y_POS_LITERAL '=' percent_calc	{ experience_drawable_set_pos (CURRENT_DRAWABLE, $3, ORIENTATION_VERTICAL); }

repeat_def:		  REPEAT_LITERAL '=' '{' uint ',' uint ',' uint ',' uint '}'	{ experience_drawable_set_repeat (CURRENT_DRAWABLE, $4, $6, $8, $10); }
inner_padding_def: INNER_PADDING_LITERAL '=' '{' int ',' int ',' int ',' int '}'	{ experience_drawable_set_inner_padding (CURRENT_DRAWABLE, $4, $6, $8, $10); }
size_def:	  WIDTH_LITERAL  '=' percent_calc		{ experience_drawable_set_width  (CURRENT_DRAWABLE, $3); }
			| HEIGHT_LITERAL '=' percent_calc		{ experience_drawable_set_height (CURRENT_DRAWABLE, $3); }

padding_def:	  PADDING_LITERAL '=' '{' int ',' int ',' int ',' int '}'		{ experience_drawable_set_padding (CURRENT_DRAWABLE, $4, $6, $8, $10); }

border_def:		  BORDER_LITERAL '=' '{' uint ',' uint ',' uint ',' uint '}'	{ experience_image_set_border (CURRENT_IMAGE, $4, $6, $8, $10); }
file_def:		  FILE_LITERAL '=' STRING
				  { experience_image_set_file (CURRENT_IMAGE, gtk_rc_find_pixmap_in_path (PST->settings, PST->scanner, $3)); g_free ($3); }

draw_component:		  WINDOW_EDGE_IDENTIFIER				{ $$ = 1 << $1; }
					| DRAW_COMPONENTS_IDENTIFIER			{ $$ = $1; }
					| BORDER_LITERAL						{ $$ = COMPONENT_BORDER; }
					| CENTER_LITERAL						{ $$ = COMPONENT_CENTER; }

draw_components:	  draw_component						{ $$ = $1; }
					| draw_components ',' draw_component	{ $$ = $1 | $3; }
					| NONE_LITERAL							{ $$ = COMPONENT_NONE; }


draw_components_def:
				  DRAW_COMPONENTS_LITERAL '=' draw_components	{ experience_image_set_draw_components (CURRENT_IMAGE, $3); }

interpolation_type_def:
				  INTERPOLATION_TYPE_LITERAL '=' INTERPOLATION_TYPE_IDENTIFIER { experience_image_set_interp_type (CURRENT_IMAGE, $3); }
/* filter */

all_filters:			  all_filter
					| all_filter all_filters

all_filter:		  SATURATION_LITERAL '=' ufloat					{ experience_filter_set_saturation (CURRENT_FILTER, $3); }
					| OPACITY_LITERAL    '=' ufloat				{ experience_filter_set_opacity    (CURRENT_FILTER, $3); }
					| BRIGHTNESS_LITERAL '=' float				{ experience_filter_set_brightness (CURRENT_FILTER, $3); }
					| PIXELATE_LITERAL   '=' BOOLEAN			{ experience_filter_set_pixelate   (CURRENT_FILTER, $3); }
					| ROTATE_LITERAL     '=' ROTATE_IDENTIFIER	{ experience_filter_set_rotation   (CURRENT_FILTER, $3); }
					| ROTATE_LITERAL     '=' NONE_LITERAL		{ experience_filter_set_rotation   (CURRENT_FILTER,  0); }
					| MIRROR_LITERAL     '=' filter_mirrors
					|  RECOLOR_LITERAL color '=' dynamic_color	{ experience_filter_add_recolor_color (CURRENT_FILTER, $2, $4); }
					| RECOLOR_GRADIENT_LITERAL '{' dynamic_color '}' 			{ experience_filter_set_recolor_gradient (CURRENT_FILTER, $3, dynamic_color_black, dynamic_color_black); }
					| RECOLOR_GRADIENT_LITERAL '{' dynamic_color ',' dynamic_color '}' 	{ experience_filter_set_recolor_gradient (CURRENT_FILTER, $3, $5, dynamic_color_black); }
					| RECOLOR_GRADIENT_LITERAL '{' dynamic_color ',' dynamic_color ',' dynamic_color '}' { experience_filter_set_recolor_gradient (CURRENT_FILTER, $3, $5, $7); }

/* other */
ufloat:	  FLOAT				{ $$ = $1; }
		| INT				{ $$ = $1; }
float:	'-' float			{ $$ = - $2; }
		| ufloat			{ $$ = $1; }
		| CENTER_LITERAL	{ $$ = 0; }

uint:	  INT				{ $$ = $1; }
int:	  uint				{ $$ = $1; }
		| '-' int			{ $$ = - $2; }

color:	  '{' FLOAT ',' FLOAT ',' FLOAT '}'		{ $$.red = CLAMP_COLOR ($2 * GDK_COLOR_MAX); $$.green = CLAMP_COLOR ($4 * GDK_COLOR_MAX); $$.red = CLAMP_COLOR ($6 * GDK_COLOR_MAX); }
		| '{' uint  ',' uint  ',' uint  '}'		{ $$.red = CLAMP_COLOR ($2); $$.green = CLAMP_COLOR ($4); $$.red = CLAMP_COLOR($6); }
		| STRING								{ if (!gdk_color_parse ($1, &$$)) { YYERROR; } g_free ($1); }

dynamic_color:	color										{ $$.source = GET_SET_COLOR; $$.output_color = $1; }
		| STYLE_COLOR_IDENTIFIER '[' STATE_IDENTIFIER ']'	{ $$.source = GET_STYLE_COLOR; $$.output_style_color.color_array = $1; $$.output_style_color.state = $3; }


filter_mirror:	  ORIENTATION_IDENTIFIER			{ $$ = $1; }
				| NONE_LITERAL						{ $$ = 0; }

filter_mirrors:	  filter_mirror						{ experience_filter_add_mirror (CURRENT_FILTER, 1 << $1); }
				| filter_mirror ',' filter_mirrors	{ experience_filter_add_mirror (CURRENT_FILTER, 1 << $1); }

percent_number:
				  float			{ $$.pixel = 0;  $$.widget = $1; }
				| float WS_LITERAL	{ $$.pixel = 0;  $$.widget = $1; $$.object = 0; }
				| int PX_LITERAL	{ $$.pixel = $1; $$.widget = 0;  $$.object = 0; }
				| int IS_LITERAL	{ $$.pixel = 0;  $$.widget = 0;  $$.object = $1;}
percent_calc: percent_number						{ $$ = $1; }
				| percent_number '+' percent_calc	{ $$.widget = $1.widget + $3.widget; $$.pixel = $1.pixel + $3.pixel; }
				| percent_number '-' percent_calc 	{ $$.widget = $1.widget - $3.widget; $$.pixel = $1.pixel - $3.pixel; }

;

%%

#include <ctype.h>
#include <stdio.h>
#include "symbols.h"

int
experience_yylex (void)
{
	if (pst->should_end)
		return 0;
	
	GTokenType token = g_scanner_get_next_token (pst->scanner);
	switch (token) {
		case G_TOKEN_ERROR:
			yyerror("g_scanner returned an error!\n");
		case G_TOKEN_INT:
			yylval.v_int = g_scanner_cur_value (pst->scanner).v_int;
			return INT;
		case G_TOKEN_FLOAT:
			yylval.v_float = g_scanner_cur_value (pst->scanner).v_float;
			return FLOAT;
		case G_TOKEN_STRING:
			yylval.v_str = g_strdup (g_scanner_cur_value (pst->scanner).v_string);
			return STRING;
		default:
			if (token > SCANNER_TOKEN_OFFSET) {
				token = token - SCANNER_TOKEN_OFFSET;
			}
			
			if ((token >= OTHER_SYMBOLS_START) && (token <= OTHER_SYMBOLS_END))
			{
				yylval.v_uint = other_symbols[token - OTHER_SYMBOLS_START].value;
				return other_symbols[token - OTHER_SYMBOLS_START].identifier;
			}
			
			return token;
	}
}

void
experience_read_gtkrcstyle (parsestate * new_pst)
{
	guint old_scope;
	guint i;
	GScannerConfig old_config;
	
	pst = new_pst;
	
	g_assert(pst);
	g_assert(pst->scanner);
	
	old_config = *pst->scanner->config;
	
	old_scope = g_scanner_set_scope(pst->scanner, experience_g_quark);

	pst->scanner->config->case_sensitive = FALSE;

	if (!g_scanner_lookup_symbol(pst->scanner, theme_symbols[0].name))
	{
		for (i = 0; i < G_N_ELEMENTS (theme_symbols); i++)
			g_scanner_scope_add_symbol (pst->scanner, experience_g_quark, theme_symbols[i].name, GINT_TO_POINTER(theme_symbols[i].token + SCANNER_TOKEN_OFFSET));
		for (i = 0; i < G_N_ELEMENTS (other_symbols); i++)
			g_scanner_scope_add_symbol (pst->scanner, experience_g_quark, other_symbols[i].name, GINT_TO_POINTER(OTHER_SYMBOLS_START + i + SCANNER_TOKEN_OFFSET));
	}
	
	pst->should_end = 0;
	
	if (experience_yyparse ()) {
		/* There was an error ... */
		experience_warning("There was a parser error. No warnings will be displayed to have a cleaner output.\n");
		parser_error = TRUE;
		if (pst->expected_token == G_TOKEN_NONE) {
			/* ... so make sure that the user gets an error message for the correct error. */
			pst->expected_token = G_TOKEN_IDENTIFIER;
		}
	}
	
	g_scanner_set_scope (pst->scanner, old_scope);
	*pst->scanner->config = old_config;
}

#include <stdio.h>

int
experience_yyerror (char*s)  /* Called by experience_yyparse on error */
{
  experience_warning ("%s\n", s);
  return 0;
}
