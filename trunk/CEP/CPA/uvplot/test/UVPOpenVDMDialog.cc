//
// Copyright (C) 2002
// ASTRON (Netherlands Foundation for Research in Astronomy)
// P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include <UVPOpenVDMDialog.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>


//===============>>>  UVPOpenVDMDialog::UVPOpenVDMDialog  <<<===============

UVPOpenVDMDialog::UVPOpenVDMDialog(const std::string& caption,
				   QWidget* parent):
  QDialog(parent, caption.c_str(), true)
{
   itsDescriptionLabel = new QLabel("Please enter header and footer HIIDs", this);

   itsHeaderLabel    = new QLabel("Header:", this);
   itsHeaderLineEdit = new QLineEdit(this);

   itsDataLabel      = new QLabel("Data: ",this);
   itsDataLineEdit   = new QLineEdit(this);
   
   itsOKButton       = new QPushButton("OK", this);
   itsCancelButton   = new QPushButton("Cancel", this);

   itsOKButton->setDefault(true);


   QVBoxLayout* vlayout = new QVBoxLayout(this);

   vlayout->addSpacing(1);
   vlayout->addWidget(itsDescriptionLabel);

   vlayout->addSpacing(1);
   vlayout->addWidget(itsHeaderLabel,1);
   vlayout->addWidget(itsHeaderLineEdit,1);

   vlayout->addSpacing(1);
   vlayout->addWidget(itsDataLabel,1);
   vlayout->addWidget(itsDataLineEdit,1);
   vlayout->addStretch();

   QHBoxLayout* hlayout = new QHBoxLayout(vlayout);
   hlayout->addStretch();
   hlayout->addWidget(itsOKButton,1);
   hlayout->addWidget(itsCancelButton,1);
   hlayout->addStretch();

   vlayout->activate();

   QObject::connect(itsOKButton, SIGNAL(clicked()),
		    this, SLOT(accept()));

   QObject::connect(itsCancelButton, SIGNAL(clicked()),
		    this, SLOT(reject()));
}




//===============>>>  UVPOpenVDMDialog::~UVPOpenVDMDialog  <<<==============

UVPOpenVDMDialog::~UVPOpenVDMDialog()
{
}






//===============>>>  UVPOpenVDMDialog::UVPOpenVDMDialog  <<<===============

std::string UVPOpenVDMDialog::getHeaderText() const
{
  return std::string(itsHeaderLineEdit->text().latin1());
}




//===============>>>  UVPOpenVDMDialog::UVPOpenVDMDialog  <<<===============

std::string UVPOpenVDMDialog::getDataText() const
{
  return std::string(itsDataLineEdit->text().latin1());
}
