#* ****************************************************************** *#
#*    OpenSees - Open System for Earthquake Engineering Simulation    *#
#*          Pacific Earthquake Engineering Research Center            *#
#* ****************************************************************** *#

target_sources(OPS_Runtime PRIVATE
# Interpreter
    "formats.cpp"
    "interpreter.cpp"
    "pragma.cpp"
    "sequential.cpp"
    "packages.cpp"
    "progress.cpp"

# Domain
    "commands.cpp"

    "domain/domain.cpp"
    "domain/element.cpp"
    "domain/nodes.cpp"
    "domain/runtime.cpp"
    "domain/rigid_links.cpp"
    "domain/recorder.cpp"
    "domain/TclUpdateMaterialStageCommand.cpp"
    "domain/TclUpdateMaterialCommand.cpp"
    "domain/parameter.cpp"
    "domain/sensitivity.cpp"

# Modeling
    "modeling/model.cpp"
    "modeling/nodes.cpp"
    "modeling/constraint.cpp"
    "modeling/geomTransf.cpp"
    #  "modeling/rigidLink.cpp"
    "modeling/element.cpp"
    "modeling/region.cpp"
    "modeling/nDMaterial.cpp"
    "modeling/section.cpp"
    "modeling/uniaxialMaterial.cpp"
    "modeling/uniaxial.cpp"
    "modeling/printing.cpp"
    "modeling/blockND.cpp"
    "modeling/Block2D.cpp"
    "modeling/Block3D.cpp"

    "invoking/invoke.cpp"
    "invoking/invoke_uniaxial.cpp"
    "invoking/invoke_section.cpp"
    "invoking/invoke_stress.cpp"

# Analysis
    "analysis/algorithm.cpp"
    "analysis/integrator.cpp"
    "analysis/analysis.cpp"
    "analysis/numberer.cpp"
    "analysis/ctest.cpp"
    "analysis/solver.cpp"
    "analysis/solver.hpp"

# LOADS & PATTERNS
    "loading/groundMotion.cpp"
    "loading/element_load.cpp"
    "loading/pattern.cpp"
    "loading/series.cpp"
    "loading/TclSeriesIntegratorCommand.cpp"
    #"domain/pattern/drm/TclPatternCommand.cpp"
)


