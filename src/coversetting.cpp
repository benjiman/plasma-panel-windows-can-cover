/*
    SPDX-FileCopyrightText: 2026 Benji Weber
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "coversetting.h"

#include <KConfigGroup>
#include <Plasma/Containment>

static const char s_key[] = "windowsCanCover";

namespace
{
// Lets every CoverSetting for the same containment (panel view, settings dialog) stay in sync.
class Hub : public QObject
{
    Q_OBJECT
public:
    static Hub *self()
    {
        static Hub hub;
        return &hub;
    }
Q_SIGNALS:
    void changed(Plasma::Containment *containment);
};
}

CoverSetting::CoverSetting(QObject *parent)
    : QObject(parent)
{
    connect(Hub::self(), &Hub::changed, this, [this](Plasma::Containment *containment) {
        if (containment == m_containment) {
            Q_EMIT windowsCanCoverChanged();
        }
    });
}

QObject *CoverSetting::containment() const
{
    return m_containment;
}

void CoverSetting::setContainment(QObject *containment)
{
    auto *c = qobject_cast<Plasma::Containment *>(containment);
    if (m_containment == c) {
        return;
    }
    m_containment = c;
    Q_EMIT containmentChanged();
    Q_EMIT windowsCanCoverChanged();
}

bool CoverSetting::windowsCanCover() const
{
    return m_containment && m_containment->config().readEntry(s_key, false);
}

void CoverSetting::setWindowsCanCover(bool cover)
{
    if (!m_containment || windowsCanCover() == cover) {
        return;
    }
    KConfigGroup cg = m_containment->config();
    if (cover) {
        cg.writeEntry(s_key, true);
    } else {
        cg.deleteEntry(s_key);
    }
    Q_EMIT m_containment->configNeedsSaving();
    Q_EMIT Hub::self()->changed(m_containment);
}

#include "coversetting.moc"
