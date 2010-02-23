--
--  getSchedulerInfo.sql: function for getting treeinfo from the OTDB
--
--  Copyright (C) 2010
--  ASTRON (Netherlands Foundation for Research in Astronomy)
--  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
--
--  This program is free software; you can redistribute it and/or modify
--  it under the terms of the GNU General Public License as published by
--  the Free Software Foundation; either version 2 of the License, or
--  (at your option) any later version.
--
--  This program is distributed in the hope that it will be useful,
--  but WITHOUT ANY WARRANTY; without even the implied warranty of
--  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--  GNU General Public License for more details.
--
--  You should have received a copy of the GNU General Public License
--  along with this program; if not, write to the Free Software
--  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
--
--  $Id: getSchedulerInfo.sql 8438 2006-05-18 19:16:37Z overeem $
--


CREATE TYPE schedulerInfo AS (
	CEPProcessingUnits     INT4,
	declination            VARCHAR(20),
	firstPossibleDate      VARCHAR(20),
	fixedDay               BOOLEAN,
	fixedTime              BOOLEAN,
	lastPossibleDate       VARCHAR(20),
	late                   BOOLEAN,
	nightTimeWeightFactor  INT4,
	nrOfSubbands           INT4,
	offlineProcessingUnits INT4,
	predMaxTimeDif         VARCHAR(20),
	predMinTimeDif         VARCHAR(20),
	predecessor            INT4,
	priority               FLOAT,
	referenceFrame         VARCHAR(20),
	rightAscention         VARCHAR(20),
	sources                TEXT,
	storageUnits           INT4,
	taskDuration           VARCHAR(20),
	taskStatus             VARCHAR(20),
	taskType               VARCHAR(20),
	unscheduledReason      VARCHAR(20),
	windowMaximumTime      VARCHAR(20),
	windowMinimumTime      VARCHAR(20)
);

CREATE OR REPLACE FUNCTION getSchedulerInfo(INT4)
  RETURNS schedulerInfo AS '
    DECLARE	vRecord	schedulerInfo;
	DECLARE fieldList CURSOR FOR
      SELECT limits FROM getvhitemlist($1, \'%Observation.Scheduler.%\') ORDER BY name ASC;

	BEGIN
      OPEN fieldList;
	  FETCH fieldList INTO vRecord.CEPProcessingUnits;
	  FETCH fieldList INTO vRecord.declination;
	  FETCH fieldList INTO vRecord.firstPossibleDate;
	  FETCH fieldList INTO vRecord.fixedDay;
	  FETCH fieldList INTO vRecord.fixedTime;
	  FETCH fieldList INTO vRecord.lastPossibleDate;
	  FETCH fieldList INTO vRecord.late;
	  FETCH fieldList INTO vRecord.nightTimeWeightFactor;
	  FETCH fieldList INTO vRecord.nrOfSubbands;
	  FETCH fieldList INTO vRecord.offlineProcessingUnits;
	  FETCH fieldList INTO vRecord.predMaxTimeDif;
	  FETCH fieldList INTO vRecord.predMinTimeDif;
	  FETCH fieldList INTO vRecord.predecessor;
	  FETCH fieldList INTO vRecord.priority;
	  FETCH fieldList INTO vRecord.referenceFrame;
	  FETCH fieldList INTO vRecord.rightAscention;
	  FETCH fieldList INTO vRecord.sources;
	  FETCH fieldList INTO vRecord.storageUnits;
	  FETCH fieldList INTO vRecord.taskDuration;
	  FETCH fieldList INTO vRecord.taskStatus;
	  FETCH fieldList INTO vRecord.taskType;
	  FETCH fieldList INTO vRecord.unscheduledReason;
	  FETCH fieldList INTO vRecord.windowMaximumTime;
	  FETCH fieldList INTO vRecord.windowMinimumTime;
      RETURN vRecord;
    END;
' language plpgsql;
