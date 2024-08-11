#include "convexhull.h"

#include <set>
#include <list>
#include <algorithm>

#include "glm/gtc/random.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

#include "primer.h"

#define print_vec(v) printf("%s: [%.2f %.2f %.2f]\n", #v, v.x, v.y, v.z)

bool popAlongDirection(std::vector<glm::vec3>& s, const glm::vec3& D, glm::vec3& rval)
{
    auto [index, distance] = lix::indexAlongDirection(s, D);

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
    //if((glm::dot(face->normal, p) + face->D) > FLT_EPSILON)
    if(glm::dot(face->normal, p - face->half_edge->vertex) > FLT_EPSILON)
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

/*auto toVertexVector(const std::vector<glm::vec3>& vector)
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

}*/

lix::ConvexHull::ConvexHull(const std::vector<glm::vec3>& points)
{
    std::vector<glm::vec3> P{points.begin(), points.end()};
    glm::vec3 dir = glm::vec3{1.0f, 0.0f, 0.0f};//glm::ballRand(1.0f);
    glm::vec3 A;
    glm::vec3 B;
    
    assert(popAlongDirection(P, dir, A));
    assert(popAlongDirection(P, -A, B));

    glm::vec3 AB = B - A;
    float t = -(glm::dot(AB, A) / glm::dot(AB, AB));
    glm::vec3 r = A + AB * t;

    assert(glm::dot(r, r) >= 0);

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

    assert(!lix::isSameVertex(A, D) && !lix::isSameVertex(B, D) && !lix::isSameVertex(C, D));

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

bool lix::ConvexHull::addPoint(const glm::vec3& p, lix::Face* face, std::list<Face*>* Q)
{
    if(face == nullptr)
    {
        for(lix::Face& f : _faces)
        {
            if(glm::dot(f.normal, p - f.half_edge->vertex) > FLT_EPSILON)
            //if((glm::dot(f.normal, p) + f.D) > FLT_EPSILON)
            {
                face = &f;
                break;
            }
        }
    }
    assert(face != nullptr);

    std::set<lix::Face*> visible;
    checkFace(visible, face, p);
    if(visible.size() < 1)
    {
        //printf("no visibles!\n");
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
            const glm::vec3& U = he->vertex;
            const glm::vec3& V = prev->vertex;
            const glm::vec3& W = p;
            
            const glm::vec3 UV = V - U;
            const glm::vec3 UW = W - U;
            const glm::vec3 UVW = glm::cross(UV, UW);
            const glm::vec3 UVW_norm = glm::normalize(UVW);

            auto& uvw = _faces.emplace_back(glm::normalize(UVW_norm));

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

void lix::ConvexHull::addPoints(std::vector<glm::vec3>& P)
{
    std::list<Face*> Q;
    std::for_each(std::begin(_faces), std::end(_faces), [&Q](Face& face) {
        Q.push_back(&face);
    });
    while(!Q.empty() && !P.empty())
    {
        lix::Face* face = Q.front();
        Q.erase(Q.begin());

        glm::vec3 p;
        if(!popAlongDirection(P, face->normal, p))
        {
            continue;
        }
        if(!addPoint(p, face, &Q))
        {
           P.push_back(p); // hey, it works.
        }
    }
}

std::pair<std::vector<glm::vec3>, std::vector<unsigned int>> lix::ConvexHull::meshData() const
{
    std::vector<glm::vec3> points;
    points.reserve(_faces.size() * 3);
    std::vector<unsigned int> indices;
    indices.reserve(_faces.size() * 3);
    for(const auto& face : _faces)
    {
        points.insert(points.end(), {
            face.half_edge->vertex,
            face.half_edge->next->vertex,
            face.half_edge->next->next->vertex
        });

        unsigned int i = static_cast<unsigned int>(indices.size());
        indices.insert(indices.end(), {
            i,
            i + 1,
            i + 2
        });
    }
    return {points, indices};
}

struct Vec3Comparator {
    bool operator()(const glm::vec3& lhs, const glm::vec3& rhs) const {
        if (lhs.x != rhs.x) return lhs.x < rhs.x;
        if (lhs.y != rhs.y) return lhs.y < rhs.y;
        return lhs.z < rhs.z;
    }
};

std::vector<glm::vec3> lix::ConvexHull::points() const
{
    std::vector<glm::vec3> rval;

    for(const auto& face : _faces)
    {
        rval.insert(rval.end(), {
            face.half_edge->vertex,
            face.half_edge->next->vertex,
            face.half_edge->next->next->vertex
        });
    }

    return rval;
}

std::vector<glm::vec3> lix::ConvexHull::uniquePoints() const
{
    std::set<glm::vec3, Vec3Comparator> rval;

    for(const auto& face : _faces)
    {
        /*rval.insert(rval.end(), {
            face.half_edge->vertex,
            face.half_edge->next->vertex,
            face.half_edge->next->next->vertex
        });*/
        rval.emplace(face.half_edge->vertex);
        rval.emplace(face.half_edge->next->vertex);
        rval.emplace(face.half_edge->next->next->vertex);
    }

    return {rval.begin(), rval.end()};
}

void lix::ConvexHull::connect(Half_Edge* self, Half_Edge* next, Half_Edge* opposite, Face* face)
{
    self->next = next;
    next->prev = self;
    self->opposite = opposite;
    self->face = face;
    face->half_edge = self;
    face->D = glm::dot(-face->half_edge->vertex, face->normal);
}

std::optional<glm::vec3> lix::ConvexHull::rayIntersect(const glm::vec3& ray_origin, const glm::vec3& ray_dir) const
{
    std::optional<glm::vec3> intersection;
    float minDistance{0.0f};

    for(auto faceIt{begin()}; faceIt != end(); ++faceIt)
    {
        const glm::vec3& a = faceIt->half_edge->vertex;
        const glm::vec3& b = faceIt->half_edge->next->vertex;
        const glm::vec3& c = faceIt->half_edge->next->next->vertex;
        const glm::vec3 ab = b - a;
        const glm::vec3 ac = c - a;
        const glm::vec3 h = glm::cross(ray_dir, ac);
        float alpha = glm::dot(ab, h);
        if(alpha > -FLT_EPSILON && alpha < FLT_EPSILON)
        {
            continue;
        }
        float f = 1.0f / alpha;
        const glm::vec3 s = ray_origin - a;
        float u = glm::dot(s, h) * f;
        if(u < 0.0f || u > 1.0f)
        {
            continue;
        }
        const glm::vec3 q = glm::cross(s, ab);
        float v = glm::dot(ray_dir, q) * f;
        if(v < 0.0f || (u + v) > 1.0f)
        {
            continue;
        }
        float t = glm::dot(ac, q) * f;
        if(t > FLT_EPSILON)
        {
            glm::vec3 candid = ray_origin + ray_dir * t;
            glm::vec3 delta = candid - ray_origin;
            float distance = glm::dot(delta, delta);
            if(intersection.has_value())
            {
                if(distance < minDistance)
                {
                    intersection = candid;
                    minDistance = distance;
                }
            }
            else
            {
                intersection = candid;
                minDistance = distance;
            }
        }
    }
    return intersection;
}