#pragma once

#include <deque>
#include <glm.hpp>

class Trail {
public:
	glm::mat4 rotation;
	glm::vec3 color;
	std::deque<glm::dvec3> queue;
	size_t parentIndex;

	Trail(glm::vec3 color = glm::vec3(0.0f), size_t parentIndex = -1)
		: color(color), parentIndex(parentIndex), rotation(glm::mat4(1.0f)) {
	}

	glm::dvec3 front() { return queue.front(); }
	glm::dvec3 back() { return queue.back(); }
	std::deque<glm::dvec3>::const_iterator begin() const { return queue.begin(); }
	std::deque<glm::dvec3>::const_iterator end() const { return queue.end(); }
	void push(const glm::dvec3 point) { queue.push_back(point); }
	void pop() { queue.pop_front(); }
	size_t size() { return queue.size(); }
};