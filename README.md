# [OpenVolumeMesh](https://www.graphics.rwth-aachen.de/software/openvolumemesh/) ⇄ [Geogram](https://github.com/BrunoLevy/geogram) mesh conversion

The initial goal was to extract a piece of code from an [archived repo](https://github.com/LIHPC-Computational-Geometry/genomesh) whose submodules are obsolete.

The ultimate goal is to subclass `MeshIOHandler` in Geogram.

## Limitations

Mesh attributes are not handled.

### Tetrahedral meshes
↓Input \ Output→      | .ovm   | .mesh/.meshb/.geogram
----------------------|--------|-----------------------
.ovm                  | *N/A*  | ❌
.mesh/.meshb/.geogram | 🚧 WIP | *N/A*

### Hexahedral meshes
↓Input \ Output→        | .ovm   | .mesh/.meshb/.geogram
------------------------|--------|-----------------------
.ovm                    | *N/A*  | ✅
.mesh/.meshb/.geogram   | ❌     | *N/A*

## Build

```bash
./configure
cd build/<platform>
make
```

## Run

```bash
./ovm.io path/to/input/mesh path/to/output/mesh
```