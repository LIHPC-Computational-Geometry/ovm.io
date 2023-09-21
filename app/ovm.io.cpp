// based on https://github.com/LIHPC-Computational-Geometry/genomesh/blob/main/apps/ovm_to_mesh.cpp
// for now, only read a .ovm hex-mesh and only write a .mesh

#include <geogram/basic/logger.h>   // for Logger::*
#include <geogram/basic/vecg.h>     // for vecng<>
#include <geogram/basic/geometry.h> // for vec3

#include <OpenVolumeMesh/FileManager/FileManager.hh>
#include <OpenVolumeMesh/Mesh/HexahedralMesh.hh>

#include <OpenVolumeMesh/Core/OpenVolumeMeshHandle.hh>
#include <OpenVolumeMesh/Core/BaseEntities.hh>
#include <OpenVolumeMesh/Core/PropertyPtr.hh>
#include <OpenVolumeMesh/Core/PropertyDefines.hh>
#include <OpenVolumeMesh/Geometry/VectorT.hh>
#include <OpenVolumeMesh/Mesh/TetrahedralGeometryKernel.hh>
#include <OpenVolumeMesh/Core/GeometryKernel.hh>
#include <OpenVolumeMesh/Mesh/HexahedralMeshIterators.hh>

#include <iostream>
#include <vector>

using namespace GEO;

typedef vecng<8,int> vec8i;

int main(int argc, char** argv) {

    GEO::initialize();

    if (argc != 3) {
        Logger::err("I/O") << "Wrong usage" << std::endl;
        Logger::err("I/O") << "ex: ./ovm.io path/to/input.ovm path/to/output.mesh" << std::endl;
        return 1;
    }

    //// Load input .ovm hex-mesh ///////////////////////////////////////////////////

    // Create an empty mesh object
    OpenVolumeMesh::GeometricHexahedralMeshV3d input_mesh;
    // Create file manager object
    OpenVolumeMesh::IO::FileManager fileManager;
    fileManager.readFile(argv[1], input_mesh);

    std::vector<vec3> vertices(input_mesh.n_vertices());
    std::vector<vec8i> hexahedra(input_mesh.n_cells());

    int i=0;
    for (OpenVolumeMesh::VertexIter viter = input_mesh.v_iter(); viter.valid(); ++viter) {
        auto v = input_mesh.vertex(*viter);
        vertices[i].x = v[0];
        vertices[i].y = v[1];
        vertices[i].z = v[2];
        i++;
    }

    // Get handle of first cell
    OpenVolumeMesh::CellHandle ch = *(input_mesh.cells_begin());
    // Iterate over the hexahedra in the same sheet that are adjacent
    // to the reference hexahedron (cell handle ch)
    
    int row=0;
    for (OpenVolumeMesh::CellIter citer = input_mesh.c_iter(); citer.valid(); ++citer){
        OpenVolumeMesh::CellHandle ch = *(citer);
        int j=0;
        for(OpenVolumeMesh::HexVertexIter hiter = input_mesh.hv_iter(ch);
            hiter.valid(); ++hiter) {
            // Now dereferencing csc_it returns a cell handle
            OpenVolumeMesh::VertexHandle ch2 = *hiter;
            int id = ch2.idx();
            hexahedra[row][j] = id;
            j++;
        }

        // swap vertex 1 and 3
        // because OVM convention is
        //       5-------6
        //      /|      /|
        //     / |     / |
        //    3-------2  |
        //    |  4----|--7
        //    | /     | /
        //    |/      |/
        //    0-------1
        // and MEDIT (.mesh) convention is
        //       5-------6
        //      /|      /|
        //     / |     / |
        //    1-------2  |
        //    |  4----|--7
        //    | /     | /
        //    |/      |/
        //    0-------3

        int tmp = hexahedra[row][1];
        hexahedra[row][1] = hexahedra[row][3];
        hexahedra[row][3] = tmp;

        row++;
    }

    //// Write output .mesh hex-mesh ///////////////////////////////////////////////////

    // based on https://github.com/LIHPC-Computational-Geometry/genomesh/blob/main/src/dot_mesh_io.cpp
    // TODO : fill a GEO::Mesh, allowing to write (GEO::mesh_save()) as .geogram, .mesh, .meshb

    std::cout << "Saving hex mesh: " << argv[2] << std::endl;

    std::ofstream out_mesh;
    out_mesh.open(argv[2]);

    out_mesh << "MeshVersionFormatted 1\n";
    out_mesh << "Dimension\n3\n";

    out_mesh << "Vertices\n";
    out_mesh << vertices.size() << "\n";
    FOR(v,vertices.size()) { // for each vertex
        out_mesh << vertices[v].x << " " << vertices[v].y << " " << vertices[v].z << " 0\n";
    }

    out_mesh << "Hexahedra\n";
    out_mesh << hexahedra.size() << "\n";
    
    FOR(h,hexahedra.size()) { // for each hexahedron
        FOR(v,8) { // for each vertex of the current hexahedron
            // Warning : .mesh indices start at 1, not 0
            out_mesh << hexahedra[h][v] + 1 << " ";
        }
        out_mesh << " 0\n";
    }

    out_mesh << "End\n";
    out_mesh.close();

    return 0;
}
