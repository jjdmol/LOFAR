from __future__ import with_statement
from contextlib import closing

import sys

import monetdb.sql as db
from monetdb.sql import Error as Error

import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.baserecipe import BaseRecipe

header_line = """\
#(Name, Type, Ra, Dec, I, Q, U, V, MajorAxis, MinorAxis, Orientation, ReferenceFrequency='60e6', SpectralIndexDegree='0', SpectralIndex:0='0.0', SpectralIndex:1='0.0') = format
"""

query_skymodel = """
SELECT t0.catsrcname, t0.src_type, ra2bbshms(t0.ra), decl2bbsdms(t0.decl), t0.i, t0.q, t0.u, t0.v, t0.MajorAxis, t0.MinorAxis, t0.Orientation, t0.ReferenceFrequency, t0.SpectralIndexDegree, t0.SpectralIndex_0
FROM (
    SELECT CAST(
        TRIM(c1.catsrcname) AS VARCHAR(20)
    ) AS catsrcname,
    CASE WHEN c1.pa IS NULL
        THEN CAST('POINT' AS VARCHAR(20))
        ELSE CAST('GAUSSIAN' AS VARCHAR(20))
    END AS src_type,
    CAST(c1.ra AS VARCHAR(20)) AS ra,
    CAST(c1.decl AS VARCHAR(20)) AS decl,
    CAST(c1.i_int_avg AS VARCHAR(20)) AS i,
    CAST(0 AS VARCHAR(20)) AS q,
    CAST(0 AS VARCHAR(20)) AS u,
    CAST(0 AS VARCHAR(20)) AS v,
    CASE WHEN c1.pa IS NULL
        THEN CAST('' AS VARCHAR(20))
        ELSE CASE WHEN c1.major IS NULL
            THEN CAST('' AS VARCHAR(20))
            ELSE CAST(c1.major AS varchar(20))
        END
    END AS MajorAxis,
    CASE WHEN c1.pa IS NULL
        THEN CAST('' AS VARCHAR(20))
        ELSE CASE WHEN c1.minor IS NULL
            THEN CAST('' AS VARCHAR(20))
            ELSE CAST(c1.minor AS varchar(20))
        END
    END AS MinorAxis,
    CASE WHEN c1.pa IS NULL
        THEN CAST('' AS VARCHAR(20))
        ELSE CAST(c1.pa AS varchar(20))
    END AS Orientation,
    CAST(c1.freq_eff AS VARCHAR(20)) AS ReferenceFrequency,
    CASE WHEN si.spindx_degree IS NULL
        THEN CAST('' AS VARCHAR(20))
        ELSE CAST(si.spindx_degree AS VARCHAR(20))
    END AS SpectralIndexDegree,
    CASE WHEN si.spindx_degree IS NULL
        THEN CASE WHEN si.c0 IS NULL
            THEN CAST(0 AS varchar(20))
            ELSE CAST(si.c0 AS varchar(20))
        END
        ELSE CASE WHEN si.c0 IS NULL
            THEN CAST('' AS varchar(20))
            ELSE CAST(si.c0 AS varchar(20))
        END
    END AS SpectralIndex_0,
    CASE WHEN si.c1 IS NULL
        THEN CAST('' AS varchar(20))
        ELSE CAST(si.c1 AS varchar(20))
    END AS SpectralIndex_1
    FROM catalogedsources c1
    LEFT OUTER JOIN spectralindices si ON c1.catsrcid = si.catsrc_id
        WHERE c1.cat_id BETWEEN %s AND %s
        AND c1.ra BETWEEN %s AND %s
        AND c1.decl BETWEEN %s AND %s
        AND c1.i_int_avg > %s
) t0
"""

query_central = """
SELECT
    catsrcname, i_int
FROM
    nearestneighborincat(%s,%s,'%s')
"""


class skymodel(BaseRecipe):
    """
    Extract basic sky model information from database
    """
    inputs = {
        'db_host': ingredient.StringField(
            '--db-host',
            help="Host with MonetDB database instance",
            default="ldb001"
        ),
        'db_port': ingredient.IntField(
            '--db-port',
            help="Host with MonetDB database instance",
            default=50000
        ),
        'db_dbase': ingredient.StringField(
            '--db-dbase',
            help="Database name",
            default="gsm"
        ),
        'db_user': ingredient.StringField(
            '--db-user',
            help="Database user",
            default="gsm"
        ),
        'db_password': ingredient.StringField(
            '--db-password',
            help="Database password",
            default="msss"
        ),
        'ra': ingredient.FloatField(
            '--ra',
            help='RA of image centre (degrees)'
        ),
        'dec': ingredient.FloatField(
            '--dec',
            help='dec of image centre (degrees)'
        ),
        'search_size': ingredient.FloatField(
            '--search-size',
            help='Distance to search in each of RA/dec (degrees)'
        ),
        'min_flux': ingredient.FloatField(
            '--min-flux',
            help="Integrated flus threshold, in Jy, for source selection"
        ),
        'skymodel_file': ingredient.StringField(
            '--skymodel-file',
            help="Output file for BBS-format sky model definition"
        )
    }

    outputs = {
        'source_name': ingredient.StringField(),
        'source_flux': ingredient.FloatField()
    }

    def go(self):
        self.logger.info("Building sky model")
        super(skymodel, self).go()

        ra_min = self.inputs['ra'] - self.inputs['search_size']
        ra_max = self.inputs['ra'] + self.inputs['search_size']
        dec_min = self.inputs['dec'] - self.inputs['search_size']
        dec_max = self.inputs['dec'] + self.inputs['search_size']

        try:
            with closing(
                db.connect(
                    hostname=self.inputs["db_host"],
                    port=int(self.inputs["db_port"]),
                    database=self.inputs["db_dbase"],
                    username=self.inputs["db_user"],
                    password=self.inputs["db_password"]
                )
            ) as db_connection:
                with closing(db_connection.cursor()) as db_cursor:
                    db_cursor.execute(
                        query_central % (float(self.inputs['ra']), float(self.inputs['dec']), "VLSS")
                    )
                    self.outputs["source_name"], self.outputs["source_flux"] = db_cursor.fetchone()
                    self.logger.info("Central source is %s; flux %f" %
                        (self.outputs["source_name"], self.outputs["source_flux"])
                    )
                    db_cursor.execute(
                        query_skymodel % (
                            4, 4, # Only using VLSS for now
                            float(ra_min),
                            float(ra_max),
                            float(dec_min),
                            float(dec_max),
                            float(self.inputs['min_flux'])
                        )
                    )
                    results = db_cursor.fetchall()

        except db.Error, my_error:
            self.logger.warn("Failed to build sky model: %s " % (my_error))
            return 1

        try:
            with open(self.inputs['skymodel_file'], 'w') as file:
                file.write(header_line)
                file.writelines(", ".join(line) + ",\n" for line in results)
        except Exception, e:
            self.logger.warn("Failed to write skymodel file")
            self.logger.warn(str(e))
            return 1

        return 0

if __name__ == '__main__':
    sys.exit(skymodel().main())
