/*
    SPDX-FileCopyrightText: 2026 Benji Weber
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QList>
#include <QObject>
#include <QPointer>
#include <QTimer>
#include <QWindow>
#include <QtQmlIntegration/qqmlintegration.h>

#include <memory>

namespace Plasma
{
class Containment;
}
class EdgeTrigger;

/**
 * Brings back Plasma 5's "Windows can cover" panel behaviour on Wayland.
 *
 * While enabled, the panel's layer-shell surface is put in the bottom layer, so
 * normal windows can cover it. A one-pixel strip in the top layer along the
 * panel's screen edge raises the panel back to the top layer when the pointer
 * touches it. The panel drops back once the pointer leaves it and no popup of
 * the panel is open.
 */
class PanelCover : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    /// The PanelView (the panel's QWindow).
    Q_PROPERTY(QWindow *view READ view WRITE setView NOTIFY viewChanged)
    /// The panel's Plasma::Containment (Panel.qml: containment.plasmoid).
    Q_PROPERTY(QObject *containment READ containment WRITE setContainment NOTIFY containmentChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    /// How long after the pointer leaves the panel before windows may cover it again.
    Q_PROPERTY(int lowerDelay READ lowerDelay WRITE setLowerDelay NOTIFY lowerDelayChanged)
    Q_PROPERTY(bool raised READ raised NOTIFY raisedChanged)

public:
    explicit PanelCover(QObject *parent = nullptr);
    ~PanelCover() override;

    QWindow *view() const;
    void setView(QWindow *view);

    QObject *containment() const;
    void setContainment(QObject *containment);

    bool enabled() const;
    void setEnabled(bool enabled);

    int lowerDelay() const;
    void setLowerDelay(int delay);

    bool raised() const;

Q_SIGNALS:
    void viewChanged();
    void containmentChanged();
    void enabledChanged();
    void lowerDelayChanged();
    void raisedChanged();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private Q_SLOTS:
    void updateTriggerPlacement();
    void scheduleEvaluate();

private:
    void evaluate();
    bool wantsRaised();
    void setLowered(bool lowered);

    QPointer<QWindow> m_view;
    QPointer<Plasma::Containment> m_containment;
    std::unique_ptr<EdgeTrigger> m_trigger;
    QList<QMetaObject::Connection> m_viewConnections;
    QList<QMetaObject::Connection> m_containmentConnections;
    QList<QMetaObject::Connection> m_transientConnections;
    QTimer m_lowerTimer;
    QTimer m_graceTimer;
    QTimer m_shortcutTimer;
    QTimer m_evaluateTimer;
    bool m_enabled = false;
    bool m_containsMouse = false;
    bool m_containsDrag = false;
    bool m_lowered = false;
};
