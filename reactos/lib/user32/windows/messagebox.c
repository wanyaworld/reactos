/*
 *  ReactOS kernel
 *  Copyright (C) 1998, 1999, 2000, 2001 ReactOS Team
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
/* $Id: messagebox.c,v 1.8 2003/07/28 08:09:51 ekohl Exp $
 *
 * PROJECT:         ReactOS user32.dll
 * FILE:            lib/user32/windows/messagebox.c
 * PURPOSE:         Input
 * PROGRAMMER:      Casper S. Hornstrup (chorns@users.sourceforge.net)
 *                  Thomas Weidenmueller (w3seek@users.sourceforge.net)
 * UPDATE HISTORY:
 *      07-27-2003  Code ported from wine
 *      09-05-2001  CSH  Created
 */

/* INCLUDES ******************************************************************/

#include <windows.h>
#include <messages.h>
#include <user32.h>
#include <string.h>
#include <ntos/rtl.h>
#include <stdio.h>
#include <stdlib.h>
#include <debug.h>


/* DEFINES *******************************************************************/

#define MSGBOX_IDICON 1088
#define MSGBOX_IDTEXT 100
#define IDS_ERROR     2

#define RT_DIALOGA         MAKEINTRESOURCEA(5)
#define RT_DIALOGW         MAKEINTRESOURCEW(5)
#define IDI_HANDA        MAKEINTRESOURCEA(32513)
#define IDI_HANDW        MAKEINTRESOURCEW(32513)
#define IDI_QUESTIONA    MAKEINTRESOURCEA(32514)
#define IDI_QUESTIONW    MAKEINTRESOURCEW(32514)
#define IDI_EXCLAMATIONA MAKEINTRESOURCEA(32515)
#define IDI_EXCLAMATIONW MAKEINTRESOURCEW(32515)
#define IDI_ASTERISKA    MAKEINTRESOURCEA(32516)
#define IDI_ASTERISKW    MAKEINTRESOURCEW(32516)
#define IDI_WINLOGOA       MAKEINTRESOURCEA(32517)
#define IDI_WINLOGOW       MAKEINTRESOURCEW(32517)

#ifndef MB_TYPEMASK
#define MB_TYPEMASK             0x0000000F
#endif
#ifndef MB_ICONMASK
#define MB_ICONMASK             0x000000F0
#endif
#ifndef MB_DEFMASK
#define MB_DEFMASK              0x00000F00
#endif


/* FUNCTIONS *****************************************************************/

static HFONT MSGBOX_OnInit(HWND hwnd, LPMSGBOXPARAMS lpmb)
{
    HFONT hFont = 0, hPrevFont = 0;
    RECT rect;
    HWND hItem;
    HDC hdc;
    int i, buttons;
    int bspace, bw, bh, theight, tleft, wwidth, wheight, bpos;
    int borheight, borwidth, iheight, ileft, iwidth, twidth, tiheight;
    LPCWSTR lpszText;
    WCHAR buf[256];
    NONCLIENTMETRICSW nclm;

    nclm.cbSize = sizeof(nclm);
    SystemParametersInfoW (SPI_GETNONCLIENTMETRICS, 0, &nclm, 0);
    hFont = CreateFontIndirectW (&nclm.lfMessageFont);
    /* set button font */
    for (i=1; i < 8; i++)
        SendDlgItemMessageW (hwnd, i, WM_SETFONT, (WPARAM)hFont, 0);
    /* set text font */
    SendDlgItemMessageW (hwnd, MSGBOX_IDTEXT, WM_SETFONT, (WPARAM)hFont, 0);

    if (HIWORD(lpmb->lpszCaption))
    {
        SetWindowTextW(hwnd, (LPCWSTR)lpmb->lpszCaption);
    }
    else
    {
        UINT res_id = LOWORD((UINT)lpmb->lpszCaption); /* FIXME: (UINT) ??? */
        if (res_id)
        {
            if (LoadStringW(lpmb->hInstance, res_id, buf, 256))
                SetWindowTextW(hwnd, buf);
        }
        else
        {
            if (LoadStringW(GetModuleHandleA("user32.dll"), IDS_ERROR, buf, 256))
                SetWindowTextW(hwnd, buf);
        }
    }
    if (HIWORD(lpmb->lpszText))
    {
        lpszText = (LPCWSTR)lpmb->lpszText;
    }
    else
    {
       lpszText = buf;
       if (!LoadStringW(lpmb->hInstance, LOWORD((UINT)lpmb->lpszText), buf, 256)) /* FIXME: (UINT) ??? */
           *buf = 0;	/* FIXME ?? */
    }
    SetWindowTextW(GetDlgItem(hwnd, MSGBOX_IDTEXT), lpszText);

    /* Hide not selected buttons */
    switch(lpmb->dwStyle & MB_TYPEMASK) {
    case MB_OK:
	ShowWindow(GetDlgItem(hwnd, IDCANCEL), SW_HIDE);
	/* fall through */
    case MB_OKCANCEL:
	ShowWindow(GetDlgItem(hwnd, IDABORT), SW_HIDE);
	ShowWindow(GetDlgItem(hwnd, IDRETRY), SW_HIDE);
	ShowWindow(GetDlgItem(hwnd, IDIGNORE), SW_HIDE);
	ShowWindow(GetDlgItem(hwnd, IDYES), SW_HIDE);
	ShowWindow(GetDlgItem(hwnd, IDNO), SW_HIDE);
	break;
    case MB_ABORTRETRYIGNORE:
	ShowWindow(GetDlgItem(hwnd, IDOK), SW_HIDE);
	ShowWindow(GetDlgItem(hwnd, IDCANCEL), SW_HIDE);
	ShowWindow(GetDlgItem(hwnd, IDYES), SW_HIDE);
	ShowWindow(GetDlgItem(hwnd, IDNO), SW_HIDE);
	break;
    case MB_YESNO:
	ShowWindow(GetDlgItem(hwnd, IDCANCEL), SW_HIDE);
	/* fall through */
    case MB_YESNOCANCEL:
	ShowWindow(GetDlgItem(hwnd, IDOK), SW_HIDE);
	ShowWindow(GetDlgItem(hwnd, IDABORT), SW_HIDE);
	ShowWindow(GetDlgItem(hwnd, IDRETRY), SW_HIDE);
	ShowWindow(GetDlgItem(hwnd, IDIGNORE), SW_HIDE);
	break;
    case MB_RETRYCANCEL:
	ShowWindow(GetDlgItem(hwnd, IDOK), SW_HIDE);
	ShowWindow(GetDlgItem(hwnd, IDABORT), SW_HIDE);
	ShowWindow(GetDlgItem(hwnd, IDIGNORE), SW_HIDE);
	ShowWindow(GetDlgItem(hwnd, IDYES), SW_HIDE);
	ShowWindow(GetDlgItem(hwnd, IDNO), SW_HIDE);
	break;
    }
	/* Set the icon */
    switch(lpmb->dwStyle & MB_ICONMASK) {
    case MB_ICONEXCLAMATION:
	SendDlgItemMessageW(hwnd, 0x0440, STM_SETICON,
			    (WPARAM)LoadIconW(0, IDI_EXCLAMATIONW), 0);
	break;
    case MB_ICONQUESTION:
	SendDlgItemMessageW(hwnd, 0x0440, STM_SETICON,
			    (WPARAM)LoadIconW(0, IDI_QUESTIONW), 0);
	break;
    case MB_ICONASTERISK:
	SendDlgItemMessageW(hwnd, 0x0440, STM_SETICON,
			    (WPARAM)LoadIconW(0, IDI_ASTERISKW), 0);
	break;
    case MB_ICONHAND:
      SendDlgItemMessageW(hwnd, 0x0440, STM_SETICON,
			    (WPARAM)LoadIconW(0, IDI_HANDW), 0);
      break;
    case MB_USERICON:
      SendDlgItemMessageW(hwnd, 0x0440, STM_SETICON,
			  (WPARAM)LoadIconW(lpmb->hInstance, (LPCWSTR)lpmb->lpszIcon), 0);
      break;
    default:
	/* By default, Windows 95/98/NT do not associate an icon to message boxes.
	 * So wine should do the same.
	 */
	break;
    }

    /* Position everything */
    GetWindowRect(hwnd, &rect);
    borheight = rect.bottom - rect.top;
    borwidth  = rect.right - rect.left;
    GetClientRect(hwnd, &rect);
    borheight -= rect.bottom - rect.top;
    borwidth  -= rect.right - rect.left;

    /* Get the icon height */
    GetWindowRect(GetDlgItem(hwnd, MSGBOX_IDICON), &rect);
    MapWindowPoints(0, hwnd, (LPPOINT)&rect, 2);
    if (!(lpmb->dwStyle & MB_ICONMASK))
    {
        rect.bottom = rect.top;
        rect.right = rect.left;
    }
    iheight = rect.bottom - rect.top;
    ileft = rect.left;
    iwidth = rect.right - ileft;

    hdc = GetDC(hwnd);
    if (hFont)
        hPrevFont = SelectObject(hdc, hFont);

    /* Get the number of visible buttons and their size */
    bh = bw = 1; /* Minimum button sizes */
    for (buttons = 0, i = 1; i < 8; i++)
    {
        hItem = GetDlgItem(hwnd, i);
        if (GetWindowLongW(hItem, GWL_STYLE) & WS_VISIBLE)
        {
            WCHAR buttonText[1024];
            int w, h;
            buttons++;
            if (GetWindowTextW(hItem, buttonText, 1024))
            {
                DrawTextW( hdc, buttonText, -1, &rect, DT_LEFT | DT_EXPANDTABS | DT_CALCRECT);
                h = rect.bottom - rect.top;
                w = rect.right - rect.left;
                if (h > bh)
                    bh = h;
                if (w > bw)
                    bw = w ;
            }
        }
    }
    bw = max(bw, bh * 2);
    /* Button white space */
    bh = bh * 2;
    bw = bw * 2;
    bspace = bw/3; /* Space between buttons */

    /* Get the text size */
    GetClientRect(GetDlgItem(hwnd, MSGBOX_IDTEXT), &rect);
    rect.top = rect.left = rect.bottom = 0;
    DrawTextW( hdc, lpszText, -1, &rect,
              DT_LEFT | DT_EXPANDTABS | DT_WORDBREAK | DT_CALCRECT);
    /* Min text width corresponds to space for the buttons */
    tleft = ileft;
    if (iwidth)
        tleft += ileft + iwidth;
    twidth = max((bw + bspace) * buttons + bspace - tleft, rect.right);
    theight = rect.bottom;

    if (hFont)
        SelectObject(hdc, hPrevFont);
    ReleaseDC(hItem, hdc);

    tiheight = 16 + max(iheight, theight);
    wwidth  = tleft + twidth + ileft + borwidth;
    wheight = 8 + tiheight + bh + borheight;

    /* Resize the window */
    SetWindowPos(hwnd, 0, 0, 0, wwidth, wheight,
                 SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);

    /* Position the icon */
    SetWindowPos(GetDlgItem(hwnd, MSGBOX_IDICON), 0, ileft, (tiheight - iheight) / 2, 0, 0,
                 SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);

    /* Position the text */
    SetWindowPos(GetDlgItem(hwnd, MSGBOX_IDTEXT), 0, tleft, (tiheight - theight) / 2, twidth, theight,
                 SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);

    /* Position the buttons */
    bpos = (wwidth - (bw + bspace) * buttons + bspace) / 2;
    for (buttons = i = 0; i < 7; i++)
    {
        /* some arithmetic to get the right order for YesNoCancel windows */
        hItem = GetDlgItem(hwnd, (i + 5) % 7 + 1);
        if (GetWindowLongW(hItem, GWL_STYLE) & WS_VISIBLE)
        {
            if (buttons++ == ((lpmb->dwStyle & MB_DEFMASK) >> 8))
            {
                SetFocus(hItem);
                SendMessageW( hItem, BM_SETSTYLE, BS_DEFPUSHBUTTON, TRUE );
            }
            SetWindowPos(hItem, 0, bpos, tiheight, bw, bh,
                         SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);
            bpos += bw + bspace;
        }
    }

    /* handle modal MessageBoxes */
    if (lpmb->dwStyle & (MB_TASKMODAL|MB_SYSTEMMODAL))
    {
        DbgPrint("%s modal msgbox ! Not modal yet.\n",
                 lpmb->dwStyle & MB_TASKMODAL ? "task" : "system");
        /* Probably do EnumTaskWindows etc. here for TASKMODAL
         * and work your way up to the top - I'm lazy (HWND_TOP) */
        SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0,
                     SWP_NOSIZE | SWP_NOMOVE);
        if (lpmb->dwStyle & MB_TASKMODAL)
            /* at least MB_TASKMODAL seems to imply a ShowWindow */
            ShowWindow(hwnd, SW_SHOW);
    }

    if (lpmb->dwStyle & MB_APPLMODAL)
        DbgPrint("app modal msgbox ! Not modal yet.\n");

    return hFont;
}


/**************************************************************************
 *           MSGBOX_DlgProc
 *
 * Dialog procedure for message boxes.
 */
static INT_PTR CALLBACK MSGBOX_DlgProc( HWND hwnd, UINT message,
                                        WPARAM wParam, LPARAM lParam )
{
  HFONT hFont;

  switch(message)
  {
    case WM_INITDIALOG:
    {
       LPMSGBOXPARAMS mbp = (LPMSGBOXPARAMS)lParam;
       SetWindowContextHelpId(hwnd, mbp->dwContextHelpId);
       hFont = MSGBOX_OnInit(hwnd, mbp);
       SetPropA(hwnd, "ROS_MSGBOX_HFONT", (HANDLE)hFont);
       SetPropA(hwnd, "ROS_MSGBOX_HELPCALLBACK", (HANDLE)mbp->lpfnMsgBoxCallback);
       break;
    }

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDOK:
        case IDCANCEL:
        case IDABORT:
        case IDRETRY:
        case IDIGNORE:
        case IDYES:
        case IDNO:
          hFont = GetPropA(hwnd, "ROS_MSGBOX_HFONT");
          EndDialog(hwnd, wParam);
          if (hFont)
            DeleteObject(hFont);
          break;
      }
      break;

   case WM_HELP:
   {
      MSGBOXCALLBACK callback = (MSGBOXCALLBACK)GetPropA(hwnd, "ROS_MSGBOX_HELPCALLBACK");
      HELPINFO hi;

      memcpy(&hi, (void *)lParam, sizeof(hi));
      hi.dwContextId = GetWindowContextHelpId(hwnd);

      if (callback)
          callback(&hi);
      else
          SendMessageW(GetWindow(hwnd, GW_OWNER), WM_HELP, 0, (LPARAM)&hi);
      break;
   }

   default:
     /* Ok. Ignore all the other messages */
     DbgPrint("Message number 0x%04x is being ignored.\n", message);
    break;
  }
  return 0;
}


/*
 * @implemented
 */
int
STDCALL
MessageBoxA(
  HWND hWnd,
  LPCSTR lpText,
  LPCSTR lpCaption,
  UINT uType)
{
    return MessageBoxExA(hWnd, lpText, lpCaption, uType, LANG_NEUTRAL);
}


/*
 * @implemented
 */
int
STDCALL
MessageBoxExA(
  HWND hWnd,
  LPCSTR lpText,
  LPCSTR lpCaption,
  UINT uType,
  WORD wLanguageId)
{
    MSGBOXPARAMS msgbox;

    msgbox.cbSize = sizeof(msgbox);
    msgbox.hwndOwner = hWnd;
    msgbox.hInstance = 0;
    msgbox.lpszText = lpText;
    msgbox.lpszCaption = lpCaption;
    msgbox.dwStyle = uType;
    msgbox.lpszIcon = NULL;
    msgbox.dwContextHelpId = 0;
    msgbox.lpfnMsgBoxCallback = NULL;
    msgbox.dwLanguageId = wLanguageId;

    return MessageBoxIndirectA(&msgbox);
}


/*
 * @implemented
 */
int
STDCALL
MessageBoxExW(
  HWND hWnd,
  LPCWSTR lpText,
  LPCWSTR lpCaption,
  UINT uType,
  WORD wLanguageId)
{
    MSGBOXPARAMS msgbox;

    msgbox.cbSize = sizeof(msgbox);
    msgbox.hwndOwner = hWnd;
    msgbox.hInstance = 0;
    msgbox.lpszText = (LPCSTR)lpText;
    msgbox.lpszCaption =(LPCSTR) lpCaption;
    msgbox.dwStyle = uType;
    msgbox.lpszIcon = NULL;
    msgbox.dwContextHelpId = 0;
    msgbox.lpfnMsgBoxCallback = NULL;
    msgbox.dwLanguageId = wLanguageId;

    return MessageBoxIndirectW(&msgbox);
}


/*
 * @implemented
 */
int
STDCALL
MessageBoxIndirectA(
  CONST LPMSGBOXPARAMS lpMsgBoxParams)
{
    MSGBOXPARAMS msgboxW;
    UNICODE_STRING textW, captionW, iconW;
    int ret;

    if (HIWORD((UINT)lpMsgBoxParams->lpszText))
        RtlCreateUnicodeStringFromAsciiz(&textW, (PCSZ)lpMsgBoxParams->lpszText);
    else
        textW.Buffer = (LPWSTR)lpMsgBoxParams->lpszText;

    if (HIWORD((UINT)lpMsgBoxParams->lpszCaption))
        RtlCreateUnicodeStringFromAsciiz(&captionW, (PCSZ)lpMsgBoxParams->lpszCaption);
    else
        captionW.Buffer = (LPWSTR)lpMsgBoxParams->lpszCaption;

    if (HIWORD((UINT)lpMsgBoxParams->lpszIcon))
        RtlCreateUnicodeStringFromAsciiz(&iconW, (PCSZ)lpMsgBoxParams->lpszIcon);
    else
        iconW.Buffer = (LPWSTR)lpMsgBoxParams->lpszIcon;

    msgboxW.cbSize = sizeof(msgboxW);
    msgboxW.hwndOwner = lpMsgBoxParams->hwndOwner;
    msgboxW.hInstance = lpMsgBoxParams->hInstance;
    msgboxW.lpszText = (LPCSTR)textW.Buffer;
    msgboxW.lpszCaption = (LPCSTR)captionW.Buffer;
    msgboxW.dwStyle = lpMsgBoxParams->dwStyle;
    msgboxW.lpszIcon = (LPCSTR)iconW.Buffer;
    msgboxW.dwContextHelpId = lpMsgBoxParams->dwContextHelpId;
    msgboxW.lpfnMsgBoxCallback = lpMsgBoxParams->lpfnMsgBoxCallback;
    msgboxW.dwLanguageId = lpMsgBoxParams->dwLanguageId;

    ret = MessageBoxIndirectW(&msgboxW);

    if (HIWORD(textW.Buffer))
        RtlFreeUnicodeString(&textW);

    if (HIWORD(captionW.Buffer))
        RtlFreeUnicodeString(&captionW);

    if (HIWORD(iconW.Buffer))
        RtlFreeUnicodeString(&iconW);

    return ret;
}


/*
 * @implemented
 */
int
STDCALL
MessageBoxIndirectW(
  CONST LPMSGBOXPARAMS lpMsgBoxParams)
{
    LPVOID tmplate;
    HRSRC hRes;
    HMODULE hUser32;

    hUser32 = GetModuleHandleW(L"user32.dll");
    if (!(hRes = FindResourceExW(hUser32, RT_DIALOGW, L"MSGBOX", lpMsgBoxParams->dwLanguageId)))
        return 0;

    if (!(tmplate = (LPVOID)LoadResource(hUser32, hRes)))
        return 0;

    return DialogBoxIndirectParamW(lpMsgBoxParams->hInstance, tmplate, lpMsgBoxParams->hwndOwner,
                                   MSGBOX_DlgProc, (LPARAM)lpMsgBoxParams);
}


/*
 * @implemented
 */
int
STDCALL
MessageBoxW(
  HWND hWnd,
  LPCWSTR lpText,
  LPCWSTR lpCaption,
  UINT uType)
{
    return MessageBoxExW(hWnd, lpText, lpCaption, uType, LANG_NEUTRAL);
}


/*
 * @unimplemented
 */
DWORD
STDCALL
SoftModalMessageBox (DWORD Unknown0)
{
  UNIMPLEMENTED;
  return 0;
}

/* EOF */
