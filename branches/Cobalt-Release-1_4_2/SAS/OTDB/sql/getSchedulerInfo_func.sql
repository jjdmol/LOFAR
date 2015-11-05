--
--  getSchedulerInfo_func.sql: function for getting treeinfo from the OTDB
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

DROP TYPE IF EXISTS schedulerInfo CASCADE;

CREATE TYPE schedulerInfo AS (
	contactEmail           VARCHAR(40),
	contactName            VARCHAR(30),
	contactPhone           VARCHAR(20),
	firstPossibleDay       INT4,
	fixedDay               BOOLEAN,
	fixedTime              BOOLEAN,
	lastPossibleDay        INT4,
	late                   BOOLEAN,
	nightTimeWeightFactor  INT4,
	predecessors           TEXT,
	predMaxTimeDif         VARCHAR(10),
	predMinTimeDif         VARCHAR(10),
	priority               FLOAT,
	reason                 VARCHAR(40),
	referenceFrame         INT4,
	reservation            INT4,
	storageSelectionMode   INT4,
	taskDuration           INT4,
	taskID                 INT4,
	taskName               VARCHAR(40),
	taskType               INT4,
	windowMaximumTime      VARCHAR(8),
	windowMinimumTime      VARCHAR(8)
);

CREATE OR REPLACE FUNCTION getSchedulerInfo(INT4)
  RETURNS schedulerInfo AS '
    DECLARE	vRecord	schedulerInfo;
	DECLARE fieldList CURSOR FOR
      SELECT limits FROM getvhitemlist($1, \'%Observation.Scheduler.%\') ORDER BY name ASC;

	BEGIN
      	OPEN fieldList;
	FETCH fieldList INTO vRecord.contactEmail;
	FETCH fieldList INTO vRecord.contactName;
	FETCH fieldList INTO vRecord.contactPhone;
	FETCH fieldList INTO vRecord.firstPossibleDay;
	FETCH fieldList INTO vRecord.fixedDay;
	FETCH fieldList INTO vRecord.fixedTime;
	FETCH fieldList INTO vRecord.lastPossibleDay;
	FETCH fieldList INTO vRecord.late;
	FETCH fieldList INTO vRecord.nightTimeWeightFactor;
	FETCH fieldList INTO vRecord.predecessors;
	FETCH fieldList INTO vRecord.predMaxTimeDif;
	FETCH fieldList INTO vRecord.predMinTimeDif;
	FETCH fieldList INTO vRecord.priority;
	FETCH fieldList INTO vRecord.reason;
	FETCH fieldList INTO vRecord.referenceFrame;
	FETCH fieldList INTO vRecord.reservation;
	FETCH fieldList INTO vRecord.storageSelectionMode;
	FETCH fieldList INTO vRecord.taskDuration;
	FETCH fieldList INTO vRecord.taskID;
	FETCH fieldList INTO vRecord.taskName;
	FETCH fieldList INTO vRecord.taskType;
	FETCH fieldList INTO vRecord.windowMaximumTime;
	FETCH fieldList INTO vRecord.windowMinimumTime;
      RETURN vRecord;
    END;
' language plpgsql;
