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
#include <core/utilities/io/FileManager.h>
#include <core/utilities/concurrent/Future.h>
#include "LAMMPSTextDumpImporter.h"
#include "LAMMPSDumpImporterSettingsDialog.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, LAMMPSTextDumpImporter, AtomsImporter)
DEFINE_PROPERTY_FIELD(LAMMPSTextDumpImporter, _isMultiTimestepFile, "IsMultiTimestepFile")

/******************************************************************************
* Opens the settings dialog for this importer.
******************************************************************************/
bool LAMMPSTextDumpImporter::showSettingsDialog(QWidget* parent)
{
	LAMMPSDumpImporterSettingsDialog dialog(this, parent);
	if(dialog.exec() != QDialog::Accepted)
		return false;
	return true;
}

/******************************************************************************
* Parses the given input file and stores the data in the given container object.
******************************************************************************/
void LAMMPSTextDumpImporter::parseFile(FutureInterface<ImportedDataPtr>& futureInterface, AtomsData& container, QIODevice& file)
{
	futureInterface.setProgressText(tr("Loading LAMMPS dump file..."));
	futureInterface.setProgressRange(100);

	container.setSimulationCell(AffineTransformation(
			Vector3(200,0,0), Vector3(0,100,0), Vector3(0,0,1000), Vector3(-100,-50,-50)));

	for(int i = 0; i < 100 && !futureInterface.isCanceled(); i++) {
		futureInterface.setProgressValue(i);
		QThread::msleep(20);
	}

}

};