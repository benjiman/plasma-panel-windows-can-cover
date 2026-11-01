/*
    SPDX-FileCopyrightText: 2026 Benji Weber
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QPointer>
#include <QtQmlIntegration/qqmlintegration.h>

namespace Plasma
{
class Containment;
}

/**
 * Per-panel "windows can cover" flag, stored in the panel containment's config.
 * The panel's own visibility mode is kept at "Windows go below" (which already makes
 * plasmashell reserve no space); this flag turns that into "Windows can cover".
 */
class CoverSetting : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    /// The panel's Plasma::Containment (Panel.qml: containment.plasmoid, settings dialog: plasmoid).
    Q_PROPERTY(QObject *containment READ containment WRITE setContainment NOTIFY containmentChanged)
    Q_PROPERTY(bool windowsCanCover READ windowsCanCover WRITE setWindowsCanCover NOTIFY windowsCanCoverChanged)

public:
    explicit CoverSetting(QObject *parent = nullptr);

    QObject *containment() const;
    void setContainment(QObject *containment);

    bool windowsCanCover() const;
    void setWindowsCanCover(bool cover);

Q_SIGNALS:
    void containmentChanged();
    void windowsCanCoverChanged();

private:
    QPointer<Plasma::Containment> m_containment;
};
