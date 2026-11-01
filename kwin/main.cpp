/*
    SPDX-FileCopyrightText: 2026 Benji Weber
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "windowscancover.h"

#include <plugin.h>

using namespace KWin;

class WindowsCanCoverFactory : public PluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginFactory_iid FILE "metadata.json")
    Q_INTERFACES(KWin::PluginFactory)

public:
    std::unique_ptr<Plugin> create() const override
    {
        return std::make_unique<WindowsCanCover>();
    }
};

#include "main.moc"
