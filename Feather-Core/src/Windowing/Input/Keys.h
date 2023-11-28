#pragma once

#include <SDL.h>

constexpr int KEY_UNKNOWN			= -1;

constexpr int F_KEY_BACKSPACE		= SDLK_BACKSPACE;
constexpr int F_KEY_TAB				= SDLK_TAB;
constexpr int F_KEY_CLEAR			= SDLK_CLEAR;
constexpr int F_KEY_RETURN			= SDLK_RETURN;
constexpr int F_KEY_PAUSE			= SDLK_PAUSE;
constexpr int F_KEY_ESCAPE			= SDLK_ESCAPE;
constexpr int F_KEY_SPACE			= SDLK_SPACE;
constexpr int F_KEY_EXCLAIM			= SDLK_EXCLAIM;
constexpr int F_KEY_QUOTEDBL		= SDLK_QUOTEDBL;
constexpr int F_KEY_HASH			= SDLK_HASH;
constexpr int F_KEY_DOLLAR			= SDLK_DOLLAR;
constexpr int F_KEY_AMPERSAND		= SDLK_AMPERSAND;
constexpr int F_KEY_QUOTE			= SDLK_QUOTE;
constexpr int F_KEY_LEFTPAREN		= SDLK_LEFTPAREN;
constexpr int F_KEY_RIGHTPAREN		= SDLK_RIGHTPAREN;
constexpr int F_KEY_ASTERISK		= SDLK_ASTERISK;
constexpr int F_KEY_PLUS			= SDLK_PLUS;
constexpr int F_KEY_COMMA			= SDLK_COMMA;
constexpr int F_KEY_MINUS			= SDLK_MINUS;
constexpr int F_KEY_PERIOD			= SDLK_PERIOD;
constexpr int F_KEY_SLASH			= SDLK_SLASH;
constexpr int F_KEY_0				= SDLK_0;
constexpr int F_KEY_1				= SDLK_1;
constexpr int F_KEY_2				= SDLK_2;
constexpr int F_KEY_3				= SDLK_3;
constexpr int F_KEY_4				= SDLK_4;
constexpr int F_KEY_5				= SDLK_5;
constexpr int F_KEY_6				= SDLK_6;
constexpr int F_KEY_7				= SDLK_7;
constexpr int F_KEY_8				= SDLK_8;
constexpr int F_KEY_9				= SDLK_9;
constexpr int F_KEY_COLON			= SDLK_COLON;
constexpr int F_KEY_SEMICOLON		= SDLK_SEMICOLON;
constexpr int F_KEY_LESS			= SDLK_LESS;
constexpr int F_KEY_EQUALS			= SDLK_EQUALS;
constexpr int F_KEY_GREATER			= SDLK_GREATER;
constexpr int F_KEY_QUESTION		= SDLK_QUESTION;
constexpr int F_KEY_AT				= SDLK_AT;

constexpr int F_KEY_LEFTBRACKET		= SDLK_LEFTBRACKET;
constexpr int F_KEY_BACKSLASH		= SDLK_BACKSLASH;
constexpr int F_KEY_RIGHTBRACKET	= SDLK_RIGHTBRACKET;
constexpr int F_KEY_CARET			= SDLK_CARET;
constexpr int F_KEY_UNDERSCORE		= SDLK_UNDERSCORE;
constexpr int F_KEY_BACKQUOTE		= SDLK_BACKQUOTE;
constexpr int F_KEY_A				= SDLK_a;
constexpr int F_KEY_B				= SDLK_b;
constexpr int F_KEY_C				= SDLK_c;
constexpr int F_KEY_D				= SDLK_d;
constexpr int F_KEY_E				= SDLK_e;
constexpr int F_KEY_F				= SDLK_f;
constexpr int F_KEY_G				= SDLK_g;
constexpr int F_KEY_H				= SDLK_h;
constexpr int F_KEY_I				= SDLK_i;
constexpr int F_KEY_J				= SDLK_j;
constexpr int F_KEY_K				= SDLK_k;
constexpr int F_KEY_L				= SDLK_l;
constexpr int F_KEY_M				= SDLK_m;
constexpr int F_KEY_N				= SDLK_n;
constexpr int F_KEY_O				= SDLK_o;
constexpr int F_KEY_P				= SDLK_p;
constexpr int F_KEY_Q				= SDLK_q;
constexpr int F_KEY_R				= SDLK_r;
constexpr int F_KEY_S				= SDLK_s;
constexpr int F_KEY_T				= SDLK_t;
constexpr int F_KEY_U				= SDLK_u;
constexpr int F_KEY_V				= SDLK_v;
constexpr int F_KEY_W				= SDLK_w;
constexpr int F_KEY_X				= SDLK_x;
constexpr int F_KEY_Y				= SDLK_y;
constexpr int F_KEY_Z				= SDLK_z;
constexpr int F_KEY_DELETE			= SDLK_DELETE;

constexpr int F_KEY_CAPSLOCK		= SDLK_CAPSLOCK;

constexpr int F_KEY_F1				= SDLK_F1;
constexpr int F_KEY_F2				= SDLK_F2;
constexpr int F_KEY_F3				= SDLK_F3;
constexpr int F_KEY_F4				= SDLK_F4;
constexpr int F_KEY_F5				= SDLK_F5;
constexpr int F_KEY_F6				= SDLK_F6;
constexpr int F_KEY_F7				= SDLK_F7;
constexpr int F_KEY_F8				= SDLK_F8;
constexpr int F_KEY_F9				= SDLK_F9;
constexpr int F_KEY_F10				= SDLK_F10;
constexpr int F_KEY_F11				= SDLK_F11;
constexpr int F_KEY_F12				= SDLK_F12;

constexpr int F_KEY_SCROLLOCK		= SDLK_SCROLLLOCK;

constexpr int F_KEY_INSERT			= SDLK_INSERT;
constexpr int F_KEY_HOME			= SDLK_HOME;
constexpr int F_KEY_PAGEUP			= SDLK_PAGEUP;
constexpr int F_KEY_END				= SDLK_END;
constexpr int F_KEY_PAGEDOWN		= SDLK_PAGEDOWN;

constexpr int F_KEY_RIGHT			= SDLK_RIGHT;
constexpr int F_KEY_LEFT			= SDLK_LEFT;
constexpr int F_KEY_DOWN			= SDLK_DOWN;
constexpr int F_KEY_UP				= SDLK_UP;

constexpr int F_KEY_NUMLOCK			= SDLK_NUMLOCKCLEAR;
constexpr int F_KEY_KP_DIVIDE		= SDLK_KP_DIVIDE;
constexpr int F_KEY_KP_MULTIPLY		= SDLK_KP_MULTIPLY;
constexpr int F_KEY_KP_MINUS		= SDLK_KP_MINUS;
constexpr int F_KEY_KP_PLUS			= SDLK_KP_PLUS;
constexpr int F_KEY_KP_ENTER		= SDLK_KP_ENTER;
constexpr int F_KEY_KP1				= SDLK_KP_1;
constexpr int F_KEY_KP2				= SDLK_KP_2;
constexpr int F_KEY_KP3				= SDLK_KP_3;
constexpr int F_KEY_KP4				= SDLK_KP_4;
constexpr int F_KEY_KP5				= SDLK_KP_5;
constexpr int F_KEY_KP6				= SDLK_KP_6;
constexpr int F_KEY_KP7				= SDLK_KP_7;
constexpr int F_KEY_KP8				= SDLK_KP_8;
constexpr int F_KEY_KP9				= SDLK_KP_9;
constexpr int F_KEY_KP0				= SDLK_KP_0;
constexpr int F_KEY_KP_PERIOD		= SDLK_KP_PERIOD;

constexpr int F_KEY_LCTRL			= SDLK_LCTRL;
constexpr int F_KEY_LSHIFT			= SDLK_LSHIFT;
constexpr int F_KEY_LALT			= SDLK_LALT;
constexpr int F_KEY_RCTRL			= SDLK_RCTRL;
constexpr int F_KEY_RSHIFT			= SDLK_RSHIFT;
constexpr int F_KEY_RALT			= SDLK_RALT;
