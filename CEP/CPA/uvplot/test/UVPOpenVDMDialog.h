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

#ifndef UVPOPENVDMDIALOG_H
#define UVPOPENVDMDIALOG_H

// $Id$


#include <qdialog.h>
#include <string>

class QLineEdit;
class QLabel;
class QPushButton;


class UVPOpenVDMDialog: public QDialog
{
 public:
    UVPOpenVDMDialog(const std::string& caption, QWidget* parent);
   ~UVPOpenVDMDialog();

   std::string getHeaderText() const;
   std::string getDataText() const;


 private:

   QLabel*      itsDescriptionLabel;

   QLabel*      itsHeaderLabel;
   QLineEdit*   itsHeaderLineEdit;

   QLabel*      itsDataLabel;
   QLineEdit*   itsDataLineEdit;
   
   QPushButton* itsOKButton;
   QPushButton* itsCancelButton;
};

#endif //UVPOPENVDMDIALOG_H
