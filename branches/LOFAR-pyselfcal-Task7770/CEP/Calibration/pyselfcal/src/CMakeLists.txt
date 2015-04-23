# $Id: CMakeLists.txt 21998 2012-09-12 07:40:19Z vilchez $

include(PythonInstall)

 
########################################################################
# LSM selfcal dependent PART
######################################################################## 
 
  
########################################################################

python_install(
  LSM_module/__init__.py  
  LSM_module/lsmtool
  DESTINATION lofar/selfcal)  
    
########################################################################
python_install(
LSM_module/module/__init__.py
LSM_module/module/runtests.py
LSM_module/module/setup.py
DESTINATION lofar/selfcal)


install(FILES
LSM_module/module/README.md
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module/module)


python_install(
LSM_module/pyvo/ah_bootstrap.py
LSM_module/pyvo/distribute_setup.py
LSM_module/pyvo/ez_setup.py
LSM_module/pyvo/__init__.py
LSM_module/pyvo/setup.py
DESTINATION lofar/selfcal)


install(FILES
LSM_module/pyvo/MANIFEST.in
LSM_module/pyvo/pip-requirements
LSM_module/pyvo/README.rst
LSM_module/pyvo/setup.cfg
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module/pyvo) 


########################################################################
python_install(
LSM_module/module/bin/lsmtool
DESTINATION lofar/selfcal)


install(FILES
LSM_module/module/docs/lsmtool.aux
LSM_module/module/docs/lsmtool.log
LSM_module/module/docs/lsmtool.out
LSM_module/module/docs/lsmtool.pdf
LSM_module/module/docs/lsmtool.synctex.gz
LSM_module/module/docs/lsmtool.tex
LSM_module/module/docs/Makefile
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module/module/docs)


python_install(
LSM_module/module/lsmtool/__init__.py
LSM_module/module/lsmtool/_logging.py
LSM_module/module/lsmtool/operations_lib.py
LSM_module/module/lsmtool/skymodel.py
LSM_module/module/lsmtool/tableio.py
LSM_module/module/lsmtool/_version.py
DESTINATION lofar/selfcal)


install(FILES
LSM_module/module/parsets/lsmtool.parset
LSM_module/module/parsets/validation2.parset
LSM_module/module/parsets/validation.parset
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module/module/parsets)


install(FILES
LSM_module/module/tests/concat.sky
LSM_module/module/tests/final.sky
LSM_module/module/tests/flux_ratio_sky.pdf
LSM_module/module/tests/flux_ratio_vs_distance.pdf
LSM_module/module/tests/flux_ratio_vs_flux.pdf
LSM_module/module/tests/no_patches.sky
LSM_module/module/tests/patches.sky
LSM_module/module/tests/plot.pdf
LSM_module/module/tests/postional_offsets.pdf
LSM_module/module/tests/stats.txt
LSM_module/module/tests/test_blank.sky
LSM_module/module/tests/test_final.sky
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module/module/tests)


python_install(
LSM_module/module/tests/test_lsmtool.py
LSM_module/module/tests/validation2.py
LSM_module/module/tests/validation.py
DESTINATION lofar/selfcal)


install(FILES
LSM_module/module/tests/test_plot.pdf
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module)


install(FILES
LSM_module/pyvo/cextern/README.rst
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module/pyvo/cextern)


python_install(
LSM_module/pyvo/docs/conf.py
DESTINATION lofar/selfcal)


install(FILES
LSM_module/pyvo/docs/index.rst
LSM_module/pyvo/docs/make.bat
LSM_module/pyvo/docs/Makefile
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module/pyvo/docs)


install(FILES
LSM_module/pyvo/licenses/ASTROPY_LICENSE.rst
LSM_module/pyvo/licenses/LICENSE.rst
LSM_module/pyvo/licenses/README.rst
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module/pyvo/licenses)


python_install(
LSM_module/pyvo/pyvo/conftest.py
LSM_module/pyvo/pyvo/__init__.py
LSM_module/pyvo/pyvo/version.py
DESTINATION lofar/selfcal)


install(FILES
LSM_module/pyvo/pyvo/pyvo.cfg
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module/pyvo/pyvo)


python_install(
LSM_module/pyvo/scripts/ex_casA_image_cat.py
LSM_module/pyvo/scripts/ex_get_cutouts.py
DESTINATION lofar/selfcal)


install(FILES
LSM_module/pyvo/scripts/README.rst
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module/pyvo/scripts)


install(FILES
LSM_module/pyvo/tests/README.txt
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module/pyvo/tests)


python_install(
LSM_module/pyvo/tests/testLicense.py
DESTINATION lofar/selfcal)



########################################################################
python_install(
LSM_module/module/docs/source/conf.py
DESTINATION lofar/selfcal)


install(FILES
LSM_module/module/docs/source/index.rst
LSM_module/module/docs/source/lsmtool.rst
LSM_module/module/docs/source/overview.rst
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module/module/docs/source)


install(FILES
LSM_module/module/docs/test/lsmtool.operations.rst
LSM_module/module/docs/test/lsmtool.rst
LSM_module/module/docs/test/modules.rst
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module/module/docs/test)


python_install(
LSM_module/module/lsmtool/operations/add.py
LSM_module/module/lsmtool/operations/_cluster.py
LSM_module/module/lsmtool/operations/compare.py
LSM_module/module/lsmtool/operations/concatenate.py
LSM_module/module/lsmtool/operations/_filter.py
LSM_module/module/lsmtool/operations/group.py
LSM_module/module/lsmtool/operations/__init__.py
LSM_module/module/lsmtool/operations/_kdtree.py
LSM_module/module/lsmtool/operations/_matching_pre04.py
LSM_module/module/lsmtool/operations/_matching.py
LSM_module/module/lsmtool/operations/merge.py
LSM_module/module/lsmtool/operations/move.py
LSM_module/module/lsmtool/operations/plot.py
LSM_module/module/lsmtool/operations/remove.py
LSM_module/module/lsmtool/operations/select.py
LSM_module/module/lsmtool/operations/setpatchpositions.py
LSM_module/module/lsmtool/operations/_tessellate.py
LSM_module/module/lsmtool/operations/_threshold.py
LSM_module/module/lsmtool/operations/transfer.py
LSM_module/module/lsmtool/operations/ungroup.py
DESTINATION lofar/selfcal)


python_install(
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/git_helpers.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/__init__.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/setup_helpers.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/test_helpers.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/utils.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/version_helpers.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/version.py
DESTINATION lofar/selfcal)


install(FILES
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/EGG-INFO/dependency_links.txt
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/EGG-INFO/not-zip-safe
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/EGG-INFO/PKG-INFO
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/EGG-INFO/SOURCES.txt
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/EGG-INFO/top_level.txt
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/EGG-INFO)


install(FILES
LSM_module/pyvo/docs/pyvo/dal.rst
LSM_module/pyvo/docs/pyvo/data_access.rst
LSM_module/pyvo/docs/pyvo/getting_started.rst
LSM_module/pyvo/docs/pyvo/index.rst
LSM_module/pyvo/docs/pyvo/install.rst
LSM_module/pyvo/docs/pyvo/ivoareferences.rst
LSM_module/pyvo/docs/pyvo/mod.rst
LSM_module/pyvo/docs/pyvo/nameresolver.rst
LSM_module/pyvo/docs/pyvo/pyvo.dal.scs.SCSQuery.rst
LSM_module/pyvo/docs/pyvo/pyvo.dal.scs.SCSRecord.rst
LSM_module/pyvo/docs/pyvo/pyvo.dal.scs.SCSResults.rst
LSM_module/pyvo/docs/pyvo/pyvo.dal.scs.SCSService.rst
LSM_module/pyvo/docs/pyvo/pyvo.dal.sia.SIAQuery.rst
LSM_module/pyvo/docs/pyvo/pyvo.dal.sia.SIARecord.rst
LSM_module/pyvo/docs/pyvo/pyvo.dal.sia.SIAResults.rst
LSM_module/pyvo/docs/pyvo/pyvo.dal.sia.SIAService.rst
LSM_module/pyvo/docs/pyvo/pyvo.dal.sla.SLAQuery.rst
LSM_module/pyvo/docs/pyvo/pyvo.dal.sla.SLARecord.rst
LSM_module/pyvo/docs/pyvo/pyvo.dal.sla.SLAResults.rst
LSM_module/pyvo/docs/pyvo/pyvo.dal.sla.SLAService.rst
LSM_module/pyvo/docs/pyvo/pyvo.dal.ssa.SSAQuery.rst
LSM_module/pyvo/docs/pyvo/pyvo.dal.ssa.SSARecord.rst
LSM_module/pyvo/docs/pyvo/pyvo.dal.ssa.SSAResults.rst
LSM_module/pyvo/docs/pyvo/pyvo.dal.ssa.SSAService.rst
LSM_module/pyvo/docs/pyvo/ref.rst
LSM_module/pyvo/docs/pyvo/registry_access.rst
LSM_module/pyvo/docs/pyvo/registry.rst
LSM_module/pyvo/docs/pyvo/resolve_names.rst
LSM_module/pyvo/docs/pyvo/scs.rst
LSM_module/pyvo/docs/pyvo/sia.rst
LSM_module/pyvo/docs/pyvo/sla.rst
LSM_module/pyvo/docs/pyvo/ssa.rst
LSM_module/pyvo/docs/pyvo/vo.rst
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module/pyvo/docs/pyvo)


python_install(
LSM_module/pyvo/pyvo/dal/dbapi2.py
LSM_module/pyvo/pyvo/dal/__init__.py
LSM_module/pyvo/pyvo/dal/query.py
LSM_module/pyvo/pyvo/dal/scs.py
LSM_module/pyvo/pyvo/dal/sia.py
LSM_module/pyvo/pyvo/dal/sla.py
LSM_module/pyvo/pyvo/dal/ssa.py
DESTINATION lofar/selfcal)


install(FILES
LSM_module/pyvo/pyvo/data/README.rst
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module/pyvo/pyvo/data)


python_install(
LSM_module/pyvo/pyvo/nameresolver/__init__.py
LSM_module/pyvo/pyvo/nameresolver/sesame.py
DESTINATION lofar/selfcal)


python_install(
LSM_module/pyvo/pyvo/registry/__init__.py
LSM_module/pyvo/pyvo/registry/vao.py
DESTINATION lofar/selfcal)


python_install(
LSM_module/pyvo/pyvo/tests/__init__.py
DESTINATION lofar/selfcal)

########################################################################
install(FILES
LSM_module/module/docs/build/doctrees/code.doctree
LSM_module/module/docs/build/doctrees/environment.pickle
LSM_module/module/docs/build/doctrees/index.doctree
LSM_module/module/docs/build/doctrees/lsmtool.doctree
LSM_module/module/docs/build/doctrees/lsmtool.operations.doctree
LSM_module/module/docs/build/doctrees/modules.doctree
LSM_module/module/docs/build/doctrees/overview.doctree
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module/module/docs/build/doctrees)


install(FILES
LSM_module/module/docs/build/html/code.html
LSM_module/module/docs/build/html/genindex.html
LSM_module/module/docs/build/html/index.html
LSM_module/module/docs/build/html/lsmtool.html
LSM_module/module/docs/build/html/lsmtool.operations.html
LSM_module/module/docs/build/html/modules.html
LSM_module/module/docs/build/html/objects.inv
LSM_module/module/docs/build/html/overview.html
LSM_module/module/docs/build/html/py-modindex.html
LSM_module/module/docs/build/html/search.html
LSM_module/module/docs/build/html/searchindex.js
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module/module/docs/build/html)


install(FILES
LSM_module/module/docs/build/latex/fncychap.sty
LSM_module/module/docs/build/latex/LSMTool.aux
LSM_module/module/docs/build/latex/LSMTool.idx
LSM_module/module/docs/build/latex/LSMTool.ilg
LSM_module/module/docs/build/latex/LSMTool.ind
LSM_module/module/docs/build/latex/LSMTool.log
LSM_module/module/docs/build/latex/LSMTool.out
LSM_module/module/docs/build/latex/LSMTool.pdf
LSM_module/module/docs/build/latex/LSMTool.tex
LSM_module/module/docs/build/latex/LSMTool.toc
LSM_module/module/docs/build/latex/Makefile
LSM_module/module/docs/build/latex/python.ist
LSM_module/module/docs/build/latex/sphinxhowto.cls
LSM_module/module/docs/build/latex/sphinxmanual.cls
LSM_module/module/docs/build/latex/sphinx.sty
LSM_module/module/docs/build/latex/tabulary.sty
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module/module/docs/build/latex)


python_install(
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/compat/__init__.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/compat/subprocess.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/conf.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/__init__.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/setup_package.py
DESTINATION lofar/selfcal)

python_install(
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/src/__init__.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/src/setup_package.py
DESTINATION lofar/selfcal)

install(FILES
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/src/compiler.c
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/src)


install(FILES
LSM_module/pyvo/docs/_templates/autosummary/base.rst
LSM_module/pyvo/docs/_templates/autosummary/class.rst
LSM_module/pyvo/docs/_templates/autosummary/module.rst
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module/pyvo/docs/_templates/autosummary)


python_install(
LSM_module/pyvo/pyvo/dal/tests/aTestSIAServer.py
LSM_module/pyvo/pyvo/dal/tests/__init__.py
LSM_module/pyvo/pyvo/dal/tests/setup_package.py
LSM_module/pyvo/pyvo/dal/tests/test_CS.py
LSM_module/pyvo/pyvo/dal/tests/test_QueryNoNet.py
LSM_module/pyvo/pyvo/dal/tests/test_SIANeedsNet.py
LSM_module/pyvo/pyvo/dal/tests/test_SIA.py
LSM_module/pyvo/pyvo/dal/tests/test_SLA.py
LSM_module/pyvo/pyvo/dal/tests/test_SSA.py
DESTINATION lofar/selfcal)


python_install(
LSM_module/pyvo/pyvo/nameresolver/tests/__init__.py
LSM_module/pyvo/pyvo/nameresolver/tests/setup_package.py
LSM_module/pyvo/pyvo/nameresolver/tests/test_SesameNeedsNet.py
LSM_module/pyvo/pyvo/nameresolver/tests/test_SesameNoNet.py
DESTINATION lofar/selfcal)


python_install(
LSM_module/pyvo/pyvo/registry/tests/__init__.py
LSM_module/pyvo/pyvo/registry/tests/setup_package.py
LSM_module/pyvo/pyvo/registry/tests/test_VaoRegNeedsNet.py
LSM_module/pyvo/pyvo/registry/tests/test_VaoReg.py
DESTINATION lofar/selfcal)

########################################################################
install(FILES
LSM_module/module/docs/build/html/_sources/code.txt
LSM_module/module/docs/build/html/_sources/index.txt
LSM_module/module/docs/build/html/_sources/lsmtool.operations.txt
LSM_module/module/docs/build/html/_sources/lsmtool.txt
LSM_module/module/docs/build/html/_sources/modules.txt
LSM_module/module/docs/build/html/_sources/overview.txt
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module/module/docs/build/html/_sources)


install(FILES
LSM_module/module/docs/build/html/_static/ajax-loader.gif
LSM_module/module/docs/build/html/_static/basic.css
LSM_module/module/docs/build/html/_static/comment-bright.png
LSM_module/module/docs/build/html/_static/comment-close.png
LSM_module/module/docs/build/html/_static/comment.png
LSM_module/module/docs/build/html/_static/default.css
LSM_module/module/docs/build/html/_static/doctools.js
LSM_module/module/docs/build/html/_static/down.png
LSM_module/module/docs/build/html/_static/down-pressed.png
LSM_module/module/docs/build/html/_static/file.png
LSM_module/module/docs/build/html/_static/jquery.js
LSM_module/module/docs/build/html/_static/minus.png
LSM_module/module/docs/build/html/_static/plus.png
LSM_module/module/docs/build/html/_static/pygments.css
LSM_module/module/docs/build/html/_static/searchtools.js
LSM_module/module/docs/build/html/_static/sidebar.js
LSM_module/module/docs/build/html/_static/underscore.js
LSM_module/module/docs/build/html/_static/up.png
LSM_module/module/docs/build/html/_static/up-pressed.png
LSM_module/module/docs/build/html/_static/websupport.js
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module/module/docs/build/html/_static)


python_install(
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/ext/astropyautosummary.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/ext/automodapi.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/ext/automodsumm.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/ext/changelog_links.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/ext/comment_eater.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/ext/compiler_unparse.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/ext/docscrape.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/ext/docscrape_sphinx.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/ext/doctest.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/ext/edit_on_github.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/ext/__init__.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/ext/numpydoc.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/ext/phantom_import.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/ext/smart_resolver.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/ext/tocdepthfix.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/ext/traitsdoc.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/ext/utils.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/ext/viewcode.py
DESTINATION lofar/selfcal)


install(FILES
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/local/python3links.inv
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/local)


install(FILES
LSM_module/pyvo/pyvo/dal/tests/data/error2-cs.xml
LSM_module/pyvo/pyvo/dal/tests/data/error2-sia.xml
LSM_module/pyvo/pyvo/dal/tests/data/error3-cs.xml
LSM_module/pyvo/pyvo/dal/tests/data/error4-cs.xml
LSM_module/pyvo/pyvo/dal/tests/data/error-cs.xml
LSM_module/pyvo/pyvo/dal/tests/data/error-sia.xml
LSM_module/pyvo/pyvo/dal/tests/data/error-sla.xml
LSM_module/pyvo/pyvo/dal/tests/data/error-ssa.xml
LSM_module/pyvo/pyvo/dal/tests/data/jhu-ssa.xml
LSM_module/pyvo/pyvo/dal/tests/data/neat-sia.xml
LSM_module/pyvo/pyvo/dal/tests/data/nrao-sla.xml
LSM_module/pyvo/pyvo/dal/tests/data/twomass-cs.xml
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module/pyvo/pyvo/dal/tests/data)


install(FILES
LSM_module/pyvo/pyvo/nameresolver/tests/data/sesame.xml
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module/pyvo/pyvo/nameresolver/tests/data)


install(FILES
LSM_module/pyvo/pyvo/registry/tests/data/reg-kw-galaxy.xml
LSM_module/pyvo/pyvo/registry/tests/data/reg-short.xml
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module/pyvo/pyvo/registry/tests/data)

########################################################################
python_install(
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/ext/tests/__init__.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/ext/tests/test_automodapi.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/ext/tests/test_automodsumm.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/ext/tests/test_docscrape.py
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/ext/tests/test_utils.py
DESTINATION lofar/selfcal)
  
  
install(FILES
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/themes/bootstrap-astropy/globaltoc.html
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/themes/bootstrap-astropy/layout.html
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/themes/bootstrap-astropy/localtoc.html
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/themes/bootstrap-astropy/searchbox.html
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/themes/bootstrap-astropy/theme.conf
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/themes/bootstrap-astropy)

########################################################################
install(FILES
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/ext/templates/autosummary_core/base.rst
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/ext/templates/autosummary_core/class.rst
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/ext/templates/autosummary_core/module.rst
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/ext/templates/autosummary_core)


install(FILES
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/themes/bootstrap-astropy/static/astropy_linkout_20.png
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/themes/bootstrap-astropy/static/astropy_logo_32.png
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/themes/bootstrap-astropy/static/astropy_logo.ico
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/themes/bootstrap-astropy/static/bootstrap-astropy.css
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/themes/bootstrap-astropy/static/copybutton.js
LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/themes/bootstrap-astropy/static/sidebar.js
DESTINATION lib/python2.7/dist-packages/lofar/selfcal/LSM_module/pyvo/astropy_helpers-0.4.3-py2.6.egg/astropy_helpers/sphinx/themes/bootstrap-astropy/static)  




########################################################################
# Selfcal PART
########################################################################


python_install(
  class_obsPreprocessing.py
  class_mergesubbands.py
  class_obsparMergedData.py
  class_obspar.py
  class_selfcalparam.py
  class_selfcalrun.py
  class_dataInit.py
  class_selfcalPeeling.py 
  old_class_obsparMergedData.py
  old_class_selfcalparam.py
  old_class_selfcalrun.py
  DESTINATION lofar/selfcal)
    

install(PROGRAMS
  selfcal.py
  mergeSB.py 
  old_selfcal.py
  DESTINATION bin)



