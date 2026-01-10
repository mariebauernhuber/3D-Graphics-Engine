#include "../include/mesh.hpp"
#include <fstream>
#include <sstream>
#include <iostream>


bool mesh::LoadFromObjectFile(const std::string& filename) {
    std::ifstream f(filename);
    if (!f.is_open())
        return false;

    std::vector<vec3d> verts;
    std::string line;

    while (std::getline(f, line)) {
        if (line.empty())
            continue;

        std::istringstream s(line);
        char type;

        s >> type;
        if (type == 'v') {
            vec3d v{};
            s >> v.x >> v.y >> v.z;
            verts.push_back(v);
        } else if (type == 'f') {
            int i0, i1, i2;
            s >> i0 >> i1 >> i2;
            // OBJ indices are 1-based
            triangle tri{
                verts[i0 - 1],
                verts[i1 - 1],
                verts[i2 - 1]
            };
            tris.push_back(tri);
        }
    }
    return true;
}
