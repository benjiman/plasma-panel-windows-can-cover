/*
    SPDX-FileCopyrightText: 2026 Benji Weber
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QQmlEngineExtensionPlugin>

extern void qml_register_types_org_windowscancover();

class WindowsCanCoverPlugin : public QQmlEngineExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlEngineExtensionInterface_iid)

public:
    WindowsCanCoverPlugin(QObject *parent = nullptr)
        : QQmlEngineExtensionPlugin(parent)
    {
        volatile auto registration = &qml_register_types_org_windowscancover;
        Q_UNUSED(registration)
    }
};

#include "plugin.moc"
