#AM_CXXFLAGS = -Wno-ignored-qualifiers -Wconversion -Werror
lib_LTLIBRARIES = libbbskernel.la \
	element_beam_YATAWATTA_LBA_phi.la \
	element_beam_YATAWATTA_LBA_theta.la \
	element_beam_YATAWATTA_HBA_phi.la \
	element_beam_YATAWATTA_HBA_theta.la

element_beam_YATAWATTA_LBA_phi_la_SOURCES = Contrib/SBY_DipoleBeam/beam_dr_phi.c
element_beam_YATAWATTA_LBA_phi_la_LDFLAGS = -module
element_beam_YATAWATTA_LBA_theta_la_SOURCES = Contrib/SBY_DipoleBeam/beam_dr_theta.c
element_beam_YATAWATTA_LBA_theta_la_LDFLAGS = -module
element_beam_YATAWATTA_HBA_phi_la_SOURCES = Contrib/SBY_DipoleBeam/hba_beam_phi.c
element_beam_YATAWATTA_HBA_phi_la_LDFLAGS = -module
element_beam_YATAWATTA_HBA_theta_la_SOURCES = Contrib/SBY_DipoleBeam/hba_beam_theta.c
element_beam_YATAWATTA_HBA_theta_la_LDFLAGS = -module

coeffdir = $(datadir)
dist_coeff_DATA = Contrib/JPH_DipoleBeam/element_beam_HAMAKER_LBA.coeff \
		  Contrib/JPH_DipoleBeam/element_beam_HAMAKER_HBA.coeff

libbbskernel_la_SOURCES = Package__Version.cc \
	ElementBeamExpr.cc \
	Equator.cc \
	Evaluator.cc \
	IonosphereExpr.cc \
	Instrument.cc \
	MeasurementAIPS.cc \
	Model.cc \
	ModelConfig.cc \
	ParmManager.cc \
	ParmProxy.cc \
	Solver.cc \
	SolverInterfaceTypes.cc \
	VisData.cc \
	VisDimensions.cc \
	VisExpr.cc \
	VisSelection.cc \
	Expr/EquatorialCentroid.cc \
	Expr/ArrayFactor.cc \
	Expr/AzEl.cc \
	Expr/Cache.cc \
	Expr/ConditionNumber.cc \
	Expr/Expr.cc \
	Expr/ExprAdaptors.cc \
	Expr/ExprParm.cc \
	Expr/ExprValue.cc \
	Expr/ExprValueView.cc \
	Expr/ExprValueIterator.cc \
	Expr/ExprVisData.cc \
	Expr/ExternalFunction.cc \
	Expr/FaradayRotation.cc \
	Expr/FlagArray.cc \
	Expr/GaussianCoherence.cc \
	Expr/GaussianSource.cc \
	Expr/HamakerDipole.cc \
	Expr/Literal.cc \
	Expr/LMN.cc \
	Expr/Matrix.cc \
	Expr/MatrixComplexArr.cc \
	Expr/MatrixComplexSca.cc \
	Expr/MatrixInverse.cc \
	Expr/MatrixMul2.cc \
	Expr/MatrixMul3.cc \
	Expr/MatrixRealArr.cc \
	Expr/MatrixRealSca.cc \
	Expr/MatrixRep.cc \
	Expr/MatrixSum.cc \
	Expr/MatrixTmp.cc \
	Expr/PhaseShift.cc \
	Expr/PiercePoint.cc \
	Expr/PointCoherence.cc \
	Expr/PointSource.cc \
	Expr/PolynomialLayer.cc \
	Expr/Request.cc \
	Expr/Resampler.cc \
	Expr/ScalarMatrixMul.cc \
	Expr/Scope.cc \
	Expr/Source.cc \
	Expr/SpectralIndex.cc \
	Expr/StationShift.cc \
	Expr/StationUVW.cc \
	Expr/TileArrayFactor.cc \
	Expr/ValueSet.cc \
	Expr/YatawattaDipole.cc

bin_PROGRAMS = versionbbskernel

versionbbskernel_SOURCES      = versionbbskernel.cc
versionbbskernel_LDADD        = libbbskernel.la
versionbbskernel_DEPENDENCIES = libbbskernel.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
