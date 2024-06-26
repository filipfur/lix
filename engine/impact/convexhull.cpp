#include "convexhull.h"

#include <set>
#include <list>
#include <algorithm>

#include "glm/gtc/random.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

#include "primer.h"

bool popAlongDirection(std::vector<lix::Vertex>& s, const glm::vec3& D, lix::Vertex& rval)
{
    int index{-1};
    float maxValue{-FLT_MAX}; //glm::dot(s[0], D)};

    assert(s.size() != 0);

    for(size_t i{0}; i < s.size(); ++i)
    {
        float value = glm::dot(s[i].position, D);
        if(value > maxValue)
        {
            index = static_cast<int>(i);
            maxValue = value;
        }
    }

    if(index < 0)
    {
        return false;
    }
    rval = s[index];
    s.erase(s.begin() + index);
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

auto toVertexVector(const std::vector<glm::vec3>& vector)
{
    std::vector<lix::Vertex> rval(vector.size());
    uint32_t index{0};
    std::transform(vector.begin(), vector.end(), rval.begin(), [&index](const glm::vec3& v) -> lix::Vertex {
        return {index++, v};
    });
    return rval;
}

lix::ConvexHull::ConvexHull(const std::vector<glm::vec3>& points)
    : ConvexHull{toVertexVector(points)}
{

}

lix::ConvexHull::ConvexHull(const std::vector<lix::Vertex>& points)
{
    std::vector<lix::Vertex> P{points.begin(), points.end()};
    glm::vec3 dir = glm::ballRand(1.0f);
    lix::Vertex A;
    lix::Vertex B;
    
    while(!popAlongDirection(P, dir, A))
    {
        dir = glm::ballRand(1.0f);
    }
    assert(popAlongDirection(P, -A.position, B));

    glm::vec3 AB = B.position - A.position;
    float t = -(glm::dot(AB, A.position) / glm::dot(AB, AB));
    glm::vec3 r = A.position + AB * t;

    assert(glm::dot(r, r) > 0);

    lix::Vertex C;
    bool status = popAlongDirection(P, r, C);
    if(!status)
    {
        printf("failed to pop P#=%zu\n", P.size());
        status = popAlongDirection(P, r, C);
    }

    glm::vec3 AC = C.position - A.position;
    glm::vec3 ABC = glm::cross(AB, AC);

    if(glm::dot(ABC, -A.position) > 0)
    {
        std::swap(B.position, C.position);
        std::swap(AC, AB);
        ABC = -ABC;
    }

    lix::Vertex D;
    assert(popAlongDirection(P, -ABC, D));

    auto& abc = _faces.emplace_back(glm::normalize(ABC));
    auto& acd = _faces.emplace_back(glm::normalize(glm::cross(C.position - A.position, D.position - A.position)));
    auto& adb = _faces.emplace_back(glm::normalize(glm::cross(D.position - A.position, B.position - A.position)));
    auto& bdc = _faces.emplace_back(glm::normalize(glm::cross(D.position - B.position, C.position - B.position)));

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

    addPoints(P);
}

lix::ConvexHull::~ConvexHull() noexcept
{
    for(auto& face : _faces)
    {
        face.unlink();
    }
    _faces.clear();
}

bool lix::ConvexHull::addPoint(const lix::Vertex& p, lix::Face* face, std::list<Face*>* Q)
{
    if(face == nullptr)
    {
        for(lix::Face& f : _faces)
        {
            if((glm::dot(f.normal, p.position) + f.D) > 0)
            {
                face = &f;
                break;
            }
        }
    }
    assert(face != nullptr);

    std::set<lix::Face*> visible;
    checkFace(visible, face, p.position);
    if(visible.size() < 1)
    {
        return false;
    }

    auto fIt = visible.begin();
    while(fIt != visible.end())
    {
        Face* f = *fIt;

        f->unlink();

        if(Q) Q->remove(f);
        _faces.remove_if([f](const auto& face){
            return f->id == face.id;
        });
        

        fIt = visible.erase(fIt);
    }

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
            const lix::Vertex& U = he->vertex;
            const lix::Vertex& V = prev->vertex;
            const lix::Vertex& W = p;
            
            const glm::vec3 UV = V.position - U.position;
            const glm::vec3 UW = W.position - U.position;
            const glm::vec3 UVW = glm::normalize(glm::cross(UV, UW));

            auto& uvw = _faces.emplace_back(glm::normalize(UVW));

            auto uv = new Half_Edge(U, &uvw);
            auto vw = new Half_Edge(V, &uvw);
            auto wu = new Half_Edge(W, &uvw);

            //printf("prev link: <%d> : f%d\n", prev->id, prev->face->id);
            connect(uv, vw, prev, &uvw);
            prev->opposite = uv;
            connect(vw, wu, nullptr, &uvw);
            connect(wu, uv, nullptr, &uvw);

            newFaces.push_back(&uvw);
            if(Q) Q->push_back(&uvw);
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
    return true;
}

void lix::ConvexHull::addPoints(std::vector<lix::Vertex>& P)
{
    std::list<Face*> Q;
    std::for_each(std::begin(_faces), std::end(_faces), [&Q](Face& face) {
        Q.push_back(&face);
    });
    while(!Q.empty() && !P.empty())
    {
        lix::Face* face = Q.front();
        Q.erase(Q.begin());

        lix::Vertex p;
        if(!popAlongDirection(P, face->normal, p))
        {
            continue;
        }
        if(!addPoint(p, face, &Q))
        {
            continue;
        }
    }
}

void lix::ConvexHull::meshData(std::vector<float>& points, std::vector<unsigned int>& indices) const
{
    for(const auto& face : _faces)
    {
        points.insert(points.end(), {
            face.half_edge->vertex.position.x,
            face.half_edge->vertex.position.y,
            face.half_edge->vertex.position.z,

            face.half_edge->next->vertex.position.x,
            face.half_edge->next->vertex.position.y,
            face.half_edge->next->vertex.position.z,

            face.half_edge->next->next->vertex.position.x,
            face.half_edge->next->next->vertex.position.y,
            face.half_edge->next->next->vertex.position.z,
        });

        unsigned int i = static_cast<unsigned int>(indices.size());
        indices.insert(indices.end(), {
            i,
            i + 1,
            i + 2
        });
    }
}

std::vector<glm::vec3> lix::ConvexHull::points() const
{
    std::vector<glm::vec3> rval;

    for(const auto& face : _faces)
    {
        rval.insert(rval.end(), {
            face.half_edge->vertex.position,
            face.half_edge->next->vertex.position,
            face.half_edge->next->next->vertex.position
        });
    }

    return rval;
}

void lix::ConvexHull::connect(Half_Edge* self, Half_Edge* next, Half_Edge* opposite, Face* face)
{
    self->next = next;
    next->prev = self;
    self->opposite = opposite;
    self->face = face;
    face->half_edge = self;
    face->D = glm::dot(-face->half_edge->vertex.position, face->normal) / glm::length(face->normal);
}