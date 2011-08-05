// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


LRESULT WINAPI RenderWindow_MessageProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern LRESULT (WINAPI *User_MessageProc)(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);


