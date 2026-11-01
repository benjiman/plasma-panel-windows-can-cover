/*
    SPDX-FileCopyrightText: 2026 Benji Weber
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "panelcover.h"
#include "edgetrigger.h"

#include <QGuiApplication>
#include <QScreen>

#include <LayerShellQt/Window>
#include <Plasma/Containment>
#include <Plasma/Corona>

using namespace std::chrono_literals;

PanelCover::PanelCover(QObject *parent)
    : QObject(parent)
{
    m_lowerTimer.setSingleShot(true);
    m_lowerTimer.setInterval(150ms);
    connect(&m_lowerTimer, &QTimer::timeout, this, &PanelCover::evaluate);

    // After the edge strip fires, keep the panel up for a moment even if the
    // pointer has not entered it yet (the restack takes a compositor round trip).
    m_graceTimer.setSingleShot(true);
    m_graceTimer.setInterval(750ms);
    connect(&m_graceTimer, &QTimer::timeout, this, &PanelCover::evaluate);

    // The "activate panel" global shortcut shows the panel for a while, as AutoHide does.
    m_shortcutTimer.setSingleShot(true);
    m_shortcutTimer.setInterval(3s);
    connect(&m_shortcutTimer, &QTimer::timeout, this, &PanelCover::evaluate);

    // Coalesce bursts of geometry/status changes.
    m_evaluateTimer.setSingleShot(true);
    m_evaluateTimer.setInterval(0);
    connect(&m_evaluateTimer, &QTimer::timeout, this, &PanelCover::evaluate);
}

PanelCover::~PanelCover()
{
    if (m_view && m_lowered) {
        LayerShellQt::Window::get(m_view)->setLayer(LayerShellQt::Window::LayerTop);
        m_view->requestUpdate();
    }
}

QWindow *PanelCover::view() const
{
    return m_view;
}

void PanelCover::setView(QWindow *view)
{
    if (m_view == view) {
        return;
    }

    if (m_view) {
        m_view->removeEventFilter(this);
        setLowered(false);
    }
    for (const auto &c : std::as_const(m_viewConnections)) {
        disconnect(c);
    }
    m_viewConnections.clear();

    m_view = view;
    m_containsMouse = false;
    m_containsDrag = false;

    if (m_view) {
        m_view->installEventFilter(this);
        auto placement = [this] {
            updateTriggerPlacement();
            scheduleEvaluate();
        };
        m_viewConnections << connect(m_view, &QWindow::xChanged, this, placement);
        m_viewConnections << connect(m_view, &QWindow::yChanged, this, placement);
        m_viewConnections << connect(m_view, &QWindow::widthChanged, this, placement);
        m_viewConnections << connect(m_view, &QWindow::heightChanged, this, placement);
        m_viewConnections << connect(m_view, &QWindow::screenChanged, this, placement);
        m_viewConnections << connect(m_view, &QWindow::visibleChanged, this, &PanelCover::scheduleEvaluate);
        // PlasmaQuick::ContainmentView signals; string based so we need no PlasmaQuick dependency.
        m_viewConnections << connect(m_view, SIGNAL(containmentChanged()), this, SLOT(updateContainment()));
        m_viewConnections << connect(m_view, SIGNAL(locationChanged(Plasma::Types::Location)), this, SLOT(updateTriggerPlacement()));
        // PanelView::userConfiguringChanged: panel settings dialog open.
        m_viewConnections << connect(m_view, SIGNAL(userConfiguringChanged()), this, SLOT(scheduleEvaluate()));
    }

    updateContainment();
    Q_EMIT viewChanged();
}

bool PanelCover::enabled() const
{
    return m_enabled;
}

void PanelCover::setEnabled(bool enabled)
{
    if (m_enabled == enabled) {
        return;
    }
    m_enabled = enabled;
    scheduleEvaluate();
    Q_EMIT enabledChanged();
}

int PanelCover::lowerDelay() const
{
    return m_lowerTimer.interval();
}

void PanelCover::setLowerDelay(int delay)
{
    if (m_lowerTimer.interval() == delay) {
        return;
    }
    m_lowerTimer.setInterval(delay);
    Q_EMIT lowerDelayChanged();
}

bool PanelCover::raised() const
{
    return !m_lowered;
}

void PanelCover::updateContainment()
{
    for (const auto &c : std::as_const(m_containmentConnections)) {
        disconnect(c);
    }
    m_containmentConnections.clear();

    m_containment = m_view ? qobject_cast<Plasma::Containment *>(m_view->property("containment").value<QObject *>()) : nullptr;

    if (m_containment) {
        m_containmentConnections << connect(m_containment, &Plasma::Applet::locationChanged, this, &PanelCover::updateTriggerPlacement);
        m_containmentConnections << connect(m_containment, &Plasma::Applet::statusChanged, this, &PanelCover::scheduleEvaluate);
        // Global shortcut to focus the panel (Plasma "activate" action).
        m_containmentConnections << connect(m_containment, &Plasma::Applet::activated, this, [this] {
            m_shortcutTimer.start();
            evaluate();
        });
        if (auto corona = m_containment->corona()) {
            m_containmentConnections << connect(corona, &Plasma::Corona::editModeChanged, this, &PanelCover::scheduleEvaluate);
        }
    }

    updateTriggerPlacement();
    scheduleEvaluate();
}

void PanelCover::updateTriggerPlacement()
{
    if (!m_trigger || !m_view) {
        return;
    }
    const auto location = m_containment ? m_containment->location() : Plasma::Types::BottomEdge;
    m_trigger->setPlacement(m_view->screen(), location, m_view->geometry());
}

bool PanelCover::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_view) {
        switch (event->type()) {
        case QEvent::Enter:
            m_containsMouse = true;
            m_lowerTimer.stop();
            evaluate();
            break;
        case QEvent::Leave:
            m_containsMouse = false;
            m_lowerTimer.start();
            break;
        case QEvent::DragEnter:
            m_containsDrag = true;
            m_lowerTimer.stop();
            evaluate();
            break;
        case QEvent::DragLeave:
        case QEvent::Drop:
            m_containsDrag = false;
            m_lowerTimer.start();
            break;
        default:
            break;
        }
    }
    return QObject::eventFilter(watched, event);
}

void PanelCover::scheduleEvaluate()
{
    m_evaluateTimer.start();
}

bool PanelCover::wantsRaised()
{
    for (const auto &c : std::as_const(m_transientConnections)) {
        disconnect(c);
    }
    m_transientConnections.clear();

    if (m_containsMouse || m_containsDrag || m_graceTimer.isActive() || m_shortcutTimer.isActive()) {
        return true;
    }
    if (m_view->property("userConfiguring").toBool()) {
        return true;
    }
    if (m_containment) {
        if (auto corona = m_containment->corona(); corona && corona->isEditMode()) {
            return true;
        }
        const auto status = m_containment->status();
        if (status >= Plasma::Types::NeedsAttentionStatus && status != Plasma::Types::HiddenStatus) {
            return true;
        }
    }
    // Keep the panel up while one of its popups (launcher, tray, task tooltips...) is open.
    bool popupOpen = false;
    for (QWindow *window : QGuiApplication::topLevelWindows()) {
        if (window->transientParent() == m_view) {
            m_transientConnections << connect(window, &QWindow::visibleChanged, this, &PanelCover::scheduleEvaluate);
            popupOpen = popupOpen || window->isVisible();
        }
    }
    return popupOpen;
}

void PanelCover::evaluate()
{
    if (!m_view) {
        return;
    }
    if (!m_enabled) {
        setLowered(false);
        m_trigger.reset();
        return;
    }
    setLowered(!wantsRaised());
}

void PanelCover::setLowered(bool lowered)
{
    if (!m_view) {
        return;
    }

    auto layerWindow = LayerShellQt::Window::get(m_view);
    const auto layer = lowered ? LayerShellQt::Window::LayerBottom : LayerShellQt::Window::LayerTop;
    if (layerWindow->layer() != layer) {
        layerWindow->setLayer(layer);
        m_view->requestUpdate();
    }

    if (lowered && m_view->isVisible()) {
        if (!m_trigger) {
            m_trigger = std::make_unique<EdgeTrigger>();
            connect(m_trigger.get(), &EdgeTrigger::triggered, this, [this] {
                m_graceTimer.start();
                evaluate();
            });
            updateTriggerPlacement();
        }
        m_trigger->arm();
    } else if (m_trigger) {
        m_trigger->disarm();
    }

    if (m_lowered != lowered) {
        m_lowered = lowered;
        Q_EMIT raisedChanged();
    }
}

#include "moc_panelcover.cpp"
