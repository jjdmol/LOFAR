#!/usr/bin/python

# Copyright (C) 2012-2015  ASTRON (Netherlands Institute for Radio Astronomy)
# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.

import os
import os.path
import sqlite3
import datetime

class EntryNotFoundException(Exception):
    pass

class LTAStorageDb:
    def __init__(self, db_filename, removeIfExisting = False):
        self.db_filename = db_filename

        if os.path.exists(self.db_filename) and removeIfExisting:
            os.remove(self.db_filename)

        if not os.path.exists(self.db_filename):
            with sqlite3.connect(self.db_filename) as conn:
                cursor = conn.cursor()
                cursor.execute("""
                    create table storage_site (
                    id                  integer primary key autoincrement not null,
                    name                text not null,
                    url                 text not null);
                    """)

                cursor.execute("""
                    create table directory (
                    id                  integer primary key autoincrement not null,
                    name                text key not null,
                    parent_directory_id integer,
                    foreign key (parent_directory_id) references directory(id) );
                    """)

                cursor.execute("""
                    create table directory_closure (
                    ancestor_id         integer not null,
                    descendant_id       integer not null,
                    depth               integer not null,
                    primary key (ancestor_id, descendant_id)
                    foreign key (ancestor_id) references directory(id)
                    foreign key (descendant_id) references directory(id)
                    );
                    """)

                cursor.executescript("""
                    create trigger directory_closure_trigger
                    after insert on directory
                    begin
                        insert into directory_closure (ancestor_id, descendant_id, depth) values (new.id, new.id, 0) ;

                        insert into directory_closure (ancestor_id, descendant_id, depth)
                            select p.ancestor_id, c.descendant_id, p.depth+c.depth+1
                            from directory_closure p, directory_closure c
                            where p.descendant_id=new.parent_directory_id and c.ancestor_id=new.id ;
                    end;
                    """)

                cursor.execute("""
                    create table storage_site_root (
                    storage_site_id                 integer not null,
                    directory_id                        integer not null,
                    primary key (storage_site_id, directory_id),
                    foreign key (storage_site_id) references storage_site(id),
                    foreign key (directory_id) references directory(id) );
                    """)

                cursor.execute("""
                    create table fileinfo (
                    id                                        integer primary key autoincrement not null,
                    name                                    text key not null,
                    size                                    integer not null,
                    creation_date                 datetime not null,
                    directory_id                    integer not null,
                    foreign key (directory_id) references directory(id) );
                    """)

                #save created tables and triggers
                conn.commit()

    def insertSite(self, siteName, srmurl):
        with sqlite3.connect(self.db_filename) as conn:
            cursor = conn.cursor()

            site_row = cursor.execute('select id from storage_site where url = ?', [srmurl]).fetchone()
            site_id = site_row[0] if site_row else cursor.execute('insert into storage_site (name, url) values (?, ?)', (siteName, srmurl)).lastrowid

            conn.commit()

            return site_id

    def insertRootDirectory(self, siteName, rootDirectory):
        with sqlite3.connect(self.db_filename) as conn:
            cursor = conn.cursor()

            site_row = cursor.execute('select id from storage_site where name = ?', [siteName]).fetchone()

            if not site_row:
                raise EntryNotFoundException()

            site_id = site_row[0]

            dir_id = cursor.execute('insert into directory (name) values (?)', [rootDirectory]).lastrowid

            cursor.execute('insert into storage_site_root (storage_site_id, directory_id) values (?, ?)', (site_id, dir_id)).lastrowid

            conn.commit()

            return dir_id

    def insertRootLocation(self, siteName, srmurl, rootDirectory):
        with sqlite3.connect(self.db_filename) as conn:
            cursor = conn.cursor()

            site_row = cursor.execute('select id from storage_site where url = ?', [srmurl]).fetchone()
            site_id = site_row[0] if site_row else cursor.execute('insert into storage_site (name, url) values (?, ?)', (siteName, srmurl)).lastrowid

            dir_id = cursor.execute('insert into directory (name) values (?)', [rootDirectory]).lastrowid

            cursor.execute('insert into storage_site_root (storage_site_id, directory_id) values (?, ?)', (site_id, dir_id)).lastrowid

            conn.commit()

            return dir_id

    def insertSubDirectory(self, parent_directory_id, sub_directory):
        with sqlite3.connect(self.db_filename) as conn:
            cursor = conn.cursor()

            dir_id = cursor.execute('insert into directory (name, parent_directory_id) values (?, ?)', (sub_directory, parent_directory_id)).lastrowid

            conn.commit()

            return dir_id

    def insertFileInfo(self, name, size, creation_date, parent_directory_id):
        with sqlite3.connect(self.db_filename) as conn:
            cursor = conn.cursor()

            fileinfo_id = cursor.execute('insert into fileinfo (name, size, creation_date, directory_id) values (?, ?, ?, ?)',
                                         (name.split('/')[-1], size, datetime.datetime.utcnow(), parent_directory_id))

            conn.commit()

            return fileinfo_id

    def insertLocationResult(self, result):
        with sqlite3.connect(self.db_filename) as conn:
            cursor = conn.cursor()

            dir_row = cursor.execute('''select directory.id from storage_site
                                                                    join storage_site_root on storage_site_root.storage_site_id = storage_site.id
                                                                    join directory on directory.id = storage_site_root.directory_id
                                                                    where storage_site.url = ?
                                                                    and directory.name = ?
                                                                    ''', (result.location.srmurl, result.location.directory)).fetchone()

            if dir_row:
                dir_id = dir_row[0]
                cursor.executemany('insert into directory (name, parent_directory_id) values (?, ?)',
                                                        [(subDir.directory, dir_id) for subDir in result.subDirectories])

                cursor.executemany('insert into fileinfo (name, size, creation_date, directory_id) values (?, ?, ?, ?)',
                                                        [(file.filename.split('/')[-1], file.size, datetime.datetime.utcnow(), dir_id) for file in    result.files])

                conn.commit()

    def sites(self):
        '''returns list of tuples (id, name, url) of all sites'''
        with sqlite3.connect(self.db_filename) as conn:
            return conn.execute('''SELECT id, name, url FROM storage_site''').fetchall()

    def site(self, site_id):
        '''returns tuple (id, name, url) for site with id=site_id'''
        with sqlite3.connect(self.db_filename) as conn:
            return conn.execute('''SELECT id, name, url FROM storage_site where id = ?''', [site_id]).fetchone()

    def rootDirectories(self):
        '''returns list of all root directories (id, name, site_id, site_name) for all sites'''
        with sqlite3.connect(self.db_filename) as conn:
            return conn.execute('''SELECT dir.id, dir.name, site.id, site.name
                FROM storage_site_root
                join directory dir on dir.id = storage_site_root.directory_id
                join storage_site site on site.id = storage_site_root.storage_site_id''').fetchall()

    def subDirectories(self, directory_id, depth = 1, includeSelf=False):
        '''returns list of all sub directories up to the given depth (id, name, site_id, site_name, depth) for the given directory_id'''
        with sqlite3.connect(self.db_filename) as conn:
            return conn.execute('''
                SELECT dir.id, dir.name, dir.parent_directory_id, directory_closure.depth FROM directory_closure
                join directory dir on dir.id = directory_closure.descendant_id
                where ancestor_id = ? and depth <= ? and depth > ?
                order by depth asc
                ''', (directory_id, depth, -1 if includeSelf else 0)).fetchall()

    def parentDirectories(self, directory_id):
        with sqlite3.connect(self.db_filename) as conn:
            return conn.execute('''
                SELECT dir.* FROM directory_closure dc
                join directory dir on dir.id = dc.ancestor_id
                where dc.descendant_id = ? and depth > 0
                order by depth desc
                ''', [directory_id]).fetchall()

    def filesInDirectory(self, directory_id):
        with sqlite3.connect(self.db_filename) as conn:
            return conn.execute('''
                SELECT * FROM fileinfo
                where directory_id = ?
                ''', [directory_id]).fetchall()

    def filesInTree(self, base_directory_id):
        with sqlite3.connect(self.db_filename) as conn:
            return conn.execute('''
                SELECT dir.id, dir.name, fileinfo.id, fileinfo.name, fileinfo.size, fileinfo.creation_date FROM directory_closure
                join directory dir on dir.id = directory_closure.descendant_id
                join fileinfo on fileinfo.directory_id = directory_closure.descendant_id
                where ancestor_id = ? and depth > 0
                order by depth asc
                ''', [base_directory_id]).fetchall()

