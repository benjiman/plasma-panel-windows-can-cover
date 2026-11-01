/*
    SPDX-FileCopyrightText: 2026 Benji Weber
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "edgetrigger.h"

#include <QPainter>
#include <QScreen>

#include <LayerShellQt/Window>

static constexpr int s_thickness = 1;

EdgeTrigger::EdgeTrigger()
{
    QSurfaceFormat format;
    format.setAlphaBufferSize(8);
    setFormat(format);
    setFlags(Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);

    m_layerWindow = LayerShellQt::Window::get(this);
    m_layerWindow->setLayer(LayerShellQt::Window::LayerTop);
    m_layerWindow->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityNone);
    m_layerWindow->setScope(QStringLiteral("windows-can-cover-edge"));
    m_layerWindow->setExclusiveZone(-1);
    m_layerWindow->setActivateOnShow(false);
    m_layerWindow->setCloseOnDismissed(false);
}

void EdgeTrigger::setPlacement(QScreen *screen, Plasma::Types::Location location, const QRect &span)
{
    if (m_screen == screen && m_location == location && m_span == span) {
        return;
    }

    // Layer surfaces cannot move between outputs: remap when the screen changes.
    const bool remap = isVisible() && m_screen != screen;
    if (remap) {
        setVisible(false);
    }

    m_screen = screen;
    m_location = location;
    m_span = span;
    applyPlacement();

    if (remap && m_armed) {
        setVisible(true);
    }
}

void EdgeTrigger::applyPlacement()
{
    if (!m_screen) {
        return;
    }

    const QRect sg = m_screen->geometry();
    // Clamp the span to the screen; an empty span falls back to the whole edge.
    const QRect span = m_span.isValid() ? m_span.intersected(sg) : sg;

    using LSW = LayerShellQt::Window;
    LSW::Anchors anchors;
    QMargins margins;
    QSize size;

    switch (m_location) {
    case Plasma::Types::TopEdge:
    case Plasma::Types::BottomEdge:
        anchors = LSW::Anchors(LSW::AnchorLeft | LSW::AnchorRight);
        anchors |= m_location == Plasma::Types::TopEdge ? LSW::AnchorTop : LSW::AnchorBottom;
        margins.setLeft(std::max(0, span.left() - sg.left()));
        margins.setRight(std::max(0, sg.right() - span.right()));
        size = QSize(0, s_thickness);
        resize(std::max(1, span.width()), s_thickness);
        break;
    case Plasma::Types::LeftEdge:
    case Plasma::Types::RightEdge:
        anchors = LSW::Anchors(LSW::AnchorTop | LSW::AnchorBottom);
        anchors |= m_location == Plasma::Types::LeftEdge ? LSW::AnchorLeft : LSW::AnchorRight;
        margins.setTop(std::max(0, span.top() - sg.top()));
        margins.setBottom(std::max(0, sg.bottom() - span.bottom()));
        size = QSize(s_thickness, 0);
        resize(s_thickness, std::max(1, span.height()));
        break;
    default:
        return;
    }

    setScreen(m_screen);
    m_layerWindow->setScreen(m_screen);
    m_layerWindow->setAnchors(anchors);
    m_layerWindow->setMargins(margins);
    m_layerWindow->setDesiredSize(size);
    requestUpdate();
}

void EdgeTrigger::arm()
{
    m_armed = true;
    if (!isVisible() && m_screen) {
        setVisible(true);
    }
}

void EdgeTrigger::disarm()
{
    m_armed = false;
    if (isVisible()) {
        setVisible(false);
    }
}

bool EdgeTrigger::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::Enter:
    case QEvent::DragEnter:
        if (m_armed) {
            Q_EMIT triggered();
        }
        break;
    default:
        break;
    }
    return QRasterWindow::event(e);
}

void EdgeTrigger::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.fillRect(QRect(QPoint(0, 0), size()), Qt::transparent);
}
