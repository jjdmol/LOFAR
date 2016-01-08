/*
# Copyright (C) 2012-2015  asTRON (Netherlands Institute for Radio Astronomy)
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
*/

-- $Id$

-- sqlite3 create script for ltastorageoverview database

PRAGMA foreign_keys = ON;

create table storage_site (
    id                  integer primary key autoincrement unique not null,
    name                text unique not null,
    url                 text not null);

create index ss_name_idx on storage_site(name);

create table directory (
    id                  integer primary key autoincrement unique not null,
    name                text key not null COLLATE NOCASE,
    parent_directory_id integer,
    foreign key (parent_directory_id) references directory(id) );

create table directory_closure (
    ancestor_id         integer not null,
    descendant_id       integer not null,
    depth               integer not null,
    primary key (ancestor_id, descendant_id)
    foreign key (ancestor_id) references directory(id)
    foreign key (descendant_id) references directory(id) );

create index dc_ancestor_id_idx on directory_closure(ancestor_id);
create index dc_descendant_id_idx on directory_closure(descendant_id);
create index dc_depth_idx on directory_closure(depth);

create trigger directory_closure_trigger
    after insert on directory
    begin
        insert into directory_closure (ancestor_id, descendant_id, depth) values (new.id, new.id, 0) ;

        insert into directory_closure (ancestor_id, descendant_id, depth)
            select p.ancestor_id, c.descendant_id, p.depth+c.depth+1
            from directory_closure p, directory_closure c
            where p.descendant_id=new.parent_directory_id and c.ancestor_id=new.id ;
    end;

create table storage_site_root (
    storage_site_id                 integer not null,
    directory_id                        integer not null,
    primary key (storage_site_id, directory_id),
    foreign key (storage_site_id) references storage_site(id),
    foreign key (directory_id) references directory(id) );

create index ssr_storage_site_id_idx on storage_site_root(storage_site_id);
create index ssr_directory_id_idx on storage_site_root(directory_id);

create table fileinfo (
    id                          integer primary key autoincrement not null,
    name                        text key not null,
    size                        integer not null,
    creation_date               datetime not null,
    directory_id                integer not null,
    foreign key (directory_id)  references directory(id) );

create index fi_directory_id_idx on fileinfo(directory_id);
create index fi_creation_date_idx on fileinfo(creation_date);

create table directory_stats (
    id                      integer primary key autoincrement unique not null,
    directory_id            integer unique not null,
    num_files               integer,
    total_file_size         integer,
    min_file_size           integer,
    max_file_size           integer,
    min_file_creation_date  datetime,
    max_file_creation_date  datetime,
    foreign key (directory_id) references directory(id) );

create index ds_directory_id_idx on directory_stats(directory_id);
create index ds_min_file_creation_date_idx on directory_stats(min_file_creation_date);
create index ds_max_file_creation_date_idx on directory_stats(max_file_creation_date);

create table _temp_fileinfo_for_dir_stats (
    size                        integer not null,
    creation_date               datetime not null );

create trigger fileinfo_to_directory_stats_trigger
    after insert on fileinfo
    begin
        insert or ignore into directory_stats (directory_id)
        values (new.directory_id) ;

        delete from _temp_fileinfo_for_dir_stats ;

        insert into _temp_fileinfo_for_dir_stats
          select fileinfo.size, fileinfo.creation_date from fileinfo
          where directory_id = new.directory_id ;

         update directory_stats set
            num_files=(select count(size) from _temp_fileinfo_for_dir_stats),
            total_file_size=(select sum(size) from _temp_fileinfo_for_dir_stats),
            min_file_size=(select min(size) from _temp_fileinfo_for_dir_stats),
            max_file_size=(select max(size) from _temp_fileinfo_for_dir_stats),
            min_file_creation_date=(select min(creation_date) from _temp_fileinfo_for_dir_stats),
            max_file_creation_date=(select max(creation_date) from _temp_fileinfo_for_dir_stats)
         where directory_id = new.directory_id ;
    end;

create table project (
    id              integer primary key autoincrement unique not null,
    name            text unique not null);

create index project_name_idx on project(name);

create table project_top_level_directory (
    project_id      integer,
    directory_id    integer,
    primary key (project_id, directory_id)
    foreign key (project_id) references project(id)
    foreign key (directory_id) references directory(id) );



create table scraper_last_directory_visit (
    directory_id       integer not null,
    visit_date         datetime not null,
    primary key (directory_id)
    foreign key (directory_id) references directory(id) );

create view root_directories as
    select dir.id as dir_id, dir.name as dir_name, ss.id as site_id, ss.name as site_name
        from storage_site_root
        join directory dir on dir.id = storage_site_root.directory_id
        join storage_site ss on ss.id = storage_site_root.storage_site_id ;

create view site_directory_tree as
    select rootdir.site_id as site_id,
        rootdir.site_name as site_name,
        rootdir.dir_id as rootdir_id,
        rootdir.dir_name as rootdir_name,
        dir.id as dir_id,
        dir.name as dir_name,
        dir.parent_directory_id as parent_directory_id,
        dc.depth as depth
        from root_directories rootdir
        inner join directory_closure dc on dc.ancestor_id = rootdir.dir_id
        inner join directory dir on dc.descendant_id = dir.id ;

create view site_scraper_last_directoy_visit as
    select rootdir.site_id as site_id,
        rootdir.site_name as site_name,
        dir.id as dir_id,
        dir.name as dir_name,
        sldv.visit_date as last_visit
        from root_directories rootdir
        inner join directory_closure dc on dc.ancestor_id = rootdir.dir_id
        inner join directory dir on dc.descendant_id = dir.id
        inner join scraper_last_directory_visit sldv on sldv.directory_id = dir.id ;

create view site_directory_file as
    select site.id as site_id,
        site.name as site_name,
        dir.id as dir_id,
        dir.name as dir_name,
        fileinfo.id as file_id,
        fileinfo.name as file_name,
        fileinfo.size as file_size,
        fileinfo.creation_date as file_creation_date
        from storage_site site
        join storage_site_root on storage_site_root.storage_site_id = site.id
        inner join directory_closure dc on dc.ancestor_id = storage_site_root.directory_id
        inner join directory dir on dc.descendant_id = dir.id
        inner join fileinfo on fileinfo.directory_id = dir.id ;

create view project_directory as
    select
        project.id as project_id,
        project.name as project_name,
        dir.id as dir_id,
        dir.name as dir_name
        from project_top_level_directory
        inner join project on project.id = project_top_level_directory.project_id
        inner join directory_closure dc on dc.ancestor_id = project_top_level_directory.directory_id
        inner join directory dir on dc.descendant_id = dir.id ;

create view project_directory_stats as
    select * from project_directory
    inner join directory_stats ds on ds.directory_id = project_directory.dir_id ;

