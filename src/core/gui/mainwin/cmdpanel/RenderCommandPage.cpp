///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2013) Alexander Stukowski
//
//  This file is part of OVITO (Open Visualization Tool).
//
//  OVITO is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  OVITO is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////

#include <core/Core.h>
#include <core/plugins/PluginManager.h>
#include <core/gui/actions/ActionManager.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/rendering/RenderSettings.h>
#include "RenderCommandPage.h"

namespace Ovito {

/******************************************************************************
* Initializes the command panel page.
******************************************************************************/
RenderCommandPage::RenderCommandPage(MainWindow* mainWindow, QWidget* parent) : QWidget(parent)
{
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(2,2,2,2);

	QToolBar* toolbar = new QToolBar(this);
	toolbar->setStyleSheet("QToolBar { padding: 0px; margin: 0px; border: 0px none black; }");
	toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	layout->addWidget(toolbar);
	toolbar->addAction(mainWindow->actionManager()->getAction(ACTION_RENDER_ACTIVE_VIEWPORT));

	// Create the properties panel.
	propertiesPanel = new PropertiesPanel(this);
	propertiesPanel->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
	layout->addWidget(propertiesPanel, 1);
}

#if 0
/******************************************************************************
* Resets the command panel page to the initial state.
******************************************************************************/
void RenderCommandPage::reset()
{
	CommandPanelPage::reset();
	DataSet* dataset = DataSetManager::instance().currentSet();
	if(dataset)
		propertiesPanel->setEditObject(dataset->renderSettings());
}
#endif

};
