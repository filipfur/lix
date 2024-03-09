#include "quickhull.h"

#include <algorithm>

#include "primer.h"

float distanceFromLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& p)
{
    // TODO: Optimize (not needing actual distance)
    // TODO: Cache results since line isn't moving.
    return glm::abs((b.x - a.x) * (a.y - p.y) - (a.x - p.x) * (b.y - a.y)) / glm::sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
};

float signDistanceFromLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& p)
{
    // TODO: Optimize (not needing actual distance)
    // TODO: Cache results since line isn't moving.
    return ((b.x - a.x) * (a.y - p.y) - (a.x - p.x) * (b.y - a.y)) / glm::sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
};

float magnitudeFromLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& p)
{
    return glm::abs((b.x - a.x) * (a.y - p.y) - (a.x - p.x) * (b.y - a.y));
};

float signFromLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& p)
{
    return (b.x - a.x) * (a.y - p.y) - (a.x - p.x) * (b.y - a.y);
};

void findHull(const std::vector<glm::vec3>& s, glm::vec3 p,
    glm::vec3 q, std::vector<glm::vec3>& CH) {
    if(s.empty())
    {
        return;
    }
    float maxDistance{0};
    auto ret = s.end();
    for(auto it=s.begin(); it != s.end(); ++it)
    {
        float d = distanceFromLine(p, q, *it);
        if(d > maxDistance)
        {
            maxDistance = d;
            ret = it;
        }
    }
    CH.push_back(*ret);
    std::vector<glm::vec3> s1;
    std::vector<glm::vec3> s2;
    for(const auto& x : s)
    {
        if(impact::pointInTriangle(x, p, *ret, q))
        {
            //s0.insert(...);
        }
        else if(signFromLine(*ret, p, x) > 0)
        {
            s1.push_back(x);
        }
        else if(signFromLine(q, *ret, x) > 0)
        {
            s2.push_back(x);
        }
    }
    findHull(s1, *ret, p, CH);
    findHull(s2, q, *ret, CH);
}

void impact::quickHull(const std::vector<glm::vec3>& s, std::vector<glm::vec3>& ch)
{
    std::vector<glm::vec3> s1;
    std::vector<glm::vec3> s2;

    const glm::vec3& a = s.front();
    const glm::vec3& b = s.back();
    ch.push_back(a);

    for(size_t i{1}; i < s.size() - 1; ++i)
    {
        const glm::vec3& p = s.at(i);
        float d = signFromLine(a, b, p);
        if(d < 0)
        {
            s1.push_back(p);
        }
        else if(d > 0)
        {
            s2.push_back(p);
        }
        else
        {
            ;
        }
    }

    findHull(s1, a, b, ch);
    findHull(s2, b, a, ch);

    ch.push_back(b);

    glm::vec3 center{0.0f, 0.0f, 0.0f};
    float ratio = 1.0f / static_cast<float>(ch.size());
    for(const auto& p : ch)
    {
        center += p * ratio;
    }

    std::sort(ch.begin(), ch.end(), [&center](const glm::vec3& a, const glm::vec3& b) -> bool {
        return atan2(a.y - center.y, a.x - center.x) < atan2(b.y - center.y, b.x - center.x);
    });
}