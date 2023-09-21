# [OpenVolumeMesh](https://www.graphics.rwth-aachen.de/software/openvolumemesh/) ⇄ [Geogram](https://github.com/BrunoLevy/geogram) mesh conversion

## Limitations

☹️ Only works for hexahedral meshes <br/>
☹️ The input mesh must be in the OVM format (.ovm) <br/>
☹️ The output mesh must be in the MEDIT format (.mesh)

So, far from a general & bidirectional mesh converter.

The initial goal was to extract a piece of code from an [archived repo](https://github.com/LIHPC-Computational-Geometry/genomesh) whose submodules are obsolete.

The ultimate goal is to subclass `MeshIOHandler` in Geogram.