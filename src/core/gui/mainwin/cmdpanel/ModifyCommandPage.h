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
#include <core/gui/widgets/ColorPickerWidget.h>
#include <core/gui/properties/PropertiesPanel.h>
#include <core/gui/mainwin/cmdpanel/CommandPanel.h>
#include <core/viewport/input/ViewportInputManager.h>
#include <core/reference/RefTargetListener.h>

namespace Ovito {

class ModificationListModel;	// defined in ModificationListModel.h
class ModificationListItem;		// defined in ModificationListModel.h
class SceneObject;				// defined in SceneObject.h
class Modifier;					// defined in Modifier.h

/******************************************************************************
* The creation page lets the user modify the selected object.
******************************************************************************/
class ModifyCommandPage : public CommandPanelPage
{
	Q_OBJECT

public:

	/// Initializes the modify page.
    ModifyCommandPage();

	/// Resets the modify page to the initial state.
	virtual void reset() override;

	/// Is called when the user selects the page.
	virtual void onEnter() override;

	/// Is called when the user selects another page.
	virtual void onLeave() override;

	/// This is called after all changes to the selection set have been completed.
	virtual void onSelectionChangeComplete(SelectionSet* newSelection) override;

	/// Returns the object that is currently being edited in the properties panel.
	RefTarget* editObject() const { return _propertiesPanel->editObject(); }

	/// Returns the list model that encapsulates the modification pipeline of the selected node(s).
	ModificationListModel* modificationListModel() const { return _modificationListModel; }

private:

	/// Updates the state of the actions that can be invoked on the currently selected item.
	void updateActions(ModificationListItem* currentItem);

protected Q_SLOTS:

	/// Is called when a new modification list item has been selected, or if the currently
	/// selected item has changed.
	void onSelectedItemChanged();

	/// Handles the ACTION_MODIFIER_DELETE command, which deleted the selected modifier from the stack.
	void onDeleteModifier();

	/// Is called when the user has selected an item in the modifier class list.
	void onModifierAdd(int index);

	/// This called when the user double clicks on an item in the modifier stack.
	void onModifierStackDoubleClicked(const QModelIndex& index);

	/// This is called by the RefTargetListener that listens to notification events generated by the
	/// current selection set.
	void onSelectionSetEvent(ReferenceEvent* event);

	/// Handles the ACTION_MODIFIER_MOVE_UP command, which moves the selected modifier up one entry in the stack.
	void onModifierMoveUp();

	/// Handles the ACTION_MODIFIER_MOVE_DOWN command, which moves the selected modifier down one entry in the stack.
	void onModifierMoveDown();

	/// Handles the ACTION_MODIFIER_TOGGLE_STATE command, which toggles the enabled/disable state of the selected modifier.
	void onModifierToggleState(bool newState);

private:

	/// This listens to notification event generated by the current selection set.
	RefTargetListener _selectionSetListener;

	/// This list box shows the modifier stack of the selected scene node(s).
	QListView* _modificationListWidget;

	/// The visual representation of the modification pipeline of the selected node(s).
	ModificationListModel* _modificationListModel;

	/// This control displays the list of available modifier classes and allows the user to apply a modifier.
	QComboBox* _modifierSelector;

	/// This panel shows the properties of the selected modifier stack entry
	PropertiesPanel* _propertiesPanel;
};

};
