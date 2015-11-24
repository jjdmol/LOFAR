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

# $Id$

# TODO: add comment to methods
# TODO: reuse connection in methods (take care of exceptions closing the connection)
# TODO: use generators and yield for faster and more memory efficient processing of results.
# TODO: use other database? MariaDB? instead of sqlite?

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
                create_script_path = os.path.join(os.path.dirname(__file__), 'create_db_ltastorageoverview.sql')

                with open(create_script_path) as script:
                    conn.executescript(script.read())

                # save created tables and triggers
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

    def insertSubDirectories(self, subDirectoryNames, parentDirId, directoryLastVisitTime = None):
        with sqlite3.connect(self.db_filename) as conn:
            cursor = conn.cursor()

            cursor.executemany('insert into directory (name, parent_directory_id) values (?, ?)',
                             [(name, parentDirId) for name in subDirectoryNames])

            if directoryLastVisitTime:
                subDirIds = cursor.execute('''select id from directory
                    where parent_directory_id = %s
                    and name in (%s)''' % (parentDirId, ', '.join(["'%s'" % x for x in subDirectoryNames]))).fetchall()

                subDirIds = [x[0] for x in subDirIds]

                for subDirId in subDirIds:
                    cursor.execute('''insert into scraper_last_directory_visit (visit_date, directory_id)
                    values (?, ?)''', (directoryLastVisitTime, subDirId))

            conn.commit()

    def insertFileInfo(self, name, size, creation_date, parent_directory_id):
        with sqlite3.connect(self.db_filename) as conn:
            cursor = conn.cursor()

            fileinfo_id = cursor.execute('insert into fileinfo (name, size, creation_date, directory_id) values (?, ?, ?, ?)',
                                         (name.split('/')[-1], size, creation_date, parent_directory_id))

            conn.commit()

            return fileinfo_id

    def insertFileInfos(self, file_infos):
        with sqlite3.connect(self.db_filename) as conn:
            conn.executemany('insert into fileinfo (name, size, creation_date, directory_id) values (?, ?, ?, ?)',
                             [(f[0].split('/')[-1], f[1], f[2], f[3]) for f in file_infos])

            conn.commit()

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

    def updateDirectoryLastVisitTime(self, directory_id, timestamp):
        with sqlite3.connect(self.db_filename) as conn:
            cursor = conn.cursor()

            updated = cursor.execute('''update or ignore scraper_last_directory_visit
                set visit_date=?
                where directory_id = ?''', (timestamp, directory_id)).rowcount

            if not updated:
                cursor.execute('''insert into scraper_last_directory_visit
                (visit_date, directory_id)
                values (?, ?)''', (timestamp, directory_id))

            conn.commit()

    def sites(self):
        '''returns list of tuples (id, name, url) of all sites'''
        with sqlite3.connect(self.db_filename) as conn:
            return conn.execute('''SELECT id, name, url FROM storage_site''').fetchall()

    def site(self, site_id):
        '''returns tuple (id, name, url) for site with id=site_id'''
        with sqlite3.connect(self.db_filename) as conn:
            return conn.execute('''SELECT id, name, url FROM storage_site where id = ?''', [site_id]).fetchone()

    def directory(self, directory_id):
        '''returns directory tuple (id, name, site_id, site_name) for the given directory_id'''
        with sqlite3.connect(self.db_filename) as conn:
            return conn.execute('''SELECT dir.id, dir.name, site.id, site.name
                FROM storage_site_root
                join storage_site site on site.id = storage_site_root.storage_site_id
                join directory_closure dc on dc.ancestor_id = storage_site_root.directory_id
                join directory dir on dir.id = dc.descendant_id
                where dc.descendant_id = ?
                ''', [directory_id]).fetchone()

    def directory_id(self, site_id, directory_name):
        '''returns directory id for the given site_id, directory_name'''
        with sqlite3.connect(self.db_filename) as conn:
            result = conn.execute('''SELECT dir.id
                FROM storage_site_root
                join directory_closure dc on dc.ancestor_id = storage_site_root.directory_id
                join directory dir on dir.id = dc.descendant_id
                where storage_site_root.storage_site_id = ?
                and dir.name = ?
                ''', [site_id, directory_name]).fetchone()

            if result:
                return result[0]

            return -1

    def rootDirectories(self):
        '''returns list of all root directories (id, name, site_id, site_name) for all sites'''
        with sqlite3.connect(self.db_filename) as conn:
            return conn.execute('''
                SELECT *
                FROM root_directories
                ''').fetchall()

    def rootDirectoriesForSite(self, site_id):
        '''returns list of all root directories (id, name) for given site_id'''
        with sqlite3.connect(self.db_filename) as conn:
            return conn.execute('''SELECT dir_id, dir_name
                FROM root_directories
                where site_id = ?''', [site_id]).fetchall()

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

    def _date_bounded(self, query, args, table_column, from_date=None, to_date=None):
        result_query = query
        result_args = args
        if from_date:
            result_query += ' and %s >= ?' % table_column
            result_args += (from_date,)

        if to_date:
            result_query += ' and %s  <= ?' % table_column
            result_args += (to_date,)

        return result_query, result_args

    def filesInDirectory(self, directory_id, from_date=None, to_date=None):
        with sqlite3.connect(self.db_filename) as conn:
            query = '''SELECT * FROM fileinfo
            where directory_id = ?'''

            args = (directory_id,)

            query, args = self._date_bounded(query, args, 'fileinfo.creation_date', from_date, to_date)

            return conn.execute(query, args).fetchall()

    def numFilesInDirectory(self, directory_id, from_date=None, to_date=None):
        with sqlite3.connect(self.db_filename) as conn:
            query = '''SELECT count(id) FROM fileinfo
            where directory_id = ?'''

            args = (directory_id,)

            query, args = self._date_bounded(query, args, 'fileinfo.creation_date', from_date, to_date)

            result = conn.execute(query, args).fetchone()

            if result:
                return result[0]

            return 0

    def filesInTree(self, base_directory_id, from_date=None, to_date=None):
        with sqlite3.connect(self.db_filename) as conn:
            query = '''SELECT dir.id, dir.name, dc.depth, fileinfo.id, fileinfo.name, fileinfo.size, fileinfo.creation_date FROM directory_closure dc
            join directory dir on dir.id = dc.descendant_id
            join fileinfo on fileinfo.directory_id = dc.descendant_id
            where dc.ancestor_id = ?'''

            args = (base_directory_id,)

            query, args = self._date_bounded(query, args, 'fileinfo.creation_date', from_date, to_date)

            return conn.execute(query, args).fetchall()

    def numFilesInTree(self, base_directory_id, from_date=None, to_date=None):
        with sqlite3.connect(self.db_filename) as conn:
            query = '''
                SELECT sum(directory_stats.num_files) FROM directory_stats
                join directory_closure dc on dc.descendant_id = directory_stats.directory_id
                where ancestor_id = ?
                '''

            args = (base_directory_id,)

            query, args = self._date_bounded(query, args, 'directory_stats.min_file_creation_date', from_date=from_date)
            query, args = self._date_bounded(query, args, 'directory_stats.max_file_creation_date', to_date=to_date)

            result = conn.execute(query, args).fetchone()

            if result[0]:
                return result[0]

            return 0

    def totalFileSizeInTree(self, base_directory_id, from_date=None, to_date=None):
        with sqlite3.connect(self.db_filename) as conn:
            query = '''
                SELECT sum(directory_stats.total_file_size) FROM directory_stats
                join directory_closure dc on dc.descendant_id = directory_stats.directory_id
                where ancestor_id = ?
                '''
            args = (base_directory_id,)

            query, args = self._date_bounded(query, args, 'directory_stats.min_file_creation_date', from_date=from_date)
            query, args = self._date_bounded(query, args, 'directory_stats.max_file_creation_date', to_date=to_date)

            result = conn.execute(query, args).fetchone()

            if result[0]:
                return result[0]
            return 0

    def numFilesInSite(self, site_id, from_date=None, to_date=None):
        num_files = 0L

        root_dirs = self.rootDirectoriesForSite(site_id)

        for root_dir in root_dirs:
            num_files += long(self.numFilesInTree(root_dir[0], from_date, to_date))

        return num_files

    def totalFileSizeInSite(self, site_id, from_date=None, to_date=None):
        total_size = 0L

        root_dirs = self.rootDirectoriesForSite(site_id)

        for root_dir in root_dirs:
            total_size += long(self.totalFileSizeInTree(root_dir[0], from_date, to_date))

        return total_size

    def datetimeRangeOfFilesInTree(self, base_directory_id = None):
        with sqlite3.connect(self.db_filename) as conn:
            query = '''
                SELECT min(fileinfo.creation_date) as min_creation_date,
                max(fileinfo.creation_date) as max_creation_date
                FROM fileinfo
                '''
            args = []

            if base_directory_id:
                query += '''\njoin directory_closure dc on dc.descendant_id = fileinfo.directory_id
                where ancestor_id = ?'''
                args.append(base_directory_id)

            result = conn.execute(query, args).fetchone()

            if result[0]:
                format = '%Y-%m-%d %H:%M:%S %Z'
                return (datetime.datetime.strptime(result[0]+' UTC', format),
                        datetime.datetime.strptime(result[1]+' UTC', format))

            utcnow = datetime.datetime.utcnow()
            return (utcnow, utcnow)

    def mostRecentVisitDate(self):
        with sqlite3.connect(self.db_filename) as conn:
            result = conn.execute('''
                SELECT visit_date FROM scraper_last_directory_visit
                order by visit_date desc
                limit 1
                ''').fetchone()

            if result:
                format = '%Y-%m-%d %H:%M:%S.%f %Z'
                return datetime.datetime.strptime(result[0]+' UTC', format)

            return datetime.datetime(2011, 1, 1)

    def numDirectoriesNotVisitedSince(self, timestamp):
        with sqlite3.connect(self.db_filename) as conn:
            result = conn.execute('''
                SELECT count(directory_id) FROM scraper_last_directory_visit
                WHERE visit_date < ?
                ''', [timestamp]).fetchone()

            if result:
                return result[0]

            return 0

    def visitStats(self, before_timestamp = None):
        if not before_timestamp:
            before_timestamp = datetime.datetime.utcnow()

        sites = self.sites()
        siteStats = {}

        with sqlite3.connect(self.db_filename) as conn:

            for site in sites:
                site_id = site[0]
                site_name = site[1]
                siteStats[site_name] = {'id': site_id}

                visits = conn.execute('''
                    select *
                    from site_scraper_last_directoy_visit
                    where site_id = ?
                    and last_visit < ?
                    order by last_visit asc
                    ''', [site_id, before_timestamp]).fetchall()

                siteStats[site_name]['queue_length'] = len(visits)
                if len(visits) > 0:
                    siteStats[site_name]['least_recent_visited_dir_id'] = visits[0][2]
                    siteStats[site_name]['least_recent_visit'] = visits[0][4]

        return siteStats
