/*
    SPDX-FileCopyrightText: 2026 Benji Weber
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QPointer>
#include <QRasterWindow>
#include <QRect>

#include <Plasma/Plasma>

namespace LayerShellQt
{
class Window;
}

/**
 * An invisible, input-only-in-spirit strip one pixel thick along a screen edge.
 * It lives in the layer-shell "top" layer so it stays above normal windows,
 * and emits triggered() when the pointer or a drag touches it.
 */
class EdgeTrigger : public QRasterWindow
{
    Q_OBJECT

public:
    EdgeTrigger();

    /// Place the strip along @p location of @p screen, spanning @p span (global, logical coords).
    void setPlacement(QScreen *screen, Plasma::Types::Location location, const QRect &span);

    void arm();
    void disarm();

Q_SIGNALS:
    void triggered();

protected:
    bool event(QEvent *e) override;
    void paintEvent(QPaintEvent *e) override;

private:
    void applyPlacement();

    LayerShellQt::Window *m_layerWindow = nullptr;
    QPointer<QScreen> m_screen;
    Plasma::Types::Location m_location = Plasma::Types::BottomEdge;
    QRect m_span;
    bool m_armed = false;
};
