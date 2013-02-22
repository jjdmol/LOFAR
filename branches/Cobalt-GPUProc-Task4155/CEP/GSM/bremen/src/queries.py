#!/usr/bin/python
"""
General query generator for GSM.
"""
import math
import os
import pwd
try:
    import pysvn

    SVN_LOCATION = os.path.join(os.getenv('HOME'), 'prog',
                                'LOFAR', 'CEP', 'GSM')
except ImportError:
    SVN_LOCATION = None


def get_svn_version():
    """
    Returns SVN version of the GSM.
    """
    if SVN_LOCATION:
        try:
            client = pysvn.Client(SVN_LOCATION)
            return client.info(SVN_LOCATION).data['revision'].number
        except:
            return -1
    else:
        return -1


def sql_insert_run():
    """
    Returns a query to insert a new run.
    """
    return """
insert into runs(status, user_id, process_id) values (0, '%s', '%s');
""" % (pwd.getpwuid(os.getuid())[0], os.getpid())


def makelistable(func):
    """
    Allows passing a list instead of the first string parameter.
    @return: comma-separated results of calls with each value from
    the list.
    """
    def listablefn(vlist, *params):
        if isinstance(vlist, list):
            return ','.join(func(col, *params) for col in vlist)
        else:
            return func(vlist, *params)
    return listablefn


def get_distance(runcat_alias, extract_alias):
    """
    Create a query part for association distance.

    @param runcat_alias: alias for runningcatalog;
    @param extract_alias: alias for extractedsources;
    """
    return """\
3600*DEGREES(2 * ASIN(SQRT(({0}.x - {1}.x) * ({0}.x - {1}.x)
                         + ({0}.y - {1}.y) * ({0}.y - {1}.y)
                         + ({0}.z - {1}.z) * ({0}.z - {1}.z)*0.5)))
""".format(runcat_alias, extract_alias)


def get_assoc_r(runcat_alias, extract_alias):
    """
    Create a query part for association distance.

    @param runcat_alias: alias for runningcatalog;
    @param extract_alias: alias for extractedsources;
    """
    return """\
    (({0}.wm_ra  - {1}.ra)*({0}.wm_ra  - {1}.ra ) * (1-{1}.z*{1}.z)
          / ({0}.wm_ra_err * {0}.wm_ra_err + {1}.ra_err * {1}.ra_err + 1e-6))
        + (({0}.wm_decl - {1}.decl) * ({0}.wm_decl - {1}.decl)
         / ({0}.wm_decl_err * {0}.wm_decl_err + {1}.decl_err * {1}.decl_err + 1e-6))
""".format(runcat_alias, extract_alias)


def get_assoc_r_extended(runcat_alias, extract_alias):
    """
    Create a query part for association distance.

    @param runcat_alias: alias for runningcatalog;
    @param extract_alias: alias for extractedsources;
    """
    return """\
DEGREES(2.0 * ASIN(0.5*SQRT(({0}.x - {1}.x) * ({0}.x - {1}.x)
                         + ({0}.y - {1}.y) * ({0}.y - {1}.y)
                         + ({0}.z - {1}.z) * ({0}.z - {1}.z))))/
({0}.wm_g_major*{0}.wm_g_major + {1}.g_major*{1}.g_major)
""".format(runcat_alias, extract_alias)


@makelistable
def get_column_insert(column_alias, prefix=None):
    """
    Columns for INSERT (4 columns with error support).
    """
    if prefix:
        prefix = '%s.' % prefix
    else:
        prefix = ''
    return """
{1}wm_{0}, {1}wm_{0}_err, {1}avg_w{0},{1}avg_weight_{0}""".format(column_alias,
                                                                   prefix)


@makelistable
def get_column_insert_values(column_alias):
    """
    Columns for SELECT (4 columns with error support).
    """
    return """
{0},{0}_err, {0}/({0}_err*{0}_err), 1/({0}_err*{0}_err)""".format(column_alias)


@makelistable
def get_column_update_total(column_alias, fluxes=False):
    """
    Update values of runningcatalog/runningcatalog_fluxes
    for a full recalculation of value with associations.
    Part I: update weights and weighted sums.
    Used in: Extended source merging.
    """
    suffix = 'runningcatalog.runcatid'
    if fluxes:
        suffix = 'runningcatalog_fluxes.runcat_id'
    return """avg_w{0} = (select sum(a.weight*e.{0}/(e.{0}_err*e.{0}_err))
                            from extractedsources e,
                                 assocxtrsources a
                           where e.xtrsrcid = a.xtrsrc_id
                             and a.runcat_id = {1}),
        avg_weight_{0} = (select sum(1/(e.{0}_err*e.{0}_err))
                            from extractedsources e,
                                 assocxtrsources a
                           where e.xtrsrcid = a.xtrsrc_id
                             and a.runcat_id = {1})
""".format(column_alias, suffix)


@makelistable
def get_column_update_second(column_alias):
    """
    Update values of runningcatalog/runningcatalog_fluxes
    for a full recalculation of value with associations.
    Part II: update values and errors.
    Used in: Extended source merging.
    """
    return """wm_{0} = avg_w{0}/avg_weight_{0},
wm_{0}_err = sqrt(1.0/avg_weight_{0})
""".format(column_alias)


def get_column_update(column_alias, new_value, new_weight):
    """
    Updater for error-columns for single item ipdate.
    """
    return """
wm_{0} = (avg_w{0} + {1}/({2}*{2}))/(avg_weight_{0} + 1/({2}*{2})),
wm_{0}_err = sqrt(1.0/(avg_weight_{0} + 1/({2}*{2}))),
avg_w{0} = avg_w{0} + {1}/({2}*{2}),
avg_weight_{0} = avg_weight_{0} + 1/({2}*{2})""".format(column_alias,
                                                        new_value, new_weight)


def get_column_deduct(column_alias, new_value, new_weight):
    """
    Updater for error-columns for single item ipdate.
    """
    return """
avg_w{0} = avg_w{0} - {1}/({2}*{2}),
avg_weight_{0} = avg_weight_{0} - 1/({2}*{2})""".format(column_alias,
                                                        new_value, new_weight)


def get_column_deduct_nonzero(column_alias, new_value, new_weight):
    """
    Updater for error-columns for single item ipdate.
    """
    return """
wm_{0} = avg_w{0}/avg_weight_{0},
wm_{0}_err = sqrt(1.0/avg_weight_{0})""".format(column_alias,
                                                        new_value, new_weight)


@makelistable
def get_column_update2(column_alias):
    """
    Updater for error-columns for single item ipdate.
    Used for updating by multiple new records (good for extendedsources).
    """
    return """
wm_{0} = (avg_w{0} + y.{0}_value)/(avg_weight_{0} + y.{0}_weight),
wm_{0}_err = sqrt(1.0/(avg_weight_{0} + y.{0}_weight)),
avg_w{0} = avg_w{0} + y.{0}_value,
avg_weight_{0} = avg_weight_{0} + y.{0}_weight""".format(column_alias)


@makelistable
def get_column_deduct2(column_alias):
    """
    Updater for error-columns for single item ipdate.
    Used for updating by multiple new records (good for extendedsources).
    """
    return """
avg_w{0} = avg_w{0} - y.{0}_value,
avg_weight_{0} = avg_weight_{0} - y.{0}_weight""".format(column_alias)


@makelistable
def get_column_deduct2_nonzero(column_alias):
    return """
wm_{0} = avg_w{0}/avg_weight_{0},
wm_{0}_err = sqrt(1.0/avg_weight_{0})""".format(column_alias)


@makelistable
def get_column_from(column_alias):
    """
    Fills "SELECT" in the subquery for multiple update.
    """
    return """
sum({0}/({0}_err*{0}_err)) as {0}_value,
sum(1/({0}_err*{0}_err)) as {0}_weight""".format(column_alias)


def get_field(ra, decl, radius, band, stokes='I', min_flux=None,
              min_datapoints=None):
    """
    Create a query to get sources for a given fov in a given band.
    """
    def get_field_conditions(x, y, z, r, min_flux, min_datapoints):
        sql = ''
        if min_flux:
            sql = "\n and f.wm_f_peak > %s" % min_flux
        if min_datapoints:
            sql = '%s\n and f.datapoints > %s' % (sql, min_datapoints)
        return """r.x * {0} + r.y * {1} + r.z * {2} > {3}
   and r.x between {0} - {3} and {0} + {3}
   and r.y between {1} - {3} and {1} + {3}
   and r.z between {2} - {3} and {2} + {3}
   and not r.deleted
   {4}""".format(x, y, z, r, sql)

    decl = math.radians(decl)
    ra = math.radians(ra)
    x = math.cos(decl) * math.cos(ra)
    y = math.cos(decl) * math.sin(ra)
    z = math.sin(decl)
    r = math.sin(math.radians(radius))
    sql = """select r.wm_ra as ra, r.wm_decl as decl, f.wm_f_peak
  from runningcatalog r,
       runningcatalog_fluxes f
 where {0}
   and r.source_kind = 0
   and f.runcat_id = r.runcatid
   and f.stokes = '{1}'
   and f.band = {2}
UNION
select r.wm_ra as ra, r.wm_decl as decl, f.wm_f_peak
  from runningcatalog r,
       runningcatalog_fluxes f
 where {0}
   and r.source_kind = 1
   and r.stokes = f.stokes
   and r.band = f.band
   and f.runcat_id = r.runcatid
   and f.stokes = '{1}'
   and f.band = {2}
""".format(get_field_conditions(x, y, z, r, min_flux, min_datapoints),
           stokes, band)
    return sql
