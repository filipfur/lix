#include <iostream>
#include <fstream>

#include "glm/glm.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

#include "primer.h"

#include "response.h"

struct MyThing
{
    float a;
    bool b;
};

#define print_var(var) std::cout << #var << "=" << var << std::endl;

std::ostream& operator<<(std::ostream& os, const glm::vec3& v)
{
    os << glm::to_string(v);
    return os;
}

int main(int argc, char* argv[])
{
    float restitution = 0.5;

    float m1_inv = 1.0f;
    float m2_inv = 0.0f;

    glm::vec3 v1{1.0f, 2.0f, 0.0f};
    glm::vec3 v2{-1.0f, -1.0f, 0.0f};

    glm::vec3 w1{0.0f, 0.0f, 0.0f};
    glm::vec3 w2{0.0f, 0.0f, 0.0f};

    glm::vec3 r1 = glm::normalize(glm::vec3{1.0f, 2.0f, 0.0f});
    glm::vec3 r2 = glm::normalize(glm::vec3{-1.0f, -1.0f, 0.0f});

    glm::vec3 n = glm::vec3{1.0f, 0.0f, 0.0f};

    glm::mat3 I1_inv = glm::mat3(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    glm::mat3 I2_inv = glm::mat3(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

    print_var(w1);
    print_var(w2);
    print_var(lix::impulse(restitution, m1_inv, m2_inv, I1_inv, I2_inv, r1, r2, n, v1, v2, w1, w2));
    print_var(w1);
    print_var(w2);

    std::vector<MyThing> things = {{1, 0}, {3, 1}, {4, 1}};

    static const MyThing* things_data = things.data();
    size_t things_size = things.size();

    for(size_t i{0}; i < things_size; ++i)
    {
        std::cout << things_data[i].a << std::endl;
    }
  return 0;
}