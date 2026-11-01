/*
    SPDX-FileCopyrightText: 2026 Benji Weber
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <plugin.h>

#include <QHash>
#include <QPointer>

namespace KWin
{
class Window;
}

/**
 * Restores the KWin 5 semantics of "Windows can cover" panels: the panel lives in the
 * normal window layer, so it interleaves with windows instead of being above or below
 * all of them.
 *
 * Protocol with the plasmashell side (org.windowscancover PanelCover), using only the
 * layer-shell layer the panel requests:
 *  - "bottom": the panel is at rest. We put it into the normal layer, just above the
 *    window that was topmost when the panel was last raised, so windows raised since
 *    then cover it.
 *  - "top": the panel was raised (screen edge, hover, popup...). We let it go above
 *    everything and remember the current topmost window.
 * Without this plugin the plasmashell side still works, with plain top/bottom layers.
 */
class WindowsCanCover : public KWin::Plugin
{
    Q_OBJECT

public:
    WindowsCanCover();
    ~WindowsCanCover() override;

private:
    struct Panel {
        bool managed = false;
        QPointer<KWin::Window> reference;
    };

    void ensureRule();
    void track(KWin::Window *window);
    void evaluateAll();
    void evaluate(KWin::Window *window, Panel &panel);
    KWin::Window *topmostNormalWindow(KWin::Window *exclude) const;

    QHash<KWin::Window *, Panel> m_panels;
};
