#pragma once

#include <memory>
#include <cstdint>

#include "glm/glm.hpp"

namespace lix
{
    struct Vertex
    {
        Vertex(uint32_t id_, glm::vec3 position_)
            : id{id_}, position{position_}
        {

        }

        Vertex(glm::vec3 position_)
            : Vertex{0, position_}
        {

        }

        Vertex()
            : Vertex{0, {}}
        {

        }

        Vertex(const Vertex& other)
            : Vertex{other.id, other.position}
        {

        }

        Vertex(Vertex&& other)
            : Vertex{other.id, std::move(other.position)}
        {
            
        }

        Vertex& operator=(const Vertex& other)
        {
            id = other.id;
            position = other.position;
            return *this;
        }

        Vertex& operator=(Vertex&& other)
        {
            id = other.id;
            position = std::move(other.position);
            return *this;
        }

        ~Vertex() noexcept
        {

        }

        /*operator glm::vec3() const
        {
            return position;
        }*/

        uint32_t id;
        glm::vec3 position;
    };

    struct Half_Edge
    {
        Half_Edge(const lix::Vertex& vertex_, struct Face* face_) : vertex{vertex_}, face{face_}
        {
            static uint32_t nextId{0};
            id = nextId++;
        }

        uint32_t id;
        Vertex vertex;
        struct Face* face;
        Half_Edge* next;
        Half_Edge* prev;
        Half_Edge* opposite;
    };
    
    struct Face
    {
        Face(const glm::vec3& normal_) : normal{normal_}
        {
            static uint32_t nextId{0};
            id = nextId++;
        }

        void unlink()
        {
            Half_Edge* he = this->half_edge;
            do
            {
                Half_Edge* next = he->next;
                //printf("delete edge: <%d> opposite=%d\n", he->id, he->opposite ? he->opposite->face->id : - 1);
                if(he->opposite)
                {
                    he->opposite->opposite = nullptr;
                    he->opposite = nullptr;
                }
                he->prev = nullptr;
                he->next = nullptr;
                he->face = nullptr;
                delete(he);
                he = next;
            } while(he != this->half_edge);
            this->half_edge = nullptr;
        }

        uint32_t id;
        struct Half_Edge* half_edge;
        glm::vec3 normal;
        float D;
    };
}