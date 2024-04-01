/*
 * This file is part of phrasebooks.
 *
 * phrasebooks is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * phrasebooks is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with phrasebooks.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "windowmarker.h"

void WindowMarker::draw(HWND window)
{
    if(!IsWindow(window))
        return;

    static HPEN pen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));

    HDC dc;
    HGDIOBJ oldPen, oldBrush;
    RECT rect;

    GetWindowRect(window, &rect);

    dc = GetWindowDC(window);

    if(!dc)
        return;

    oldPen = SelectObject(dc, pen);
    oldBrush = SelectObject(dc, GetStockObject(HOLLOW_BRUSH));

    Rectangle(dc, 0, 0, rect.right - rect.left, rect.bottom - rect.top);

    SelectObject(dc, oldPen);
    SelectObject(dc, oldBrush);

    ReleaseDC(window, dc);
}

void WindowMarker::remove(HWND *window)
{
    if(!window || !IsWindow(*window))
        return;

    InvalidateRect(*window, 0, TRUE);
    UpdateWindow(*window);
    RedrawWindow(*window, 0, 0, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);

    *window = 0;
}
