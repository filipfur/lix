#include "convexhull.h"

#include <set>
#include <list>
#include <algorithm>

#include "glm/gtc/random.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

#include "primer.h"

bool popAlongDirection(std::vector<glm::vec3>& s, const glm::vec3& D, glm::vec3& rval)
{
    auto i = lix::indexAlongDirection(s, D);
    if(i < 0)
    {
        return false;
    }
    rval = s[i];
    s.erase(s.begin() + i);
    return true;
}

void checkFace(std::set<lix::Face*>& visible, lix::Face* face, const glm::vec3& p)
{
    if((glm::dot(face->normal, p) + face->D) > 0)
    {
        if(visible.emplace(face).second)
        {
            checkFace(visible, face->half_edge->opposite->face, p);
            checkFace(visible, face->half_edge->next->opposite->face, p);
            checkFace(visible, face->half_edge->next->next->opposite->face, p);
        }
    }
}

void printFaces(const std::list<lix::Face>& faces)
{
    for(auto& face : faces)
    {
        printf("\n%d\n", face.id);
        lix::Half_Edge* e = face.half_edge;
        do
        {
            printf(" <%d> next: <%d>, opposite: <%d> f%d\n", e->id, e->next->id, e->opposite ? e->opposite->id : -1, e->opposite ? e->opposite->face->id : -1);
            if(e->opposite)
            {
                assert(e->opposite->opposite == e);
            }
            assert(e->face == &face);
            e = e->next;
        } while(e != face.half_edge);
    }
}

lix::Convex_Hull::Convex_Hull(const std::vector<glm::vec3>& points)
{
    std::vector<glm::vec3> P{points.begin(), points.end()};
    printf("#P: %zu\n", P.size());
    //float BIG = 1.0e32f;
    glm::vec3 dir = glm::ballRand(1.0f);
    glm::vec3 A;
    glm::vec3 B;
    
    while(!popAlongDirection(P, dir, A))
    {
        dir = glm::ballRand(1.0f);
    }
    assert(popAlongDirection(P, -A, B));

    glm::vec3 AB = B - A;
    float t = -(glm::dot(AB, A) / glm::dot(AB, AB));
    glm::vec3 r = A + AB * t;

    assert(glm::dot(r, r) > 0);

    glm::vec3 C;
    assert(popAlongDirection(P, r, C));

    glm::vec3 AC = C - A;
    glm::vec3 ABC = glm::cross(AB, AC);

    if(glm::dot(ABC, -A) > 0)
    {
        std::swap(B, C);
        std::swap(AC, AB);
        ABC = -ABC;
    }

    glm::vec3 D;
    assert(popAlongDirection(P, -ABC, D));

    /*auto a = std::make_shared<Vertex>(A);
    auto b = std::make_shared<Vertex>(B);
    auto c = std::make_shared<Vertex>(C);
    auto d = std::make_shared<Vertex>(D);*/

    auto& abc = _faces.emplace_back(glm::normalize(ABC));
    auto& acd = _faces.emplace_back(glm::normalize(glm::cross(C - A, D - A)));
    auto& adb = _faces.emplace_back(glm::normalize(glm::cross(D - A, B - A)));
    auto& bdc = _faces.emplace_back(glm::normalize(glm::cross(D - B, C - B)));

    auto ab = new Half_Edge(A, &abc);
    auto bc = new Half_Edge(B, &abc);
    auto ca = new Half_Edge(C, &abc);

    auto ac = new Half_Edge(A, &acd);
    auto cd = new Half_Edge(C, &acd);
    auto da = new Half_Edge(D, &acd);

    auto ad = new Half_Edge(A, &adb);
    auto db = new Half_Edge(D, &adb);
    auto ba = new Half_Edge(B, &adb);

    auto bd = new Half_Edge(B, &bdc);
    auto dc = new Half_Edge(D, &bdc);
    auto cb = new Half_Edge(C, &bdc);

    connect(ab, bc, ba, &abc);
    connect(bc, ca, cb, &abc);
    connect(ca, ab, ac, &abc);

    connect(ac, cd, ca, &acd);
    connect(cd, da, dc, &acd);
    connect(da, ac, ad, &acd);

    connect(ad, db, da, &adb);
    connect(db, ba, bd, &adb);
    connect(ba, ad, ab, &adb);

    connect(bd, dc, db, &bdc);
    connect(dc, cb, cd, &bdc);
    connect(cb, bd, bc, &bdc);

    //printFaces(_faces);

    std::list<Face*> Q;
    std::for_each(std::begin(_faces), std::end(_faces), [&Q](Face& face) {
        Q.push_back(&face);
    });
    int ctr{0};
    while(!Q.empty())
    {
        lix::Face* face = Q.front();
        Q.erase(Q.begin());

        glm::vec3 p;
        if(!popAlongDirection(P, face->normal, p))
        {
            continue;
        }

        std::set<lix::Face*> visible;
        checkFace(visible, face, p);
        if(visible.size() < 1)
        {
            continue;
        }
        /*printf("p: %s visible faces:", glm::to_string(p).c_str());
        for(Face* f : visible)
        {
            printf(" id=%d", f->id);
        }
        printf("\n");*/

        auto fIt = visible.begin();
        while(fIt != visible.end())
        {
            Face* f = *fIt;
            //printf("face=%s %f\n", glm::to_string(f->normal).c_str(), f->D);

            f->unlink();

            Q.remove(f);
            _faces.remove_if([f](const auto& face){
                return f->id == face.id;
            });
            

            fIt = visible.erase(fIt);
        }

        if(ctr == 1000) break;

        Half_Edge* boundary{nullptr};
        for(const auto& f : _faces)
        {
            Half_Edge* he = f.half_edge;
            do
            {
                if(he->opposite == nullptr)
                {
                    boundary = he;
                    break;
                }
                else
                {
                    he = he->next;
                }
            } while(he != f.half_edge);
            if(boundary)
            {
                break;
            }
        }
        Half_Edge* he = boundary;
        Half_Edge* prev{nullptr};
        //bool first{true};
        std::vector<Face*> newFaces;
        do
        {
            //assert(he->opposite == nullptr);
            if(prev)
            {
                const glm::vec3& U = he->vertex;
                const glm::vec3& V = prev->vertex;
                const glm::vec3& W = p;
                
                const glm::vec3 UV = V - U;
                const glm::vec3 UW = W - U;
                const glm::vec3 UVW = glm::normalize(glm::cross(UV, UW));

                auto& uvw = _faces.emplace_back(glm::normalize(UVW));

                auto uv = new Half_Edge(A, &uvw);
                auto vw = new Half_Edge(B, &uvw);
                auto wu = new Half_Edge(C, &uvw);

                //printf("prev link: <%d> : f%d\n", prev->id, prev->face->id);
                connect(uv, vw, prev, &uvw);
                prev->opposite = uv;
                connect(vw, wu, nullptr, &uvw);
                connect(wu, uv, nullptr, &uvw);

                newFaces.push_back(&uvw);
                Q.push_back(&uvw);
            }
            prev = he;
            he = he->next;
            while(he != boundary && he->opposite != nullptr)
            {
                he = he->opposite->next;
            }
            //first = false;
        } while(prev->opposite == nullptr);

        for(size_t i{0}; i < newFaces.size(); ++i)
        {
            Face* f = newFaces[i];
            auto n = newFaces.size();
            auto k1 = (i == 0) ? (n - 1) : (i - 1);
            auto k2 = (i == (n - 1)) ? 0 : (i + 1);
            Face* fp = newFaces[k1];
            Face* fn = newFaces[k2];
            assert(f->half_edge->opposite == nullptr); // ca
            assert(f->half_edge->prev->opposite == nullptr); // bc
            f->half_edge->opposite = fn->half_edge->prev; // bc(i) -> ca(i+1)
            f->half_edge->prev->opposite = fp->half_edge; // ca(i) -> ca(i-1)
        }
        if(ctr == 1000)
        {
            break;
        }
        ++ctr;
    }

    printf("#faces: %zu\n", _faces.size());
    printf("A: %s\nB: %s\nC: %s\nD: %s\n",
        glm::to_string(A).c_str(),
        glm::to_string(B).c_str(),
        glm::to_string(C).c_str(),
        glm::to_string(D).c_str());

    //printFaces(_faces);
}

lix::Convex_Hull::~Convex_Hull() noexcept
{
    for(auto& face : _faces)
    {
        face.unlink();
    }
    _faces.clear();
}

void lix::Convex_Hull::add_point(const glm::vec3& /*p*/)
{
    
}

void lix::Convex_Hull::mesh_data(std::vector<float>& points, std::vector<unsigned int>& indices)
{
    for(const auto& face : _faces)
    {
        points.insert(points.end(), {
            face.half_edge->vertex.x,
            face.half_edge->vertex.y,
            face.half_edge->vertex.z,

            face.half_edge->next->vertex.x,
            face.half_edge->next->vertex.y,
            face.half_edge->next->vertex.z,

            face.half_edge->next->next->vertex.x,
            face.half_edge->next->next->vertex.y,
            face.half_edge->next->next->vertex.z,
        });

        unsigned int i = static_cast<unsigned int>(indices.size());
        indices.insert(indices.end(), {
            i,
            i + 1,
            i + 2
        });
    }
    printf("created mesh_data #faces=%zu #vertices=%zu #indices=%zu\n", _faces.size(), points.size(), indices.size());
}

void lix::Convex_Hull::connect(Half_Edge* self, Half_Edge* next, Half_Edge* opposite, Face* face)
{
    self->next = next;
    next->prev = self;
    self->opposite = opposite;
    self->face = face;
    face->half_edge = self;
    face->D = glm::dot(-face->half_edge->vertex, face->normal) / glm::length(face->normal);
}