### 
### imgannotator.g: Glish script to mark positions on an image
###
### Copyright (C) 2002
### ASTRON (Netherlands Foundation for Research in Astronomy)
### P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
###
### This program is free software; you can redistribute it and/or modify
### it under the terms of the GNU General Public License as published by
### the Free Software Foundation; either version 2 of the License, or
### (at your option) any later version.
###
### This program is distributed in the hope that it will be useful,
### but WITHOUT ANY WARRANTY; without even the implied warranty of
### MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
### GNU General Public License for more details.
###
### You should have received a copy of the GNU General Public License
### along with this program; if not, write to the Free Software
### Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
###
### $Id$

pragma include once

include 'viewer.g'

#
# Template shapes to start with. This will be converted to World Coordinates.
#
marker_template := [color=[value='green'], hashandles=F, drawhandles=F, type='marker', markerstyle=0, center=[x=[value=0, unit='pix'], y=[value=0, unit='pix']], coords='pix', linewidth=[value=1], size=[value=1]]
text_template := [color=[value='green'], hashandles=F, drawhandles=F, type='text', fontsize=[value=18], text=[value='default'], center=[x=[value=0, unit='pix'], y=[value=0, unit='pix']], coords='pix']

const imgannotator := function(fname, type='contour')
{
	self   := [=];
	public := [=];

	public.self := ref self;

	#
	# Construction starts here
	# 
	self.shape_index := 1;
	self.marker_index := 1;

	self.mdd := dv.loaddata(fname, type);
	self.mdd.setoptions([axislabelswitch=T]);

	self.mdp := dv.newdisplaypanel();
	self.mdp.register(self.mdd);

	self.annotator := self.mdp.annotator();

	# construction ends here

	public.add_marker := function(ra, dec)
	{
		wider self;
		a := self.annotator;

		# create the shape and lock it to World Coordinates
		a.newshape(marker_template);
#DEBUG		print a.getshapeoptions(self.shape_index);
		a.locktowc(self.shape_index);
	
		#
		# Modify position and lock to World Coordinates again
		#
		opts:=a.getshapeoptions(self.shape_index)
		opts.center.x.value:=ra;
		opts.center.y.value:=dec;
		opts.linewidth.value:=2;
		opts.size.value:=10
		a.setshapeoptions(self.shape_index,opts);
		a.locktowc(self.shape_index);

		self.shape_index +:= 1;

		public.add_text(ra, dec, spaste(self.marker_index));
		self.marker_index +:= 1;

		return T;
	}

	public.add_text := function(ra, dec, text='emtpy')
	{
		wider self;
		a := self.annotator;

		# create the shape and lock it to World Coordinates
		a.newshape(text_template);
#DEBUG		print a.getshapeoptions(self.shape_index);
		a.locktowc(self.shape_index);
	
		#
		# Modify position and lock to World Coordinates again
		#
		opts:=a.getshapeoptions(self.shape_index)
		opts.center.x.value:=ra;
		opts.center.y.value:=dec;
		opts.text.value:=text;
		a.setshapeoptions(self.shape_index,opts);
		a.locktowc(self.shape_index);

		self.shape_index +:= 1;

		return T;
	}

	public.done := function()
	{
		wider self, public;

		self.mdp.done();
		self.mpd := F;
		self.mdd := F;

		self := F;
		val public := F;

		return T;
	}

	return ref public;
}

imgannotator_demo := function()
{
	a := imgannotator('/aips++/wierenga/demo/michiel.demo.img', 'contour');

	a.add_marker(2.734, 0.45379);

	return ref a;
}

