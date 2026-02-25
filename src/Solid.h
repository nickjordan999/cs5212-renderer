#ifndef SOLID_H
#define SOLID_H

#include "vec.h"
#include <vector>
#include <cmath>

class Solid {
public:
    Solid(std::vector<vec3> vertices, std::vector<int> faces)
        : vertices(std::move(vertices)), faces(std::move(faces)) {}

    // Static factories for the five Platonic solids
    static Solid Tetrahedron() {
        return Solid(
            { vec3(1,1,1), vec3(1,-1,-1), vec3(-1,1,-1), vec3(-1,-1,1) },
            { 0,1,2, 0,3,1, 0,2,3, 1,3,2 }
        );
    }

    static Solid Cube() {
        return Solid(
            {
                vec3(1,1,1), vec3(1,1,-1), vec3(1,-1,1), vec3(1,-1,-1),
                vec3(-1,1,1), vec3(-1,1,-1), vec3(-1,-1,1), vec3(-1,-1,-1)
            },
            {
                0,2,3, 0,3,1,   // +X
                4,5,7, 4,7,6,   // -X
                0,1,5, 0,5,4,   // +Y
                2,6,7, 2,7,3,   // -Y
                0,4,6, 0,6,2,   // +Z
                1,3,7, 1,7,5    // -Z
            }
        );
    }

    static Solid Octahedron() {
        return Solid(
            {
                vec3(1,0,0), vec3(-1,0,0), vec3(0,1,0),
                vec3(0,-1,0), vec3(0,0,1), vec3(0,0,-1)
            },
            { 0,2,4, 0,4,3, 0,3,5, 0,5,2, 1,4,2, 1,3,4, 1,5,3, 1,2,5 }
        );
    }

    static Solid Icosahedron() {
        const float phi = (1.0f + std::sqrt(5.0f)) / 2.0f;
        return Solid(
            {
                vec3(0,1,phi), vec3(0,1,-phi),
                vec3(0,-1,phi), vec3(0,-1,-phi),
                vec3(1,phi,0), vec3(1,-phi,0),
                vec3(-1,phi,0), vec3(-1,-phi,0),
                vec3(phi,0,1), vec3(phi,0,-1),
                vec3(-phi,0,1), vec3(-phi,0,-1)
            },
            {
                0,2,8, 0,8,4, 0,4,6, 0,6,10, 0,10,2,   // top cap
                1,3,9, 1,9,4, 1,4,6, 1,6,11, 1,11,3,    // bottom cap
                2,8,5, 2,5,7, 2,7,10,                     // mid from v2
                3,9,5, 3,5,7, 3,7,11,                     // mid from v3
                8,5,9, 8,9,4,                              // mid from v8
                10,7,11, 10,11,6                           // mid from v10
            }
        );
    }

    static Solid Dodecahedron() {
        const float phi = (1.0f + std::sqrt(5.0f)) / 2.0f;
        const float psi = phi - 1.0f;
        return Solid(
            {
                vec3(1,1,1), vec3(1,1,-1), vec3(1,-1,1), vec3(1,-1,-1),
                vec3(-1,1,1), vec3(-1,1,-1), vec3(-1,-1,1), vec3(-1,-1,-1),
                vec3(0,psi,phi), vec3(0,-psi,phi),
                vec3(0,psi,-phi), vec3(0,-psi,-phi),
                vec3(psi,phi,0), vec3(-psi,phi,0),
                vec3(psi,-phi,0), vec3(-psi,-phi,0),
                vec3(phi,0,psi), vec3(-phi,0,psi),
                vec3(phi,0,-psi), vec3(-phi,0,-psi)
            },
            {
                0,8,9, 0,9,2, 0,2,16,
                0,12,13, 0,13,4, 0,4,8,
                0,16,18, 0,18,1, 0,1,12,
                9,6,17, 9,17,4, 9,4,8,
                2,14,15, 2,15,6, 2,6,9,
                18,3,14, 18,14,2, 18,2,16,
                13,5,10, 13,10,1, 13,1,12,
                4,13,5, 4,5,19, 4,19,17,
                6,15,7, 6,7,19, 6,19,17,
                14,3,11, 14,11,7, 14,7,15,
                18,1,10, 18,10,11, 18,11,3,
                5,10,11, 5,11,7, 5,7,19
            }
        );
    }

    const std::vector<vec3>& getVertices() const { return vertices; }
    const std::vector<int>& getFaces() const { return faces; }
    int faceCount() const { return static_cast<int>(faces.size()) / 3; }

private:
    std::vector<vec3> vertices;
    std::vector<int> faces;
};

#endif // SOLID_H
