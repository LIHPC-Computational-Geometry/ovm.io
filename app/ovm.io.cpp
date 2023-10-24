// based on https://github.com/LIHPC-Computational-Geometry/genomesh/blob/main/apps/ovm_to_mesh.cpp

#include <geogram/basic/logger.h>       // for Logger::*
#include <geogram/basic/vecg.h>         // for vecng<>
#include <geogram/basic/geometry.h>     // for vec3
#include <geogram/basic/file_system.h>  // for extension()
#include <geogram/mesh/mesh.h>          // for Mesh
#include <geogram/mesh/mesh_io.h>       // for mesh_load(), mesh_save()

#include <OpenVolumeMesh/FileManager/FileManager.hh>
#include <OpenVolumeMesh/Mesh/HexahedralMesh.hh>
#include <OpenVolumeMesh/Mesh/PolyhedralMesh.hh>

#include <OpenVolumeMesh/Core/Handles.hh>
#include <OpenVolumeMesh/Core/BaseEntities.hh>
#include <OpenVolumeMesh/Core/Properties/PropertyPtr.hh>
#include <OpenVolumeMesh/Geometry/VectorT.hh>
#include <OpenVolumeMesh/Mesh/TetrahedralGeometryKernel.hh>
#include <OpenVolumeMesh/Core/GeometryKernel.hh>
#include <OpenVolumeMesh/Mesh/HexahedralMeshIterators.hh>

#include <iostream>
#include <vector>
#include <string>

using namespace GEO;

typedef vecng<8,int> vec8i;

int main(int argc, char** argv) {

    GEO::initialize();

    if (argc != 3) {
        Logger::err("I/O") << "Wrong usage" << std::endl;
        Logger::err("I/O") << "ex: ./ovm.io path/to/input.ovm path/to/output.mesh" << std::endl;
        return 1;
    }

    std::string input_path  = argv[1];
    std::string output_path = argv[2];

    Mesh input_mesh;

    ///////////////////////////////////////////////
    // Load input mesh and fill a GEO::Mesh
    ///////////////////////////////////////////////

    if(FileSystem::extension(input_path) == "ovm") {
        // assert mesh is an hexahedral mesh

        // Create an empty mesh object
        OpenVolumeMesh::GeometricHexahedralMeshV3d input_ovm_mesh;
        // Create file manager object
        OpenVolumeMesh::IO::FileManager fileManager;
        fileManager.readFile(input_path, input_ovm_mesh);

        std::vector<vec3> vertices(input_ovm_mesh.n_vertices());
        std::vector<vec8i> hexahedra(input_ovm_mesh.n_cells());

        int i=0;
        for (OpenVolumeMesh::VertexIter viter = input_ovm_mesh.v_iter(); viter.valid(); ++viter) {
            auto v = input_ovm_mesh.vertex(*viter);
            vertices[i].x = v[0];
            vertices[i].y = v[1];
            vertices[i].z = v[2];
            i++;
        }

        // Get handle of first cell
        OpenVolumeMesh::CellHandle ch = *(input_ovm_mesh.cells_begin());
        // Iterate over the hexahedra in the same sheet that are adjacent
        // to the reference hexahedron (cell handle ch)
        
        int row=0;
        for (OpenVolumeMesh::CellIter citer = input_ovm_mesh.c_iter(); citer.valid(); ++citer){
            OpenVolumeMesh::CellHandle ch = *(citer);
            int j=0;
            for(OpenVolumeMesh::HexVertexIter hiter = input_ovm_mesh.hv_iter(ch);
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

        // Write output .mesh hex-mesh
        // based on https://github.com/LIHPC-Computational-Geometry/genomesh/blob/main/src/dot_mesh_io.cpp
        // TODO : fill a GEO::Mesh, allowing to write (GEO::mesh_save()) as .geogram, .mesh, .meshb

        std::cout << "Saving hex mesh: " << output_path << std::endl;

        std::ofstream out_mesh;
        out_mesh.open(output_path);

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
    else {
        if(!mesh_load(input_path,input_mesh)) {
            Logger::err("I/O") << "Unable to open " << input_path << " with Geogram" << std::endl;
            return 1;
        }
    }

    ///////////////////////////////////////////////
    // Save output mesh
    ///////////////////////////////////////////////

    if(FileSystem::extension(output_path) == "ovm") {

        geo_assert(input_mesh.cells.nb()!=0); // must be a volume mesh
        geo_assert(input_mesh.cells.are_simplices()); // must only contain tetrahedra

        input_mesh.vertices.set_double_precision();

        // create an empty mesh object
        OpenVolumeMesh::GeometricPolyhedralMeshV3d output_ovm_mesh;

        // transfert vertices
        std::vector<OpenVolumeMesh::VertexHandle> Geogram_vertex_index_to_OVM_handle(input_mesh.vertices.nb());
        FOR(v,input_mesh.vertices.nb()) {
            vec3 point = input_mesh.vertices.point(v);
            Geogram_vertex_index_to_OVM_handle[v] = output_ovm_mesh.add_vertex(OpenVolumeMesh::Geometry::Vec3d(
                point.x,
                point.y,
                point.z
            ));
        }

        // transfert facets
        std::vector<OpenVolumeMesh::FaceHandle> Geogram_facet_index_to_OVM_handle(input_mesh.facets.nb());
        FOR(f,input_mesh.facets.nb()) {
            std::vector<OpenVolumeMesh::VertexHandle> vertices_of_current_facet;
            FOR(lv,3) { // for each local vertex of the current facet
                vertices_of_current_facet.push_back(
                    Geogram_vertex_index_to_OVM_handle[
                        input_mesh.facet_corners.vertex(input_mesh.facets.corner(f,lv))
                    ]
                );
            }
            Geogram_facet_index_to_OVM_handle[f] = output_ovm_mesh.add_face(vertices_of_current_facet);
        }

        FOR(c,input_mesh.cells.nb()) {
            FOR(lf,4) { // for each local facet of the current cell

                // THIS PART DOESN'T WORK YET
                // We have to construct halffaces for OVM from Geogram, which doesn't have halffaces but oriented cell facets
                // About the OVM format : https://github.com/cgg-bern/AlgoHex/issues/2#issuecomment-1733448574
                
                index_t current_facet = input_mesh.cells.facet(c,lf);
                index_t v0 = input_mesh.facet_corners.vertex(input_mesh.facets.corner(current_facet,0));
                index_t v1 = input_mesh.facet_corners.vertex(input_mesh.facets.corner(current_facet,1));
                index_t v2 = input_mesh.facet_corners.vertex(input_mesh.facets.corner(current_facet,2));
                // index_t v0 = input_mesh.facets.vertex(current_facet,0);
                // index_t v1 = input_mesh.facets.vertex(current_facet,1);
                // index_t v2 = input_mesh.facets.vertex(current_facet,2);
                index_t other_v = input_mesh.cells.corner(c,0);
                input_mesh.cells.find_tet_adjacent(c,c);
                // should I link to output_ovm_mesh.halfface_handle(to_handle[current_facet], 0) or ...(...,1) ?
                Logger::err("Debug") << "cell " << c << "local facet " << lf << " = vertices {" << v0 << "," << v1 << "," << v2 << "}, other is " << other_v << std::endl;
                // output_ovm_mesh.face_halffaces()

                // std::vector<OpenVolumeMesh::HalfFaceHandle> halffaces_of_current_cell;
                // halffaces_of_current_cell.push_back(output_ovm_mesh.halfface_handle(f0, 1)); // 2nd argument : flipped facet or not
                // halffaces_of_current_cell.push_back(output_ovm_mesh.halfface_handle(f1, 1));
                // halffaces_of_current_cell.push_back(output_ovm_mesh.halfface_handle(f2, 0));
                // halffaces_of_current_cell.push_back(output_ovm_mesh.halfface_handle(f3, 1));
                // output_ovm_mesh.add_cell(halffaces_of_current_cell);
            }
        }

        Logger::err("I/O") << "Cells transfert not implemented" << std::endl;
        geo_assert_not_reached;
        OpenVolumeMesh::IO::FileManager fileManager;
        fileManager.writeFile(output_path,output_ovm_mesh);

    }
    else {
        if(!mesh_save(input_mesh,output_path)) {
            Logger::err("I/O") << "Unable to export to " << output_path << " with Geogram" << std::endl;
            return 1;
        }
    }    

    return 0;
}
