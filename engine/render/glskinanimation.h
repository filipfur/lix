#pragma once

#include <map>
#include <list>
#include <string>

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

namespace lix
{
    class Node;

    class SkinAnimation
    {
    public:
        class Channel
        {
        public:
            enum Type{TRANSLATION, ROTATION, SCALE};

            glm::vec3 translation(float time) const;
            glm::quat rotation(float time) const;
            glm::vec3 scale(float time) const;

            std::map<float, glm::vec3>& translations();
            std::map<float, glm::quat>& rotations();
            std::map<float, glm::vec3>& scales();

            void setType(Type type);
            Type type() const;
            void setNode(lix::Node* node);
            lix::Node* node() const;
            
        private:
            Type _type;
            lix::Node* _node{nullptr};
            std::map<float, glm::vec3> _translations;
            std::map<float, glm::quat> _rotations;
            std::map<float, glm::vec3> _scales;
        };

        SkinAnimation(const std::string& name) : _name{name}
        {

        }

        void setTime(float time);
        float time() const;
        void setStart(float start);
        float start() const;
        void setEnd(float end);
        float end() const;

        std::list<Channel>& channels();

        void update(float dt);

        std::string name() const
        {
            return _name;
        }

        void setLooping(bool looping)
        {
            _looping = looping;
        }

        bool looping() const
        {
            return _looping;
        }

    private:
        const std::string _name;
        std::list<Channel> _channels;
        float _start{0.0f};
        float _end{1.0f};
        float _time{0.0f};
        bool _looping{true};
    };
}